import assert from "node:assert/strict";
import YAML from "yaml";

import {
  parseGlobalConfigFromYAML,
  serializeGlobalConfigToYAML,
} from "../src/core/global-config.js";

{
  const yaml = serializeGlobalConfigToYAML({
    variables: [
      { name: "Affinity", type: "Int", intValue: 3 },
      { name: "HasTicket", type: "Bool", boolValue: true },
      { name: "Affinity", type: "Int", intValue: 99 },
    ],
    presetSpeakers: [
      { id: "Hero", displayName: "主角", color: "#7c3aed" },
      { id: "Hero", displayName: "重复角色" },
      "Conductor",
    ],
  });

  const parsedYaml = YAML.parse(yaml);
  assert.equal(parsedYaml.meta.configType, "GlobalConfig");
  assert.deepEqual(parsedYaml.variables.map((variable) => variable.name), [
    "Affinity",
    "HasTicket",
  ]);

  const parsed = parseGlobalConfigFromYAML(yaml);
  assert.deepEqual(parsed.variables, [
    { name: "Affinity", type: "Int", scope: "Global", intValue: 3 },
    { name: "HasTicket", type: "Bool", scope: "Global", boolValue: true },
  ]);
  assert.deepEqual(parsed.presetSpeakers, [
    { id: "Hero", displayName: "主角", color: "#7c3aed" },
    { id: "Conductor" },
  ]);
}

{
  assert.throws(
    () =>
      parseGlobalConfigFromYAML(`
meta:
  schemaVersion: 0
  configType: GlobalConfig
variables: []
presetSpeakers: []
`),
    /schemaVersion/,
  );
}

console.log("global config tests passed");
