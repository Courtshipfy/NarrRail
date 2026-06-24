const VALID_LOGICS = new Set(["All", "Any"]);
const VALID_OPERATORS = new Set(["==", "!=", ">", ">=", "<", "<="]);
const VALID_VAR_TYPES = new Set(["Bool", "Int", "Float", "String"]);
const VALID_SCOPES = new Set(["Session", "Global"]);

export function validateStory(nodes, edges, meta, variables = []) {
  const errors = [];
  const warnings = [];

  const safeNodes = Array.isArray(nodes) ? nodes : [];
  const safeEdges = Array.isArray(edges) ? edges : [];
  const safeMeta = meta || {};
  const variableMap = buildVariableMap(variables);

  const nodeMap = new Map();
  const edgeIdSet = new Set();

  const addError = (message, extra = {}) =>
    errors.push({ type: "error", message, ...extra });
  const addWarning = (message, extra = {}) =>
    warnings.push({ type: "warning", message, ...extra });

  for (const node of safeNodes) {
    const nodeId = node?.id;
    if (!nodeId || typeof nodeId !== "string" || !nodeId.trim()) {
      addError("存在空节点 ID");
      continue;
    }

    if (nodeMap.has(nodeId)) {
      addError(`重复的节点 ID: ${nodeId}`, { nodeId });
    } else {
      nodeMap.set(nodeId, node);
    }
  }

  if (!safeMeta.entryNodeId) {
    addError("未设置入口节点");
  } else if (!nodeMap.has(safeMeta.entryNodeId)) {
    addError(`入口节点不存在: ${safeMeta.entryNodeId}`);
  }

  for (let i = 0; i < safeEdges.length; i += 1) {
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
      addError(`边 ${edgeLabel} 引用了不存在的源节点: ${edge?.source ?? "(空)"}`);
    }
    if (!edge?.target || !nodeMap.has(edge.target)) {
      addError(`边 ${edgeLabel} 引用了不存在的目标节点: ${edge?.target ?? "(空)"}`);
    }

    if (edge?.data?.priority != null && !Number.isInteger(edge.data.priority)) {
      addWarning(`边 ${edgeLabel}: priority 建议为整数，当前值 ${edge.data.priority}`);
    }

    if (edge?.data?.condition != null || edge?.condition != null) {
      addError(`边 ${edgeLabel}: 边条件已废弃，请改用 Condition 节点进行条件分支`);
    }
  }

  for (const node of safeNodes) {
    if (!node?.id) continue;

    const nodeType = node.type;
    const data = node.data || {};

    if (nodeType === "dialogue" || nodeType === "multidialogue") {
      const lines = Array.isArray(data.lines)
        ? data.lines
        : [{ textKey: data.textKey || "" }];
      if (lines.length === 0) {
        addError(`节点 ${node.id}: 对话至少需要一行文本`, {
          nodeId: node.id,
        });
      } else {
        lines.forEach((line, index) => {
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
        variableMap,
        addError,
        addWarning,
      );
    } else if (nodeType === "setvariable") {
      validateSetVariableNode(node, variableMap, addError);
    } else if (nodeType === "emitevent") {
      validateEmitEventPayload(`节点 ${node.id}: EmitEvent`, data, (message) =>
        addError(message, { nodeId: node.id }),
      );
    } else {
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

    validateEventActions(node, addError);
  }

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

function isPlainObject(value) {
  return value != null && typeof value === "object" && !Array.isArray(value);
}

function readEventFields(payload) {
  return {
    eventType: String(payload?.eventType ?? "").trim(),
    params: payload?.params,
  };
}

function validateEmitEventPayload(ownerLabel, payload, addError) {
  const event = readEventFields(payload || {});
  if (!event.eventType) {
    addError(`${ownerLabel}: eventType 不能为空`);
  }

  if (event.params != null && !isPlainObject(event.params)) {
    addError(`${ownerLabel}: params 必须是 object/dictionary`);
  }
}

function validateEventActions(node, addError) {
  const data = node?.data || {};
  for (const actionField of ["actions", "enterActions", "exitActions"]) {
    const actions = Array.isArray(data[actionField]) ? data[actionField] : [];
    actions.forEach((action, index) => {
      if (action?.actionType !== "EmitEvent") return;
      validateEmitEventPayload(
        `节点 ${node.id}: ${actionField}[${index}] EmitEvent`,
        action,
        (message) => addError(message, { nodeId: node.id }),
      );
    });
  }
}

function buildVariableMap(variables) {
  const map = new Map();
  for (const variable of Array.isArray(variables) ? variables : []) {
    const name = String(variable?.name || variable?.variableName || "").trim();
    if (!name) continue;
    map.set(name, {
      name,
      type: String(variable?.type || variable?.variableType || "String"),
      scope:
        variable?.scope === "Global" || variable?.bGlobalScope === true
          ? "Global"
          : "Session",
    });
  }
  return map;
}

function validateVariableRef(
  ownerLabel,
  variableRef,
  variableMap,
  addError,
  options = {},
) {
  const variableName = String(
    variableRef?.name || variableRef?.variableName || options.variableName || "",
  ).trim();
  if (!variableName) {
    addError(`${ownerLabel}: 缺少变量`);
    return;
  }

  const variable = variableMap.get(variableName);
  if (!variable) {
    addError(`${ownerLabel}: 引用了未在 GlobalConfig 中定义的变量 ${variableName}`);
    return;
  }

  const declaredType = String(
    variableRef?.type || variableRef?.variableType || options.variableType || "",
  ).trim();
  if (declaredType && declaredType !== variable.type) {
    addError(
      `${ownerLabel}: 变量 ${variableName} 类型不匹配，节点为 ${declaredType}，GlobalConfig 为 ${variable.type}`,
    );
  }

  const hasScope =
    variableRef?.scope != null ||
    variableRef?.bGlobalScope != null ||
    options.scope != null ||
    options.bGlobalScope != null;
  const declaredScope =
    variableRef?.scope === "Global" ||
    variableRef?.bGlobalScope === true ||
    options.scope === "Global" ||
    options.bGlobalScope === true
      ? "Global"
      : "Session";

  if (hasScope && declaredScope !== variable.scope) {
    addError(
      `${ownerLabel}: 变量 ${variableName} scope 不匹配，节点为 ${declaredScope}，GlobalConfig 为 ${variable.scope}`,
    );
  }
}

function validateSetVariableNode(node, variableMap, addError) {
  const data = node?.data || {};
  const variableName = String(data.variableName || "").trim();
  validateVariableRef(
    `节点 ${node.id}: SetVariable`,
    null,
    variableMap,
    (message) => addError(message, { nodeId: node.id }),
    {
      variableName,
      variableType: data.variableType,
      scope: data.scope,
      bGlobalScope: data.bGlobalScope,
    },
  );

  const operation = String(data.operation || "Set");
  const variable = variableMap.get(variableName);
  if (
    variable &&
    (operation === "Add" || operation === "Subtract") &&
    variable.type !== "Int" &&
    variable.type !== "Float"
  ) {
    addError(`节点 ${node.id}: ${operation} 只能用于 Int/Float 变量`, {
      nodeId: node.id,
    });
  }
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
  validateChoiceTimer(node, choices, sourceEdges, nodeMap, addError);

  if (choiceMode === "ExhaustiveUntilComplete") {
    const completionEdges = sourceEdges.filter(
      (e) => e?.sourceHandle === "choice-complete",
    );

    if (completionEdges.length === 0) {
      addError(`节点 ${node.id}: 穷举选择模式缺少完成连线 (choice-complete)`, {
        nodeId: node.id,
      });
    } else if (completionEdges.length > 1) {
      addError(`节点 ${node.id}: 穷举选择模式只允许一条完成连线`, {
        nodeId: node.id,
      });
    }

    for (const edge of completionEdges) {
      if (!nodeMap.has(edge.target)) {
        addError(`节点 ${node.id}: 穷举选择完成目标不存在: ${edge.target || "(空)"}`, {
          nodeId: node.id,
        });
      }
    }
  }

  for (let i = 0; i < choices.length; i += 1) {
    const choice = choices[i] || {};
    if (!choice.textKey || !String(choice.textKey).trim()) {
      addError(`节点 ${node.id}: 选项 #${i + 1} 缺少 textKey`, {
        nodeId: node.id,
      });
    }

    if (choice.availability != null || choice.condition != null) {
      addError(`节点 ${node.id}: 选项 #${i + 1} 的条件字段已废弃，请改用 Condition 节点`, {
        nodeId: node.id,
      });
    }

    const expectedHandle = `choice-${i}`;
    const mappedEdges = sourceEdges.filter(
      (e) => e?.sourceHandle === expectedHandle,
    );

    if (mappedEdges.length === 0) {
      addError(`节点 ${node.id}: 选项 #${i + 1} 未连接目标边 (${expectedHandle})`, {
        nodeId: node.id,
      });
      continue;
    }

    if (mappedEdges.length > 1) {
      addError(`节点 ${node.id}: 选项 #${i + 1} 存在多条边 (${expectedHandle})`, {
        nodeId: node.id,
      });
    }

    for (const edge of mappedEdges) {
      if (!nodeMap.has(edge.target)) {
        addError(`节点 ${node.id}: 选项 #${i + 1} 连接到了不存在的目标节点`, {
          nodeId: node.id,
        });
      }
    }
  }

  for (const edge of sourceEdges) {
    const sh = edge?.sourceHandle;
    if (!sh) {
      addWarning(`节点 ${node.id}: 存在未绑定选项句柄的边 ${edge.id || "(无ID)"}`);
      continue;
    }

    if (sh === "choice-timeout") {
      if (!node?.data?.choiceTimer?.enabled) {
        addWarning(`节点 ${node.id}: 未启用倒计时时不建议存在 choice-timeout 连线`);
      }
      continue;
    }

    if (sh === "choice-complete") {
      if (choiceMode !== "ExhaustiveUntilComplete") {
        addWarning(`节点 ${node.id}: 非穷举模式下不建议存在 choice-complete 连线`);
      }
      continue;
    }

    if (!sh.startsWith("choice-")) {
      addWarning(`节点 ${node.id}: 边 ${edge.id || "(无ID)"} 使用了非标准 sourceHandle=${sh}`);
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

function validateChoiceTimer(node, choices, sourceEdges, nodeMap, addError) {
  const timer = node?.data?.choiceTimer;
  if (!timer || !timer.enabled) return;

  const durationSeconds = Number(timer.durationSeconds);
  if (!Number.isFinite(durationSeconds) || durationSeconds <= 0) {
    addError(`节点 ${node.id}: Choice 倒计时秒数必须大于 0`, {
      nodeId: node.id,
    });
  }

  if (!String(timer.timeoutChoiceTextKey || "").trim()) {
    addError(`节点 ${node.id}: Choice 倒计时超时选项文本为空`, {
      nodeId: node.id,
    });
  }

  const timeoutEdges = sourceEdges.filter(
    (edge) => edge?.sourceHandle === "choice-timeout",
  );
  if (timeoutEdges.length === 0) {
    addError(`节点 ${node.id}: 启用倒计时时缺少超时选择连线 (choice-timeout)`, {
      nodeId: node.id,
    });
  } else if (timeoutEdges.length > 1) {
    addError(`节点 ${node.id}: 超时选择只允许一条连线 (choice-timeout)`, {
      nodeId: node.id,
    });
  }

  for (const edge of timeoutEdges) {
    if (!nodeMap.has(edge.target)) {
      addError(`节点 ${node.id}: 超时选择目标不存在: ${edge.target || "(空)"}`, {
        nodeId: node.id,
      });
    }
  }
}

function validateConditionNodeEdgeMapping(
  node,
  edges,
  nodeMap,
  variableMap,
  addError,
  addWarning,
) {
  const sourceEdges = edges.filter((e) => e?.source === node.id);
  const condition = node?.data?.condition;
  if (!condition || typeof condition !== "object" || Array.isArray(condition)) {
    addError(`节点 ${node.id}: Condition 节点缺少 data.condition`, {
      nodeId: node.id,
    });
    return;
  }

  const branches = getConditionBranches(condition);
  if (branches.length === 0) {
    addError(`节点 ${node.id}: Condition 至少需要 1 个条件分支`, {
      nodeId: node.id,
    });
  }

  for (let i = 0; i < branches.length; i += 1) {
    const branch = branches[i] || {};
    const handle = `condition-${i}`;
    const mappedEdges = sourceEdges.filter((e) => e?.sourceHandle === handle);

    if (mappedEdges.length === 0) {
      addError(`节点 ${node.id}: 条件分支 #${i + 1} 未连接目标边 (${handle})`, {
        nodeId: node.id,
      });
    } else if (mappedEdges.length > 1) {
      addError(`节点 ${node.id}: 条件分支 #${i + 1} 存在多条出边 (${handle})`, {
        nodeId: node.id,
      });
    }

    for (const edge of mappedEdges) {
      if (!nodeMap.has(edge.target)) {
        addError(`节点 ${node.id}: 条件分支 #${i + 1} 目标不存在: ${edge.target || "(空)"}`, {
          nodeId: node.id,
        });
      }
    }

    validateConditionBranch(node, branch, i, variableMap, addError, addWarning);
  }

  const fallbackEdges = sourceEdges.filter(
    (e) => e?.sourceHandle === "condition-fallback",
  );
  if (fallbackEdges.length > 1) {
    addError(`节点 ${node.id}: condition-fallback 只能连接 1 条边`, {
      nodeId: node.id,
    });
  }
  for (const edge of fallbackEdges) {
    if (!nodeMap.has(edge.target)) {
      addError(`节点 ${node.id}: condition-fallback 目标不存在: ${edge.target || "(空)"}`, {
        nodeId: node.id,
      });
    }
  }
  if (fallbackEdges.length === 0) {
    addWarning(`节点 ${node.id}: 未连接 condition-fallback，所有条件都不满足时会中断`, {
      nodeId: node.id,
    });
  }

  for (const edge of sourceEdges) {
    const sh = String(edge?.sourceHandle || "");
    if (sh === "condition-fallback") continue;

    const match = /^condition-(\d+)$/.exec(sh);
    if (!match) {
      addError(`节点 ${node.id}: 非法 sourceHandle=${sh || "(空)"}`, {
        nodeId: node.id,
      });
      continue;
    }

    const index = Number(match[1]);
    if (!Number.isInteger(index) || index < 0 || index >= branches.length) {
      addError(
        `节点 ${node.id}: 边 ${edge.id || "(无ID)"} 的 sourceHandle=${sh} 越界（当前条件分支数 ${branches.length}）`,
        { nodeId: node.id },
      );
    }
  }

  validateConditionBranchUniqueness(node, branches, addError, addWarning);
}

function getConditionBranches(condition) {
  if (Array.isArray(condition?.branches)) {
    return condition.branches;
  }
  if (Array.isArray(condition?.terms)) {
    return [
      {
        label: "条件 1",
        logic: condition?.logic || "All",
        terms: condition.terms,
      },
    ];
  }
  return [];
}

function validateConditionBranch(
  node,
  branch,
  index,
  variableMap,
  addError,
  addWarning,
) {
  const label = `条件分支 #${index + 1}`;
  const logic = branch?.logic ?? "All";
  if (!VALID_LOGICS.has(logic)) {
    addError(`节点 ${node.id}: ${label} logic 非法（允许 All/Any）`, {
      nodeId: node.id,
    });
  }

  const terms = Array.isArray(branch?.terms) ? branch.terms : null;
  if (!terms) {
    addError(`节点 ${node.id}: ${label} terms 必须是数组`, { nodeId: node.id });
    return;
  }

  if (terms.length === 0) {
    addWarning(`节点 ${node.id}: ${label} 没有条件项，将恒真`, {
      nodeId: node.id,
    });
  }

  terms.forEach((term, termIndex) => {
    const variableName = String(term?.variable?.name || "").trim();
    const operator = String(term?.operator || "");
    const type = String(term?.variable?.type || "String");
    const scope = String(term?.variable?.scope || "Session");

    if (!variableName) {
      addError(`节点 ${node.id}: ${label} 条件项 #${termIndex + 1} 缺少变量`, {
        nodeId: node.id,
      });
    } else {
      validateVariableRef(
        `节点 ${node.id}: ${label} 条件项 #${termIndex + 1}`,
        term?.variable,
        variableMap,
        (message) => addError(message, { nodeId: node.id }),
      );
    }
    if (!VALID_OPERATORS.has(operator)) {
      addError(`节点 ${node.id}: ${label} 条件项 #${termIndex + 1} operator 非法`, {
        nodeId: node.id,
      });
    }
    if (!VALID_VAR_TYPES.has(type)) {
      addError(`节点 ${node.id}: ${label} 条件项 #${termIndex + 1} 变量类型非法`, {
        nodeId: node.id,
      });
    }
    if (!VALID_SCOPES.has(scope)) {
      addError(`节点 ${node.id}: ${label} 条件项 #${termIndex + 1} scope 非法`, {
        nodeId: node.id,
      });
    }
  });

  const contradiction = findSimpleContradiction(branch);
  if (contradiction) {
    addWarning(`节点 ${node.id}: ${label} 条件内部可能互相冲突：${contradiction}`, {
      nodeId: node.id,
    });
  }
}

function validateConditionBranchUniqueness(node, branches, addError, addWarning) {
  const seen = new Map();

  branches.forEach((branch, index) => {
    const signature = getConditionSignature(branch);
    if (seen.has(signature)) {
      addError(
        `节点 ${node.id}: 条件分支 #${seen.get(signature) + 1} 与 #${index + 1} 完全重复`,
        { nodeId: node.id },
      );
    } else {
      seen.set(signature, index);
    }
  });

  for (let i = 0; i < branches.length; i += 1) {
    for (let j = i + 1; j < branches.length; j += 1) {
      if (getConditionSignature(branches[i]) === getConditionSignature(branches[j])) {
        continue;
      }
      if (mayConditionsOverlap(branches[i], branches[j])) {
        addWarning(
          `节点 ${node.id}: 条件分支 #${i + 1} 与 #${j + 1} 可能同时满足，将按从上到下的第一个分支执行`,
          { nodeId: node.id },
        );
      }
    }
  }
}

function getConditionSignature(branch) {
  const logic = String(branch?.logic || "All");
  const terms = Array.isArray(branch?.terms) ? branch.terms : [];
  const termSignatures = terms
    .map((term) => {
      const name = String(term?.variable?.name || "").trim();
      const op = String(term?.operator || "==");
      const value = normalizeCompareText(term?.compareValue);
      return `${name}:${op}:${value}`;
    })
    .sort();
  return `${logic}|${termSignatures.join("|")}`;
}

function normalizeCompareText(value) {
  return String(value ?? "").trim().toLowerCase();
}

function findSimpleContradiction(branch) {
  if (String(branch?.logic || "All") !== "All") return "";
  const terms = Array.isArray(branch?.terms) ? branch.terms : [];
  const byVariable = new Map();

  for (const term of terms) {
    const variableName = String(term?.variable?.name || "").trim();
    if (!variableName) continue;
    if (!byVariable.has(variableName)) {
      byVariable.set(variableName, []);
    }
    byVariable.get(variableName).push(term);
  }

  for (const [variableName, variableTerms] of byVariable.entries()) {
    const equalityValues = variableTerms
      .filter((term) => term?.operator === "==")
      .map((term) => normalizeCompareText(term?.compareValue));
    const equalitySet = new Set(equalityValues);
    if (equalitySet.size > 1) {
      return `${variableName} 同时要求等于多个不同值`;
    }

    const equalValue = equalityValues[0];
    if (
      equalValue != null &&
      variableTerms.some(
        (term) =>
          term?.operator === "!=" &&
          normalizeCompareText(term?.compareValue) === equalValue,
      )
    ) {
      return `${variableName} 同时要求等于且不等于 ${equalValue}`;
    }

    const numericConflict = findNumericRangeConflict(variableName, variableTerms);
    if (numericConflict) return numericConflict;
  }

  return "";
}

function findNumericRangeConflict(variableName, terms) {
  let lower = null;
  let upper = null;

  for (const term of terms) {
    const op = String(term?.operator || "");
    const value = Number(term?.compareValue);
    if (!Number.isFinite(value)) continue;

    if (op === ">" || op === ">=") {
      if (!lower || value > lower.value || (value === lower.value && op === ">")) {
        lower = { value, strict: op === ">" };
      }
    }
    if (op === "<" || op === "<=") {
      if (!upper || value < upper.value || (value === upper.value && op === "<")) {
        upper = { value, strict: op === "<" };
      }
    }
  }

  if (!lower || !upper) return "";
  if (lower.value > upper.value) {
    return `${variableName} 的数值范围为空`;
  }
  if (lower.value === upper.value && (lower.strict || upper.strict)) {
    return `${variableName} 的数值范围为空`;
  }
  return "";
}

function mayConditionsOverlap(left, right) {
  const leftTerms = Array.isArray(left?.terms) ? left.terms : [];
  const rightTerms = Array.isArray(right?.terms) ? right.terms : [];
  if (leftTerms.length === 0 || rightTerms.length === 0) return true;
  if (String(left?.logic || "All") !== "All") return false;
  if (String(right?.logic || "All") !== "All") return false;

  const combined = {
    logic: "All",
    terms: [...leftTerms, ...rightTerms],
  };
  return !findSimpleContradiction(combined);
}
