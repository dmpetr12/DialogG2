const fs = require("fs");
const vm = require("vm");

const html = fs.readFileSync("web/index.html", "utf8");
const runtimeSource = fs.readFileSync("src/EngineRuntime.cpp", "utf8");
const script = html.match(/<script>([\s\S]*)<\/script>/)[1];
const elements = new Map();
const listeners = new Map();
const requests = [];
const removedStorageKeys = [];
const intervals = [];

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
  setInterval(handler, msec) { intervals.push({handler, msec}); },
  setTimeout,
  localStorage: {
    getItem() { return "test-token"; },
    setItem() {},
    removeItem(key) { removedStorageKeys.push(key); }
  },
  document: {
    getElementById: element,
    querySelectorAll(selector) {
      return selector === "[data-auth]"
        ? [element("lineSettingsNav"), element("fireBtn"), element("functionalBtn"), element("durationBtn")]
        : [];
    }
  },
  window: {prompt() { return null; }, confirm() { return true; }},
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
  expect(html.includes("<th>Дни недели</th>"),
         "schedule table must have a week days column");
  expect(html.includes('<button data-view="line-settings" data-auth>Настройки</button>'),
         "protected line settings navigation must be named Settings");
  expect(html.includes('id="scheduleAddButton"')
         && html.includes('id="scheduleEditForm"')
         && html.includes('id="scheduleEditWeekDays"'),
         "schedule page must provide add and edit controls");
  expect(html.includes('id="scheduleDeleteButton"')
         && !html.includes('<th>Вкл</th>')
         && !html.includes('id="scheduleEditEnabled"'),
         "schedule page must treat every entry as active and delete the selected entry");
  expect(runtimeSource.includes('QStringLiteral("/api/auth/check")'),
         "backend must provide a non-mutating token validation endpoint");
  expect(runtimeSource.includes('QStringLiteral("/api/schedule/add")')
         && runtimeSource.includes('QStringLiteral("/api/schedule/<arg>/update")')
         && runtimeSource.includes('QStringLiteral("/api/schedule/<arg>/remove")'),
         "backend must provide schedule add, update and remove endpoints");
  vm.runInContext(script, context);
  expect(element("loginBtn").hidden === false && element("fireBtn").disabled === true,
         "protected controls must stay locked while a stored token is being checked");
  await new Promise((resolve) => setTimeout(resolve, 0));
  expect(requests.some(({path}) => path === "/api/auth/check"),
         "web UI must validate a stored token at startup");
  expect(element("loginBtn").hidden === true && element("fireBtn").disabled === false,
         "valid stored token must unlock protected controls after validation");

  context.fetch = async (path, options = {}) => {
    requests.push({path, options});
    return {
      ok: true,
      status: 200,
      async text() {
        return path === "/api/schedule"
          ? JSON.stringify({ok: true, data: [
              {enabled: true, period: "дни недели", startDate: "2026-10-03",
               startTime: "10:00", testType: "Функциональный тест",
               weekDays: ["Mon", "Wed", "Fri"]}
            ]})
          : '{"ok":true,"data":{}}';
      }
    };
  };
  await vm.runInContext("refreshSchedule()", context);
  expect(element("scheduleRows").innerHTML.includes("Пн, Ср, Пт"),
         "schedule table must show localized week days");
  expect(element("scheduleRows").innerHTML.includes("Настроить"),
         "schedule rows must provide an edit action");
  expect(!element("scheduleRows").innerHTML.includes("Да"),
         "schedule rows must not show a separate enabled state");
  vm.runInContext("openScheduleEditor(0)", context);
  expect(element("scheduleEditPeriod").value === "дни недели"
         && element("scheduleEditStartDate").value === "2026-10-03"
         && element("scheduleEditStartTime").value === "10:00"
         && element("scheduleEditTestType").value === "Функциональный тест"
         && element("scheduleDayMon").checked
         && element("scheduleDayWed").checked
         && element("scheduleDayFri").checked,
         "schedule editor must load the selected entry");
  element("scheduleEditPeriod").value = "ежедневно";
  listeners.get("scheduleEditPeriod:change")();
  expect(element("scheduleEditWeekDays").hidden,
         "weekday controls must be hidden for non-weekly schedules");
  element("scheduleEditPeriod").value = "дни недели";
  listeners.get("scheduleEditPeriod:change")();
  element("scheduleEditStartTime").value = "11:30";
  element("scheduleEditTestType").value = "Тест на время";
  const scheduleSubmit = listeners.get("scheduleEditForm:submit");
  expect(typeof scheduleSubmit === "function", "schedule editor must save through its form");
  await scheduleSubmit({preventDefault() {}});
  const scheduleUpdate = requests.find(({path}) => path === "/api/schedule/0/update");
  expect(scheduleUpdate, "schedule editor must call the backend update endpoint");
  expect(JSON.stringify(JSON.parse(scheduleUpdate.options.body)) === JSON.stringify({
    period: "дни недели",
    startDate: "2026-10-03",
    startTime: "11:30",
    testType: "Тест на время",
    weekDays: ["Mon", "Wed", "Fri"]
  }), "schedule editor must send all editable fields");
  await listeners.get("scheduleAddButton:click")();
  await listeners.get("scheduleEditForm:submit")({preventDefault() {}});
  const scheduleAdd = requests.find(({path}) => path === "/api/schedule/add");
  expect(scheduleAdd, "schedule page must add a schedule entry");
  vm.runInContext("selectSchedule(0)", context);
  await listeners.get("scheduleDeleteButton:click")();
  expect(requests.some(({path}) => path === "/api/schedule/0/remove"),
         "schedule page must remove a schedule entry");

  context.testData = {
    linesAvailable: true,
    linesOk: true,
    maintenance: {
      lastLongTestAt: "2026-10-02T10:20:00Z",
      lastLongTestStatusCode: "passed",
      longTestOverdue: false,
      lines: [{lineIndex: 1, lineName: "Гараж", lastTestAt: "2026-10-02T10:20:00Z",
               lastTestStatusCode: "passed", overdue: false}]
    },
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
  expect(element("maintenanceRows").innerHTML.includes("Тест длительности")
         && element("maintenanceRows").innerHTML.includes("2026")
         && element("maintenanceRows").innerHTML.includes("Норма"),
         "maintenance summary must show the last completed duration test");

  context.testData.maintenance = {
    lastLongTestAt: "2026-10-03T10:20:00Z",
    lastLongTestStatusCode: "failed",
    longTestOverdue: false,
    lines: [{lineIndex: 1, lineName: "Гараж", lastTestAt: "2026-10-03T10:20:00Z",
             lastTestStatusCode: "failed", overdue: false}]
  };
  vm.runInContext("renderState(testData)", context);
  expect((element("maintenanceRows").innerHTML.match(/Неисправно/g) || []).length === 2,
         "maintenance summary must show failed duration and line tests");

  context.testData.maintenance = {
    lastLongTestAt: null, lastLongTestStatusCode: "none", longTestOverdue: true,
    lines: [{lineIndex: 2, lineName: "Линия 2", lastTestAt: null,
             lastTestStatusCode: "none", overdue: true}]
  };
  vm.runInContext("renderState(testData)", context);
  expect(element("maintenanceRows").innerHTML.includes("Тест длительности")
         && (element("maintenanceRows").innerHTML.match(/<td>—<\/td>/g) || []).length >= 4,
         "maintenance summary must show dashes when duration or line tests were not completed");

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

  const timeClick = listeners.get("settingsTimeButton:click");
  expect(typeof timeClick === "function", "settings must provide computer time synchronization");
  await timeClick();
  const timeRequest = requests.find(({path}) => path === "/api/system/time");
  expect(timeRequest && Number(JSON.parse(timeRequest.options.body).msec) > 0,
         "time synchronization must send the current computer time");

  element("settingsNewPassword").value = "new-secret";
  element("settingsRepeatPassword").value = "new-secret";
  const passwordSubmit = listeners.get("settingsPasswordForm:submit");
  expect(typeof passwordSubmit === "function", "settings must provide password change form");
  await passwordSubmit({preventDefault() {}});
  const passwordRequest = requests.find(({path}) => path === "/api/password/change");
  expect(passwordRequest
         && JSON.stringify(JSON.parse(passwordRequest.options.body)) === JSON.stringify({password: "new-secret"}),
         "password form must send the confirmed new password");
  expect(vm.runInContext("state.token", context) === ""
         && removedStorageKeys.includes("dialogG2Token")
         && element("loginBtn").hidden === false,
         "successful password change must require a new login");

  const authInterval = intervals.find(({msec}) => msec === 5000);
  expect(authInterval, "web UI must revalidate authorization while it remains open");
  vm.runInContext('state.token = "stale-token"; state.authenticated = true; authUi()', context);
  context.fetch = async (path, options = {}) => {
    requests.push({path, options});
    return {
      ok: false,
      status: 401,
      async text() { return '{"ok":false,"error":"unauthorized"}'; }
    };
  };
  await authInterval.handler();
  expect(vm.runInContext("state.token", context) === ""
         && element("loginBtn").hidden === false
         && element("fireBtn").disabled === true,
         "expired token must lock protected controls without a user action");

  console.log("Web settings checks passed");
}

main().catch((error) => {
  console.error(error.message);
  process.exit(1);
});
