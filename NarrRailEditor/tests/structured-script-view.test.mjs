import assert from "node:assert/strict";

import { createStructuredScriptRows } from "../src/core/index.js";

const nodes = [
  {
    id: "N_Start",
    type: "dialogue",
    data: {
      speakerId: "Hero",
      lines: [{ textKey: "我们到了。" }, { textKey: "先观察一下。" }],
    },
  },
  {
    id: "N_Action",
    type: "dialogue",
    data: {
      speakerId: "",
      lines: [{ textKey: "走廊尽头的警示灯正在闪烁。" }],
    },
  },
  {
    id: "N_Choice",
    type: "choice",
    data: {
      choiceMode: "SinglePass",
      choices: [{ textKey: "撬开门" }, { textKey: "呼叫支援" }],
    },
  },
  {
    id: "N_Condition",
    type: "condition",
    data: {
      condition: {
        branches: [
          {
            label: "有门禁卡",
            logic: "All",
            terms: [
              {
                variable: { name: "HasKeycard" },
                operator: "==",
                compareValue: "true",
              },
            ],
          },
        ],
      },
    },
  },
  {
    id: "N_Set",
    type: "setvariable",
    data: {
      variableName: "AlarmLevel",
      operation: "Add",
      value: "1",
    },
  },
  {
    id: "N_Event",
    type: "emitevent",
    data: {
      eventType: "DoorOpened",
      params: { doorId: "A-17" },
    },
  },
  {
    id: "N_Jump",
    type: "jump",
    data: { targetNodeId: "N_End" },
  },
  { id: "N_End", type: "end", data: {} },
];

const edges = [
  { source: "N_Start", target: "N_Action", data: { priority: 0 } },
  { source: "N_Action", target: "N_Choice", data: { priority: 0 } },
  {
    source: "N_Choice",
    sourceHandle: "choice-0",
    target: "N_Condition",
    data: { priority: 0 },
  },
  {
    source: "N_Choice",
    sourceHandle: "choice-1",
    target: "N_Set",
    data: { priority: 1 },
  },
  {
    source: "N_Condition",
    sourceHandle: "condition-0",
    target: "N_Event",
    data: { priority: 0 },
  },
  {
    source: "N_Condition",
    sourceHandle: "condition-fallback",
    target: "N_Jump",
    data: { priority: 1 },
  },
  { source: "N_Set", target: "N_End", data: { priority: 0 } },
  { source: "N_Event", target: "N_End", data: { priority: 0 } },
  { source: "N_Jump", target: "N_End", data: { priority: 0 } },
];

{
  const rows = createStructuredScriptRows({
    nodes,
    edges,
    reviewItems: [
      {
        id: "review-1",
        severity: "warning",
        message: "这个选择需要补充失败反馈",
        target: { nodeId: "N_Choice" },
      },
    ],
  });

  assert.deepEqual(
    rows.map((row) => row.kind),
    [
      "dialogue",
      "dialogue",
      "narration",
      "choice",
      "choice-option",
      "choice-option",
      "condition",
      "condition-branch",
      "condition-fallback",
      "variable-operation",
      "event",
      "jump",
      "end",
    ],
  );

  assert.deepEqual(rows[0], {
    id: "N_Start:dialogue:0",
    kind: "dialogue",
    nodeId: "N_Start",
    lineIndex: 0,
    speaker: "Hero",
    text: "我们到了。",
    indent: 0,
    reviewItems: [],
  });
  assert.equal(rows[2].kind, "narration");
  assert.equal(rows[2].text, "走廊尽头的警示灯正在闪烁。");
  assert.equal(rows[3].reviewItems[0].message, "这个选择需要补充失败反馈");
  assert.equal(rows[4].text, "撬开门");
  assert.equal(rows[4].targetNodeId, "N_Condition");
  assert.equal(rows[7].label, "有门禁卡");
  assert.equal(rows[7].targetNodeId, "N_Event");
  assert.equal(rows[8].targetNodeId, "N_Jump");
  assert.equal(rows[9].text, "AlarmLevel Add 1");
  assert.deepEqual(rows[10].params, { doorId: "A-17" });
  assert.equal(rows[11].targetNodeId, "N_End");
}

console.log("structured script view tests passed");
