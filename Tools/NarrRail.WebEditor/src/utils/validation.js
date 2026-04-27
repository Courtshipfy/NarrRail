const VALID_LOGICS = new Set(["All", "Any", "Not"]);
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

  // 3) Edge references + condition schema
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

    validateEdgeCondition(edge, edgeLabel, addError, addWarning);
  }

  // 4) Required fields per node type + choice-edge mapping
  for (const node of safeNodes) {
    if (!node?.id) continue;

    const nodeType = node.type;
    const data = node.data || {};

    if (nodeType === "dialogue") {
      if (!data.speakerId) {
        addError(`节点 ${node.id}: 缺少说话人 ID`, { nodeId: node.id });
      }
      if (!data.textKey) {
        addError(`节点 ${node.id}: 缺少对话文本`, { nodeId: node.id });
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
    } else {
      // 非 choice 节点不应使用 choice-* 句柄
      const badEdges = safeEdges.filter(
        (e) =>
          e?.source === node.id &&
          typeof e?.sourceHandle === "string" &&
          e.sourceHandle.startsWith("choice-"),
      );
      for (const bad of badEdges) {
        addError(
          `边 ${bad.id || "(无ID)"}: 非 Choice 节点 ${node.id} 不应使用 sourceHandle=${bad.sourceHandle}`,
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

  // 每个 choice 文本必填 + 每个 index 必须有且只有一条对应边
  for (let i = 0; i < choices.length; i++) {
    const c = choices[i] || {};
    if (!c.textKey || !String(c.textKey).trim()) {
      addError(`节点 ${node.id}: 选项 #${i + 1} 缺少 textKey`, {
        nodeId: node.id,
      });
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

function validateEdgeCondition(edge, edgeLabel, addError, addWarning) {
  const data = edge?.data ?? {};
  const condition = data.condition;

  if (data.priority != null && !Number.isInteger(data.priority)) {
    addWarning(`边 ${edgeLabel}: priority 建议为整数，当前值=${data.priority}`);
  }

  // condition 可省略；省略则视为恒真
  if (condition == null) return;

  if (typeof condition !== "object" || Array.isArray(condition)) {
    addError(`边 ${edgeLabel}: condition 必须是对象`);
    return;
  }

  const logic = condition.logic ?? "All";
  if (!VALID_LOGICS.has(logic)) {
    addError(
      `边 ${edgeLabel}: condition.logic 非法（允许: All/Any/Not），当前=${logic}`,
    );
  }

  if (!Array.isArray(condition.terms)) {
    addError(`边 ${edgeLabel}: condition.terms 必须是数组`);
    return;
  }

  if (logic === "Not" && condition.terms.length !== 1) {
    addWarning(`边 ${edgeLabel}: logic=Not 时建议且通常应只有 1 个 term`);
  }

  condition.terms.forEach((term, idx) => {
    if (!term || typeof term !== "object" || Array.isArray(term)) {
      addError(`边 ${edgeLabel}: term[${idx}] 必须是对象`);
      return;
    }

    const variable = term.variable;
    const operator = term.operator;
    const compareValue = term.compareValue;

    if (!variable || typeof variable !== "object" || Array.isArray(variable)) {
      addError(`边 ${edgeLabel}: term[${idx}].variable 必须是对象`);
    } else {
      if (!variable.name || !String(variable.name).trim()) {
        addError(`边 ${edgeLabel}: term[${idx}].variable.name 不能为空`);
      }
      if (!VALID_VAR_TYPES.has(variable.type)) {
        addError(
          `边 ${edgeLabel}: term[${idx}].variable.type 非法（允许: Bool/Int/Float/String），当前=${variable.type}`,
        );
      }
      if (variable.scope != null && !VALID_SCOPES.has(variable.scope)) {
        addError(
          `边 ${edgeLabel}: term[${idx}].variable.scope 非法（允许: Session/Global），当前=${variable.scope}`,
        );
      }
    }

    if (!VALID_OPERATORS.has(operator)) {
      addError(
        `边 ${edgeLabel}: term[${idx}].operator 非法（允许: == != > >= < <=），当前=${operator}`,
      );
    }

    if (compareValue == null) {
      addWarning(`边 ${edgeLabel}: term[${idx}].compareValue 为空`);
    }
  });
}
