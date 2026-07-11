function toText(value) {
  return String(value ?? "").trim();
}

function toNodeType(node) {
  return toText(node?.type).toLowerCase();
}

function getNodeLines(node) {
  const lines = Array.isArray(node?.data?.lines)
    ? node.data.lines
    : [{ textKey: node?.data?.textKey || "" }];

  return lines
    .map((line) => toText(typeof line === "string" ? line : line?.textKey))
    .filter(Boolean);
}

function getOutgoingEdges(edges, nodeId, sourceHandle = null) {
  return (Array.isArray(edges) ? edges : [])
    .filter((edge) => {
      if (edge?.source !== nodeId) return false;
      if (sourceHandle == null) return true;
      return toText(edge?.sourceHandle) === sourceHandle;
    })
    .sort((a, b) => {
      const pa = Number(a?.data?.priority ?? 0);
      const pb = Number(b?.data?.priority ?? 0);
      if (pa !== pb) return pa - pb;
      return toText(a?.id).localeCompare(toText(b?.id));
    });
}

function firstTarget(edges, nodeId, sourceHandle = null) {
  return toText(getOutgoingEdges(edges, nodeId, sourceHandle)[0]?.target);
}

function groupReviewItemsByNode(reviewItems) {
  return (Array.isArray(reviewItems) ? reviewItems : []).reduce((grouped, item) => {
    const nodeId = toText(item?.target?.nodeId || item?.nodeId);
    if (!nodeId) return grouped;
    grouped[nodeId] = grouped[nodeId] || [];
    grouped[nodeId].push(item);
    return grouped;
  }, {});
}

function makeBaseRow({ id, kind, nodeId, indent = 0, reviewItemsByNode }) {
  return {
    id,
    kind,
    nodeId,
    indent,
    reviewItems: reviewItemsByNode[nodeId] || [],
  };
}

function describeConditionTerms(branch) {
  const terms = Array.isArray(branch?.terms) ? branch.terms : [];
  if (terms.length === 0) return "";
  return terms
    .map((term) => {
      const name = toText(term?.variable?.name || term?.variableName);
      const operator = toText(term?.operator || "==");
      const value = toText(term?.compareValue);
      return [name, operator, value].filter(Boolean).join(" ");
    })
    .filter(Boolean)
    .join(` ${toText(branch?.logic || "All")} `);
}

function appendDialogueRows(rows, node, reviewItemsByNode) {
  const speaker = toText(node?.data?.speakerId);
  const lines = getNodeLines(node);
  const normalizedLines = lines.length > 0 ? lines : [""];

  normalizedLines.forEach((line, lineIndex) => {
    rows.push({
      ...makeBaseRow({
        id: `${node.id}:dialogue:${lineIndex}`,
        kind: speaker ? "dialogue" : "narration",
        nodeId: node.id,
        reviewItemsByNode,
      }),
      lineIndex,
      speaker,
      text: line,
    });
  });
}

function appendChoiceRows(rows, node, edges, reviewItemsByNode) {
  const choices = Array.isArray(node?.data?.choices) ? node.data.choices : [];
  const choiceMode =
    node?.data?.choiceMode === "ExhaustiveUntilComplete"
      ? "ExhaustiveUntilComplete"
      : "SinglePass";

  rows.push({
    ...makeBaseRow({
      id: `${node.id}:choice`,
      kind: "choice",
      nodeId: node.id,
      reviewItemsByNode,
    }),
    choiceMode,
    text: choiceMode === "ExhaustiveUntilComplete" ? "穷举选择" : "选择",
  });

  choices.forEach((choice, index) => {
    const handle = `choice-${index}`;
    rows.push({
      ...makeBaseRow({
        id: `${node.id}:choice-option:${index}`,
        kind: "choice-option",
        nodeId: node.id,
        indent: 1,
        reviewItemsByNode,
      }),
      handle,
      text: toText(choice?.textKey),
      targetNodeId: firstTarget(edges, node.id, handle),
    });
  });

  if (choiceMode === "ExhaustiveUntilComplete") {
    rows.push({
      ...makeBaseRow({
        id: `${node.id}:choice-complete`,
        kind: "choice-complete",
        nodeId: node.id,
        indent: 1,
        reviewItemsByNode,
      }),
      handle: "choice-complete",
      text: "全部选项完成后",
      targetNodeId: firstTarget(edges, node.id, "choice-complete"),
    });
  }
}

