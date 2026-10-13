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
  querySelectorAll: () => [], querySelector: () => makeNode("q"), addEventListener() {}, body: makeNode("body"),
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
  findings: [{ file: "a.c", line: 1, col: 1, category: "required", rule: "7.1", key: "k", message: "octal constant" },
             { file: "a.c", line: 3, col: 1, category: "advisory", rule: "15.1", key: "k", message: "goto", library: false }],
  deviated: [{ file: "a.c", line: 2, col: 1, category: "advisory", rule: "19.2", key: "k", message: "union declared", deviation: "D1" }],
};
const responses = {
  "/api/me": { status: 200, body: { user: "bob", role: "analyst", csrf: "tok", auth: true, db_paths: false } },
  "/api/rules": { status: 200, body: { version: "misra-checker x", rules: [
    { id: "7.1", category: "Required", decidability: "D", scope: "S", topic: "Constants" },
    { id: "15.1", category: "Advisory", decidability: "D", scope: "S", topic: "Control flow" },
    { id: "19.2", category: "Advisory", decidability: "D", scope: "S", topic: "Overlapping" }] } },
  "/api/samples": { status: 200, body: { samples: [{ rule: "7.1", name: "r7_1_bad", code: "int x = 010;" }] } },
  "/api/analyze": { status: 200, body: analysis },
  "/api/project": { status: 200, body: { id: "p1", name: "fw.zip", c_files: 1, h_files: 0, library_files: 0,
    files: [{ path: "src/a.c", kind: "c", library: false }] } },
  "/api/file": { status: 200, body: { source: "int x = 010;\n" } },
  "/api/logout": { status: 200, body: { ok: true } },
};
const calls = [];
globalThis.fetch = async (url, opts = {}) => {
  const path = url.split("?")[0];
  calls.push({ path, method: opts.method || "GET", headers: opts.headers || {} });
  const r = responses[path] || { status: 404, body: { error: "not found" } };
  return { ok: r.status < 400, status: r.status, json: async () => r.body };
};
const origCreate = document.createElement;
document.createElement = (tag) => {
  const n = origCreate(tag);
  if (tag === "a") n.click = function () { downloads.push(this.download); };
  return n;
};

const api = new Function(source + `
;return { getMe: () => me, csrf: () => csrf, runCode, files: () => files, visibleFindings,
  setLast: (r) => { lastRes = r; }, renderResult, showLogin, project: () => project, devOffSet: (v) => { devOff.v = v; },
  exportCsv, exportJson, exportReport, setDeviations: (t, n) => { deviationsText = t; deviationCount = n; },
  analyze, runProject, setProject: (p) => { project = p; }, openProjectFile, leaveProject };`)();

const tick = () => new Promise((resolve) => GLib.timeout_add(GLib.PRIORITY_DEFAULT, 30, () => { resolve(); return false; }));
const loop = new GLib.MainLoop(null, false);

(async () => {
  await tick();
  expect("boot reads session", api.getMe() && api.getMe().user === "bob", JSON.stringify(api.getMe()));
  expect("csrf kept", api.csrf() === "tok");
  expect("login hidden when signed in", document.getElementById("login").hidden !== false);
  expect("rules loaded", calls.some((c) => c.path === "/api/rules"));

  // analysis sends the CSRF header and the deviations
  api.files()[0].code = "int x = 010;\n";
  api.setDeviations("{\"deviations\":[]}", 0);
  api.runCode();
  await tick();
  const post = calls.find((c) => c.path === "/api/analyze");
  expect("analyze POST has CSRF header", post && post.headers["X-CSRF-Token"] === "tok", JSON.stringify(post));
  const rows = api.visibleFindings();
  expect("deviated rows included", rows.length === 3 && rows.some((r) => r.deviated && r.deviation === "D1"), JSON.stringify(rows));
  api.devOffSet(true);
  expect("deviated rows can be hidden", api.visibleFindings().length === 2);
  api.devOffSet(false);

  // exports
  api.exportCsv(); api.exportJson(); api.exportReport();
  expect("exports download", ["misra-findings.csv", "misra-findings.json", "misra-report.html"].every((n) => downloads.includes(n)), downloads.join());

  // project mode
  api.setProject({ id: "p1", name: "fw.zip", files: [{ path: "src/a.c", kind: "c", library: false }] });
  await api.openProjectFile("src/a.c");
  expect("project file opened", document.getElementById("code").value.startsWith("int x"));
  await api.runProject();
  expect("project analyze sends project id", calls.filter((c) => c.path === "/api/analyze").length >= 2);
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
