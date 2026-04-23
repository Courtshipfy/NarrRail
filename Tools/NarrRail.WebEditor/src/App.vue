<template>
  <div class="editor-container">
    <Toolbar
      @new="handleNew"
      @import="handleImport"
      @export="handleExport"
      @validate="handleValidate"
      @help="handleHelp"
    />

    <div class="main-content">
      <div class="graph-editor-wrapper dot-grid">
        <div class="graph-placeholder">
          <div class="placeholder-content jelly-node glass-morphism-strong bouncy-feedback scale-up-hover spring-animation">
            <span class="material-symbols-outlined placeholder-icon">auto_graph</span>
            <h3>图编辑器</h3>
            <p>节点数: {{ nodes.length }} | 边数: {{ edges.length }}</p>
            <div class="placeholder-hint">
              <span class="material-symbols-outlined">info</span>
              <span>Svelte Flow 集成开发中</span>
            </div>
          </div>
        </div>
      </div>
    </div>

    <StatusBar
      :node-count="nodes.length"
      :edge-count="edges.length"
      :story-id="storyMeta.storyId"
    />

    <PropertyPanel
      :selected-node="selectedNode"
      @update="handleNodeUpdate"
    />

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
import { ref, onMounted, onUnmounted } from 'vue';
import Toolbar from './components/Toolbar.vue';
import PropertyPanel from './components/PropertyPanel.vue';
import StatusBar from './components/StatusBar.vue';
import { exportToYAML } from './utils/yaml-exporter.js';
import { importFromYAML } from './utils/yaml-importer.js';
import { validateStory } from './utils/validation.js';
import storage from './utils/storage.js';

const nodes = ref([]);
const edges = ref([]);
const selectedNode = ref(null);
const storyMeta = ref({
  storyId: 'NewStory',
  entryNodeId: '',
  schemaVersion: 1
});
const variables = ref([]);
const fileInput = ref(null);

let autoSaveTimer = null;

onMounted(() => {
  console.log('App mounted');

  // 尝试加载上次编辑的剧情
  const lastStory = storage.load(storyMeta.value.storyId);
  if (lastStory) {
    nodes.value = lastStory.nodes || [];
    edges.value = lastStory.edges || [];
    storyMeta.value = lastStory.meta || storyMeta.value;
    variables.value = lastStory.variables || [];
  }

  // 启动自动保存
  autoSaveTimer = storage.setupAutoSave(
    storyMeta.value.storyId,
    () => ({
      nodes: nodes.value,
      edges: edges.value,
      meta: storyMeta.value,
      variables: variables.value
    })
  );
});

onUnmounted(() => {
  if (autoSaveTimer) {
    clearInterval(autoSaveTimer);
  }
});

function handleNew() {
  if (nodes.value.length > 0 && !confirm('创建新剧情将清空当前内容，是否继续？')) {
    return;
  }
  nodes.value = [];
  edges.value = [];
  selectedNode.value = null;
  storyMeta.value = {
    storyId: 'NewStory',
    entryNodeId: '',
    schemaVersion: 1
  };
  variables.value = [];
}

function handleImport() {
  fileInput.value.click();
}

function handleFileChange(event) {
  const file = event.target.files[0];
  if (!file) return;

  const reader = new FileReader();
  reader.onload = (e) => {
    try {
      const yamlContent = e.target.result;
      const imported = importFromYAML(yamlContent);

      nodes.value = imported.nodes;
      edges.value = imported.edges;
      storyMeta.value = imported.meta;
      variables.value = imported.variables;
      selectedNode.value = null;

      alert(`导入成功！\n节点数: ${imported.nodes.length}\n边数: ${imported.edges.length}`);
    } catch (error) {
      alert('导入失败: ' + error.message);
      console.error(error);
    }
  };
  reader.readAsText(file);
  event.target.value = '';
}

function handleExport() {
  if (nodes.value.length === 0) {
    alert('没有可导出的内容');
    return;
  }

  try {
    exportToYAML(nodes.value, edges.value, variables.value, storyMeta.value);
    alert('导出成功！');
  } catch (error) {
    alert('导出失败: ' + error.message);
    console.error(error);
  }
}

function handleValidate() {
  if (nodes.value.length === 0) {
    alert('没有可验证的内容');
    return;
  }

  const result = validateStory(nodes.value, edges.value, storyMeta.value);

  if (result.errors.length === 0 && result.warnings.length === 0) {
    alert('验证通过！没有发现错误。');
  } else {
    let message = '验证结果：\n\n';

    if (result.errors.length > 0) {
      message += `错误 (${result.errors.length}):\n`;
      result.errors.forEach(err => {
        message += `- ${err.message}\n`;
      });
    }

    if (result.warnings.length > 0) {
      message += `\n警告 (${result.warnings.length}):\n`;
      result.warnings.forEach(warn => {
        message += `- ${warn.message}\n`;
      });
    }

    alert(message);
  }
}

function handleHelp() {
  alert(
    'NarrRail Story Editor\n\n' +
    '快捷操作：\n' +
    '- 右键画布：添加节点（开发中）\n' +
    '- 拖拽节点：移动位置\n' +
    '- 连接节点：拖拽节点边缘的圆点\n' +
    '- 点击节点：编辑属性\n' +
    '- Delete键：删除选中的节点或边\n\n' +
    '更多帮助请访问项目文档'
  );
}

function handleNodeUpdate(updatedNode) {
  const index = nodes.value.findIndex(n => n.id === updatedNode.id);
  if (index !== -1) {
    nodes.value[index] = updatedNode;
  }
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
}

.main-content {
  flex: 1;
  display: flex;
  overflow: hidden;
  padding: 0 16px;
}

.graph-editor-wrapper {
  flex: 1;
  position: relative;
  margin-right: 0;
  border-radius: 20px;
  overflow: hidden;
}

.graph-placeholder {
  width: 100%;
  height: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 32px;
}

.placeholder-content {
  text-align: center;
  padding: 48px;
  border-radius: 24px;
  cursor: pointer;
  max-width: 400px;
}

.placeholder-icon {
  font-size: 80px;
  color: #a855f7;
  margin-bottom: 16px;
  display: block;
  opacity: 0.6;
}

.placeholder-content h3 {
  font-family: 'Plus Jakarta Sans', sans-serif;
  font-size: 32px;
  font-weight: 700;
  color: #1d1d1f;
  margin-bottom: 12px;
}

.placeholder-content p {
  font-size: 16px;
  color: #86868b;
  font-weight: 500;
  margin-bottom: 24px;
}

.placeholder-hint {
  display: inline-flex;
  align-items: center;
  gap: 8px;
  padding: 8px 16px;
  background: rgba(168, 85, 247, 0.08);
  border-radius: 12px;
  font-size: 12px;
  font-weight: 600;
  color: #a855f7;
  border: 0.5px solid rgba(168, 85, 247, 0.1);
}

.placeholder-hint .material-symbols-outlined {
  font-size: 16px;
}
</style>
