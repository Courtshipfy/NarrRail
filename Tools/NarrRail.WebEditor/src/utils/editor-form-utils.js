function toTrimmedText(value) {
  return String(value ?? "").trim();
}

export function validateVariableName(variables, name, messages = {}) {
  const requiredMessage = messages.required || "请输入变量名";
  const duplicateMessage = messages.duplicate || "变量名已存在";
  const normalizedName = toTrimmedText(name);

  if (!normalizedName) {
    return { ok: false, value: "", message: requiredMessage };
  }

  const exists = (Array.isArray(variables) ? variables : []).some(
    (variable) => toTrimmedText(variable?.name) === normalizedName,
  );

  if (exists) {
    return { ok: false, value: normalizedName, message: duplicateMessage };
  }

  return { ok: true, value: normalizedName, message: "" };
}

export function validateSpeakerId(speakers, speakerId, messages = {}) {
  const requiredMessage = messages.required || "请输入角色 ID";
  const duplicateMessage = messages.duplicate || "该角色 ID 已存在";
  const normalizedId = toTrimmedText(speakerId);

  if (!normalizedId) {
    return { ok: false, value: "", message: requiredMessage };
  }

  const exists = (Array.isArray(speakers) ? speakers : []).some((speaker) =>
    toTrimmedText(typeof speaker === "string" ? speaker : speaker?.id) ===
    normalizedId,
  );

  if (exists) {
    return { ok: false, value: normalizedId, message: duplicateMessage };
  }

  return { ok: true, value: normalizedId, message: "" };
}

export function formatSpeakerLabel(speaker) {
  if (typeof speaker === "string") return speaker;

  const id = toTrimmedText(speaker?.id);
  const displayName = toTrimmedText(speaker?.displayName);

  if (displayName && displayName !== id) {
    return `${displayName} (${id})`;
  }

  return id || "未命名角色";
}
