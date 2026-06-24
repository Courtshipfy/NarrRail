function toText(value) {
  return String(value || "").trim();
}

function toNumberLike(value) {
  if (typeof value === "number") return value;
  const n = Number(value);
  return Number.isFinite(n) ? n : 0;
}

export function normalizeVariableValue(variable) {
  const type = String(variable?.type || "");
  if (type === "Bool") return !!variable?.boolValue;
  if (type === "Int") return Math.trunc(toNumberLike(variable?.intValue));
  if (type === "Float") return toNumberLike(variable?.floatValue);
  if (type === "String") return String(variable?.stringValue || "");
  return variable?.stringValue ?? "";
}

export function buildInitialVarSnapshot(variables) {
  const snapshot = {};
  (Array.isArray(variables) ? variables : []).forEach((variable) => {
    const name = String(variable?.name || "").trim();
    if (!name) return;
    snapshot[name] = normalizeVariableValue(variable);
  });
  return snapshot;
}

export function coerceCompareValue(raw) {
  if (typeof raw === "boolean" || typeof raw === "number") return raw;
  const text = String(raw ?? "").trim();
  if (text === "true") return true;
  if (text === "false") return false;
  const n = Number(text);
  if (Number.isFinite(n) && text !== "") return n;
  return text;
}

export function compareValues(left, operator, right) {
  switch (operator) {
    case "==":
      return left == right;
    case "!=":
      return left != right;
    case ">":
      return left > right;
    case ">=":
      return left >= right;
    case "<":
      return left < right;
    case "<=":
      return left <= right;
    default:
      return false;
  }
}

export function evaluateTerms(branch, vars) {
  const logic = String(branch?.logic || "All");
  const terms = Array.isArray(branch?.terms) ? branch.terms : [];
  if (terms.length === 0) return true;

  const checks = terms.map((term) => {
    const variableName = String(term?.variable?.name || "").trim();
    const operator = String(term?.operator || "==");
    const compareValue = coerceCompareValue(term?.compareValue);
    return compareValues(vars[variableName], operator, compareValue);
  });

  if (logic === "Any") return checks.some(Boolean);
  if (logic === "Not") return !checks[0];
  return checks.every(Boolean);
}

function normalizeChoiceTimer(timer) {
  const durationSeconds = Number(timer?.durationSeconds);
  return {
    enabled: Boolean(timer?.enabled),
    durationSeconds:
      Number.isFinite(durationSeconds) && durationSeconds > 0
        ? durationSeconds
        : 8,
    timeoutChoiceTextKey: toText(timer?.timeoutChoiceTextKey) || "超时",
  };
}

