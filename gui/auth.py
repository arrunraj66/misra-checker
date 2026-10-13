"""Authentication for the web GUI: users, roles, sessions, throttling, audit log.

Standard library only. Passwords are stored as salted scrypt hashes; sessions are
random server-side tokens delivered in an HttpOnly, SameSite=Strict cookie, with
a CSRF token required on state-changing requests.
"""

from __future__ import annotations

import getpass
import hashlib
import hmac
import json
import os
import re
import secrets
import threading
import time
from pathlib import Path

ROLES = {"viewer": 1, "analyst": 2, "admin": 3}
MIN_PASSWORD_LENGTH = 10
USERNAME_RE = re.compile(r"^[A-Za-z0-9_.@-]{1,64}$")
SESSION_IDLE_SECONDS = 2 * 3600
SESSION_MAX_SECONDS = 12 * 3600
MAX_FAILURES = 5
LOCKOUT_SECONDS = 300

_SCRYPT = {"n": 2 ** 14, "r": 8, "p": 1}


def hash_password(password: str, salt: bytes | None = None) -> str:
    salt = salt or secrets.token_bytes(16)
    digest = hashlib.scrypt(password.encode(), salt=salt, maxmem=64 * 1024 * 1024, dklen=32, **_SCRYPT)
    return f"scrypt${_SCRYPT['n']}${_SCRYPT['r']}${_SCRYPT['p']}${salt.hex()}${digest.hex()}"


def _check_hash(password: str, stored: str) -> bool:
    try:
        scheme, n, r, p, salt, digest = stored.split("$")
        if scheme != "scrypt":
            return False
        candidate = hashlib.scrypt(password.encode(), salt=bytes.fromhex(salt), n=int(n), r=int(r),
                                   p=int(p), maxmem=64 * 1024 * 1024, dklen=32)
        return hmac.compare_digest(candidate, bytes.fromhex(digest))
    except (ValueError, TypeError):
        return False


# Verified against when the user does not exist, so timing does not reveal accounts.
_DUMMY_HASH = hash_password(secrets.token_hex(16))


class UserStore:
    def __init__(self, path: Path) -> None:
        self.path = path
        self._lock = threading.Lock()

    def _load(self) -> dict:
        try:
            return json.loads(self.path.read_text())
        except FileNotFoundError:
            return {}

    def _save(self, users: dict) -> None:
        self.path.parent.mkdir(parents=True, exist_ok=True, mode=0o700)
        tmp = self.path.with_suffix(".tmp")
        fd = os.open(tmp, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o600)
        with os.fdopen(fd, "w") as handle:
            json.dump(users, handle, indent=2)
        os.replace(tmp, self.path)

    def is_empty(self) -> bool:
        with self._lock:
            return not self._load()

    def names(self) -> dict[str, str]:
        with self._lock:
            return {name: record["role"] for name, record in self._load().items()}

    def add(self, name: str, password: str, role: str) -> None:
        if not USERNAME_RE.match(name):
            raise ValueError("user names may use letters, digits and _ . @ - (max 64)")
        if role not in ROLES:
            raise ValueError(f"role must be one of: {', '.join(ROLES)}")
        if len(password) < MIN_PASSWORD_LENGTH:
            raise ValueError(f"password must be at least {MIN_PASSWORD_LENGTH} characters")
        with self._lock:
            users = self._load()
            users[name] = {"hash": hash_password(password), "role": role}
            self._save(users)

    def remove(self, name: str) -> bool:
        with self._lock:
            users = self._load()
            if name not in users:
                return False
            del users[name]
            self._save(users)
            return True

    def verify(self, name: str, password: str) -> str | None:
        """Return the user's role when the credentials are right."""
        with self._lock:
            record = self._load().get(name)
        ok = _check_hash(password, record["hash"] if record else _DUMMY_HASH)
        return record["role"] if (record and ok) else None


class Sessions:
    def __init__(self) -> None:
        self._data: dict[str, dict] = {}
        self._lock = threading.Lock()

    def create(self, user: str, role: str) -> dict:
        token = secrets.token_urlsafe(32)
        now = time.time()
        session = {"token": token, "user": user, "role": role, "csrf": secrets.token_urlsafe(24),
                   "created": now, "seen": now}
        with self._lock:
            self._data[token] = session
        return session

    def get(self, token: str | None) -> dict | None:
        if not token:
            return None
        now = time.time()
        with self._lock:
            for stale in [t for t, s in self._data.items()
                          if now - s["seen"] > SESSION_IDLE_SECONDS or now - s["created"] > SESSION_MAX_SECONDS]:
                del self._data[stale]
            session = self._data.get(token)
            if session:
                session["seen"] = now
            return dict(session) if session else None

    def delete(self, token: str | None) -> None:
        with self._lock:
            self._data.pop(token or "", None)


class LoginLimiter:
    """Locks a (client, user name) pair after repeated failures."""

    def __init__(self) -> None:
        self._failures: dict[tuple[str, str], list[float]] = {}
        self._lock = threading.Lock()

    def locked_for(self, client: str, name: str) -> int:
        now = time.time()
        with self._lock:
            recent = [t for t in self._failures.get((client, name), []) if now - t < LOCKOUT_SECONDS]
            self._failures[(client, name)] = recent
            if len(recent) >= MAX_FAILURES:
                return int(LOCKOUT_SECONDS - (now - recent[0])) + 1
        return 0

    def failed(self, client: str, name: str) -> None:
        with self._lock:
            self._failures.setdefault((client, name), []).append(time.time())

    def succeeded(self, client: str, name: str) -> None:
        with self._lock:
            self._failures.pop((client, name), None)


class AuditLog:
    def __init__(self, path: Path | None) -> None:
        self.path = path
        self._lock = threading.Lock()

    def write(self, user: str, action: str, **detail) -> None:
        if self.path is None:
            return
        record = {"time": time.strftime("%Y-%m-%dT%H:%M:%S%z"), "user": user, "action": action, **detail}
        with self._lock:
            self.path.parent.mkdir(parents=True, exist_ok=True, mode=0o700)
            fd = os.open(self.path, os.O_WRONLY | os.O_CREAT | os.O_APPEND, 0o600)
            with os.fdopen(fd, "a") as handle:
                handle.write(json.dumps(record) + "\n")


def read_password(confirm: bool = True) -> str:
    """Password for the CLI: MISRA_GUI_PASSWORD (non-interactive) or a prompt."""
    env = os.environ.get("MISRA_GUI_PASSWORD")
    if env:
        return env
    first = getpass.getpass("Password: ")
    if confirm and getpass.getpass("Repeat password: ") != first:
        raise ValueError("passwords do not match")
    return first
