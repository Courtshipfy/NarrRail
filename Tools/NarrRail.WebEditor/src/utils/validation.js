export function validateStory(nodes, edges, meta) {
  const errors = [];
  const warnings = [];

  // 1. 检查入口节点
  if (!meta.entryNodeId) {
    errors.push({ type: 'error', message: '未设置入口节点' });
  } else if (!nodes.find(n => n.id === meta.entryNodeId)) {
    errors.push({ type: 'error', message: `入口节点不存在: ${meta.entryNodeId}` });
  }

  // 2. 检查节点 ID 唯一性
  const ids = nodes.map(n => n.id);
  const duplicates = ids.filter((id, index) => ids.indexOf(id) !== index);
  if (duplicates.length > 0) {
    errors.push({
      type: 'error',
      message: `重复的节点 ID: ${[...new Set(duplicates)].join(', ')}`
    });
  }

  // 3. 检查边引用
  edges.forEach((edge, index) => {
    if (!nodes.find(n => n.id === edge.source)) {
      errors.push({
        type: 'error',
        message: `边 #${index} 引用了不存在的源节点: ${edge.source}`
      });
    }
    if (!nodes.find(n => n.id === edge.target)) {
      errors.push({
        type: 'error',
        message: `边 #${index} 引用了不存在的目标节点: ${edge.target}`
      });
    }
  });

  // 4. 检查孤立节点
  const connectedNodes = new Set();
  edges.forEach(edge => {
    connectedNodes.add(edge.source);
    connectedNodes.add(edge.target);
  });
  nodes.forEach(node => {
    if (!connectedNodes.has(node.id) && node.id !== meta.entryNodeId) {
      warnings.push({
        type: 'warning',
        message: `孤立节点: ${node.id}`,
        nodeId: node.id
      });
    }
  });

  // 5. 检查必填字段
  nodes.forEach(node => {
    if (node.type === 'dialogue') {
      if (!node.data.speakerId) {
        errors.push({
          type: 'error',
          message: `节点 ${node.id}: 缺少说话人 ID`,
          nodeId: node.id
        });
      }
      if (!node.data.textKey) {
        errors.push({
          type: 'error',
          message: `节点 ${node.id}: 缺少对话文本`,
          nodeId: node.id
        });
      }
    } else if (node.type === 'choice') {
      if (!node.data.choices || node.data.choices.length === 0) {
        errors.push({
          type: 'error',
          message: `节点 ${node.id}: 选择节点至少需要一个选项`,
          nodeId: node.id
        });
      }
    } else if (node.type === 'jump') {
      if (!node.data.targetNodeId) {
        errors.push({
          type: 'error',
          message: `节点 ${node.id}: 缺少跳转目标`,
          nodeId: node.id
        });
      }
    }
  });

  return { errors, warnings };
}
