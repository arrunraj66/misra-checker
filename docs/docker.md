# Running everything in Docker

One image holds the checker, the web GUI and the CLI. An optional second
container runs a local model (Ollama) on an AMD GPU for the converter.

## Quick start

```bash
docker compose up -d --build          # GUI at http://localhost:8765
docker compose down                   # stop
```

The GUI requires a login. On the first start an `admin` user is created: set
`MISRA_GUI_ADMIN_PASSWORD` (at least 10 characters) in the environment or an
`.env` file, or read the random password the first start prints:

```bash
docker compose logs misra | grep -A1 "First start"
```
Users, roles and the audit log live in the `misra-data` volume. Add people:

```bash
docker compose exec -e MISRA_GUI_PASSWORD='a long password' misra \
    python3 /opt/misra/gui/misra_gui.py --add-user alice --role analyst
docker compose exec misra python3 /opt/misra/gui/misra_gui.py --list-users
```
Roles: `viewer` (read rules and samples), `analyst` (analyze, upload projects,
validate rules), `admin` (also analyze compile databases under `/workspace`).
The port is published to loopback only; for access from other machines put a TLS
reverse proxy in front (or start the server with `--tls-cert/--tls-key`) and add
the public name with `--allowed-host`.

Put projects you want to use from the CLI in `./workspace` (mounted at `/workspace`).

CLI inside the container:

```bash
docker compose run --rm misra misra-checker --version
docker compose run --rm misra misra-checker analyze --compile-commands /workspace/compile_commands.json
```

## Moving the image to another PC

```bash
docker/export-image.sh                # dist/misra-checker-app.tar.gz + docker-compose.yml
# on the target PC (Docker installed):
docker load < misra-checker-app.tar.gz
docker compose up -d                  # uses the loaded image; no build needed
```

Add `--with-llm` to also export the Ollama image (several GB).

## Local model on the RX 6700 XT

Requires the AMD driver with `/dev/kfd` and `/dev/dri` on the host. The 6700 XT
(gfx1031) is not an officially supported ROCm target; the compose file sets
`HSA_OVERRIDE_GFX_VERSION=10.3.0`, the usual workaround. Check your group ids and
override if they differ: `getent group video render`, then
`VIDEO_GID=... RENDER_GID=... docker compose --profile llm up -d`.

```bash
docker compose --profile llm up -d
docker/import-model.sh path/to/fine-tuned-q4_k_m.gguf misra-fixer   # from Colab
```

Then run the converter with the Ollama provider (inside the `misra` service the
model server is reachable at `http://llm:11434`):

```bash
docker compose run --rm misra misra-checker convert \
  --compile-commands /workspace/compile_commands.json --file /workspace/src/uart.c \
  --provider-cmd /opt/misra/scripts/ai_providers/ollama_provider.py \
  --verify-cmd "<your build/test command>" --license /workspace/license.lic
```

If the AMD GPU cannot be used, drop the `devices`/`group_add`/`HSA_*` lines: Ollama
then runs on the CPU, much more slowly.

## Notes

- Source code and models stay on your machine; nothing in these containers calls
  an external service unless you choose the Claude provider.
- The `misra` image runs as a non-root user.
- Changing the C++ sources requires `docker compose build misra`.

## CI tokens for the shared history

```bash
docker compose exec misra python3 /opt/misra/gui/misra_gui.py --create-token ci-main --token-role analyst
```
The token is printed once and stored hashed in the `misra-data` volume together with the history database.
See [central-history.md](central-history.md).
