import YAML from "yaml";

function normalizeMultiDialogueLines(linesInput) {
  const lines = Array.isArray(linesInput)
    ? linesInput.map((line) => {
        if (typeof line === "string") return { textKey: line };
        return { textKey: line?.textKey || "" };
      })
    : [];

  let lastNonEmpty = -1;
  for (let i = 0; i < lines.length; i += 1) {
    if (String(lines[i]?.textKey || "").trim().length > 0) {
      lastNonEmpty = i;
    }
  }

  if (lastNonEmpty < 0) return [{ textKey: "" }];
  return lines.slice(0, lastNonEmpty + 1);
}

const nodeTypeMap = {
  dialogue: "Dialogue",
  multidialogue: "MultiDialogue",
  choice: "Choice",
  jump: "Jump",
  setvariable: "SetVariable",
  emitevent: "EmitEvent",
  condition: "Condition",
  end: "End",
};

function normalizeVariableType(type) {
  const text = String(type || "").trim();
  return ["Bool", "Int", "Float", "String"].includes(text) ? text : "String";
}

function getVariableDefaultValue(variable, type) {
  if (variable?.defaultValue != null) {
    return String(variable.defaultValue);
  }

  if (type === "Bool") {
    return variable?.boolValue ? "true" : "false";
  }

  if (type === "Int") {
    const value = Number(variable?.intValue);
    return Number.isFinite(value) ? String(Math.trunc(value)) : "0";
  }

  if (type === "Float") {
    const value = Number(variable?.floatValue);
    return Number.isFinite(value) ? String(value) : "0";
  }

  return String(variable?.stringValue ?? "");
}

function normalizeVariablesForExport(variables) {
  return (Array.isArray(variables) ? variables : [])
    .map((variable) => {
      const name = String(variable?.name || "").trim();
      if (!name) return null;

      const type = normalizeVariableType(variable?.type);
      return {
        name,
        type,
        scope:
          variable?.scope === "Global" || variable?.bGlobalScope
            ? "Global"
            : "Session",
        defaultValue: getVariableDefaultValue(variable, type),
      };
    })
    .filter(Boolean);
}

function buildVariableLookup(variables) {
  return new Map(
    normalizeVariablesForExport(variables).map((variable) => [
      variable?.name,
      variable,
    ]),
  );
}

function normalizeConditionForExport(condition) {
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

function normalizeChoiceTimerForExport(timer) {
  const enabled = Boolean(timer?.enabled);
  const durationSeconds = Number(timer?.durationSeconds);

  return {
    enabled,
    durationSeconds:
      Number.isFinite(durationSeconds) && durationSeconds > 0
        ? durationSeconds
        : 8,
    timeoutChoiceTextKey: String(
      timer?.timeoutChoiceTextKey || "超时",
    ),
  };
}

function normalizeActionForExport(action) {
  if (!action || typeof action !== "object") return null;

  const normalized = { ...action };
  if (normalized.actionType === "EmitEvent") {
    normalized.eventId =
      normalized.eventId ?? normalized.emitEvent?.eventId ?? "";
    delete normalized.emitEvent;
  }

  return normalized;
}

function normalizeActionListForExport(actions) {
  return (Array.isArray(actions) ? actions : [])
    .map(normalizeActionForExport)
    .filter(Boolean);
}

export function buildYAMLString(nodes, edges, variables, meta) {
  const variableByName = buildVariableLookup(variables);

  // 转换节点
  const yamlNodes = nodes.map((node) => {
    const base = {
      nodeId: node.id,
      nodeType: nodeTypeMap[node.type],
    };

    // 根据类型添加特定字段
    if (node.type === "dialogue") {
      const lines = normalizeMultiDialogueLines(
        Array.isArray(node.data?.lines)
          ? node.data.lines
          : [{ textKey: node.data?.textKey || "" }],
      );
      if (lines.length > 1) {
        base.nodeType = "MultiDialogue";
        base.multiDialogue = {
          speakerId: node.data?.speakerId || "",
          lines,
        };
      } else {
        base.nodeType = "Dialogue";
        base.dialogue = {
          speakerId: node.data?.speakerId || "",
          textKey: lines[0]?.textKey || "",
          speechRate: node.data?.speechRate || 1.0,
          voiceAsset: node.data?.voiceAsset || "",
        };
      }
    } else if (node.type === "multidialogue") {
      base.multiDialogue = {
        speakerId: node.data?.speakerId || "",
        lines: normalizeMultiDialogueLines(node.data?.lines),
      };
    } else if (node.type === "choice") {
      // 从边中找到每个选项对应的目标节点
      const choices = (node.data.choices || []).map((choice, index) => {
        // 查找从这个选项连接点出发的边
        const edge = edges.find(
          (e) => e.source === node.id && e.sourceHandle === `choice-${index}`,
        );

        return {
          textKey: choice.textKey || "",
          targetNodeId: edge ? edge.target : "",
        };
      });

      // 按 NarrRail SCRIPT_FORMAT 规范：Choice 节点直接使用 choices 字段
      base.choices = choices;
      base.choiceMode =
        node.data?.choiceMode === "ExhaustiveUntilComplete"
          ? "ExhaustiveUntilComplete"
          : "SinglePass";

      const completionEdge = edges.find(
        (e) => e.source === node.id && e.sourceHandle === "choice-complete",
      );
      base.choiceCompletionTargetNodeId = completionEdge
        ? completionEdge.target
        : "";
      base.choiceTimer = normalizeChoiceTimerForExport(node.data?.choiceTimer);
    } else if (node.type === "jump") {
      base.jump = {
        targetNodeId: node.data.targetNodeId || "",
      };
    } else if (node.type === "setvariable") {
      const variable = variableByName.get(node.data.variableName);
      base.actions = [
        {
          actionType: node.data.operation || "Set",
          variable: {
            variableName: node.data.variableName || "",
            variableType: variable?.type || node.data.variableType || "String",
            bGlobalScope:
              variable?.scope === "Global" ||
              node.data.scope === "Global" ||
              Boolean(node.data.bGlobalScope),
          },
          value: node.data.value || "",
        },
      ];
    } else if (node.type === "emitevent") {
      base.eventId = node.data.eventId || node.data.emitEvent?.eventId || "";
    } else if (node.type === "condition") {
      base.condition = normalizeConditionForExport(node.data?.condition);
    }

    for (const actionField of ["actions", "enterActions", "exitActions"]) {
      const normalizedActions = normalizeActionListForExport(
        node.data?.[actionField],
      );
      if (normalizedActions.length > 0) {
        base[actionField] = normalizedActions;
      }
    }

    return base;
  });

  // 转换边 - 导出全部边（包括 Choice 分支边）
  const yamlEdges = edges.map((edge) => ({
    sourceNodeId: edge.source,
    sourceHandle: edge.sourceHandle || "",
    targetNodeId: edge.target,
    priority: edge.data?.priority || 0,
  }));

  // 构建完整结构
  const yamlData = {
    meta: {
      schemaVersion: 1,
      storyId: meta.storyId,
      entryNodeId: meta.entryNodeId,
    },
    variables: [],
    nodes: yamlNodes,
    edges: yamlEdges,
  };

  return YAML.stringify(yamlData);
}

export function exportToYAML(nodes, edges, variables, meta) {
  const yamlString = buildYAMLString(nodes, edges, variables, meta);

  // 触发下载
  const blob = new Blob([yamlString], { type: "text/yaml" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = `${meta.storyId}.nrstory`;
  a.click();
  URL.revokeObjectURL(url);
}