function appendConditionRows(rows, node, edges, reviewItemsByNode) {
  const branches = Array.isArray(node?.data?.condition?.branches)
    ? node.data.condition.branches
    : [];

  rows.push({
    ...makeBaseRow({
      id: `${node.id}:condition`,
      kind: "condition",
      nodeId: node.id,
      reviewItemsByNode,
    }),
    text: "条件",
  });

  branches.forEach((branch, index) => {
    const handle = `condition-${index}`;
    rows.push({
      ...makeBaseRow({
        id: `${node.id}:condition-branch:${index}`,
        kind: "condition-branch",
        nodeId: node.id,
        indent: 1,
        reviewItemsByNode,
      }),
      handle,
      label: toText(branch?.label) || `条件 ${index + 1}`,
      logic: toText(branch?.logic || "All"),
      text: describeConditionTerms(branch),
      targetNodeId: firstTarget(edges, node.id, handle),
    });
  });

  rows.push({
    ...makeBaseRow({
      id: `${node.id}:condition-fallback`,
      kind: "condition-fallback",
      nodeId: node.id,
      indent: 1,
      reviewItemsByNode,
    }),
    handle: "condition-fallback",
    label: "否则",
    text: "否则",
    targetNodeId: firstTarget(edges, node.id, "condition-fallback"),
  });
}

export function createStructuredScriptRows(input = {}) {
  const nodes = Array.isArray(input.nodes) ? input.nodes : [];
  const edges = Array.isArray(input.edges) ? input.edges : [];
  const reviewItemsByNode = groupReviewItemsByNode(input.reviewItems);
  const rows = [];

  for (const node of nodes) {
    const nodeId = toText(node?.id);
    if (!nodeId) continue;
    const type = toNodeType(node);

    if (type === "dialogue" || type === "multidialogue") {
      appendDialogueRows(rows, node, reviewItemsByNode);
    } else if (type === "choice") {
      appendChoiceRows(rows, node, edges, reviewItemsByNode);
    } else if (type === "condition") {
      appendConditionRows(rows, node, edges, reviewItemsByNode);
    } else if (type === "setvariable") {
      const variableName = toText(node?.data?.variableName);
      const operation = toText(node?.data?.operation || "Set");
      const value = toText(node?.data?.value);
      rows.push({
        ...makeBaseRow({
          id: `${nodeId}:variable-operation`,
          kind: "variable-operation",
          nodeId,
          reviewItemsByNode,
        }),
        variableName,
        operation,
        value,
        text: [variableName, operation, value].filter(Boolean).join(" "),
      });
    } else if (type === "emitevent") {
      rows.push({
        ...makeBaseRow({
          id: `${nodeId}:event`,
          kind: "event",
          nodeId,
          reviewItemsByNode,
        }),
        eventType: toText(node?.data?.eventType),
        params: node?.data?.params || {},
        text: toText(node?.data?.eventType) || "(未命名事件)",
      });
    } else if (type === "jump") {
      rows.push({
        ...makeBaseRow({
          id: `${nodeId}:jump`,
          kind: "jump",
          nodeId,
          reviewItemsByNode,
        }),
        targetNodeId: toText(node?.data?.targetNodeId),
        text: `跳转到 ${toText(node?.data?.targetNodeId)}`,
      });
    } else if (type === "end") {
      rows.push({
        ...makeBaseRow({
          id: `${nodeId}:end`,
          kind: "end",
          nodeId,
          reviewItemsByNode,
        }),
        text: "结束",
      });
    } else {
      rows.push({
        ...makeBaseRow({
          id: `${nodeId}:unknown`,
          kind: "unknown",
          nodeId,
          reviewItemsByNode,
        }),
        text: type || "unknown",
      });
    }
  }

  return rows;
}
