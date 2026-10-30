// Runs gui/index.html's script against a minimal fake DOM (gjs / SpiderMonkey):
// startup, login, analysis, results, deviations, exports, project mode.
// Usage: gjs tests/ui_smoke.js gui/index.html
const { GLib, Gio } = imports.gi;
const html = new TextDecoder().decode(GLib.file_get_contents(ARGV[0])[1]);
const source = /<script>([\s\S]*)<\/script>/.exec(html)[1];

const failures = [];
const expect = (name, ok, detail = "") => { if (!ok) failures.push(`${name}: ${detail}`); };

function makeNode(tag) {
  return {
    tag, children: [], dataset: {}, style: {}, value: "", hidden: false, disabled: false, textContent: "",
    scrollTop: 0, offsetWidth: 0, className: "",
    classList: { add() {}, remove() {}, toggle() {}, contains() { return false; } },
    append(...k) { this.children.push(...k); }, replaceChildren(...k) { this.children = k; },
    addEventListener(type, fn) { (this.handlers = this.handlers || {})[type] = fn; },
    setAttribute() {}, setRangeText() {}, focus() {}, setSelectionRange() {}, dispatchEvent() {},
    scrollIntoView() {}, animate() {}, remove() {}, click() {}, firstChild: null,
  };
}
const nodes = {};
nodes.login = Object.assign(makeNode("login"), { hidden: true });  // the page ships it hidden
globalThis.document = {
  documentElement: { dataset: {} },
  getElementById(id) { return nodes[id] || (nodes[id] = makeNode(id)); },
  createElement: makeNode, createDocumentFragment: () => makeNode("frag"),
  createElementNS: (ns, tag) => makeNode(tag), querySelectorAll: () => [], querySelector: () => makeNode("q"), addEventListener() {}, body: makeNode("body"),
};
globalThis.window = { matchMedia: () => ({ matches: false }) };
globalThis.matchMedia = window.matchMedia;
globalThis.requestAnimationFrame = (f) => f();
globalThis.Event = function () {};
globalThis.localStorage = { d: {}, getItem(k) { return this.d[k] ?? null; }, setItem(k, v) { this.d[k] = v; } };
globalThis.CSS = { escape: (s) => s };
globalThis.URL = { createObjectURL: () => "blob:x" };
globalThis.Blob = function (parts) { this.parts = parts; };
globalThis.prompt = () => null;
globalThis.alert = () => {};

