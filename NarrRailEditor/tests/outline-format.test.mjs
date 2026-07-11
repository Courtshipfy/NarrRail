import assert from "node:assert/strict";
import YAML from "yaml";

import {
  createEmptyOutline,
  parseOutlineFromYAML,
  serializeOutlineToYAML,
  validateOutlineDocument,
} from "../src/core/outline-format.js";

{
  const outline = {
    meta: {
      schemaVersion: 1,
      railId: "main_story",
      title: "主线总纲",
      entryNodeId: "rail_start",
    },
    nodes: [
      {
        id: "rail_start",
        type: "railstory",
        position: { x: 0, y: 0 },
        data: { title: "第一章", storyId: "chapter_01", summary: "" },
      },
      {
        id: "rail_branch",
        type: "railbranch",
        position: { x: 280, y: 0 },
        data: {
          title: "路线判断",
          branches: [
            {
              label: "HasTicket",
              logic: "All",
              terms: [
                {
                  variable: { name: "HasTicket", type: "Bool", scope: "Global" },
                  operator: "==",
                  compareValue: "true",
                },
              ],
            },
          ],
        },
      },
      {
        id: "rail_end",
        type: "railend",
        position: { x: 560, y: 0 },
        data: { title: "结束", summary: "" },
      },
    ],
    edges: [
      {
        id: "rail-edge-start-branch",
        source: "rail_start",
        sourceHandle: "",
        target: "rail_branch",
        data: { priority: 0 },
      },
      {
        id: "rail-edge-ticket-end",
        source: "rail_branch",
        sourceHandle: "branch-0",
        target: "rail_end",
        data: { priority: 0 },
      },
      {
        id: "rail-edge-fallback-end",
        source: "rail_branch",
        sourceHandle: "branch-fallback",
        target: "rail_end",
        data: { priority: 1 },
      },
    ],
  };

  const yaml = serializeOutlineToYAML(outline);
  const parsedYaml = YAML.parse(yaml);
  assert.equal(parsedYaml.meta.railId, "main_story");
  assert.equal(parsedYaml.nodes[0].nodeType, "Story");

  const imported = parseOutlineFromYAML(yaml);
  const result = validateOutlineDocument(imported, {
    storyEntries: [{ storyId: "chapter_01" }],
    variables: [{ name: "HasTicket" }],
  });

  assert.deepEqual(result.errors, []);
}

{
  const empty = createEmptyOutline("empty_outline");
  const result = validateOutlineDocument(empty);
  assert.deepEqual(result.errors, []);
}

{
  const importedLegacyNrrail = parseOutlineFromYAML(`
meta:
  schemaVersion: 1
  railId: legacy_outline
  title: Legacy outline
  entryNodeId: rail_missing_story
nodes:
  - nodeId: rail_missing_story
    nodeType: Story
    title: Missing story
    storyId: missing_story
edges: []
`);
  const result = validateOutlineDocument(importedLegacyNrrail, {
    storyEntries: [{ storyId: "chapter_01" }],
  });

  assert.equal(result.errors.length, 1);
  assert.match(result.errors[0], /引用的脚本不存在/);
}

console.log("outline format tests passed");
