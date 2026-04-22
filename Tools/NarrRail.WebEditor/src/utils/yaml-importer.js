import YAML from 'yaml';

const reverseNodeTypeMap = {
  'Dialogue': 'dialogue',
  'Choice': 'choice',
  'Jump': 'jump',
  'SetVariable': 'setVariable',
  'EmitEvent': 'emitEvent',
  'End': 'end'
};

export function importFromYAML(yamlString) {
  // 解析 YAML
  const data = YAML.parse(yamlString);

  // 转换节点
  const nodes = data.nodes.map((node, index) => {
    const base = {
      id: node.nodeId,
      type: reverseNodeTypeMap[node.nodeType],
      position: calculatePosition(index, data.nodes.length),
      data: {}
    };

    // 提取节点数据
    if (node.dialogue) {
      base.data = { ...node.dialogue };
    } else if (node.choices) {
      base.data = { choices: node.choices };
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
  const edges = data.edges.map((edge, index) => ({
    id: `e${index}`,
    source: edge.sourceNodeId,
    target: edge.targetNodeId,
    data: {
      priority: edge.priority,
      condition: edge.condition
    }
  }));

  return {
    nodes,
    edges,
    variables: data.variables,
    meta: data.meta
  };
}

// 简单的自动布局算法（网格布局）
function calculatePosition(index, total) {
  const cols = Math.ceil(Math.sqrt(total));
  const row = Math.floor(index / cols);
  const col = index % cols;
  return {
    x: col * 250 + 50,
    y: row * 150 + 50
  };
}
