import assert from "node:assert/strict";

import {
  PROJECT_ASSET_KINDS,
  buildProjectReviewQueue,
  classifyProjectAsset,
  createProjectSnapshot,
  normalizeProjectReviewItem,
} from "../src/core/story-project.js";

const globalConfigYaml = `
meta:
  schemaVersion: 1
  configType: GlobalConfig
variables:
  - name: HasTicket
    type: Bool
    scope: Global
    defaultValue: "true"
presetSpeakers:
  - id: Hero
    displayName: 主角
`;

const chapterYaml = `
meta:
  schemaVersion: 1
  storyId: chapter_01
  entryNodeId: N_Start
variables: []
nodes:
  - nodeId: N_Start
    nodeType: Dialogue
    dialogue:
      speakerId: Hero
      textKey: 开场
      speechRate: 1
      voiceAsset: ""
  - nodeId: N_End
    nodeType: End
edges:
  - sourceNodeId: N_Start
    sourceHandle: ""
    targetNodeId: N_End
    priority: 0
`;

const outlineYaml = `
meta:
  schemaVersion: 1
  railId: main_story
  title: 主线总纲
  entryNodeId: rail_chapter_01
nodes:
  - nodeId: rail_chapter_01
    nodeType: Story
    title: 第一章
    storyId: chapter_01
  - nodeId: rail_end
    nodeType: End
    title: 完
edges:
  - sourceNodeId: rail_chapter_01
    sourceHandle: ""
    targetNodeId: rail_end
    priority: 0
`;

{
  assert.equal(
    classifyProjectAsset({
      path: "Config/global-config.nrstory",
      content: globalConfigYaml,
    }),
    PROJECT_ASSET_KINDS.globalConfig,
  );
  assert.equal(
    classifyProjectAsset({ path: "Stories/main_story.nroutline", content: outlineYaml }),
    PROJECT_ASSET_KINDS.outline,
  );
  assert.equal(
    classifyProjectAsset({ path: "Config/conversion-profile.md", content: "" }),
    PROJECT_ASSET_KINDS.conversionProfile,
  );
}

{
  const snapshot = createProjectSnapshot({
    project: { id: "demo", title: "Demo Project", branch: "draft" },
    assets: [
      { path: "Stories/chapter_01.nrstory", content: chapterYaml },
      { path: "Stories/main_story.nroutline", content: outlineYaml },
      { path: "Config/global-config.nrstory", content: globalConfigYaml },
    ],
  });

  assert.equal(snapshot.project.title, "Demo Project");
  assert.equal(snapshot.assetIndex["Stories/chapter_01.nrstory"].kind, "story");
  assert.deepEqual(snapshot.storyEntries, [
    { path: "Stories/chapter_01.nrstory", storyId: "chapter_01" },
  ]);

  const queue = buildProjectReviewQueue(snapshot);
  assert.deepEqual(queue.summary, {
    total: 0,
    errorCount: 0,
    warningCount: 0,
    reviewCount: 0,
  });
  assert.deepEqual(queue.storyEntries, [
    { path: "Stories/chapter_01.nrstory", storyId: "chapter_01" },
  ]);
  assert.equal(queue.variables[0].name, "HasTicket");
}

{
  const queue = buildProjectReviewQueue({
    project: { id: "missing-ref" },
    assets: [
      {
        path: "Stories/main_story.nroutline",
        content: outlineYaml.replace("storyId: chapter_01", "storyId: missing_story"),
      },
      { path: "Config/global-config.nrstory", content: globalConfigYaml },
    ],
  });

  assert.equal(queue.summary.errorCount, 1);
  assert.equal(queue.byAsset["Stories/main_story.nroutline"].length >= 1, true);
  assert.equal(
    queue.items.some((item) => /引用的脚本不存在/.test(item.message)),
    true,
  );
}

{
  const item = normalizeProjectReviewItem({
    severity: "warning",
    category: "manual-check",
    message: "需要确认",
    assetPath: "Stories/check.nrstory",
  });
  assert.equal(item.severity, "warning");
  assert.equal(item.category, "manual-check");
  assert.equal(item.assetPath, "Stories/check.nrstory");
}

{
  const queue = buildProjectReviewQueue({
    project: { id: "import-review" },
    assets: [
      { path: "Stories/chapter_01.nrstory", content: chapterYaml },
      { path: "Stories/main_story.nroutline", content: outlineYaml },
    ],
    importPackages: [
      {
        id: "pkg-001",
        title: "Imported draft",
        reviewItems: [
          {
            severity: "review",
            category: "ambiguous-source",
            message: "第 3 场的说话人需要人工确认",
            assetPath: "Stories/imported_story.nrstory",
          },
        ],
      },
    ],
  });

  assert.equal(queue.summary.reviewCount, 1);
  assert.equal(queue.items.at(-1).target.importPackageId, "pkg-001");
}

{
  const invalidImportedStory = chapterYaml.replace(
    "entryNodeId: N_Start",
    "entryNodeId: Missing",
  );
  const queue = buildProjectReviewQueue({
    project: { id: "import-file-review" },
    assets: [
      { path: "Stories/chapter_01.nrstory", content: chapterYaml },
      { path: "Stories/main_story.nroutline", content: outlineYaml },
    ],
    importPackages: [
      {
        id: "pkg-002",
        title: "Imported file draft",
        files: [
          {
            path: "Stories/imported_story.nrstory",
            content: invalidImportedStory,
          },
        ],
      },
    ],
  });

  assert.equal(queue.summary.errorCount, 1);
  assert.equal(
    queue.items.some(
      (item) =>
        item.assetPath === "Stories/imported_story.nrstory" &&
        item.target.importPackageId === "pkg-002" &&
        /入口节点不存在/.test(item.message),
    ),
    true,
  );
}

console.log("story project tests passed");
