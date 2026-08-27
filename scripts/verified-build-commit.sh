#!/usr/bin/env bash

set -euo pipefail

usage() {
  echo "Usage: $0 <commit-message> -- <build-and-test-command> [args...]" >&2
}

if (( $# < 3 )); then
  usage
  exit 64
fi

commit_message=$1
shift

if [[ $1 != "--" ]]; then
  usage
  exit 64
fi
shift

if [[ -z ${commit_message//[[:space:]]/} ]]; then
  echo "Commit message must not be empty." >&2
  exit 64
fi

repo_root=$(git rev-parse --show-toplevel 2>/dev/null) || {
  echo "This command must run inside a Git repository." >&2
  exit 69
}
cd "$repo_root"

if ! "$@"; then
  echo "Build or test command failed; nothing was staged or committed." >&2
  exit 1
fi

prohibited_paths=$(git ls-files --cached --others --exclude-standard | \
  grep -Ei '(^|/)(\.env($|\.)|credentials|secrets)|\.(pdf|pem|key|p12|pfx|jks|keystore)$' || true)

if [[ -n $prohibited_paths ]]; then
  echo "Refusing to commit prohibited or potentially sensitive files:" >&2
  echo "$prohibited_paths" >&2
  exit 65
fi

allowed_paths=(
  .github
  .gitignore
  CMakeLists.txt
  README.md
  ROADMAP.md
  cmake
  docs
  include
  rules
  scripts
  src
  tests
  tools
)

existing_paths=()
for path in "${allowed_paths[@]}"; do
  if [[ -e $path ]] || git ls-files --error-unmatch -- "$path" >/dev/null 2>&1; then
    existing_paths+=("$path")
  fi
done

if (( ${#existing_paths[@]} == 0 )); then
  echo "No allow-listed project paths exist; nothing to commit."
  exit 0
fi

git add -A -- "${existing_paths[@]}"

if git diff --cached --quiet; then
  echo "Verified build succeeded; there are no allow-listed changes to commit."
  exit 0
fi

git diff --cached --check

staged_prohibited=$(git diff --cached --name-only --diff-filter=ACMR | \
  grep -Ei '(^|/)(\.env($|\.)|credentials|secrets)|\.(pdf|pem|key|p12|pfx|jks|keystore)$' || true)

if [[ -n $staged_prohibited ]]; then
  while IFS= read -r prohibited_path; do
    git reset -q -- "$prohibited_path"
  done <<< "$staged_prohibited"
  echo "Refusing to commit prohibited staged files:" >&2
  echo "$staged_prohibited" >&2
  exit 65
fi

git commit -m "$commit_message"
echo "Verified build succeeded and commit was created."
