import YAML from "yaml";

import {
  parseStoryFromYAML,
  validateStoryDocument,
} from "./story-format.js";
import {
  parseOutlineFromYAML,
  validateOutlineDocument,
} from "./outline-format.js";
import { parseGlobalConfigFromYAML } from "./global-config.js";

export const PROJECT_LAYOUT = Object.freeze({
  storiesDir: "Stories/",
  configDir: "Config/",
  defaultOutlinePath: "Stories/main_story.nroutline",
  defaultGlobalConfigPath: "Config/global-config.nrstory",
});

export const PROJECT_ASSET_KINDS = Object.freeze({
  story: "story",
  outline: "outline",
  globalConfig: "global-config",
  conversionProfile: "conversion-profile",
  conversionNotes: "conversion-notes",
  unknown: "unknown",
});

export const REVIEW_SEVERITIES = Object.freeze({
  error: "error",
  warning: "warning",
  review: "review",
});

function toTrimmedString(value) {
  return String(value ?? "").trim();
}

function normalizePath(path) {
  return toTrimmedString(path).replace(/^\/+/, "");
}

function fileNameFromPath(path) {
  return normalizePath(path).split("/").pop() || "";
}

function fileStemFromPath(path) {
  return fileNameFromPath(path).replace(/\.[^.]+$/, "");
}

function readYamlMeta(content) {
  try {
    const parsed = YAML.parse(String(content ?? "")) || {};
    return parsed?.meta && typeof parsed.meta === "object" ? parsed.meta : {};
  } catch {
    return {};
  }
}

export function classifyProjectAsset(input = {}) {
  const path = normalizePath(input.path);
  const lowerPath = path.toLowerCase();
  const fileName = fileNameFromPath(path).toLowerCase();

  if (lowerPath.endsWith(".nroutline") || lowerPath.endsWith(".nrrail")) {
    return PROJECT_ASSET_KINDS.outline;
  }

  if (lowerPath.endsWith(".nrstory")) {
    const meta = readYamlMeta(input.content);
    return meta.configType === "GlobalConfig"
      ? PROJECT_ASSET_KINDS.globalConfig
      : PROJECT_ASSET_KINDS.story;
  }

  if (fileName === "conversion-profile.md") {
    return PROJECT_ASSET_KINDS.conversionProfile;
  }

  if (fileName === "conversion-notes.md") {
    return PROJECT_ASSET_KINDS.conversionNotes;
  }

  return PROJECT_ASSET_KINDS.unknown;
}

export function normalizeProjectAsset(input = {}) {
  const path = normalizePath(input.path);
  const kind = input.kind || classifyProjectAsset({ ...input, path });
  return {
    path,
    kind,
    content: String(input.content ?? ""),
    legacy: path.toLowerCase().endsWith(".nrrail"),
    source: input.source || "project",
  };
}

export function normalizeStoryProject(input = {}) {
  return {
    id: toTrimmedString(input.id) || "story-project",
    title: toTrimmedString(input.title) || "Story Project",
    rootPath: normalizePath(input.rootPath || ""),
    storage: input.storage || "github",
    branch: toTrimmedString(input.branch) || "main",
  };
}

export function normalizeImportPackage(input = {}) {
  const id = toTrimmedString(input.id) || "import-package";
  return {
    id,
    title: toTrimmedString(input.title) || "Import Package",
    files: (Array.isArray(input.files) ? input.files : []).map((file) =>
      normalizeProjectAsset({
        ...file,
        source: file?.source || `import-package:${id}`,
      }),
    ),
    reviewItems: Array.isArray(input.reviewItems) ? input.reviewItems : [],
  };
}

export function normalizeConversionProfile(input = {}) {
  return {
    path: normalizePath(input.path || PROJECT_LAYOUT.configDir + "conversion-profile.md"),
    content: String(input.content ?? ""),
    source: input.source || "project",
  };
}

