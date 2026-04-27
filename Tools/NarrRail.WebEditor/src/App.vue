<template>
  <div class="editor-container">
    <div class="main-content">
      <div class="graph-editor-wrapper">
        <GraphEditorWrapper :nodes="nodes" :edges="edges" />
      </div>
    </div>

    <Toolbar
      :focus-mode-enabled="focusModeEnabled"
      @new="handleNew"
      @import="handleImport"
      @export="handleExport"
      @validate="handleValidate"
      @auto-layout="handleAutoLayout"
      @toggle-focus-mode="handleToggleFocusMode"
      @help="handleHelp"
    />

    <StatusBar
      :node-count="nodes.length"
      :edge-count="edges.length"
      :story-id="storyMeta.storyId"
      :entry-node-id="storyMeta.entryNodeId"
      :error-count="validationResult.errors.length"
      :warning-count="validationResult.warnings.length"
    />

    <PropertyPanel
      :selected-node="selectedNode"
      :entry-node-id="storyMeta.entryNodeId"
      @update="handleNodeUpdate"
      @set-entry-node="handleSetEntryNode"
    />

    <VariablePanel :variables="variables" @update="handleVariablesUpdate" />

    <div v-if="selectedEdge" class="edge-editor glass-morphism-strong">
      <div class="edge-editor-header">
        <h3>边属性</h3>
        <button class="close-btn" @click="clearEdgeSelection">✕</button>
      </div>

      <div class="form-group">
        <label>Edge ID</label>
        <input class="form-input" :value="selectedEdge.id" readonly />
      </div>

      <div class="form-group">
        <label>Source → Target</label>
        <input
          class="form-input"
          :value="`${selectedEdge.source} → ${selectedEdge.target}`"
          readonly
        />
      </div>

      <div class="form-group">
        <label>优先级 (priority)</label>
        <input type="number" class="form-input" v-model.number="edgeDraft.priority" />
      </div>

      <div class="form-group">
        <label>条件逻辑 (condition.logic)</label>
        <select class="form-input" v-model="edgeDraft.logic">
          <option value="All">All</option>
          <option value="Any">Any</option>
          <option value="Not">Not</option>
        </select>
      </div>

      <div class="form-group">
        <label>条件项 JSON (condition.terms)</label>
        <textarea
          class="form-textarea"
          v-model="edgeDraft.termsText"
          placeholder='[{"variable":{"name":"Affinity","type":"Int","scope":"Session"},"operator":">=","compareValue":"10"}]'
        />
      </div>

      <div class="edge-editor-actions">
        <button class="action-btn primary" @click="applyEdgeDraft">应用</button>
        <button class="action-btn" @click="resetEdgeDraft">重置</button>
      </div>
    </div>

    <div class="validation-badge" :class="{ error: hasErrors, warning: !hasErrors && hasWarnings }">
      <span v-if="hasErrors">错误 {{ validationResult.errors.length }}</span>
      <span v-else-if="hasWarnings">警告 {{ validationResult.warnings.length }}</span>
      <span v-else>实时验证通过</span>
    </div>

    <input
      ref="fileInput"
      type="file"
      accept=".yaml,.yml"
      style="display: none"
      @change="handleFileChange"
    />
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted, onUnmounted, watch } from "vue";
import Toolbar from "./components/Toolbar.vue";
import GraphEditorWrapper from "./components/GraphEditorWrapper.vue";
import PropertyPanel from "./components/PropertyPanel.vue";
import VariablePanel from "./components/VariablePanel.vue";
import StatusBar from "./components/StatusBar.vue";
import { exportToYAML } from "./utils/yaml-exporter.js";
import { importFromYAML } from "./utils/yaml-importer.js";
import { validateStory } from "./utils/validation.js";
import storage from "./utils/storage.js";

const nodes = ref([
  {
    id: "node-1",
    type: "dialogue",
    position: { x: 220, y: 120 },
    data: { speakerId: "Alice", textKey: "你好！欢迎使用 NarrRail 编辑器。" },
  },
  {
    id: "node-2",
    type: "choice",
    position: { x: 560, y: 120 },
    data: { choices: [{ textKey: "继续对话" }, { textKey: "结束对话" }] },
  },
  {
    id: "node-3",
    type: "dialogue",
    position: { x: 900, y: 40 },
    data: { speakerId: "Bob", textKey: "很高兴认识你！" },
  },
  {
    id: "node-4",
    type: "end",
    position: { x: 900, y: 220 },
    data: {},
  },
]);

