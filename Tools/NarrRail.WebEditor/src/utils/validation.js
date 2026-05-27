const VALID_LOGICS = new Set(["All", "Any"]);
const VALID_OPERATORS = new Set(["==", "!=", ">", ">=", "<", "<="]);
const VALID_VAR_TYPES = new Set(["Bool", "Int", "Float", "String"]);
const VALID_SCOPES = new Set(["Session", "Global"]);

export function validateStory(nodes, edges, meta) {
  const errors = [];
  const warnings = [];

  const safeNodes = Array.isArray(nodes) ? nodes : [];
  const safeEdges = Array.isArray(edges) ? edges : [];
  const safeMeta = meta || {};

  const nodeMap = new Map();
  const nodeIds = [];
  const edgeIdSet = new Set();

  const addError = (message, extra = {}) =>
    errors.push({ type: "error", message, ...extra });

  const addWarning = (message, extra = {}) =>
    warnings.push({ type: "warning", message, ...extra });

  // 1) Node id uniqueness + map
  for (const node of safeNodes) {
    const nodeId = node?.id;
    if (!nodeId || typeof nodeId !== "string" || !nodeId.trim()) {
      addError("存在空节点 ID");
      continue;
    }

    nodeIds.push(nodeId);
    if (nodeMap.has(nodeId)) {
      addError(`重复的节点 ID: ${nodeId}`, { nodeId });
    } else {
      nodeMap.set(nodeId, node);
    }
  }

  // 2) Entry node check
  if (!safeMeta.entryNodeId) {
    addError("未设置入口节点");
  } else if (!nodeMap.has(safeMeta.entryNodeId)) {
    addError(`入口节点不存在: ${safeMeta.entryNodeId}`);
  }

  // 3) Edge references
  for (let i = 0; i < safeEdges.length; i++) {
    const edge = safeEdges[i];
    const edgeLabel = edge?.id || `#${i}`;

    if (!edge?.id) {
      addWarning(`边 ${edgeLabel} 缺少 edge.id，建议补充稳定 ID`);
    } else if (edgeIdSet.has(edge.id)) {
      addError(`重复的边 ID: ${edge.id}`, { edgeId: edge.id });
    } else {
      edgeIdSet.add(edge.id);
    }

    if (!edge?.source || !nodeMap.has(edge.source)) {
      addError(
        `边 ${edgeLabel} 引用了不存在的源节点: ${edge?.source ?? "(空)"}`,
      );
    }
    if (!edge?.target || !nodeMap.has(edge.target)) {
      addError(
        `边 ${edgeLabel} 引用了不存在的目标节点: ${edge?.target ?? "(空)"}`,
      );
    }

    if (edge?.data?.priority != null && !Number.isInteger(edge.data.priority)) {
      addWarning(
        `边 ${edgeLabel}: priority 建议为整数，当前值=${edge.data.priority}`,
      );
    }

    if (edge?.data?.condition != null || edge?.condition != null) {
      addError(
        `边 ${edgeLabel}: 边条件已废弃，请改用 Condition 节点进行条件分支`,
      );
    }
  }

  // 4) Required fields per node type + choice-edge mapping
  for (const node of safeNodes) {
    if (!node?.id) continue;

    const nodeType = node.type;
    const data = node.data || {};

    if (nodeType === "dialogue") {
      if (!data.speakerId) {
        addError(`节点 ${node.id}: 缺少角色 ID`, { nodeId: node.id });
      }
      if (!data.textKey) {
        addError(`节点 ${node.id}: 缺少对话文本`, { nodeId: node.id });
      }
    }

    if (nodeType === "multidialogue") {
      if (!Array.isArray(data.lines) || data.lines.length === 0) {
        addError(`节点 ${node.id}: 多行对话至少需要一行文本`, {
          nodeId: node.id,
        });
      } else {
        data.lines.forEach((line, index) => {
          const text = typeof line === "string" ? line : line?.textKey;
          if (!String(text || "").trim()) {
            addWarning(`节点 ${node.id}: 第 ${index + 1} 行文本为空`, {
              nodeId: node.id,
            });
          }
        });
      }
    }

    if (nodeType === "jump") {
      if (!data.targetNodeId) {
        addError(`节点 ${node.id}: 缺少跳转目标`, { nodeId: node.id });
      } else if (!nodeMap.has(data.targetNodeId)) {
        addError(`节点 ${node.id}: 跳转目标不存在: ${data.targetNodeId}`, {
          nodeId: node.id,
        });
      }
    }

    if (nodeType === "choice") {
      validateChoiceNodeEdgeMapping(
        node,
        safeEdges,
        nodeMap,
        addError,
        addWarning,
      );
    } else if (nodeType === "condition") {
      validateConditionNodeEdgeMapping(
        node,
        safeEdges,
        nodeMap,
        addError,
        addWarning,
      );
    } else {
      // 非 choice/condition 节点不应使用专用句柄
      const badEdges = safeEdges.filter(
        (e) =>
          e?.source === node.id &&
          typeof e?.sourceHandle === "string" &&
          (e.sourceHandle.startsWith("choice-") ||
            e.sourceHandle.startsWith("condition-")),
      );
      for (const bad of badEdges) {
        addError(
          `边 ${bad.id || "(无ID)"}: 节点 ${node.id} 不应使用 sourceHandle=${bad.sourceHandle}`,
        );
      }
    }
  }

  // 5) Isolated nodes
  const connected = new Set();
  for (const edge of safeEdges) {
    if (edge?.source) connected.add(edge.source);
    if (edge?.target) connected.add(edge.target);
  }
  for (const node of safeNodes) {
    if (!node?.id) continue;
    if (!connected.has(node.id) && node.id !== safeMeta.entryNodeId) {
      addWarning(`孤立节点: ${node.id}`, { nodeId: node.id });
    }
  }

  return { errors, warnings };
}

