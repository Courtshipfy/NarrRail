import YAML from "yaml";

const reverseNodeTypeMap = {
  Dialogue: "dialogue",
  MultiDialogue: "multidialogue",
  Choice: "choice",
  Jump: "jump",
  SetVariable: "setvariable",
  EmitEvent: "emitevent",
  Condition: "condition",
  End: "end",
};

export function importFromYAML(yamlString) {
  // 解析 YAML
  const data = YAML.parse(yamlString);

  // 兼容 Choice 节点两种格式：
  // 1) 新格式：choices: []
  // 2) 旧格式：choice: { choices: [] }
  const getChoiceArray = (node) => {
    if (Array.isArray(node?.choices)) return node.choices;
    if (node?.choice && Array.isArray(node.choice.choices))
      return node.choice.choices;
    return [];
  };

  // 转换节点
  const nodes = data.nodes.map((node, index) => {
    const base = {
      id: node.nodeId,
      type: reverseNodeTypeMap[node.nodeType],
      position: calculatePosition(index, data.nodes.length),
      data: {},
    };

    // 提取节点数据
    if (node.nodeType === "Choice" || node.choice || node.choices) {
      // Choice 节点：兼容新旧格式，只保存选项文本，不保存 targetNodeId（通过边表示）
      const choiceArray = getChoiceArray(node);
      if (choiceArray.some((choice) => choice?.availability != null)) {
        throw new Error(
          `节点 ${node.nodeId}: choices[].availability 已废弃，请改用 Condition 节点控制条件分支`,
        );
      }
      const choiceMode =
        node.choiceMode === "ExhaustiveUntilComplete"
          ? "ExhaustiveUntilComplete"
          : "SinglePass";
      base.data = {
        choices: choiceArray.map((c) => ({
          textKey: c.textKey,
        })),
        choiceMode,
      };
    } else if (node.nodeType === "MultiDialogue" || node.multiDialogue) {
      const md = node.multiDialogue || {};
      const lines = Array.isArray(md.lines)
        ? md.lines.map((line) => {
            if (typeof line === "string") return { textKey: line };
            return { textKey: line?.textKey || "" };
          })
        : [];
      base.data = {
        speakerId: md.speakerId || "",
        lines: lines.length > 0 ? lines : [{ textKey: "" }],
      };
    } else if (node.nodeType === "Condition" || node.condition) {
      base.data = {
        condition: node.condition || { logic: "All", terms: [] },
      };
    } else if (node.dialogue) {
      base.data = { ...node.dialogue };
    } else if (node.jump) {
      base.data = { ...node.jump };
    } else if (node.actions) {
      const firstAction = Array.isArray(node.actions)
        ? node.actions[0] || {}
        : {};
      base.data = {
        variableName: firstAction?.variable?.variableName || "",
        operation: firstAction?.actionType || "Set",
        value: firstAction?.value || "",
      };
    } else if (node.emitEvent) {
      base.data = { ...node.emitEvent };
    }

    return base;
  });

  // 转换边
  const edges = [];
  let edgeIndex = 0;
  const yamlEdges = migrateLegacyEdgeConditions(data.edges || [], nodes);

  // 处理普通边
  yamlEdges.forEach((edge) => {
    edges.push({
      id: `e${edgeIndex++}`,
      source: edge.sourceNodeId,
      sourceHandle: edge.sourceHandle || undefined,
      target: edge.targetNodeId,
      type: "default",
      animated: false,
      style: "stroke: rgba(168, 85, 247, 0.6); stroke-width: 2px;",
      data: {
        priority: edge.priority,
      },
    });
  });

  // 处理 Choice 节点的选项边（兼容新旧格式）
  // 若 edges 中已存在 source-target 对应边，则优先复用该边并补 sourceHandle，避免重复边
  data.nodes.forEach((node) => {
    const choiceArray = getChoiceArray(node);
    if (choiceArray.length > 0) {
      choiceArray.forEach((choice, index) => {
        if (!choice.targetNodeId) return;

        const existing = edges.find(
          (e) =>
            e.source === node.nodeId &&
            e.target === choice.targetNodeId &&
            (e.sourceHandle == null || e.sourceHandle === ""),
        );

        if (existing) {
          existing.sourceHandle = `choice-${index}`;
          if (!existing.data) {
            existing.data = {
              priority: 0,
            };
          }
          return;
        }

        edges.push({
          id: `e${edgeIndex++}`,
          source: node.nodeId,
          target: choice.targetNodeId,
          sourceHandle: `choice-${index}`,
          type: "default",
          animated: false,
          style: "stroke: rgba(168, 85, 247, 0.6); stroke-width: 2px;",
          data: {
            priority: 0,
          },
        });
      });
    }

    if (
      node.choiceMode === "ExhaustiveUntilComplete" &&
      node.choiceCompletionTargetNodeId
    ) {
      const existingCompletion = edges.find(
        (e) =>
          e.source === node.nodeId &&
          e.sourceHandle === "choice-complete" &&
          e.target === node.choiceCompletionTargetNodeId,
      );

      if (!existingCompletion) {
        const plainCompletionEdge = edges.find(
          (e) =>
            e.source === node.nodeId &&
            (e.sourceHandle == null || e.sourceHandle === "") &&
            e.target === node.choiceCompletionTargetNodeId,
        );

        if (plainCompletionEdge) {
          plainCompletionEdge.sourceHandle = "choice-complete";
          plainCompletionEdge.style =
            "stroke: rgba(124, 58, 237, 0.72); stroke-width: 2px;";
        } else {
          edges.push({
            id: `e${edgeIndex++}`,
            source: node.nodeId,
            sourceHandle: "choice-complete",
            target: node.choiceCompletionTargetNodeId,
            type: "default",
            animated: false,
            style: "stroke: rgba(124, 58, 237, 0.72); stroke-width: 2px;",
            data: {
              priority: 0,
            },
          });
        }
      }
    }
  });

  return {
    nodes,
    edges,
    variables: data.variables,
    meta: data.meta,
  };
}