const edges = ref([
  {
    id: "e1-2",
    source: "node-1",
    target: "node-2",
    type: "smoothstep",
    animated: false,
    data: { priority: 0, condition: { logic: "All", terms: [] } },
  },
  {
    id: "e2-3",
    source: "node-2",
    sourceHandle: "choice-0",
    target: "node-3",
    type: "smoothstep",
    animated: false,
    data: { priority: 0, condition: { logic: "All", terms: [] } },
  },
  {
    id: "e2-4",
    source: "node-2",
    sourceHandle: "choice-1",
    target: "node-4",
    type: "smoothstep",
    animated: false,
    data: { priority: 0, condition: { logic: "All", terms: [] } },
  },
]);

const selectedNode = ref(null);
const selectedEdge = ref(null);
const focusModeEnabled = ref(true);

const storyMeta = ref({
  storyId: "DemoStory",
  entryNodeId: "node-1",
  schemaVersion: 1,
});

const variables = ref([]);
const fileInput = ref(null);

const validationResult = ref({ errors: [], warnings: [] });
const hasErrors = computed(() => validationResult.value.errors.length > 0);
const hasWarnings = computed(() => validationResult.value.warnings.length > 0);

const edgeDraft = reactive({
  priority: 0,
  logic: "All",
  termsText: "[]",
});

let autoSaveTimer = null;

function safeClone(obj) {
  return JSON.parse(JSON.stringify(obj));
}

function isDeepEqual(a, b) {
  return JSON.stringify(a) === JSON.stringify(b);
}

function normalizeEdge(edge) {
  return {
    ...edge,
    type: edge?.type || "smoothstep",
    animated: edge?.animated ?? false,
    style: edge?.style || "stroke: rgba(168, 85, 247, 0.55); stroke-width: 2px;",
    data: {
      priority: edge?.data?.priority ?? 0,
      condition: {
        logic: edge?.data?.condition?.logic || "All",
        terms: Array.isArray(edge?.data?.condition?.terms) ? edge.data.condition.terms : [],
      },
    },
  };
}

function isValidChoiceHandle(node, sourceHandle) {
  if (!node || node.type !== "choice") return true;
  if (!sourceHandle) return true;
  const match = /^choice-(\d+)$/.exec(sourceHandle);
  if (!match) return false;
  const index = Number(match[1]);
  const count = Array.isArray(node.data?.choices) ? node.data.choices.length : 0;
  return Number.isInteger(index) && index >= 0 && index < count;
}

function sanitizeEdges(rawEdges) {
  const seenIds = new Set();
  return (rawEdges || [])
    .map(normalizeEdge)
    .filter((edge) => {
      if (!edge?.id || seenIds.has(edge.id)) return false;
      seenIds.add(edge.id);

      const sourceNode = nodes.value.find((n) => n.id === edge.source);
      const targetNode = nodes.value.find((n) => n.id === edge.target);
      if (!sourceNode || !targetNode) return false;

      return isValidChoiceHandle(sourceNode, edge.sourceHandle);
    });
}

function syncChoiceEdgesForNode(choiceNode, currentEdges) {
  if (!choiceNode || choiceNode.type !== "choice") return currentEdges;
  return (currentEdges || []).filter((edge) => {
    if (edge.source !== choiceNode.id) return true;
    return isValidChoiceHandle(choiceNode, edge.sourceHandle);
  });
}

function isEdgeFocused(edge) {
  if (selectedEdge.value) return selectedEdge.value.id === edge.id;
  if (selectedNode.value) {
    return edge.source === selectedNode.value.id || edge.target === selectedNode.value.id;
  }
  return false;
}