function validateChoiceNodeEdgeMapping(
  node,
  edges,
  nodeMap,
  addError,
  addWarning,
) {
  const choices = Array.isArray(node?.data?.choices) ? node.data.choices : [];
  const sourceEdges = edges.filter((e) => e?.source === node.id);

  if (choices.length === 0) {
    addError(`节点 ${node.id}: 选择节点至少需要一个选项`, { nodeId: node.id });
    return;
  }

  const choiceMode =
    node?.data?.choiceMode === "ExhaustiveUntilComplete"
      ? "ExhaustiveUntilComplete"
      : "SinglePass";
  if (choiceMode === "ExhaustiveUntilComplete") {
    const completionEdges = sourceEdges.filter(
      (e) => e?.sourceHandle === "choice-complete",
    );

    if (completionEdges.length === 0) {
      addError(`节点 ${node.id}: 穷举选择模式缺少完成连线（choice-complete）`, {
        nodeId: node.id,
      });
    } else if (completionEdges.length > 1) {
      addError(`节点 ${node.id}: 穷举选择模式只允许一条完成连线`, {
        nodeId: node.id,
      });
    }

    for (const edge of completionEdges) {
      if (!nodeMap.has(edge.target)) {
        addError(
          `节点 ${node.id}: 穷举选择完成目标不存在: ${edge.target || "(空)"}`,
          { nodeId: node.id },
        );
      }
    }
  }

  // 每个 choice 文本必填 + 每个 index 必须有且只有一条对应边
  for (let i = 0; i < choices.length; i++) {
    const c = choices[i] || {};
    if (!c.textKey || !String(c.textKey).trim()) {
      addError(`节点 ${node.id}: 选项 #${i + 1} 缺少 textKey`, {
        nodeId: node.id,
      });
    }

    if (c.availability != null || c.condition != null) {
      addError(
        `节点 ${node.id}: 选项 #${i + 1} 的条件字段已废弃，请改用 Condition 节点`,
        { nodeId: node.id },
      );
    }

    const expectedHandle = `choice-${i}`;
    const mappedEdges = sourceEdges.filter(
      (e) => e?.sourceHandle === expectedHandle,
    );

    if (mappedEdges.length === 0) {
      addError(
        `节点 ${node.id}: 选项 #${i + 1} 未连接目标边（缺少 sourceHandle=${expectedHandle}）`,
        { nodeId: node.id },
      );
      continue;
    }

    if (mappedEdges.length > 1) {
      addError(
        `节点 ${node.id}: 选项 #${i + 1} 存在多条边（sourceHandle=${expectedHandle}）`,
        { nodeId: node.id },
      );
    }

    for (const e of mappedEdges) {
      if (!nodeMap.has(e.target)) {
        addError(`节点 ${node.id}: 选项 #${i + 1} 连接到了不存在的目标节点`, {
          nodeId: node.id,
        });
      }
    }
  }

  // 检查多余/非法 sourceHandle
  for (const edge of sourceEdges) {
    const sh = edge?.sourceHandle;
    if (!sh) {
      addWarning(
        `节点 ${node.id}: 存在未绑定选项句柄的边 ${edge.id || "(无ID)"}`,
      );
      continue;
    }

    if (sh === "choice-complete") {
      if (choiceMode !== "ExhaustiveUntilComplete") {
        addWarning(
          `节点 ${node.id}: 非穷举模式下不建议存在 choice-complete 连线`,
        );
      }
      continue;
    }

    if (!sh.startsWith("choice-")) {
      addWarning(
        `节点 ${node.id}: 边 ${edge.id || "(无ID)"} 使用了非标准 sourceHandle=${sh}`,
      );
      continue;
    }

    const idx = Number(sh.replace("choice-", ""));
    if (!Number.isInteger(idx) || idx < 0 || idx >= choices.length) {
      addError(
        `节点 ${node.id}: 边 ${edge.id || "(无ID)"} 的 sourceHandle=${sh} 越界（当前选项数 ${choices.length}）`,
      );
    }
  }
}

