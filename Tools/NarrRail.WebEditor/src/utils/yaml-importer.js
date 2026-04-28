import YAML from "yaml";

const reverseNodeTypeMap = {
  Dialogue: "dialogue",
  Choice: "choice",
  Jump: "jump",
  SetVariable: "setVariable",
  EmitEvent: "emitEvent",
  End: "end",
};

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
      base.data = {
        choices: choiceArray.map((c) => ({
          textKey: c.textKey,
        })),
      };
    } else if (node.dialogue) {
      base.data = { ...node.dialogue };
    } else if (node.jump) {
      base.data = { ...node.jump };
    } else if (node.actions) {
      base.data = { actions: node.actions };
    } else if (node.emitEvent) {
      base.data = { ...node.emitEvent };
    }

    return base;
  });

  // 转换边
  const edges = [];
  let edgeIndex = 0;

  // 处理普通边
  data.edges.forEach((edge) => {
    edges.push({
      id: `e${edgeIndex++}`,
      source: edge.sourceNodeId,
      target: edge.targetNodeId,
      type: "default",
      animated: false,
      style: "stroke: rgba(168, 85, 247, 0.6); stroke-width: 2px;",
      data: {
        priority: edge.priority,
        condition: edge.condition,
      },
    });
  });

  // 处理 Choice 节点的选项边（兼容新旧格式）
  data.nodes.forEach((node) => {
    const choiceArray = getChoiceArray(node);
    if (choiceArray.length > 0) {
      choiceArray.forEach((choice, index) => {
        if (choice.targetNodeId) {
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
              condition: { logic: "All", terms: [] },
            },
          });
        }
      });
    }
  });

  return {
    nodes,
    edges,
    variables: data.variables,
    meta: data.meta,
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