function deriveEdgeStyle(edge) {
  const focused = isEdgeFocused(edge);
  const baseColor = "168, 85, 247";

  if (focusModeEnabled.value) {
    if (focused) {
      return {
        style: "stroke: rgba(168, 85, 247, 0.95); stroke-width: 3.5px;",
        animated: true,
      };
    }
    return {
      style: "stroke: rgba(168, 85, 247, 0.12); stroke-width: 1.5px;",
      animated: false,
    };
  }

  if (focused) {
    return {
      style: `stroke: rgba(${baseColor}, 0.88); stroke-width: 3px;`,
      animated: true,
    };
  }

  return {
    style: `stroke: rgba(${baseColor}, 0.52); stroke-width: 2px;`,
    animated: false,
  };
}

function applyEdgeVisualStyles() {
  const next = edges.value.map((edge) => {
    const { style, animated } = deriveEdgeStyle(edge);
    return { ...edge, style, animated, type: "smoothstep" };
  });

  if (!isDeepEqual(next, edges.value)) {
    edges.value = safeClone(next);
  }
}

function runRealtimeValidation() {
  validationResult.value = validateStory(nodes.value, edges.value, storyMeta.value);
}

function setupAutoSave() {
  if (autoSaveTimer) clearInterval(autoSaveTimer);
  autoSaveTimer = storage.setupAutoSave(storyMeta.value.storyId || "NewStory", () => ({
    nodes: nodes.value,
    edges: edges.value,
    meta: storyMeta.value,
    variables: variables.value,
  }));
}

function syncEdgeDraftFromSelection() {
  if (!selectedEdge.value) return;
  edgeDraft.priority = selectedEdge.value.data?.priority ?? 0;
  edgeDraft.logic = selectedEdge.value.data?.condition?.logic || "All";
  edgeDraft.termsText = JSON.stringify(selectedEdge.value.data?.condition?.terms || [], null, 2);
}

function applyEdgeDraft() {
  if (!selectedEdge.value) return;
  let parsedTerms = [];
  try {
    parsedTerms = JSON.parse(edgeDraft.termsText || "[]");
    if (!Array.isArray(parsedTerms)) throw new Error("condition.terms 必须是数组");
  } catch (err) {
    alert(`边条件 JSON 格式错误: ${err.message}`);
    return;
  }

  handleEdgeUpdate({
    ...selectedEdge.value,
    data: {
      ...(selectedEdge.value.data || {}),
      priority: Number.isFinite(edgeDraft.priority) ? edgeDraft.priority : 0,
      condition: { logic: edgeDraft.logic || "All", terms: parsedTerms },
    },
  });
}

function resetEdgeDraft() {
  syncEdgeDraftFromSelection();
}

function clearEdgeSelection() {
  selectedEdge.value = null;
  applyEdgeVisualStyles();
}

function handleNodeClick(event) {
  selectedNode.value = event.detail ? safeClone(event.detail) : null;
  if (selectedNode.value) selectedEdge.value = null;
  applyEdgeVisualStyles();
}

function handleEdgeClick(event) {
  selectedEdge.value = event.detail ? normalizeEdge(safeClone(event.detail)) : null;
  if (selectedEdge.value) selectedNode.value = null;
  syncEdgeDraftFromSelection();
  applyEdgeVisualStyles();
}

function handleNodesChange(event) {
  const nextNodes = Array.isArray(event.detail) ? event.detail : [];
  if (!isDeepEqual(nextNodes, nodes.value)) nodes.value = safeClone(nextNodes);

  if (selectedNode.value) {
    const latestNode = nodes.value.find((n) => n.id === selectedNode.value.id);
    selectedNode.value = latestNode ? safeClone(latestNode) : null;
  }

  if (selectedEdge.value) {
    const latestEdge = edges.value.find((e) => e.id === selectedEdge.value.id);
    selectedEdge.value = latestEdge ? normalizeEdge(safeClone(latestEdge)) : null;
    syncEdgeDraftFromSelection();
  }

  applyEdgeVisualStyles();
}

function handleEdgesChange(event) {
  const incomingEdges = Array.isArray(event.detail) ? event.detail : [];
  const sanitized = sanitizeEdges(incomingEdges);
  if (!isDeepEqual(sanitized, edges.value)) edges.value = safeClone(sanitized);

  if (selectedEdge.value) {
    const latestEdge = edges.value.find((e) => e.id === selectedEdge.value.id);
    selectedEdge.value = latestEdge ? normalizeEdge(safeClone(latestEdge)) : null;
    syncEdgeDraftFromSelection();
  }

  applyEdgeVisualStyles();
}

