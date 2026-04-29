import YAML from "yaml";

const nodeTypeMap = {
  dialogue: "Dialogue",
  choice: "Choice",
  jump: "Jump",
  setVariable: "SetVariable",
  emitEvent: "EmitEvent",
  end: "End",
};

export function buildYAMLString(nodes, edges, variables, meta) {
  // 转换节点
  const yamlNodes = nodes.map((node) => {
    const base = {
      nodeId: node.id,
      nodeType: nodeTypeMap[node.type],
    };

    // 根据类型添加特定字段
    if (node.type === "dialogue") {
      base.dialogue = {
        speakerId: node.data.speakerId || "",
        textKey: node.data.textKey || "",
        speechRate: node.data.speechRate || 1.0,
        voiceAsset: node.data.voiceAsset || "",
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
    } else if (node.type === "jump") {
      base.jump = {
        targetNodeId: node.data.targetNodeId || "",
      };
    } else if (node.type === "setVariable") {
      base.actions = node.data.actions || [];
    } else if (node.type === "emitEvent") {
      base.emitEvent = {
        eventId: node.data.eventId || "",
      };
    }

    return base;
  });

  // 转换边 - 过滤掉 Choice 节点的边（已经包含在节点的 choices 中）
  const yamlEdges = edges
    .filter((edge) => {
      // 检查源节点是否是 Choice 类型
      const sourceNode = nodes.find((n) => n.id === edge.source);
      // 如果是 Choice 节点且有 sourceHandle，说明是选项连接，不需要单独导出
      return !(sourceNode && sourceNode.type === "choice" && edge.sourceHandle);
    })
    .map((edge) => ({
      sourceNodeId: edge.source,
      targetNodeId: edge.target,
      priority: edge.data?.priority || 0,
      condition: edge.data?.condition || { logic: "All", terms: [] },
    }));

  // 构建完整结构
  const yamlData = {
    meta: {
      schemaVersion: 1,
      storyId: meta.storyId,
      entryNodeId: meta.entryNodeId,
    },
    variables: variables,
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
  a.download = `${meta.storyId}.narrrail.yaml`;
  a.click();
  URL.revokeObjectURL(url);
}