function validateConditionNodeEdgeMapping(
  node,
  edges,
  nodeMap,
  addError,
  addWarning,
) {
  const sourceEdges = edges.filter((e) => e?.source === node.id);
  const trueEdges = sourceEdges.filter(
    (e) => e?.sourceHandle === "condition-true",
  );
  const falseEdges = sourceEdges.filter(
    (e) => e?.sourceHandle === "condition-false",
  );

  if (trueEdges.length !== 1) {
    addError(`节点 ${node.id}: condition-true 出边必须且只能有 1 条`, {
      nodeId: node.id,
    });
  }

  if (falseEdges.length !== 1) {
    addError(`节点 ${node.id}: condition-false 出边必须且只能有 1 条`, {
      nodeId: node.id,
    });
  }

  for (const edge of [...trueEdges, ...falseEdges]) {
    if (!nodeMap.has(edge.target)) {
      addError(`节点 ${node.id}: 分支目标不存在: ${edge.target || "(空)"}`, {
        nodeId: node.id,
      });
    }
  }

  for (const edge of sourceEdges) {
    const sh = String(edge?.sourceHandle || "");
    if (sh !== "condition-true" && sh !== "condition-false") {
      addError(
        `节点 ${node.id}: 非法 sourceHandle=${sh || "(空)"}，仅允许 condition-true / condition-false`,
        { nodeId: node.id },
      );
    }
  }

  const condition = node?.data?.condition;
  if (!condition || typeof condition !== "object" || Array.isArray(condition)) {
    addError(`节点 ${node.id}: condition 节点缺少 data.condition`, {
      nodeId: node.id,
    });
    return;
  }

  const logic = condition.logic ?? "All";
  if (!VALID_LOGICS.has(logic)) {
    addError(`节点 ${node.id}: condition.logic 非法（允许: All/Any）`, {
      nodeId: node.id,
    });
  }

  const terms = Array.isArray(condition.terms) ? condition.terms : null;
  if (!terms) {
    addError(`节点 ${node.id}: condition.terms 必须是数组`, {
      nodeId: node.id,
    });
    return;
  }

  if (terms.length === 0) {
    addWarning(`节点 ${node.id}: condition.terms 为空，条件将恒真`, {
      nodeId: node.id,
    });
  }
}
