import assert from "node:assert/strict";
import YAML from "yaml";

import {
  parseStoryFromYAML,
  serializeStoryToYAML,
  validateStoryDocument,
} from "../src/core/story-format.js";

const storyYaml = `
meta:
  schemaVersion: 1
  storyId: format_demo
  entryNodeId: N_Start
variables: []
nodes:
  - nodeId: N_Start
    nodeType: Dialogue
    dialogue:
      speakerId: Hero
      textKey: Welcome aboard.
      speechRate: 1
      voiceAsset: ""
  - nodeId: N_Choice
    nodeType: Choice
    choices:
      - textKey: Ask about the route
        targetNodeId: N_Check
      - textKey: Stay quiet
        targetNodeId: N_End
    choiceMode: ExhaustiveUntilComplete
    choiceCompletionTargetNodeId: N_End
    choiceTimer:
      enabled: true
      durationSeconds: 12
      timeoutChoiceTextKey: Stay quiet
  - nodeId: N_Check
    nodeType: Condition
    condition:
      branches:
        - label: KnowsRoute
          logic: All
          terms:
            - variable:
                name: KnowsRoute
                type: Bool
                scope: Global
              operator: "=="
              compareValue: "true"
  - nodeId: N_Event
    nodeType: EmitEvent
    eventType: route.explained
    params:
      routeId: night_train
  - nodeId: N_End
    nodeType: End
edges:
  - sourceNodeId: N_Start
    sourceHandle: ""
    targetNodeId: N_Choice
    priority: 0
  - sourceNodeId: N_Choice
    sourceHandle: choice-0
    targetNodeId: N_Check
    priority: 0
  - sourceNodeId: N_Choice
    sourceHandle: choice-1
    targetNodeId: N_End
    priority: 1
  - sourceNodeId: N_Choice
    sourceHandle: choice-timeout
    targetNodeId: N_End
    priority: 99
  - sourceNodeId: N_Choice
    sourceHandle: choice-complete
    targetNodeId: N_End
    priority: 100
  - sourceNodeId: N_Check
    sourceHandle: condition-0
    targetNodeId: N_Event
    priority: 0
  - sourceNodeId: N_Check
    sourceHandle: condition-fallback
    targetNodeId: N_End
    priority: 1
  - sourceNodeId: N_Event
    sourceHandle: ""
    targetNodeId: N_End
    priority: 0
`;

{
  const imported = parseStoryFromYAML(storyYaml);
  const variables = [
    { name: "KnowsRoute", type: "Bool", scope: "Global", boolValue: true },
  ];
  const result = validateStoryDocument(imported, { variables });

  assert.deepEqual(result.errors, []);

  const exported = YAML.parse(
    serializeStoryToYAML({ ...imported, variables }),
  );
  const choice = exported.nodes.find((node) => node.nodeId === "N_Choice");
  const condition = exported.nodes.find((node) => node.nodeId === "N_Check");

  assert.equal(exported.meta.storyId, "format_demo");
  assert.equal(choice.choiceMode, "ExhaustiveUntilComplete");
  assert.equal(choice.choiceTimer.durationSeconds, 12);
  assert.equal(choice.choiceCompletionTargetNodeId, "N_End");
  assert.equal(condition.condition.branches[0].label, "KnowsRoute");
  assert.equal(
    exported.edges.some((edge) => Object.hasOwn(edge, "condition")),
    false,
  );
}

{
  const legacyChoiceYaml = `
meta:
  schemaVersion: 1
  storyId: legacy_choice
  entryNodeId: N_Choice
variables: []
nodes:
  - nodeId: N_Choice
    nodeType: Choice
    choice:
      choices:
        - textKey: Legacy path
          targetNodeId: N_End
  - nodeId: N_End
    nodeType: End
edges: []
`;
  const imported = parseStoryFromYAML(legacyChoiceYaml);
  assert.equal(imported.nodes[0].data.choices[0].textKey, "Legacy path");
  assert.equal(imported.edges[0].sourceHandle, "choice-0");
}

{
  const deprecatedEdgeYaml = `
meta:
  schemaVersion: 1
  storyId: bad_edge_condition
  entryNodeId: N_Start
nodes:
  - nodeId: N_Start
    nodeType: Dialogue
    dialogue: { textKey: Start }
  - nodeId: N_End
    nodeType: End
edges:
  - sourceNodeId: N_Start
    targetNodeId: N_End
    condition: { variable: Flag }
`;
  assert.throws(
    () => parseStoryFromYAML(deprecatedEdgeYaml),
    /deprecated edge condition/i,
  );
}

console.log("story format tests passed");
