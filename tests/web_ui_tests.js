const fs = require("fs");
const vm = require("vm");

const html = fs.readFileSync("web/index.html", "utf8");
const script = html.match(/<script>([\s\S]*)<\/script>/)[1];
const elements = new Map();
const listeners = new Map();
const requests = [];

function element(id) {
  if (!elements.has(id)) {
    elements.set(id, {
      classList: {toggle() {}},
      addEventListener(event, handler) { listeners.set(`${id}:${event}`, handler); },
      textContent: "",
      innerHTML: "",
      value: "",
      hidden: false,
      disabled: false
    });
  }
  return elements.get(id);
}

const context = vm.createContext({
  console,
  Date,
  Math,
  Number,
  JSON,
  Promise,
  setInterval() {},
  setTimeout,
  localStorage: {
    getItem() { return "test-token"; },
    setItem() {},
    removeItem() {}
  },
  document: {
    getElementById: element,
    querySelectorAll() { return []; }
  },
  window: {prompt() { return null; }},
  async fetch(path, options = {}) {
    requests.push({path, options});
    return {
      ok: true,
      status: 200,
      async text() { return '{"ok":true,"data":{}}'; }
    };
  }
});

function expect(condition, message) {
  if (!condition)
    throw new Error(message);
}

async function main() {
  expect(!html.includes('id="linesNote"'),
         "fixed line count must not be shown in the dashboard status card");
  vm.runInContext(script, context);

  context.testData = {
    linesAvailable: true,
    linesOk: true,
    lines: [
      {index: 1, description: "Гараж", mode: 0, mpower: 124, tolerance: 5,
       power: 987.6, voltage: 221, current: 0.6, leakage: 0.2,
       displayModeText: "ПОСТ", outputStateText: "ВКЛ", displayStateText: "ВКЛ", displayStateOk: true},
      {index: 2, description: "Резерв", mode: 2, mpower: 130.123456789, tolerance: 7.00000001,
       displayModeText: "ОТКЛ", outputStateText: "ВЫКЛ", displayStateText: "ОТКЛ", displayStateOk: true}
    ]
  };
  vm.runInContext("renderState(testData)", context);

  expect(element("linesRows").innerHTML.includes("Гараж"),
         "enabled line must be visible in the operational overview");
  expect(!element("linesRows").innerHTML.includes("Резерв"),
         "disabled line must be hidden from the operational overview");
  expect(element("linesRows").innerHTML.includes("ПОСТОЯН."),
         "operational overview must use an unambiguous abbreviated line mode");
  expect(element("linesRows").innerHTML.includes("<td>988</td>")
         && element("linesRows").innerHTML.includes("<td>221</td>")
         && element("linesRows").innerHTML.includes("<td>0.6</td>")
         && element("linesRows").innerHTML.includes("<td>0.2</td>"),
         "operational overview must show measured power, voltage, current and leakage");
  expect(element("lineSettingsRows").innerHTML.includes("Гараж")
         && element("lineSettingsRows").innerHTML.includes("Резерв"),
         "line settings must list enabled and disabled lines");
  expect(element("lineSettingsRows").innerHTML.includes("Постоянная")
         && element("lineSettingsRows").innerHTML.includes("Отключена")
         && !element("lineSettingsRows").innerHTML.includes("ПОСТОЯН."),
         "line settings must use full mode names");

  context.noData = {modeCode: "normal", modeText: "Нет данных", systemAvailable: false, testRunning: false};
  vm.runInContext("renderState(noData)", context);
  expect(element("mode").textContent === "НЕТ ДАННЫХ",
         "mode must not be shown as normal when cabinet data is unavailable");
  expect(element("testInfo").textContent === "",
         "inactive test status must not obscure the cabinet mode");

  context.fireData = {modeCode: "fire", modeText: "Пожар", systemAvailable: true, testRunning: false};
  vm.runInContext("renderState(fireData)", context);
  expect(element("mode").textContent === "ПОЖАР", "fire mode must be shown by the web UI");
  expect(element("testInfo").textContent === "", "fire mode must not show inactive test status");

  vm.runInContext("renderState(testData)", context);

  expect(vm.runInContext("typeof openLineEditor", context) === "function",
         "line settings must provide an editor");
  vm.runInContext("openLineEditor(1)", context);
  expect(element("lineEditName").value === "Резерв", "editor must load the selected line name");
  expect(element("lineEditPower").value === "130.1",
         "editor must normalize measured nominal power to its supported precision");
  expect(element("lineEditTolerance").value === "7.0",
         "editor must normalize tolerance to its supported precision");
  expect(element("lineEditMode").value === "2", "editor must load line mode");

  vm.runInContext("openLineEditor(0)", context);
  expect(element("lineEditMeasuredPower").textContent === "987.6 Вт",
         "editor must show measured power as read-only reference data");
  vm.runInContext("openLineEditor(1)", context);

  element("lineEditName").value = "Коридор";
  element("lineEditPower").value = "145";
  element("lineEditTolerance").value = "6";
  element("lineEditMode").value = "1";
  const submit = listeners.get("lineEditForm:submit");
  expect(typeof submit === "function", "line editor must save through its form");
  await submit({preventDefault() {}});

  const request = requests.find(({path}) => path === "/api/lines/1/update");
  expect(request, "line editor must call the backend update endpoint");
  expect(JSON.stringify(JSON.parse(request.options.body)) === JSON.stringify({
    description: "Коридор",
    mpower: 145,
    tolerance: 6,
    mode: 1
  }), "line editor must send exactly the editable fields");

  listeners.get("functionalBtn:click")();
  await new Promise((resolve) => setTimeout(resolve, 0));
  const functionalRequest = requests.find(({path}) => path === "/api/test/start-functional");
  expect(functionalRequest && JSON.parse(functionalRequest.options.body).warmupSec === 120,
         "web functional test must request 120 seconds");

  console.log("Web line settings checks passed");
}

main().catch((error) => {
  console.error(error.message);
  process.exit(1);
});
