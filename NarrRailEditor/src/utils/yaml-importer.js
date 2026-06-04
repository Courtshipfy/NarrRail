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

function normalizeVariableType(type) {
  const text = String(type || "").trim();
  return ["Bool", "Int", "Float", "String"].includes(text) ? text : "String";
}

function readVariableDefaultValue(variable, type) {
  if (variable?.defaultValue != null) {
    return String(variable.defaultValue);
  }

  if (type === "Bool") return variable?.boolValue ? "true" : "false";
  if (type === "Int") return String(variable?.intValue ?? "0");
  if (type === "Float") return String(variable?.floatValue ?? "0");
  return String(variable?.stringValue ?? "");
}

function normalizeVariablesForEditor(variables) {
  return (Array.isArray(variables) ? variables : [])
    .map((variable) => {
      const name = String(variable?.name || "").trim();
      if (!name) return null;

      const type = normalizeVariableType(variable?.type);
      const defaultValue = readVariableDefaultValue(variable, type);
      const normalized = {
        name,
        type,
        scope:
          variable?.scope === "Global" || variable?.bGlobalScope
            ? "Global"
            : "Session",
      };

      if (type === "Bool") {
        normalized.boolValue =
          defaultValue.toLowerCase() === "true" || defaultValue === "1";
      } else if (type === "Int") {
        const value = Number(defaultValue);
        normalized.intValue = Number.isFinite(value) ? Math.trunc(value) : 0;
      } else if (type === "Float") {
        const value = Number(defaultValue);
        normalized.floatValue = Number.isFinite(value) ? value : 0;
      } else {
        normalized.stringValue = defaultValue;
      }

      return normalized;
    })
    .filter(Boolean);
}

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
        condition: normalizeCondition(node.condition),
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
        variableType: firstAction?.variable?.variableType || "String",
        bGlobalScope: Boolean(firstAction?.variable?.bGlobalScope),
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
  const yamlEdges = rejectDeprecatedEdgeConditions(data.edges || []);

  // 处理普通边
  yamlEdges.forEach((edge) => {
    const sourceNode = nodes.find((node) => node.id === edge.sourceNodeId);
    let sourceHandle = edge.sourceHandle || undefined;
    if (sourceNode?.type === "condition") {
      if (sourceHandle === "condition-true") sourceHandle = "condition-0";
      if (sourceHandle === "condition-false") sourceHandle = "condition-fallback";
    }
    edges.push({
      id: `e${edgeIndex++}`,
      source: edge.sourceNodeId,
      sourceHandle,
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
    variables: normalizeVariablesForEditor(data.variables),
    meta: data.meta,
  };
}

function rejectDeprecatedEdgeConditions(edges) {
  return (Array.isArray(edges) ? edges : []).map((edge, index) => {
    if (edge?.condition != null) {
      throw new Error(
        `Edge ${edge?.sourceNodeId || `#${index}`} -> ${edge?.targetNodeId || "(empty)"} uses deprecated edge condition. Use a Condition node with condition-N / condition-fallback outputs.`,
      );
    }
    return edge;
  });
}

function normalizeCondition(condition) {
  if (condition && Array.isArray(condition.branches)) {
    return {
      branches: condition.branches.map((branch, index) => ({
        label: branch?.label || `条件 ${index + 1}`,
        logic: branch?.logic || "All",
        terms: Array.isArray(branch?.terms) ? branch.terms : [],
      })),
    };
  }

  if (condition && Array.isArray(condition.terms)) {
    return {
      branches: [
        {
          label: "条件 1",
          logic: condition.logic || "All",
          terms: condition.terms,
        },
      ],
    };
  }

  return {
    branches: [
      {
        label: "条件 1",
        logic: "All",
        terms: [],
      },
    ],
  };
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
