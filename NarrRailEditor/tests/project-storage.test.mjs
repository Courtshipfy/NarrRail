import assert from "node:assert/strict";

import {
  PROJECT_ASSET_KINDS,
  PROJECT_STORAGE_KINDS,
  normalizeProjectStorageContext,
} from "../src/core/index.js";
import { createGithubProjectStorageAdapter } from "../src/utils/github-project-storage.js";

function createFakeTransport(fixtures = {}) {
  const requests = [];
  return {
    requests,
    async request(request) {
      requests.push(request);
      const key = `${request.method || "GET"} ${request.path}`;
      const handler = fixtures[key];
      if (!handler) {
        throw new Error(`Unexpected request: ${key}`);
      }
      return handler(request);
    },
  };
}

{
  const context = normalizeProjectStorageContext({
    kind: PROJECT_STORAGE_KINDS.github,
    owner: "Courtshipfy",
    repo: "NarrRail",
    branch: "draft",
    rootPath: "/Stories/",
  });

  assert.deepEqual(context, {
    kind: "github",
    owner: "Courtshipfy",
    repo: "NarrRail",
    branch: "draft",
    rootPath: "Stories/",
  });
}

{
  const transport = createFakeTransport({
    "GET /api/github/file-content": ({ query }) => {
      assert.deepEqual(query, {
        mode: "list",
        owner: "Courtshipfy",
        repo: "NarrRail",
        branch: "draft",
        path: "Stories/",
      });
      return {
        files: [
          {
            path: "Stories/chapter_01.nrstory",
            content: "",
            source: "github",
          },
        ],
      };
    },
  });
  const storage = createGithubProjectStorageAdapter({
    owner: "Courtshipfy",
    repo: "NarrRail",
    branch: "draft",
    rootPath: "Stories/",
    transport,
  });

  const assets = await storage.listAssets();

  assert.equal(transport.requests.length, 1);
  assert.equal(assets[0].path, "Stories/chapter_01.nrstory");
  assert.equal(assets[0].kind, PROJECT_ASSET_KINDS.story);
}

{
  const transport = createFakeTransport({
    "GET /api/github/file-content": ({ query }) => {
      assert.deepEqual(query, {
        mode: "content",
        owner: "Courtshipfy",
        repo: "NarrRail",
        branch: "main",
        path: "Stories/chapter_01.nrstory",
      });
      return {
        path: "Stories/chapter_01.nrstory",
        sha: "old-sha",
        content: "meta:\n  storyId: chapter_01\nnodes: []\nedges: []\n",
      };
    },
    "POST /api/github/commit-file": ({ body }) => {
      assert.deepEqual(body, {
        owner: "Courtshipfy",
        repo: "NarrRail",
        branch: "main",
        path: "Stories/chapter_01.nrstory",
        content: "updated",
        message: "feat(script): update chapter",
        sha: "old-sha",
      });
      return {
        ok: true,
        commit: { sha: "commit-sha", message: "feat(script): update chapter" },
        content: { path: "Stories/chapter_01.nrstory", sha: "new-sha" },
      };
    },
  });
  const storage = createGithubProjectStorageAdapter({
    owner: "Courtshipfy",
    repo: "NarrRail",
    transport,
  });

  const asset = await storage.readAsset("Stories/chapter_01.nrstory");
  const result = await storage.writeAsset({
    path: asset.path,
    content: "updated",
    message: "feat(script): update chapter",
    sha: asset.sha,
  });

  assert.equal(asset.kind, PROJECT_ASSET_KINDS.story);
  assert.equal(asset.sha, "old-sha");
  assert.equal(result.content.sha, "new-sha");
}

{
  const transport = createFakeTransport({
    "GET /api/github/file-content": ({ query }) => {
      assert.equal(query.path, "Stories/old.nrstory");
      return {
        path: "Stories/old.nrstory",
        sha: "delete-sha",
        content: "",
      };
    },
    "POST /api/github/delete-file": ({ body }) => {
      assert.deepEqual(body, {
        owner: "Courtshipfy",
        repo: "NarrRail",
        branch: "main",
        path: "Stories/old.nrstory",
        sha: "delete-sha",
        message: "chore(script): delete old",
      });
      return { ok: true, content: { path: "Stories/old.nrstory" } };
    },
  });
  const storage = createGithubProjectStorageAdapter({
    owner: "Courtshipfy",
    repo: "NarrRail",
    transport,
  });

  const result = await storage.deleteAsset({
    path: "Stories/old.nrstory",
    message: "chore(script): delete old",
  });

  assert.equal(result.ok, true);
  assert.equal(transport.requests.length, 2);
}

console.log("project storage tests passed");