export function createProjectSnapshot(input = {}) {
  const assets = (Array.isArray(input.assets) ? input.assets : []).map(
    normalizeProjectAsset,
  );
  const importPackages = (Array.isArray(input.importPackages)
    ? input.importPackages
    : []
  ).map(normalizeImportPackage);
  const conversionProfiles = (Array.isArray(input.conversionProfiles)
    ? input.conversionProfiles
    : []
  ).map(normalizeConversionProfile);

  return {
    project: normalizeStoryProject(input.project),
    assets,
    assetIndex: Object.fromEntries(assets.map((asset) => [asset.path, asset])),
    storyEntries: assets
      .filter((asset) => asset.kind === PROJECT_ASSET_KINDS.story)
      .map((asset) => ({
        path: asset.path,
        storyId: readYamlMeta(asset.content).storyId || fileStemFromPath(asset.path),
      })),
    outlineEntries: assets
      .filter((asset) => asset.kind === PROJECT_ASSET_KINDS.outline)
      .map((asset) => ({
        path: asset.path,
        railId: readYamlMeta(asset.content).railId || fileStemFromPath(asset.path),
        legacy: asset.legacy,
      })),
    configEntries: assets
      .filter((asset) => asset.kind === PROJECT_ASSET_KINDS.globalConfig)
      .map((asset) => ({ path: asset.path })),
    importPackages,
    conversionProfiles,
  };
}

function makeReviewItem({
  severity = REVIEW_SEVERITIES.review,
  category = "review",
  message,
  assetPath = "",
  target = {},
  source = "project",
  index = 0,
}) {
  const normalizedSeverity =
    severity === REVIEW_SEVERITIES.error ||
    severity === REVIEW_SEVERITIES.warning ||
    severity === REVIEW_SEVERITIES.review
      ? severity
      : REVIEW_SEVERITIES.review;
  const pathPart = assetPath || source;
  return {
    id: `${pathPart}:${source}:${normalizedSeverity}:${category}:${index}`,
    severity: normalizedSeverity,
    category,
    message: String(message || ""),
    assetPath,
    target,
    source,
  };
}

export function normalizeProjectReviewItem(input = {}) {
  return makeReviewItem(input);
}

function addValidationItems(items, asset, result, source) {
  const importPackageTarget = asset.importPackageId
    ? { importPackageId: asset.importPackageId }
    : {};
  (result?.errors || []).forEach((error, index) => {
    items.push(
      makeReviewItem({
        severity: REVIEW_SEVERITIES.error,
        category: "validation",
        message: error.message || error,
        assetPath: asset.path,
        target: {
          ...importPackageTarget,
          nodeId: error.nodeId || "",
          edgeId: error.edgeId || "",
        },
        source,
        index,
      }),
    );
  });

  (result?.warnings || []).forEach((warning, index) => {
    items.push(
      makeReviewItem({
        severity: REVIEW_SEVERITIES.warning,
        category: "validation",
        message: warning.message || warning,
        assetPath: asset.path,
        target: {
          ...importPackageTarget,
          nodeId: warning.nodeId || "",
          edgeId: warning.edgeId || "",
        },
        source,
        index,
      }),
    );
  });
}

function addParseError(items, asset, error) {
  items.push(
    makeReviewItem({
      severity: REVIEW_SEVERITIES.error,
      category: "parse",
      message: error?.message || "无法解析项目资产",
      assetPath: asset.path,
      target: asset.importPackageId
        ? { importPackageId: asset.importPackageId }
        : {},
      source: "parser",
    }),
  );
}

function summarizeReviewItems(items) {
  const summary = {
    total: items.length,
    errorCount: 0,
    warningCount: 0,
    reviewCount: 0,
  };

  items.forEach((item) => {
    if (item.severity === REVIEW_SEVERITIES.error) summary.errorCount += 1;
    else if (item.severity === REVIEW_SEVERITIES.warning) summary.warningCount += 1;
    else summary.reviewCount += 1;
  });

  return summary;
}

function groupReviewItemsByAsset(items) {
  return items.reduce((grouped, item) => {
    const key = item.assetPath || item.source || "project";
    grouped[key] = grouped[key] || [];
    grouped[key].push(item);
    return grouped;
  }, {});
}

function collectImportPackageAssets(importPackages) {
  return (Array.isArray(importPackages) ? importPackages : []).flatMap(
    (importPackage) =>
      (importPackage.files || []).map((asset) => ({
        ...asset,
        importPackageId: importPackage.id,
      })),
  );
}

