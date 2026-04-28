import YAML from "yaml";

export const GLOBAL_CONFIG_SCHEMA_VERSION = 1;

/**
 * YAML structure:
 * meta:
 *   schemaVersion: 1
 *   configType: GlobalConfig
 * variables: []
 * presetSpeakers: []
 */

function safeArray(value) {
  return Array.isArray(value) ? value : [];
}

function toTrimmedString(value) {
  return String(value ?? "").trim();
}

function normalizeVariableType(type) {
  const t = toTrimmedString(type);
  return ["Bool", "Int", "Float", "String"].includes(t) ? t : "String";
}

function normalizeVariable(input = {}) {
  const name = toTrimmedString(input.name);
  const type = normalizeVariableType(input.type);

  const out = { name, type };

  if (type === "Bool") {
    out.boolValue = Boolean(input.boolValue);
  } else if (type === "Int") {
    out.intValue = Number.isFinite(Number(input.intValue))
      ? Math.trunc(Number(input.intValue))
      : 0;
  } else if (type === "Float") {
    out.floatValue = Number.isFinite(Number(input.floatValue))
      ? Number(input.floatValue)
      : 0;
  } else {
    out.stringValue = String(input.stringValue ?? "");
  }

  return out;
}

function normalizePresetSpeaker(input) {
  if (typeof input === "string") {
    const id = toTrimmedString(input);
    return id ? { id } : null;
  }

  const id = toTrimmedString(input?.id);
  const displayName = toTrimmedString(input?.displayName);
  if (!id) return null;
  return displayName ? { id, displayName } : { id };
}

export function normalizeGlobalConfig(input = {}) {
  const rawVariables = safeArray(input.variables);
  const rawSpeakers = safeArray(input.presetSpeakers);

  const variableNameSet = new Set();
  const variables = [];

  for (const item of rawVariables) {
    const variable = normalizeVariable(item);
    if (!variable.name) continue;
    if (variableNameSet.has(variable.name)) continue;
    variableNameSet.add(variable.name);
    variables.push(variable);
  }

  const speakerIdSet = new Set();
  const presetSpeakers = [];

  for (const item of rawSpeakers) {
    const speaker = normalizePresetSpeaker(item);
    if (!speaker?.id) continue;
    if (speakerIdSet.has(speaker.id)) continue;
    speakerIdSet.add(speaker.id);
    presetSpeakers.push(speaker);
  }

  return {
    variables,
    presetSpeakers,
  };
}

export function serializeGlobalConfigToYAML(config = {}) {
  const normalized = normalizeGlobalConfig(config);

  const payload = {
    meta: {
      schemaVersion: GLOBAL_CONFIG_SCHEMA_VERSION,
      configType: "GlobalConfig",
    },
    variables: normalized.variables,
    presetSpeakers: normalized.presetSpeakers,
  };

  return YAML.stringify(payload);
}

export function parseGlobalConfigFromYAML(yamlString) {
  const parsed = YAML.parse(String(yamlString ?? "")) || {};

  const schemaVersion = Number(parsed?.meta?.schemaVersion ?? 1);
  if (!Number.isInteger(schemaVersion) || schemaVersion < 1) {
    throw new Error("全局配置文件 schemaVersion 非法");
  }

  return normalizeGlobalConfig({
    variables: parsed.variables,
    presetSpeakers: parsed.presetSpeakers,
  });
}

export function downloadGlobalConfigYAML(
  config = {},
  fileName = "global-config.narrrail.yaml",
) {
  const yamlString = serializeGlobalConfigToYAML(config);
  const blob = new Blob([yamlString], { type: "text/yaml;charset=utf-8" });
  const url = URL.createObjectURL(blob);

  const a = document.createElement("a");
  a.href = url;
  a.download = fileName;
  a.click();

  URL.revokeObjectURL(url);
}
