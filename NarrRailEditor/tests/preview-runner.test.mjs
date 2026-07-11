import assert from "node:assert/strict";

import {
  buildInitialVarSnapshot,
  createStoryPreviewRunner,
} from "../src/utils/story-preview-runner.js";
import { createRailPreviewRunner } from "../src/utils/rail-preview-runner.js";

function makePreviewStory() {
  return {
    meta: { storyId: "chapter_01", entryNodeId: "N_Start" },
    nodes: [
      {
        id: "N_Start",
        type: "dialogue",
        data: { speakerId: "Hero", lines: [{ textKey: "开场" }] },
      },
      {
        id: "N_AddAffinity",
        type: "setvariable",
        data: { variableName: "Affinity", operation: "Add", value: "2" },
      },
      {
        id: "N_Check",
        type: "condition",
        data: {
          condition: {
            branches: [
              {
                label: "HighAffinity",
                logic: "All",
                terms: [
                  {
                    variable: { name: "Affinity", type: "Int", scope: "Global" },
                    operator: ">=",
                    compareValue: "5",
                  },
                ],
              },
            ],
          },
        },
      },
      {
        id: "N_Choice",
        type: "choice",
        data: {
          choices: [{ textKey: "继续询问" }],
          choiceTimer: { enabled: true, durationSeconds: 9, timeoutChoiceTextKey: "沉默" },
        },
      },
      {
        id: "N_Event",
        type: "emitevent",
        data: { eventType: "route.explained", params: { routeId: "night" } },
      },
      { id: "N_End", type: "end", data: {} },
    ],
    edges: [
      { id: "e1", source: "N_Start", target: "N_AddAffinity", data: { priority: 0 } },
      { id: "e2", source: "N_AddAffinity", target: "N_Check", data: { priority: 0 } },
      {
        id: "e3",
        source: "N_Check",
        sourceHandle: "condition-0",
        target: "N_Choice",
        data: { priority: 0 },
      },
      {
        id: "e4",
        source: "N_Check",
        sourceHandle: "condition-fallback",
        target: "N_End",
        data: { priority: 1 },
      },
      {
        id: "e5",
        source: "N_Choice",
        sourceHandle: "choice-0",
        target: "N_Event",
        data: { priority: 0 },
      },
      {
        id: "e6",
        source: "N_Choice",
        sourceHandle: "choice-timeout",
        target: "N_End",
        data: { priority: 99 },
      },
      { id: "e7", source: "N_Event", target: "N_End", data: { priority: 0 } },
    ],
  };
}

{
  const vars = buildInitialVarSnapshot([
    { name: "Affinity", type: "Int", intValue: 3 },
  ]);
  const runner = createStoryPreviewRunner(makePreviewStory(), vars);

  runner.advance();
  assert.deepEqual(runner.state.timeline.at(-1), {
    kind: "dialogue",
    nodeId: "N_Start",
    speaker: "Hero",
    text: "开场",
  });

  runner.advance();
  assert.equal(runner.state.status, "await-choice");
  assert.equal(runner.state.vars.Affinity, 5);
  assert.deepEqual(runner.state.pendingChoices, [
    { handle: "choice-0", text: "继续询问" },
  ]);
  assert.equal(runner.state.choiceTimer.remainingSeconds, 9);

  runner.choose("choice-0");
  assert.equal(runner.state.status, "ended");
  assert.equal(
    runner.state.timeline.some(
      (item) => item.kind === "emitevent" && item.payload.eventType === "route.explained",
    ),
    true,
  );
}

{
  const vars = buildInitialVarSnapshot([
    { name: "Affinity", type: "Int", intValue: 0 },
  ]);
  const runner = createStoryPreviewRunner(makePreviewStory(), vars);

  runner.advance();
  runner.advance();
  assert.equal(runner.state.status, "ended");
  assert.equal(
    runner.state.timeline.some((item) => item.kind === "condition"),
    true,
  );
}

{
  const story = makePreviewStory();
  const railRunner = createRailPreviewRunner(
    {
      meta: { railId: "main_story", entryNodeId: "rail_chapter_01" },
      nodes: [
        {
          id: "rail_chapter_01",
          type: "railstory",
          data: { title: "第一章", storyId: "chapter_01" },
        },
        { id: "rail_end", type: "railend", data: { title: "完" } },
      ],
      edges: [
        {
          id: "rail-edge-1",
          source: "rail_chapter_01",
          target: "rail_end",
          data: { priority: 0 },
        },
      ],
    },
    {
      variables: [{ name: "Affinity", type: "Int", intValue: 3 }],
      resolveStoryById: async (storyId) =>
        storyId === "chapter_01" ? story : null,
    },
  );

  await railRunner.advance();
  await railRunner.advance();
  assert.equal(railRunner.state.status, "await-choice");
  assert.equal(railRunner.state.timeline[0].railTitle, "第一章");

  await railRunner.choose("choice-0");
  assert.equal(railRunner.state.status, "ended");
  assert.equal(railRunner.state.timeline.at(-1).kind, "railend");
}

console.log("preview runner tests passed");
