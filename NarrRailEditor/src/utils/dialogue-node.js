export function normalizeDialogueLines(linesInput, fallbackText = "") {
  const sourceLines = Array.isArray(linesInput)
    ? linesInput
    : fallbackText != null
      ? [{ textKey: fallbackText }]
      : [];

  const lines = sourceLines.map((line) => {
    if (typeof line === "string") return { textKey: line };
    return { textKey: String(line?.textKey ?? "") };
  });

  return lines.length > 0 ? lines : [{ textKey: "" }];
}

export function normalizeEditorDialogueNode(node) {
  if (!node || typeof node !== "object") return node;
  if (node.type !== "dialogue" && node.type !== "multidialogue") return node;

  const data = node.data || {};
  const fallbackText = data.textKey ?? "";
  return {
    ...node,
    type: "dialogue",
    data: {
      speakerId: data.speakerId || "",
      lines: normalizeDialogueLines(data.lines, fallbackText),
      speechRate: data.speechRate ?? 1.0,
      voiceAsset: data.voiceAsset || "",
    },
  };
}

export function normalizeEditorStoryNodes(nodes) {
  return (Array.isArray(nodes) ? nodes : []).map((node) =>
    normalizeEditorDialogueNode(node),
  );
}

export function getDialogueLineTexts(data) {
  return normalizeDialogueLines(data?.lines, data?.textKey).map((line) =>
    String(line?.textKey || ""),
  );
}
