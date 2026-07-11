export const PROJECT_STORAGE_KINDS = Object.freeze({
  github: "github",
  localFilesystem: "local-filesystem",
  gitCommand: "git-command",
  browserDraft: "browser-draft",
});

export const PROJECT_STORAGE_OPERATIONS = Object.freeze([
  "listAssets",
  "readAsset",
  "writeAsset",
  "deleteAsset",
]);

function toTrimmedString(value) {
  return String(value ?? "").trim();
}

export function normalizeProjectStoragePath(path) {
  return toTrimmedString(path).replace(/^\/+/, "");
}

export function normalizeProjectStorageContext(input = {}) {
  const kind = Object.values(PROJECT_STORAGE_KINDS).includes(input.kind)
    ? input.kind
    : PROJECT_STORAGE_KINDS.github;

  return {
    kind,
    owner: toTrimmedString(input.owner),
    repo: toTrimmedString(input.repo),
    branch: toTrimmedString(input.branch) || "main",
    rootPath: normalizeProjectStoragePath(input.rootPath || ""),
  };
}

export function assertProjectStorageAdapter(adapter) {
  const missing = PROJECT_STORAGE_OPERATIONS.filter(
    (operation) => typeof adapter?.[operation] !== "function",
  );
  if (missing.length > 0) {
    throw new TypeError(
      `Project storage adapter is missing: ${missing.join(", ")}`,
    );
  }
  return adapter;
}
