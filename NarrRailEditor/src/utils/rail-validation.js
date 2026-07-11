function issue(message, nodeId = "") {
  return nodeId ? `${nodeId}: ${message}` : message;
}

function getBranches(node) {
  return Array.isArray(node?.data?.branches) ? node.data.branches : [];
}

function collectReachable(entryNodeId, nodes, edges) {
  const nodeIds = new Set(nodes.map((node) => node.id));
  const outgoing = new Map();
  edges.forEach((edge) => {
    if (!outgoing.has(edge.source)) outgoing.set(edge.source, []);
    outgoing.get(edge.source).push(edge.target);
  });

  const reachable = new Set();
  const stack = entryNodeId && nodeIds.has(entryNodeId) ? [entryNodeId] : [];
  while (stack.length > 0) {
    const id = stack.pop();
    if (reachable.has(id)) continue;
    reachable.add(id);
    for (const next of outgoing.get(id) || []) {
      if (nodeIds.has(next) && !reachable.has(next)) stack.push(next);
    }
  }
  return reachable;
}

export function validateRail(nodes, edges, meta, options = {}) {
  const errors = [];
  const warnings = [];
  const safeNodes = Array.isArray(nodes) ? nodes : [];
  const safeEdges = Array.isArray(edges) ? edges : [];
  const storyIds = new Set(
    (Array.isArray(options.storyEntries) ? options.storyEntries : [])
      .map((entry) => String(entry?.storyId || "").trim())
      .filter(Boolean),
  );
  const variableNames = new Set(
    (Array.isArray(options.variables) ? options.variables : [])
      .map((variable) => String(variable?.name || "").trim())
      .filter(Boolean),
  );

  if (!String(meta?.railId || "").trim()) {
    errors.push(issue("railId 不能为空"));
  }

  if (!String(meta?.entryNodeId || "").trim()) {
    errors.push(issue("entryNodeId 不能为空"));
  }

  const nodeIds = new Set();
  safeNodes.forEach((node) => {
    const id = String(node?.id || "").trim();
    if (!id) {
      errors.push(issue("存在空 nodeId"));
      return;
    }
    if (nodeIds.has(id)) {
      errors.push(issue("nodeId 重复", id));
      return;
    }
    nodeIds.add(id);
  });

  if (meta?.entryNodeId && !nodeIds.has(String(meta.entryNodeId))) {
    errors.push(issue(`entryNodeId 不存在: ${meta.entryNodeId}`));
  }

  const outgoingBySource = new Map();
  safeEdges.forEach((edge) => {
    if (!nodeIds.has(edge.source)) {
      errors.push(issue("边 sourceNodeId 不存在", edge.source || ""));
    }
    if (!nodeIds.has(edge.target)) {
      errors.push(issue("边 targetNodeId 不存在", edge.target || ""));
    }
    if (!outgoingBySource.has(edge.source)) outgoingBySource.set(edge.source, []);
    outgoingBySource.get(edge.source).push(edge);
  });

  safeNodes.forEach((node) => {
    const id = String(node?.id || "");
    const type = String(node?.type || "");

    if (type === "story" || type === "railstory") {
      const storyId = String(node?.data?.storyId || "").trim();
      if (!storyId) {
        errors.push(issue("Story 节点缺少 storyId", id));
      } else if (
        (storyIds.size > 0 || options.requireKnownStoryReferences) &&
        !storyIds.has(storyId)
      ) {
        errors.push(issue(`引用的脚本不存在: ${storyId}`, id));
      }
    }

    if (type === "branch" || type === "railbranch") {
      const branches = getBranches(node);
      const outgoing = outgoingBySource.get(id) || [];
      if (branches.length === 0) {
        warnings.push(issue("Branch 节点没有条件分支，将只使用 fallback", id));
      }

      outgoing.forEach((edge) => {
        const handle = String(edge.sourceHandle || "");
        if (handle === "branch-fallback") return;
        const match = /^branch-(\d+)$/.exec(handle);
        if (!match) {
          errors.push(issue(`Branch 出口无效: ${handle || "(空)"}`, id));
          return;
        }
        const index = Number(match[1]);
        if (index < 0 || index >= branches.length) {
          errors.push(issue(`Branch 出口 ${handle} 没有对应分支`, id));
        }
      });

      const hasAnyBranchEdge = outgoing.some((edge) =>
        /^branch-\d+$/.test(String(edge.sourceHandle || "")),
      );
      const hasFallback = outgoing.some(
        (edge) => String(edge.sourceHandle || "") === "branch-fallback",
      );
      if (!hasAnyBranchEdge && !hasFallback) {
        errors.push(issue("Branch 节点没有可用出边", id));
      }
      if (!hasFallback) {
        warnings.push(issue("Branch 节点缺少 branch-fallback 出口", id));
      }

      branches.forEach((branch, branchIndex) => {
        (Array.isArray(branch?.terms) ? branch.terms : []).forEach((term) => {
          const varName = String(term?.variable?.name || "").trim();
          if (varName && variableNames.size > 0 && !variableNames.has(varName)) {
            warnings.push(
              issue(`分支 ${branchIndex + 1} 引用未定义变量: ${varName}`, id),
            );
          }
        });
      });
    }

    if (type !== "end" && type !== "railend" && (outgoingBySource.get(id) || []).length === 0) {
      warnings.push(issue("非 End 节点没有出边", id));
    }
  });

  const reachable = collectReachable(meta?.entryNodeId, safeNodes, safeEdges);
  safeNodes.forEach((node) => {
    if (node?.id && !reachable.has(node.id)) {
      warnings.push(issue("节点不可达", node.id));
    }
  });

  return { errors, warnings };
}