const downloads = [];
const analysis = {
  exit_code: 1, summary: "Analyzed 1 translation unit(s)", elapsed_ms: 12, compile_errors: false, error: "",
  compiler_output: "", sarif: "{\"version\":\"2.1.0\"}", deviation_notes: ["Deviation D9 matched no finding."],
  workspace_key: "files:a.c",
  baseline: { saved: "2026-10-01T10:00:00+0000", new: 1, unchanged: 1, fixed: 1, new_by_category: { required: 1 },
    fixed_entries: [{ file: "a.c", line: 7, rule: "9.1", text: "int u;" }] },
  cross: { tool: "Cppcheck 2.7", rules_covered: 130, agree: 1, ours_only: 1, not_comparable: 0, cppcheck_only: 1, notes: [],
    cppcheck_only_findings: [{ file: "a.c", line: 9, rule: "17.7", message: "Return value not used", cross: "cppcheck-only" }],
    cppcheck_misra: [{ file: "a.c", line: 1, rule: "7.1", message: "", also_ours: true }, { file: "a.c", line: 9, rule: "17.7", message: "", also_ours: false }],
    native: [{ file: "a.c", line: 5, severity: "error", id: "arrayIndexOutOfBounds", message: "Array index out of bounds" }] },
  findings: [{ file: "a.c", line: 1, col: 1, category: "required", rule: "7.1", key: "k", message: "octal constant", cross: "both", fp: "aaaaaaaaaaaaaaaa", status: "new" },
             { file: "a.c", line: 3, col: 1, category: "advisory", rule: "15.1", key: "k", message: "goto", library: false, cross: "ours-only", fp: "bbbbbbbbbbbbbbbb", status: "unchanged" }],
  deviated: [{ file: "a.c", line: 2, col: 1, category: "advisory", rule: "19.2", key: "k", message: "union declared", deviation: "D1" }],
};
const responses = {
  "/api/me": { status: 200, body: { user: "bob", role: "analyst", csrf: "tok", auth: true, db_paths: false,
    tools: { cppcheck: { available: true, version: "Cppcheck 2.7", rules_covered: 130 } } } },
  "/api/compilers": { status: 200, body: { compilers: [
    { id: "default", label: "clang 14 [x86_64]", usable: true }, { id: "arm", label: "arm-none-eabi-gcc 10 [arm-none-eabi]", usable: true },
    { id: "xt", label: "xt", usable: false, reason: "no target" }] } },
  "/api/rules": { status: 200, body: { version: "misra-checker x", rules: [
    { id: "7.1", category: "Required", decidability: "D", scope: "S", topic: "Constants" },
    { id: "15.1", category: "Advisory", decidability: "D", scope: "S", topic: "Control flow" },
    { id: "19.2", category: "Advisory", decidability: "D", scope: "S", topic: "Overlapping" }] } },
  "/api/samples": { status: 200, body: { samples: [{ rule: "7.1", name: "r7_1_bad", code: "int x = 010;" }] } },
  "/api/analyze": { status: 200, body: analysis },
  "/api/project": { status: 200, body: { id: "p1", name: "fw.zip", c_files: 1, h_files: 0, library_files: 0,
    files: [{ path: "src/a.c", kind: "c", library: false }] } },
  "/api/projects": { status: 200, body: { projects: [{ name: "fw", runs: 3, branches: ["main", "dev"], created: "x", created_by: "ci",
    latest: { id: 3, total: 5, mandatory: 1, required: 2, advisory: 2, new_count: 1, fixed_count: 0 } }] } },
  "/api/history": { status: 200, body: { runs: [
    { id: 1, created: "2026-10-01T10:00:00+0000", branch: "main", commit_id: "abcdef123", total: 8, mandatory: 1, required: 3, advisory: 4, new_count: 0, fixed_count: 0, source: "ci", submitted_by: "ci", prev_run: null },
    { id: 2, created: "2026-10-02T10:00:00+0000", branch: "main", commit_id: "fedcba321", total: 6, mandatory: 1, required: 2, advisory: 3, new_count: 0, fixed_count: 2, source: "ci", submitted_by: "ci", prev_run: 1 },
    { id: 3, created: "2026-10-03T10:00:00+0000", branch: "main", commit_id: "1111111", total: 5, mandatory: 1, required: 2, advisory: 2, new_count: 1, fixed_count: 2, source: "gui", submitted_by: "bob", prev_run: 2 }] } },
  "/api/history-diff": { status: 200, body: { new: [{ rule: "8.4", message: "m", file: "a.c", line: 1, category: "required" }], fixed: [], unchanged: 4 } },
  "/api/runs": { status: 200, body: { run: { id: 9, new_count: 1, fixed_count: 0 } } },
  "/api/file": { status: 200, body: { source: "int x = 010;\n" } },
  "/api/logout": { status: 200, body: { ok: true } },
};
const calls = [];
globalThis.fetch = async (url, opts = {}) => {
  const path = url.split("?")[0];
  calls.push({ path, method: opts.method || "GET", headers: opts.headers || {}, body: opts.body });
  const r = responses[path] || { status: 404, body: { error: "not found" } };
  return { ok: r.status < 400, status: r.status, json: async () => r.body };
};
const origCreate = document.createElement;
document.createElement = (tag) => {
  const n = origCreate(tag);
  if (tag === "a") n.click = function () { downloads.push(this.download || this.href); };
  return n;
};

const api = new Function(source + `
;return { getMe: () => me, csrf: () => csrf, runCode, files: () => files, visibleFindings,
  setLast: (r) => { lastRes = r; }, crossOffAdd: (k) => crossOff.add(k), renderResult, showLogin, project: () => project, devOffSet: (v) => { devOff.v = v; },
  exportCsv, exportJson, exportReport, setDeviations: (t, n) => { deviationsText = t; deviationCount = n; },
  loadHistory, selectProject, showRun, getRuns: () => hRuns, loadTriage, openTriage, verdictsFor, statusOffAdd: (k) => statusOff.add(k), baselineBlock, fieldText, renderCompare, showCompareDetail, setCmpKey: (k) => { cmpKey = k; }, getLast: () => lastRes, renderEditorLines, setViewPath: (v) => { viewPath = v; }, analyze, runProject, setProject: (p) => { project = p; }, openProjectFile, leaveProject };`)();

const tick = () => new Promise((resolve) => GLib.timeout_add(GLib.PRIORITY_DEFAULT, 30, () => { resolve(); return false; }));
const loop = new GLib.MainLoop(null, false);

