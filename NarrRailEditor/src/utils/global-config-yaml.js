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

function readDefaultValue(input = {}, type) {
  if (input.defaultValue != null) {
    return String(input.defaultValue);
  }

  if (type === "Bool") {
    return Boolean(input.boolValue) ? "true" : "false";
  }

  if (type === "Int") {
    const value = Number(input.intValue);
    return Number.isFinite(value) ? String(Math.trunc(value)) : "0";
  }

  if (type === "Float") {
    const value = Number(input.floatValue);
    return Number.isFinite(value) ? String(value) : "0";
  }

  return String(input.stringValue ?? "");
}

function normalizeVariable(input = {}) {
  const name = toTrimmedString(input.name);
  const type = normalizeVariableType(input.type);
  const defaultValue = readDefaultValue(input, type);
  const out = { name, type, scope: "Global" };

  if (type === "Bool") {
    out.boolValue =
      defaultValue.toLowerCase() === "true" || defaultValue === "1";
  } else if (type === "Int") {
    const value = Number(defaultValue);
    out.intValue = Number.isFinite(value) ? Math.trunc(value) : 0;
  } else if (type === "Float") {
    const value = Number(defaultValue);
    out.floatValue = Number.isFinite(value) ? value : 0;
  } else {
    out.stringValue = defaultValue;
  }

  return out;
}

function normalizeVariableForYAML(input = {}) {
  const variable = normalizeVariable(input);
  return {
    name: variable.name,
    type: variable.type,
    scope: "Global",
    defaultValue: readDefaultValue(variable, variable.type),
  };
}

function normalizePresetSpeaker(input) {
  if (typeof input === "string") {
    const id = toTrimmedString(input);
    return id ? { id } : null;
  }

  const id = toTrimmedString(input?.speakerId || input?.id);
  const displayName = toTrimmedString(input?.displayName);
  const color = toTrimmedString(input?.color);
  if (!id) return null;
  return {
    id,
    ...(displayName ? { displayName } : {}),
    ...(color ? { color } : {}),
  };
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
    variables: normalized.variables.map(normalizeVariableForYAML),
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
  fileName = "global-config.nrstory",
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
