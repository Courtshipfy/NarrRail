import {
  PROJECT_STORAGE_KINDS,
  assertProjectStorageAdapter,
  normalizeProjectStorageContext,
  normalizeProjectStoragePath,
} from "../core/project-storage.js";
import { normalizeProjectAsset } from "../core/story-project.js";

const GITHUB_FILE_CONTENT_ROUTE = "/api/github/file-content";
const GITHUB_COMMIT_FILE_ROUTE = "/api/github/commit-file";
const GITHUB_DELETE_FILE_ROUTE = "/api/github/delete-file";

function createRequiredGithubContext(input = {}) {
  const context = normalizeProjectStorageContext({
    ...input,
    kind: PROJECT_STORAGE_KINDS.github,
  });

  if (!context.owner || !context.repo) {
    throw new Error("GitHub project storage requires owner and repo");
  }

  return context;
}

function buildProjectQuery(context, query = {}) {
  return {
    owner: context.owner,
    repo: context.repo,
    branch: context.branch,
    ...query,
  };
}

function buildProjectBody(context, body = {}) {
  return {
    owner: context.owner,
    repo: context.repo,
    branch: context.branch,
    ...body,
  };
}

export function createGithubProjectStorageFetchTransport({
  baseUrl = window.location.origin,
  fetchImpl = fetch,
} = {}) {
  return {
    async request({ path, method = "GET", query = {}, body }) {
      const url = new URL(path, baseUrl);
      Object.entries(query).forEach(([key, value]) => {
        if (value !== undefined && value !== null && value !== "") {
          url.searchParams.set(key, String(value));
        }
      });

      const response = await fetchImpl(url.toString(), {
        method,
        credentials: "include",
        headers: body ? { "Content-Type": "application/json" } : undefined,
        body: body ? JSON.stringify(body) : undefined,
      });
      const data = await response.json();

      if (!response.ok) {
        const error = new Error(
          data?.error || `Project storage request failed (${response.status})`,
        );
        error.status = response.status;
        error.payload = data;
        throw error;
      }

      return data;
    },
  };
}

export function createGithubProjectStorageAdapter(input = {}) {
  const context = createRequiredGithubContext(input);
  const transport =
    input.transport || createGithubProjectStorageFetchTransport(input.transportOptions);

  const adapter = {
    context,

    async listAssets(options = {}) {
      const path = normalizeProjectStoragePath(options.path ?? context.rootPath);
      const data = await transport.request({
        path: GITHUB_FILE_CONTENT_ROUTE,
        method: "GET",
        query: buildProjectQuery(context, {
          mode: "list",
          path,
        }),
      });

      return (Array.isArray(data?.files) ? data.files : []).map((asset) => ({
        ...asset,
        ...normalizeProjectAsset({
          ...asset,
          source: asset?.source || PROJECT_STORAGE_KINDS.github,
        }),
      }));
    },

    async readAsset(path) {
      const assetPath = normalizeProjectStoragePath(path);
      const data = await transport.request({
        path: GITHUB_FILE_CONTENT_ROUTE,
        method: "GET",
        query: buildProjectQuery(context, {
          mode: "content",
          path: assetPath,
        }),
      });

      return {
        ...normalizeProjectAsset({
          ...data,
          path: data?.path || assetPath,
          source: data?.source || PROJECT_STORAGE_KINDS.github,
        }),
        sha: data?.sha || "",
        size: data?.size || 0,
      };
    },

    async writeAsset(inputAsset = {}) {
      const path = normalizeProjectStoragePath(inputAsset.path);
      const data = await transport.request({
        path: GITHUB_COMMIT_FILE_ROUTE,
        method: "POST",
        body: buildProjectBody(context, {
          path,
          content: String(inputAsset.content ?? ""),
          message: inputAsset.message,
          sha: inputAsset.sha,
        }),
      });

      return data;
    },

    async deleteAsset(inputAsset = {}) {
      const path = normalizeProjectStoragePath(inputAsset.path);
      const sha = inputAsset.sha || (await adapter.readAsset(path)).sha;
      const data = await transport.request({
        path: GITHUB_DELETE_FILE_ROUTE,
        method: "POST",
        body: buildProjectBody(context, {
          path,
          sha,
          message: inputAsset.message,
        }),
      });

      return data;
    },

    async commit() {
      return {
        ok: true,
        message: "GitHub project storage commits changes per asset write.",
      };
    },

    async getStatus() {
      return {
        kind: context.kind,
        owner: context.owner,
        repo: context.repo,
        branch: context.branch,
        rootPath: context.rootPath,
        canCommit: true,
      };
    },
  };

  return assertProjectStorageAdapter(adapter);
}