function parsedAssetKey(asset) {
  return asset.source && asset.source !== "project"
    ? `${asset.source}:${asset.path}`
    : asset.path;
}

export function buildProjectReviewQueue(input = {}) {
  const snapshot = input.assets ? createProjectSnapshot(input) : input;
  const items = [];
  const parsedAssets = {};
  const globalVariables = [];
  const storyEntries = [];
  const projectAssets = snapshot.assets || [];
  const importPackageAssets = collectImportPackageAssets(snapshot.importPackages);
  const reviewAssets = [...projectAssets, ...importPackageAssets];

  if (!projectAssets.some((asset) => asset.path.startsWith(PROJECT_LAYOUT.storiesDir))) {
    items.push(
      makeReviewItem({
        severity: REVIEW_SEVERITIES.warning,
        category: "project-layout",
        message: `缺少 ${PROJECT_LAYOUT.storiesDir} 项目目录中的故事资产`,
        source: "project-layout",
      }),
    );
  }

  for (const asset of reviewAssets) {
    try {
      if (asset.kind === PROJECT_ASSET_KINDS.globalConfig) {
        const parsed = parseGlobalConfigFromYAML(asset.content);
        parsedAssets[parsedAssetKey(asset)] = parsed;
        globalVariables.push(...parsed.variables);
      } else if (asset.kind === PROJECT_ASSET_KINDS.story) {
        const parsed = parseStoryFromYAML(asset.content);
        parsedAssets[parsedAssetKey(asset)] = parsed;
        storyEntries.push({
          path: asset.path,
          storyId: parsed?.meta?.storyId || fileStemFromPath(asset.path),
        });
      } else if (asset.kind === PROJECT_ASSET_KINDS.outline) {
        parsedAssets[parsedAssetKey(asset)] = parseOutlineFromYAML(asset.content);
      }
    } catch (error) {
      addParseError(items, asset, error);
    }
  }

  if (storyEntries.length === 0) {
    items.push(
      makeReviewItem({
        severity: REVIEW_SEVERITIES.warning,
        category: "project-layout",
        message: "Story Project 中还没有可预览或导出的 .nrstory",
        source: "project-layout",
        index: 1,
      }),
    );
  }

  if (!projectAssets.some((asset) => asset.kind === PROJECT_ASSET_KINDS.outline)) {
    items.push(
      makeReviewItem({
        severity: REVIEW_SEVERITIES.warning,
        category: "project-layout",
        message: `缺少项目总纲，建议创建 ${PROJECT_LAYOUT.defaultOutlinePath}`,
        source: "project-layout",
        index: 2,
      }),
    );
  }

  for (const asset of reviewAssets) {
    const parsed = parsedAssets[parsedAssetKey(asset)];
    if (!parsed) continue;

    if (asset.kind === PROJECT_ASSET_KINDS.story) {
      addValidationItems(
        items,
        asset,
        validateStoryDocument(parsed, { variables: globalVariables }),
        "story-validation",
      );
    } else if (asset.kind === PROJECT_ASSET_KINDS.outline) {
      addValidationItems(
        items,
        asset,
        validateOutlineDocument(parsed, {
          storyEntries,
          variables: globalVariables,
          requireKnownStoryReferences: true,
        }),
        "outline-validation",
      );
    }
  }

  (snapshot.importPackages || []).forEach((importPackage, packageIndex) => {
    (importPackage.reviewItems || []).forEach((item, itemIndex) => {
      items.push(
        makeReviewItem({
          severity: item.severity || REVIEW_SEVERITIES.review,
          category: item.category || "import-package",
          message: item.message,
          assetPath: item.assetPath || "",
          target: {
            importPackageId: importPackage.id,
            ...(item.target || {}),
          },
          source: "import-package",
          index: packageIndex * 1000 + itemIndex,
        }),
      );
    });
  });

  return {
    project: snapshot.project,
    items,
    summary: summarizeReviewItems(items),
    byAsset: groupReviewItemsByAsset(items),
    parsedAssets,
    storyEntries,
    variables: globalVariables,
    importPackages: snapshot.importPackages || [],
  };
}
