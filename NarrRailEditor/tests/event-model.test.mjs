import assert from "node:assert/strict";
import YAML from "yaml";

import { buildYAMLString } from "../src/utils/yaml-exporter.js";
import { importFromYAML } from "../src/utils/yaml-importer.js";
import { validateStory } from "../src/utils/validation.js";

function getErrors(nodes) {
  return validateStory(nodes, [], { entryNodeId: nodes[0]?.id }, []).errors;
}

function exportSingleNode(data) {
  const yaml = buildYAMLString(
    [{ id: "N_Event", type: "emitevent", data }],
    [],
    [],
    { storyId: "S_Event", entryNodeId: "N_Event" },
  );
  return YAML.parse(yaml).nodes[0];
}

{
  const node = exportSingleNode({ eventId: "legacy_event" });
  assert.equal(node.eventId, "legacy_event");
  assert.equal(node.eventType, undefined);
  assert.deepEqual(node.params, {});
  assert.deepEqual(
    getErrors([{ id: "N_Event", type: "emitevent", data: { eventId: "legacy_event" } }]),
    [],
  );
}

{
  const nodes = [
    {
      id: "N_Start",
      type: "dialogue",
      data: {
        lines: [{ textKey: "line_start" }],
        enterActions: [
          {
            actionType: "EmitEvent",
            eventType: "inventory.add_item",
            params: { itemId: "key_red", count: 1 },
          },
        ],
      },
    },
  ];
  assert.deepEqual(getErrors(nodes), []);

  const yamlNode = YAML.parse(
    buildYAMLString(nodes, [], [], { storyId: "S_Action", entryNodeId: "N_Start" }),
  ).nodes[0];
  assert.equal(yamlNode.enterActions[0].eventType, "inventory.add_item");
  assert.deepEqual(yamlNode.enterActions[0].params, {
    itemId: "key_red",
    count: 1,
  });
}

{
  const node = exportSingleNode({
    eventType: "audio.play_bgm",
    params: { bgmId: "train_theme", fadeSeconds: 1.5 },
  });
  assert.equal(node.eventId, undefined);
  assert.equal(node.eventType, "audio.play_bgm");
  assert.deepEqual(node.params, {
    bgmId: "train_theme",
    fadeSeconds: 1.5,
  });
  assert.deepEqual(
    getErrors([
      {
        id: "N_PlayBgm",
        type: "emitevent",
        data: {
          eventType: "audio.play_bgm",
          params: { bgmId: "train_theme", fadeSeconds: 1.5 },
        },
      },
    ]),
    [],
  );
}

{
  const node = exportSingleNode({
    eventId: "evt_001",
    eventType: "inventory.add_item",
    params: { itemId: "key_red", count: 1 },
  });
  assert.equal(node.eventId, "evt_001");
  assert.equal(node.eventType, "inventory.add_item");
  assert.deepEqual(node.params, { itemId: "key_red", count: 1 });
}

{
  const errors = getErrors([
    { id: "N_Event", type: "emitevent", data: { eventId: "", eventType: "" } },
  ]);
  assert.equal(errors.length, 1);
  assert.match(errors[0].message, /eventId 和 eventType/);
}

{
  const errors = getErrors([
    {
      id: "N_Event",
      type: "emitevent",
      data: { eventType: "bad.params", params: ["not", "object"] },
    },
  ]);
  assert.equal(errors.length, 1);
  assert.match(errors[0].message, /params 必须是 object/);
}

{
  const imported = importFromYAML(`
nodes:
  - nodeId: N_Legacy
    nodeType: EmitEvent
    emitEvent:
      eventId: legacy_nested
      eventType: debug.legacy
      params:
        source: old
edges: []
`);
  assert.deepEqual(imported.nodes[0].data, {
    eventId: "legacy_nested",
    eventType: "debug.legacy",
    params: { source: "old" },
  });
}

console.log("event model tests passed");