function buildLayers(nodesInput, edgesInput, entryNodeId) {
  const nodeIds = nodesInput.map((n) => n.id);
  const inDegree = new Map(nodeIds.map((id) => [id, 0]));
  const outgoing = new Map(nodeIds.map((id) => [id, []]));

  edgesInput.forEach((e) => {
    if (inDegree.has(e.target) && outgoing.has(e.source)) {
      inDegree.set(e.target, inDegree.get(e.target) + 1);
      outgoing.get(e.source).push(e.target);
    }
  });

  const roots = nodeIds.filter((id) => inDegree.get(id) === 0);
  const orderedRoots = [
    ...(entryNodeId && roots.includes(entryNodeId) ? [entryNodeId] : []),
    ...roots.filter((id) => id !== entryNodeId),
  ];

  const queue = [...orderedRoots];
  const layer = new Map(nodeIds.map((id) => [id, 0]));
  const visited = new Set();

  while (queue.length > 0) {
    const cur = queue.shift();
    if (visited.has(cur)) continue;
    visited.add(cur);

    const curLayer = layer.get(cur) ?? 0;
    for (const next of outgoing.get(cur) || []) {
      layer.set(next, Math.max(layer.get(next) ?? 0, curLayer + 1));
      inDegree.set(next, inDegree.get(next) - 1);
      if (inDegree.get(next) <= 0) queue.push(next);
    }
  }

  // 处理有环或未访问节点
  let maxLayer = 0;
  layer.forEach((v) => {
    if (v > maxLayer) maxLayer = v;
  });

  nodeIds.forEach((id) => {
    if (!visited.has(id)) {
      maxLayer += 1;
      layer.set(id, maxLayer);
    }
  });

  return layer;
}

function handleAutoLayout() {
  if (nodes.value.length === 0) return;

  const rankSep = 340;
  const nodeSep = 170;
  const startX = 140;
  const startY = 110;

  const layerMap = buildLayers(nodes.value, edges.value, storyMeta.value.entryNodeId);

  const grouped = new Map();
  nodes.value.forEach((node) => {
    const l = layerMap.get(node.id) ?? 0;
    if (!grouped.has(l)) grouped.set(l, []);
    grouped.get(l).push(node);
  });

  grouped.forEach((list) => {
    list.sort((a, b) => (a.position?.y ?? 0) - (b.position?.y ?? 0));
  });

  const nextNodes = nodes.value.map((node) => {
    const l = layerMap.get(node.id) ?? 0;
    const list = grouped.get(l) || [];
    const idx = list.findIndex((n) => n.id === node.id);
    return {
      ...node,
      position: { x: startX + l * rankSep, y: startY + Math.max(idx, 0) * nodeSep },
    };
  });

  nodes.value = safeClone(nextNodes);
}

function handleToggleFocusMode() {
  focusModeEnabled.value = !focusModeEnabled.value;
  applyEdgeVisualStyles();
}

onMounted(() => {
  window.addEventListener("node-click", handleNodeClick);
  window.addEventListener("edge-click", handleEdgeClick);
  window.addEventListener("nodes-change", handleNodesChange);
  window.addEventListener("edges-change", handleEdgesChange);

  setupAutoSave();
  runRealtimeValidation();
  applyEdgeVisualStyles();
});

onUnmounted(() => {
  if (autoSaveTimer) clearInterval(autoSaveTimer);
  window.removeEventListener("node-click", handleNodeClick);
  window.removeEventListener("edge-click", handleEdgeClick);
  window.removeEventListener("nodes-change", handleNodesChange);
  window.removeEventListener("edges-change", handleEdgesChange);
});

watch(
  () => [nodes.value, edges.value, storyMeta.value.entryNodeId],
  () => runRealtimeValidation(),
  { deep: true, immediate: true },
);

watch(
  () => storyMeta.value.storyId,
  () => setupAutoSave(),
);

watch(
  () => [selectedNode.value?.id, selectedEdge.value?.id, focusModeEnabled.value],
  () => applyEdgeVisualStyles(),
);

