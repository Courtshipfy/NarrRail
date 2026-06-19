import {
  buildInitialVarSnapshot,
  createStoryPreviewRunner,
  evaluateTerms,
} from "./story-preview-runner.js";

function toText(value) {
  return String(value || "").trim();
}

function sortEdges(edges) {
  return [...edges].sort((a, b) => {
    const pa = Number(a?.data?.priority ?? 0);
    const pb = Number(b?.data?.priority ?? 0);
    if (pa !== pb) return pa - pb;
    return String(a?.id || "").localeCompare(String(b?.id || ""));
  });
}

function formatTermValue(value) {
  if (typeof value === "boolean") return value ? "true" : "false";
  if (typeof value === "number") return String(value);
  const text = toText(value);
  return text || "空值";
}

function formatRailBranchExpression(branch) {
  const terms = Array.isArray(branch?.terms) ? branch.terms : [];
  if (terms.length === 0) return "无条件";

  const joiner = branch?.logic === "Any" ? " 或 " : " 且 ";
  return terms
    .map((term) => {
      const variableName = toText(term?.variable?.name) || "未选择变量";
      const operator = toText(term?.operator) || "==";
      return `${variableName} ${operator} ${formatTermValue(term?.compareValue)}`;
    })
    .join(joiner);
}

export function createRailPreviewRunner(rail, options = {}) {
  const nodes = Array.isArray(rail?.nodes) ? rail.nodes : [];
  const edges = Array.isArray(rail?.edges) ? rail.edges : [];
  const nodeMap = new Map(nodes.map((node) => [node.id, node]));
  const resolveStoryById =
    typeof options.resolveStoryById === "function"
      ? options.resolveStoryById
      : () => null;

  const state = {
    status: "running",
    currentRailNodeId: String(rail?.meta?.entryNodeId || ""),
    activeStory: null,
    pendingChoices: [],
    timeline: [],
    error: "",
    vars: buildInitialVarSnapshot(options.variables || []),
  };

  function getOutgoingEdges(sourceId) {
    return sortEdges(edges.filter((edge) => edge?.source === sourceId));
  }

  function getHandleEdge(sourceId, handle) {
    return getOutgoingEdges(sourceId).find(
      (edge) => String(edge?.sourceHandle || "") === String(handle || ""),
    );
  }

  function firstNextTarget(sourceId) {
    return getOutgoingEdges(sourceId)[0]?.target || "";
  }

  function setEnded(error = "") {
    state.status = "ended";
    state.currentRailNodeId = "";
    state.activeStory = null;
    state.pendingChoices = [];
    state.error = error;
  }

  function enterNextRailNodeFrom(sourceId) {
    const next = firstNextTarget(sourceId);
    if (!next) {
      setEnded();
      return;
    }
    state.currentRailNodeId = next;
    state.status = "running";
  }

  function absorbStoryTimeline(storyRunner, storyTitle) {
    const items = storyRunner.state.timeline.splice(0);
    items.forEach((item) => {
      state.timeline.push({
        ...item,
        railTitle: storyTitle,
      });
    });
  }

  async function advance(maxSteps = 1000) {
    let steps = 0;
    while (state.status === "running" && steps < maxSteps) {
      steps += 1;

      if (state.activeStory) {
        const story = state.activeStory;
        story.runner.advance();
        absorbStoryTimeline(story.runner, story.title);

        if (story.runner.state.status === "await-choice") {
          state.status = "await-choice";
          state.pendingChoices = story.runner.state.pendingChoices;
          return state;
        }

        if (story.runner.state.status === "ended") {
          if (story.runner.state.error) {
            setEnded(story.runner.state.error);
            return state;
          }
          const sourceId = story.railNodeId;
          state.activeStory = null;
          enterNextRailNodeFrom(sourceId);
          continue;
        }
        return state;
      }

      const node = nodeMap.get(state.currentRailNodeId);
      if (!node) {
        setEnded(`总纲节点不存在: ${state.currentRailNodeId}`);
        return state;
      }

      const type = String(node.type || "");
      const title = toText(node?.data?.title) || node.id;

      if (type === "story" || type === "railstory") {
        const storyId = toText(node?.data?.storyId);
        if (!storyId) {
          setEnded(`Story 节点 ${node.id} 缺少 storyId`);
          return state;
        }
        const story = await resolveStoryById(storyId);
        if (!story) {
          setEnded(`找不到脚本: ${storyId}`);
          return state;
        }

        state.activeStory = {
          railNodeId: node.id,
          title,
          runner: createStoryPreviewRunner(story, state.vars),
        };
        continue;
      }

      if (type === "branch" || type === "railbranch") {
        const branches = Array.isArray(node?.data?.branches)
          ? node.data.branches
          : [];
        const index = branches.findIndex((branch) => evaluateTerms(branch, state.vars));
        const handle = index >= 0 ? `branch-${index}` : "branch-fallback";
        const edge = getHandleEdge(node.id, handle);
        const matchedBranch = index >= 0 ? branches[index] : null;
        const branchLabel = matchedBranch ? toText(matchedBranch?.label) : "未匹配";
        const expression = matchedBranch
          ? formatRailBranchExpression(matchedBranch)
          : "未匹配任何分支";
        if (!edge?.target) {
          setEnded(`Branch 节点 ${node.id} 缺少 ${handle} 出口`);
          return state;
        }
        state.timeline.push({
          kind: "railbranch",
          nodeId: node.id,
          text: title,
          summary: [
            toText(node?.data?.summary),
            `判断：${expression}`,
            `结果：${branchLabel || handle}`,
          ]
            .filter(Boolean)
            .join(" · "),
        });
        state.currentRailNodeId = edge.target;
        return state;
      }

      if (type === "note" || type === "railnote") {
        state.timeline.push({
          kind: "railnote",
          nodeId: node.id,
          text: title,
          summary: toText(node?.data?.summary),
        });
        const next = firstNextTarget(node.id);
        if (!next) setEnded();
        else state.currentRailNodeId = next;
        return state;
      }

      if (type === "end" || type === "railend") {
        state.timeline.push({
          kind: "railend",
          nodeId: node.id,
          text: title,
          summary: toText(node?.data?.summary) || "总纲预览结束",
        });
        setEnded();
        return state;
      }

      enterNextRailNodeFrom(node.id);
    }

    if (steps >= maxSteps && state.status === "running") {
      setEnded("总纲执行超过最大步数，疑似存在循环。");
    }
    return state;
  }

  async function choose(handle) {
    if (state.status !== "await-choice" || !state.activeStory) return state;
    state.activeStory.runner.choose(handle);
    absorbStoryTimeline(state.activeStory.runner, state.activeStory.title);
    if (state.activeStory.runner.state.status === "await-choice") {
      state.pendingChoices = state.activeStory.runner.state.pendingChoices;
      return state;
    }
    state.status = "running";
    state.pendingChoices = [];
    return advance();
  }

  return { state, advance, choose };
}