export function createStoryPreviewRunner(story, vars) {
  const nodes = Array.isArray(story?.nodes) ? story.nodes : [];
  const edges = Array.isArray(story?.edges) ? story.edges : [];
  const nodeMap = new Map(nodes.map((node) => [node.id, node]));

  const state = {
    status: "running",
    currentNodeId: String(story?.meta?.entryNodeId || ""),
    pendingChoices: [],
    timeline: [],
    error: "",
    vars,
    multiLineCursor: { nodeId: "", nextIndex: 0 },
    exhaustiveSelectedByNode: {},
    exhaustiveReturnStack: [],
    choiceTimer: {
      enabled: false,
      remainingSeconds: 0,
      durationSeconds: 0,
      timeoutChoiceTextKey: "超时",
    },
  };

  function getOutgoingEdges(sourceId) {
    return edges
      .filter((edge) => edge?.source === sourceId)
      .sort((a, b) => {
        const pa = Number(a?.data?.priority ?? 0);
        const pb = Number(b?.data?.priority ?? 0);
        if (pa !== pb) return pa - pb;
        return String(a?.id || "").localeCompare(String(b?.id || ""));
      });
  }

  function firstNextTarget(nodeId) {
    return getOutgoingEdges(nodeId)[0]?.target || "";
  }

  function getHandleEdge(nodeId, handle) {
    return getOutgoingEdges(nodeId).find(
      (edge) => String(edge?.sourceHandle || "") === String(handle || ""),
    );
  }

  function getHandleEdges(nodeId, handle) {
    return getOutgoingEdges(nodeId).filter(
      (edge) => String(edge?.sourceHandle || "") === String(handle || ""),
    );
  }

  function isExhaustiveChoiceNode(node) {
    return String(node?.data?.choiceMode || "") === "ExhaustiveUntilComplete";
  }

  function isChoiceHandleSelected(nodeId, handle) {
    return !!state.exhaustiveSelectedByNode[String(nodeId)]?.[String(handle)];
  }

  function markChoiceHandleSelected(nodeId, handle) {
    const key = String(nodeId);
    state.exhaustiveSelectedByNode[key] = {
      ...(state.exhaustiveSelectedByNode[key] || {}),
      [String(handle)]: true,
    };
  }

  function resolveChoiceCompleteTarget(nodeId) {
    return getHandleEdges(nodeId, "choice-complete")[0]?.target || "";
  }

  function setEnded(error = "") {
    state.status = "ended";
    state.currentNodeId = "";
    state.pendingChoices = [];
    state.error = error;
    state.choiceTimer = {
      enabled: false,
      remainingSeconds: 0,
      durationSeconds: 0,
      timeoutChoiceTextKey: "超时",
    };
  }

  function handleBranchEnd() {
    const returnNodeId = String(state.exhaustiveReturnStack.pop() || "").trim();
    if (returnNodeId && nodeMap.has(returnNodeId)) {
      state.status = "running";
      state.currentNodeId = returnNodeId;
      return;
    }
    setEnded();
  }

  function applySetVariable(node) {
    const name = toText(node?.data?.variableName);
    if (!name) return "变量名为空，已跳过";
    const op = String(node?.data?.operation || "Set");
    const value = coerceCompareValue(node?.data?.value);
    const current = state.vars[name];
    let next = current;

    if (op === "Set") next = value;
    else if (op === "Add") next = toNumberLike(current) + toNumberLike(value);
    else if (op === "Subtract") next = toNumberLike(current) - toNumberLike(value);
    else if (op === "Multiply") next = toNumberLike(current) * toNumberLike(value);
    else if (op === "Divide") {
      const div = toNumberLike(value);
      next = div === 0 ? toNumberLike(current) : toNumberLike(current) / div;
    }
    else next = value;

    state.vars[name] = next;
    return `${name} ${op} ${String(value)} => ${String(next)}`;
  }

  function getConditionBranches(condition) {
    if (Array.isArray(condition?.branches)) return condition.branches;
    if (Array.isArray(condition?.terms)) {
      return [{ label: "Condition 1", logic: condition?.logic || "All", terms: condition.terms }];
    }
    return [];
  }

  function advance(maxSteps = 1000) {
    let steps = 0;
    while (state.status === "running" && steps < maxSteps) {
      steps += 1;
      const node = nodeMap.get(state.currentNodeId);
      if (!node) {
        setEnded(`节点不存在: ${state.currentNodeId}`);
        return state;
      }

      const kind = String(node.type || "").toLowerCase();
      if (kind === "dialogue" || kind === "multidialogue") {
        const lines = Array.isArray(node?.data?.lines)
          ? node.data.lines
              .map((line) => toText(typeof line === "string" ? line : line?.textKey))
              .filter(Boolean)
          : [toText(node?.data?.textKey)].filter(Boolean);
        const same = state.multiLineCursor.nodeId === node.id;
        const index = same ? state.multiLineCursor.nextIndex : 0;
        if (index < lines.length) {
          state.timeline.push({
            kind: "dialogue",
            nodeId: node.id,
            speaker: toText(node?.data?.speakerId) || "旁白",
            text: lines[index],
          });
          state.multiLineCursor =
            index + 1 < lines.length
              ? { nodeId: node.id, nextIndex: index + 1 }
              : { nodeId: "", nextIndex: 0 };
          if (index + 1 >= lines.length) {
            const next = firstNextTarget(node.id);
            if (!next) handleBranchEnd();
            else state.currentNodeId = next;
          }
          return state;
        }
      }

      if (kind === "choice") {
        const choices = Array.isArray(node?.data?.choices) ? node.data.choices : [];
        const exhaustiveMode = isExhaustiveChoiceNode(node);
        const pending = choices
          .map((choice, index) => ({
            handle: `choice-${index}`,
            text: toText(choice?.textKey) || `选项 ${index + 1}`,
          }))
          .filter((choice) => {
            if (exhaustiveMode && isChoiceHandleSelected(node.id, choice.handle)) {
              return false;
            }
            return !!getHandleEdge(node.id, choice.handle);
          });
        if (pending.length === 0) {
          if (exhaustiveMode) {
            const completeTarget = resolveChoiceCompleteTarget(node.id);
            if (!completeTarget) {
              setEnded("穷举 Choice 节点已耗尽选项，但未找到 choice-complete 出口。");
              return state;
            }
            state.currentNodeId = completeTarget;
            continue;
          }
          const next = firstNextTarget(node.id);
          if (!next) setEnded(`Choice 节点 ${node.id} 没有可用分支`);
          else state.currentNodeId = next;
          continue;
        }
        state.pendingChoices = pending;
        state.status = "await-choice";
        const timer = normalizeChoiceTimer(node?.data?.choiceTimer);
        state.choiceTimer =
          timer.enabled && pending.length > 0
            ? { ...timer, remainingSeconds: timer.durationSeconds }
            : {
                enabled: false,
                remainingSeconds: 0,
                durationSeconds: 0,
                timeoutChoiceTextKey: "超时",
              };
        return state;
      }

      if (kind === "condition") {
        const branches = getConditionBranches(node?.data?.condition);
        const index = branches.findIndex((branch) => evaluateTerms(branch, state.vars));
        const handle = index >= 0 ? `condition-${index}` : "condition-fallback";
        const target = getHandleEdge(node.id, handle)?.target;
        state.timeline.push({
          kind: "condition",
          nodeId: node.id,
          text: `${node.id} => ${index >= 0 ? branches[index]?.label || handle : handle}`,
        });
        if (!target) setEnded(`Condition 节点 ${node.id} 缺少 ${handle} 出口`);
        else state.currentNodeId = target;
        continue;
      }

      if (kind === "setvariable") {
        state.timeline.push({ kind: "setvariable", nodeId: node.id, text: applySetVariable(node) });
        const next = firstNextTarget(node.id);
        if (!next) handleBranchEnd();
        else state.currentNodeId = next;
        continue;
      }

      if (kind === "emitevent") {
        const eventLabel =
          toText(node?.data?.eventType) ||
          toText(node?.data?.eventId) ||
          "(未命名事件)";
        state.timeline.push({
          kind: "emitevent",
          nodeId: node.id,
          text: eventLabel,
          payload: {
            nodeId: node.id,
            phase: "node",
            eventId: toText(node?.data?.eventId),
            eventType: toText(node?.data?.eventType),
            params: node?.data?.params || node?.data?.parameters || {},
          },
        });
        const next = firstNextTarget(node.id);
        if (!next) handleBranchEnd();
        else state.currentNodeId = next;
        continue;
      }

      if (kind === "jump") {
        const target = toText(node?.data?.targetNodeId);
        state.timeline.push({ kind: "jump", nodeId: node.id, text: `${node.id} -> ${target}` });
        if (!target) setEnded(`Jump 节点 ${node.id} 未配置 targetNodeId`);
        else state.currentNodeId = target;
        continue;
      }

      if (kind === "end") {
        state.timeline.push({ kind: "end", nodeId: node.id, text: node.id });
        handleBranchEnd();
        return state;
      }

      const next = firstNextTarget(node.id);
      if (!next) handleBranchEnd();
      else state.currentNodeId = next;
    }

    if (steps >= maxSteps && state.status === "running") {
      setEnded("执行超过最大步数，疑似存在循环。");
    }
    return state;
  }

  function choose(handle) {
    if (state.status !== "await-choice") return state;
    const node = nodeMap.get(state.currentNodeId);
    const edge = getHandleEdge(node?.id, handle);
    const choice = state.pendingChoices.find((item) => item.handle === handle);
    if (!edge?.target) {
      setEnded("所选分支没有可用目标节点");
      return state;
    }
    state.timeline.push({
      kind: "choice",
      nodeId: node.id,
      text:
        String(handle || "") === "choice-timeout"
          ? `超时选择：${toText(node?.data?.choiceTimer?.timeoutChoiceTextKey) || "超时"}`
          : choice?.text || String(handle || ""),
    });
    state.pendingChoices = [];
    state.choiceTimer = {
      enabled: false,
      remainingSeconds: 0,
      durationSeconds: 0,
      timeoutChoiceTextKey: "超时",
    };
    state.status = "running";
    state.currentNodeId = edge.target;
    if (isExhaustiveChoiceNode(node) && handle !== "choice-timeout") {
      markChoiceHandleSelected(node.id, handle);
      state.exhaustiveReturnStack.push(String(node.id));
    }
    return advance();
  }

  return { state, advance, choose };
}