function handleNew() {
  if (nodes.value.length > 0 && !confirm("创建新剧情将清空当前内容，是否继续？")) return;

  nodes.value = [];
  edges.value = [];
  selectedNode.value = null;
  selectedEdge.value = null;
  storyMeta.value = { storyId: "NewStory", entryNodeId: "", schemaVersion: 1 };
  variables.value = [];
  runRealtimeValidation();
}

function handleImport() {
  fileInput.value?.click();
}

function handleFileChange(event) {
  const file = event.target.files?.[0];
  if (!file) return;

  const reader = new FileReader();
  reader.onload = (e) => {
    try {
      const imported = importFromYAML(String(e.target?.result || ""));
      nodes.value = safeClone(imported.nodes || []);
      edges.value = safeClone(sanitizeEdges(imported.edges || []));
      storyMeta.value = {
        schemaVersion: imported.meta?.schemaVersion ?? 1,
        storyId: imported.meta?.storyId || "ImportedStory",
        entryNodeId: imported.meta?.entryNodeId || "",
      };
      variables.value = safeClone(imported.variables || []);
      selectedNode.value = null;
      selectedEdge.value = null;
      runRealtimeValidation();
      applyEdgeVisualStyles();
      alert(`导入成功！\n节点数: ${nodes.value.length}\n边数: ${edges.value.length}`);
    } catch (error) {
      alert(`导入失败: ${error.message}`);
    }
  };

  reader.readAsText(file);
  event.target.value = "";
}

function handleExport() {
  if (nodes.value.length === 0) {
    alert("没有可导出的内容");
    return;
  }

  try {
    exportToYAML(nodes.value, edges.value, variables.value, storyMeta.value);
    alert(`导出成功！\n节点数: ${nodes.value.length}\n边数: ${edges.value.length}`);
  } catch (error) {
    alert(`导出失败: ${error.message}`);
  }
}

function handleValidate() {
  runRealtimeValidation();
  const result = validationResult.value;
  if (result.errors.length === 0 && result.warnings.length === 0) {
    alert("验证通过！没有发现错误。");
    return;
  }

  let message = "验证结果：\n\n";
  if (result.errors.length > 0) {
    message += `错误 (${result.errors.length}):\n`;
    result.errors.forEach((err) => (message += `- ${err.message}\n`));
  }
  if (result.warnings.length > 0) {
    message += `\n警告 (${result.warnings.length}):\n`;
    result.warnings.forEach((warn) => (message += `- ${warn.message}\n`));
  }
  alert(message);
}

function handleHelp() {
  alert(
    "NarrRail Story Editor\n\n" +
      "快捷操作：\n" +
      "- 右键画布：添加节点\n" +
      "- 点击“自动排布”：按层级整理节点\n" +
      "- 点击“焦点模式”：弱化非相关连线\n" +
      "- 点击边：编辑边的优先级和条件\n" +
      "- Delete键：删除选中的节点或边",
  );
}

function handleNodeUpdate(updatedNode) {
  if (!updatedNode) return;

  const prevId = selectedNode.value?.id;
  let index = -1;
  if (prevId) index = nodes.value.findIndex((n) => n.id === prevId);
  if (index === -1) index = nodes.value.findIndex((n) => n.id === updatedNode.id);
  if (index === -1) return;

  const oldId = nodes.value[index].id;
  const nextNode = safeClone(updatedNode);
  if (isDeepEqual(nodes.value[index], nextNode)) return;
  nodes.value[index] = nextNode;

  let nextEdges = [...edges.value];
  if (oldId !== nextNode.id) {
    nextEdges = nextEdges.map((edge) => ({
      ...edge,
      source: edge.source === oldId ? nextNode.id : edge.source,
      target: edge.target === oldId ? nextNode.id : edge.target,
    }));

    if (storyMeta.value.entryNodeId === oldId) {
      storyMeta.value.entryNodeId = nextNode.id;
    }
  }

  nextEdges = syncChoiceEdgesForNode(nextNode, nextEdges);
  edges.value = safeClone(sanitizeEdges(nextEdges));

  if (selectedEdge.value) {
    const latestEdge = edges.value.find((e) => e.id === selectedEdge.value.id);
    selectedEdge.value = latestEdge ? normalizeEdge(safeClone(latestEdge)) : null;
    syncEdgeDraftFromSelection();
  }

  selectedNode.value = safeClone(nextNode);
  applyEdgeVisualStyles();
}

