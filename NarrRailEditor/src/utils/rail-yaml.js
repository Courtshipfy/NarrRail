import YAML from "yaml";

const railNodeTypeMap = {
  railstory: "Story",
  railbranch: "Branch",
  railnote: "Note",
  railend: "End",
  story: "Story",
  branch: "Branch",
  note: "Note",
  end: "End",
};

const reverseRailNodeTypeMap = {
  Story: "railstory",
  Branch: "railbranch",
  Note: "railnote",
  End: "railend",
};

function toStringValue(value, fallback = "") {
  const text = String(value ?? "").trim();
  return text || fallback;
}

function normalizeBranch(branch, index = 0) {
  return {
    label: toStringValue(branch?.label, `分支 ${index + 1}`),
    logic: branch?.logic === "Any" ? "Any" : "All",
    terms: Array.isArray(branch?.terms) ? branch.terms : [],
  };
}

export function buildEmptyRailData(railId = "main_story") {
  return {
    meta: {
      schemaVersion: 1,
      railId,
      title: "剧情总纲",
      entryNodeId: "rail-start",
    },
    nodes: [
      {
        id: "rail-start",
        type: "railnote",
        position: { x: 120, y: 160 },
        data: {
          title: "总纲开始",
          summary: "",
        },
      },
      {
        id: "rail-end",
        type: "railend",
        position: { x: 520, y: 160 },
        data: {
          title: "总纲结束",
          summary: "",
        },
      },
    ],
    edges: [
      {
        id: "rail-edge-start-end",
        source: "rail-start",
        sourceHandle: "",
        target: "rail-end",
        data: { priority: 0 },
      },
    ],
  };
}

export function buildRailYAMLString(nodes, edges, meta) {
  const yamlData = {
    meta: {
      schemaVersion: 1,
      railId: toStringValue(meta?.railId, "main_story"),
      title: String(meta?.title || ""),
      entryNodeId: toStringValue(meta?.entryNodeId),
    },
    nodes: (Array.isArray(nodes) ? nodes : []).map((node) => {
      const base = {
        nodeId: node.id,
        nodeType: railNodeTypeMap[node.type] || "Note",
        position: {
          x: Number(node.position?.x ?? 0),
          y: Number(node.position?.y ?? 0),
        },
        title: String(node.data?.title || ""),
        summary: String(node.data?.summary || ""),
      };

      if (node.type === "railstory" || node.type === "story") {
        base.storyId = String(node.data?.storyId || "");
      }

      if (node.type === "railbranch" || node.type === "branch") {
        base.branches = (Array.isArray(node.data?.branches)
          ? node.data.branches
          : []
        ).map(normalizeBranch);
      }

      return base;
    }),
    edges: (Array.isArray(edges) ? edges : []).map((edge) => ({
      sourceNodeId: edge.source,
      sourceHandle: edge.sourceHandle || "",
      targetNodeId: edge.target,
      priority: Number(edge.data?.priority ?? 0),
    })),
  };

  return YAML.stringify(yamlData, {
    lineWidth: 0,
    minContentWidth: 0,
  });
}

export function exportRailToYAML(nodes, edges, meta) {
  const yamlString = buildRailYAMLString(nodes, edges, meta);
  const railId = toStringValue(meta?.railId, "main_story");
  const blob = new Blob([yamlString], { type: "text/yaml;charset=utf-8" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = `${railId}.nrrail`;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}

export function importRailFromYAML(yamlString) {
  const data = YAML.parse(yamlString);
  if (!data || typeof data !== "object") {
    throw new Error("总纲 YAML 为空或格式无效");
  }

  const rawNodes = Array.isArray(data.nodes) ? data.nodes : [];
  const hasNodePositions = rawNodes.some(
    (node) =>
      Number.isFinite(Number(node?.position?.x)) &&
      Number.isFinite(Number(node?.position?.y)),
  );
  const nodes = rawNodes.map((node, index) => {
    const type = reverseRailNodeTypeMap[node.nodeType] || "note";
    return {
      id: toStringValue(node.nodeId, `rail-node-${index + 1}`),
      type,
      position: {
        x: Number(node.position?.x ?? 140 + index * 280),
        y: Number(node.position?.y ?? 160 + (index % 3) * 130),
      },
      data: {
        title: String(node.title || ""),
        summary: String(node.summary || ""),
        storyId: String(node.storyId || ""),
        branches:
          type === "railbranch" && Array.isArray(node.branches)
            ? node.branches.map(normalizeBranch)
            : [],
      },
    };
  });

  let edgeIndex = 0;
  const edges = (Array.isArray(data.edges) ? data.edges : []).map((edge) => ({
    id: `rail-edge-${edgeIndex++}`,
    source: String(edge.sourceNodeId || ""),
    sourceHandle: String(edge.sourceHandle || ""),
    target: String(edge.targetNodeId || ""),
    data: { priority: Number(edge.priority ?? 0) },
  }));

  return {
    meta: {
      schemaVersion: Number(data.meta?.schemaVersion || 1),
      railId: toStringValue(data.meta?.railId, "main_story"),
      title: String(data.meta?.title || ""),
      entryNodeId: String(data.meta?.entryNodeId || ""),
    },
    nodes,
    edges,
    hasNodePositions,
  };
}