function migrateLegacyEdgeConditions(edges, nodes) {
  const nodeIds = new Set(nodes.map((node) => node.id));
  const groups = new Map();
  const groupOrder = [];

  edges.forEach((edge, index) => {
    const key = `${edge?.sourceNodeId || ""}\u0000${edge?.sourceHandle || ""}`;
    if (!groups.has(key)) {
      groups.set(key, []);
      groupOrder.push(key);
    }
    groups.get(key).push({ ...edge, __index: index });
  });

  const migrated = [];

  groupOrder.forEach((key) => {
    const group = groups.get(key);
    const hasLegacyCondition = group.some((edge) => edge?.condition != null);
    if (!hasLegacyCondition) {
      migrated.push(...group.map(stripImporterMeta));
      return;
    }

    const hasMeaningfulCondition = group.some((edge) =>
      isMeaningfulCondition(edge?.condition),
    );

    if (!hasMeaningfulCondition) {
      migrated.push(
        ...group.map((edge) => stripImporterMeta(stripLegacyCondition(edge))),
      );
      return;
    }

    const sorted = [...group].sort((a, b) => {
      const priorityDiff = (a.priority ?? 0) - (b.priority ?? 0);
      return priorityDiff !== 0 ? priorityDiff : a.__index - b.__index;
    });

    const first = sorted[0];
    let pendingSourceNodeId = first.sourceNodeId;
    let pendingSourceHandle = first.sourceHandle || "";
    let fallbackCreated = false;

    sorted.forEach((edge, conditionIndex) => {
      if (fallbackCreated) return;

      if (!isMeaningfulCondition(edge?.condition)) {
        migrated.push({
          sourceNodeId: pendingSourceNodeId,
          sourceHandle: pendingSourceHandle,
          targetNodeId: edge.targetNodeId,
          priority: pendingSourceNodeId === first.sourceNodeId ? (edge.priority ?? 0) : 0,
        });
        fallbackCreated = true;
        return;
      }

      const conditionNodeId = uniqueNodeId(
        `legacy-condition-${sanitizeId(edge.sourceNodeId)}-${conditionIndex}`,
        nodeIds,
      );
      const sourceNode = nodes.find((node) => node.id === edge.sourceNodeId);
      nodes.push({
        id: conditionNodeId,
        type: "condition",
        position: {
          x: (sourceNode?.position?.x || 0) + 220,
          y: (sourceNode?.position?.y || 0) + conditionIndex * 150,
        },
        data: {
          condition: edge.condition,
        },
      });

      migrated.push({
        sourceNodeId: pendingSourceNodeId,
        sourceHandle: pendingSourceHandle,
        targetNodeId: conditionNodeId,
        priority: pendingSourceNodeId === first.sourceNodeId ? (edge.priority ?? 0) : 0,
      });
      migrated.push({
        sourceNodeId: conditionNodeId,
        sourceHandle: "condition-true",
        targetNodeId: edge.targetNodeId,
        priority: 0,
      });

      pendingSourceNodeId = conditionNodeId;
      pendingSourceHandle = "condition-false";
    });

    if (!fallbackCreated) {
      const endNodeId = uniqueNodeId(
        `legacy-no-match-${sanitizeId(first.sourceNodeId)}`,
        nodeIds,
      );
      const sourceNode = nodes.find((node) => node.id === first.sourceNodeId);
      nodes.push({
        id: endNodeId,
        type: "end",
        position: {
          x: (sourceNode?.position?.x || 0) + 440,
          y: (sourceNode?.position?.y || 0) + sorted.length * 150,
        },
        data: {},
      });
      migrated.push({
        sourceNodeId: pendingSourceNodeId,
        sourceHandle: pendingSourceHandle,
        targetNodeId: endNodeId,
        priority: 0,
      });
    }
  });

  return migrated;
}

function isMeaningfulCondition(condition) {
  if (condition == null) return false;
  return !Array.isArray(condition.terms) || condition.terms.length > 0;
}

function stripLegacyCondition(edge) {
  const { condition, ...rest } = edge;
  return rest;
}

function stripImporterMeta(edge) {
  const { __index, ...rest } = edge;
  return rest;
}

function sanitizeId(value) {
  return String(value || "edge")
    .replace(/[^a-zA-Z0-9_-]/g, "-")
    .replace(/-+/g, "-")
    .replace(/^-|-$/g, "")
    .slice(0, 48);
}

function uniqueNodeId(baseId, nodeIds) {
  let candidate = baseId || "legacy-condition";
  let suffix = 1;
  while (nodeIds.has(candidate)) {
    candidate = `${baseId}-${suffix++}`;
  }
  nodeIds.add(candidate);
  return candidate;
}

// 简单的自动布局算法（网格布局）
function calculatePosition(index, total) {
  const cols = Math.ceil(Math.sqrt(total));
  const row = Math.floor(index / cols);
  const col = index % cols;
  return {
    x: col * 250 + 50,
    y: row * 150 + 50,
  };
}