(async () => {
  await tick();
  expect("boot reads session", api.getMe() && api.getMe().user === "bob", JSON.stringify(api.getMe()));
  expect("csrf kept", api.csrf() === "tok");
  expect("login hidden when signed in", document.getElementById("login").hidden !== false);
  expect("rules loaded", calls.some((c) => c.path === "/api/rules"));

  // analysis sends the CSRF header and the deviations
  expect("compilers requested", calls.some((c) => c.path === "/api/compilers"));
  document.getElementById("compiler").value = "arm";
  document.getElementById("cflags").value = "-mthumb";
  expect("cppcheck option enabled when the server has it", document.getElementById("xcheck").disabled === false);
  document.getElementById("xcheck").checked = true;
  api.files()[0].code = "int x = 010;\n";
  api.setDeviations("{\"deviations\":[]}", 0);
  api.runCode();
  await tick();
  const post = calls.find((c) => c.path === "/api/analyze");
  expect("analyze POST has CSRF header", post && post.headers["X-CSRF-Token"] === "tok", JSON.stringify(post));
  const sent = JSON.parse(post.body);
  expect("payload carries compiler and flags", sent.compiler === "arm" && sent.flags === "-mthumb", post.body);
  expect("payload asks for the cppcheck cross-check", sent.cppcheck === true, post.body);
  const rows = api.visibleFindings();
  expect("rows from cppcheck are merged in", rows.some((r) => r.cross === "cppcheck-only" && r.rule === "17.7"), JSON.stringify(rows));
  expect("deviated rows included", rows.length === 4 && rows.some((r) => r.deviated && r.deviation === "D1"), JSON.stringify(rows));
  api.devOffSet(true);
  expect("deviated rows can be hidden", api.visibleFindings().length === 3);
  api.devOffSet(false);
  api.statusOffAdd("unchanged");
  expect("the baseline status filter hides unchanged findings", !api.visibleFindings().some((r) => r.status === "unchanged"));
  const block = api.baselineBlock(api.getLast());
  expect("the baseline summary is shown", block.children.length >= 4, String(block.children.length));
  api.crossOffAdd("cppcheck-only");
  expect("cppcheck-only rows can be hidden", !api.visibleFindings().some((r) => r.cross === "cppcheck-only"));

  // triage: verdicts shown next to findings, and saved with the analysed sources
  responses["/api/triage"] = { status: 200, body: { cases: [{ id: "abc", kind: "false-positive", rule: "7.1", file: "a.c", line: 1, note: "n", state: "open", fp: "aaaaaaaaaaaaaaaa" }],
    summary: { "7.1": { true_positive: 0, false_positive: 1, false_negative: 0, precision: 0 } }, case: {} } };
  await api.loadTriage();
  expect("a saved verdict is matched to its finding", api.verdictsFor({ file: "a.c", rule: "7.1", line: 1 }).length === 1);
  expect("a finding without a fingerprint falls back to the line", api.verdictsFor({ file: "a.c", rule: "7.1", line: 1 }).length === 1);
  expect("a different line has no verdict", api.verdictsFor({ file: "a.c", rule: "7.1", line: 2 }).length === 0);
  expect("a verdict follows its violation to a new line by fingerprint",
    api.verdictsFor({ file: "a.c", rule: "7.1", line: 40, fp: "aaaaaaaaaaaaaaaa" }).length === 1);
  expect("a different fingerprint on the old line is not that violation",
    api.verdictsFor({ file: "a.c", rule: "7.1", line: 1, fp: "cccccccccccccccc" }).length === 0);
  expect("rule summary text", api.fieldText("7.1") === "1 FP" && api.fieldText("9.9") === "—", api.fieldText("7.1"));
  api.openTriage({ kind: "false-positive", rule: "15.1", file: "a.c", line: 3, note: "wrong" });
  expect("the dialog opens prefilled", document.getElementById("tri-dialog").hidden === false && document.getElementById("tri-line").value === 3);
  await document.getElementById("tri-form").handlers.submit({ preventDefault() {} });
  const saved = calls.filter((c) => c.path === "/api/triage" && c.method === "POST").pop();
  const savedBody = saved ? JSON.parse(saved.body) : {};
  expect("the verdict is posted with the analysed sources", savedBody.kind === "false-positive" && savedBody.rule === "15.1"
    && savedBody.line === 3 && Array.isArray(savedBody.files) && savedBody.files.length > 0, saved && saved.body);
  expect("the dialog closes after saving", document.getElementById("tri-dialog").hidden === true);

  // history: shared projects with a trend and run-to-run differences
  await api.loadHistory();
  expect("projects are listed", document.getElementById("h-projects").children.length === 1);
  await api.selectProject("fw", "");
  expect("the runs of the project are loaded", api.getRuns().length === 3);
  const detail = document.getElementById("h-detail").children;
  expect("the detail has cards, a chart, a legend and a table", detail.length >= 4, String(detail.length));
  await api.showRun(api.getRuns()[2]);
  expect("a run is compared with its predecessor", calls.some((c) => c.path === "/api/history-diff" && c.method === "GET"));
  globalThis.prompt = (q) => (/Project name/.test(q) ? "fw" : "main");
  await document.getElementById("exp-publish").handlers.click();
  const pub = calls.filter((c) => c.path === "/api/runs").pop();
  expect("publishing sends the workspace key, project and branch", pub && JSON.parse(pub.body).key === "files:a.c"
    && JSON.parse(pub.body).project === "fw" && JSON.parse(pub.body).branch === "main", pub && pub.body);

  // compare view: three panes side by side
  api.renderCompare();
  const paneSizes = ["cmp-ours", "cmp-misra", "cmp-native"].map((id) => document.getElementById(id).children.length);
  expect("compare view fills the three panes", paneSizes.join() === "3,2,1", paneSizes.join());
  document.getElementById("cmp-diff").checked = true;
  api.renderCompare();
  const diffSizes = ["cmp-ours", "cmp-misra", "cmp-native"].map((id) => document.getElementById(id).children.length);
  expect("only-differences keeps just the disagreements", diffSizes.join() === "1,1,1", diffSizes.join());
  document.getElementById("cmp-diff").checked = false;
  api.setCmpKey("a.c:5");
  await api.showCompareDetail();
  expect("selecting a line opens the detail panel", document.getElementById("cmp-detail").hidden === false);

  // compliance documents: enabled when the analysis has a workspace, and fetched for that workspace
  expect("compliance buttons are enabled after an analysis", document.getElementById("exp-comp-html").disabled === false);
  document.getElementById("exp-comp-html").handlers.click();
  document.getElementById("exp-comp-csv").handlers.click();
  expect("compliance downloads name the workspace and format",
    downloads.includes("/api/compliance?key=files%3Aa.c&format=html") && downloads.includes("/api/compliance?key=files%3Aa.c&format=csv"), downloads.join());

  // exports
  api.exportCsv(); api.exportJson(); api.exportReport();
  expect("exports download", ["misra-findings.csv", "misra-findings.json", "misra-report.html"].every((n) => downloads.includes(n)), downloads.join());

  // project mode
  api.setProject({ id: "p1", name: "fw.zip", files: [{ path: "src/a.c", kind: "c", library: false }] });
  await api.openProjectFile("src/a.c");
  expect("project file opened", document.getElementById("code").value.startsWith("int x"));
  await api.runProject();
  expect("project analyze sends project id", calls.filter((c) => c.path === "/api/analyze").length >= 2);
  // Violations must be marked in the code of the project file being viewed (this was broken:
  // markers were looked up under the scratch file name instead of the viewed file).
  document.getElementById("code").value = "int x = 010;\nint y;\n";
  api.setProject({ id: "p1", name: "fw.zip", files: [] });
  api.setViewPath("Core/Src/main.c");
  api.setLast({ findings: [{ file: "Core/Src/main.c", line: 1, col: 1, category: "mandatory", rule: "17.3", key: "k", message: "m" },
                           { file: "other.c", line: 2, col: 1, category: "required", rule: "7.1", key: "k", message: "m" }],
               deviated: [], project: { id: "p1" } });
  api.renderEditorLines();
  const gutterLines = document.getElementById("gutter").children[0].children;
  expect("project mode marks a mandatory violation in the viewed file", /\bmandatory\b/.test(gutterLines[0].className), gutterLines[0].className);
  expect("findings of other files are not marked here", !/\bhas\b/.test(gutterLines[1].className), gutterLines[1].className);
  api.leaveProject();
  expect("left project mode", api.project() === null);

  // session expiry shows the login form
  responses["/api/rules"] = { status: 401, body: { error: "login required" } };
  responses["/api/analyze"] = { status: 401, body: { error: "login required" } };
  api.runCode();
  await tick();
  expect("401 shows login", document.getElementById("login").hidden === false);
  expect("session cleared", api.getMe() === null && api.csrf() === "");
})().catch((e) => failures.push("exception: " + e + "\n" + e.stack)).finally(() => {
  if (failures.length) { print(failures.join("\n")); imports.system.exit(1); }
  print("ui smoke passed");
  loop.quit();
});
loop.run();