function handleEdgeUpdate(updatedEdge) {
  if (!updatedEdge) return;

  const edgeId = updatedEdge.id || selectedEdge.value?.id;
  if (!edgeId) return;

  const normalized = normalizeEdge({ ...updatedEdge, id: edgeId });
  const sourceNode = nodes.value.find((n) => n.id === normalized.source);
  const targetNode = nodes.value.find((n) => n.id === normalized.target);
  if (!sourceNode || !targetNode) {
    alert("边更新失败：源节点或目标节点不存在。");
    return;
  }

  if (!isValidChoiceHandle(sourceNode, normalized.sourceHandle)) {
    alert("边更新失败：Choice 节点 sourceHandle 无效。");
    return;
  }

  const index = edges.value.findIndex((e) => e.id === edgeId);
  const nextEdges = [...edges.value];
  if (index === -1) nextEdges.push(normalized);
  else nextEdges[index] = normalized;

  edges.value = safeClone(sanitizeEdges(nextEdges));
  const latestEdge = edges.value.find((e) => e.id === edgeId);
  selectedEdge.value = latestEdge ? safeClone(latestEdge) : null;
  syncEdgeDraftFromSelection();
  applyEdgeVisualStyles();
}

function handleSetEntryNode(nodeId) {
  storyMeta.value.entryNodeId = nodeId;
}

function handleVariablesUpdate(updatedVariables) {
  variables.value = safeClone(updatedVariables || []);
}
</script>

<style scoped>
.editor-container {
  width: 100vw;
  height: 100vh;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  padding: 0;
  position: relative;
}

.main-content {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  display: flex;
  overflow: hidden;
}

.graph-editor-wrapper {
  flex: 1;
  position: relative;
}

.edge-editor {
  position: fixed;
  right: 16px;
  top: 100px;
  width: 360px;
  max-height: calc(100vh - 190px);
  overflow: auto;
  z-index: 60;
  padding: 16px;
  border-radius: 16px;
}

.edge-editor-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 12px;
}

.edge-editor-header h3 {
  margin: 0;
  font-size: 18px;
  font-weight: 700;
}

.close-btn {
  border: none;
  border-radius: 8px;
  width: 28px;
  height: 28px;
  cursor: pointer;
  background: rgba(0, 0, 0, 0.06);
}

.form-group {
  display: flex;
  flex-direction: column;
  gap: 6px;
  margin-bottom: 12px;
}

.form-group label {
  font-size: 12px;
  font-weight: 700;
  color: #475569;
}

.form-input,
.form-textarea {
  width: 100%;
  border: none;
  border-radius: 8px;
  background: rgba(255, 255, 255, 0.55);
  padding: 10px 12px;
  font-size: 14px;
  color: #1e293b;
}

.form-input:focus,
.form-textarea:focus {
  outline: 2px solid rgba(59, 130, 246, 0.35);
}

.form-textarea {
  min-height: 100px;
  resize: vertical;
}

.edge-editor-actions {
  display: flex;
  gap: 8px;
}

.action-btn {
  border: none;
  border-radius: 8px;
  padding: 10px 14px;
  cursor: pointer;
  background: rgba(0, 0, 0, 0.08);
  font-weight: 600;
}

.action-btn.primary {
  background: rgba(59, 130, 246, 0.22);
  color: #1d4ed8;
}

.validation-badge {
  position: fixed;
  right: 16px;
  bottom: 68px;
  z-index: 70;
  padding: 8px 12px;
  border-radius: 12px;
  background: rgba(52, 199, 89, 0.18);
  color: #166534;
  font-size: 12px;
  font-weight: 700;
  backdrop-filter: blur(10px);
}

.validation-badge.warning {
  background: rgba(245, 158, 11, 0.22);
  color: #92400e;
}

.validation-badge.error {
  background: rgba(239, 68, 68, 0.22);
  color: #991b1b;
}
</style>
