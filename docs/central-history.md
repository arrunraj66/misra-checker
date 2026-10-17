# Central history: projects, runs and trends

One place where the whole team sees how a project's violations change: runs from CI and from GUI users go
into a shared database, each run records what is new and fixed compared with the previous run of the same
branch, and the **History** tab shows the trend.

## What is stored
A **run** is one analysis result: project, branch, commit, time, who or what submitted it (a CI token or a
user), tool version, counts per category, how many findings were deviated, how many are new and fixed since
the previous run of the same branch, and every finding with its fingerprint (see baseline-and-ci.md). The
database is `history.db` in the data directory (SQLite, mode 0600); the last 500 runs per project and
branch are kept.

Projects are shared: every signed-in user can read them (viewers included); analysts and CI tokens can add
runs; only administrators can delete a project. Your private baselines and triage cases are unchanged.

## Publishing from CI
Create a token on the server (shown once, stored only as a hash):

```bash
python3 gui/misra_gui.py --create-token ci-main --token-role analyst     # prints mt_...
python3 gui/misra_gui.py --list-tokens
python3 gui/misra_gui.py --revoke-token ci-main                          # takes effect immediately
```
In Docker: `docker compose exec misra python3 /opt/misra/gui/misra_gui.py --create-token ci-main`.
Tokens can be `viewer` (read) or `analyst` (read and publish); there is no admin token. Keep the token in
the CI system's secret store, not in the repository.

Then add the publish options to the gate:

```bash
export MISRA_HISTORY_TOKEN=mt_...
python3 scripts/misra_ci.py --compile-commands build/compile_commands.json --baseline misra-baseline.json \
    --publish https://misra.example.com --project my-firmware --branch "$BRANCH" --commit "$COMMIT"
```
The run is published whether or not the gate passes, so failing builds show up in the trend. A failed
publish only prints a warning unless `--require-publish` is given. For a server that is not on localhost,
start it with `--allowed-host misra.example.com` and put a TLS proxy in front (or use `--tls-cert/--tls-key`);
send tokens only over HTTPS. The API: `POST /api/runs` with `Authorization: Bearer <token>`.

## Publishing from the GUI
After an analysis press **Publish** in the results header, give a project name and a branch or label. The
run records you as the submitter.

## Reading the history
The **History** tab lists the projects with their latest run. Selecting one shows cards for the latest run,
a chart of all findings, required + mandatory and mandatory per run, and the run table (when, branch,
commit, totals as mandatory/required/advisory, new, fixed, who). Click a run to see exactly which findings
are new and which were fixed compared with the previous run. Pick a branch to follow it alone.

## Limits
- New and fixed are computed by fingerprint against the previous run of the *same branch*; the first run of
  a branch has nothing to compare with.
- The server is a single process with a SQLite file: fine for a team, not for hundreds of concurrent CI jobs.
- Findings are stored as the CI job sent them; the server cannot verify them, so treat tokens as write access
  to the history.
