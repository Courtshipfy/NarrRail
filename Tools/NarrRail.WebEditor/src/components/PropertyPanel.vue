<template>
  <div class="property-panel">
    <div class="property-panel-header">属性面板</div>
    <div class="property-panel-content">
      <div v-if="selectedNode" class="property-form">
        <div class="form-group">
          <label class="form-label">节点 ID</label>
          <input
            type="text"
            class="form-input"
            :value="selectedNode.id"
            @input="updateNodeId"
          />
        </div>

        <div class="form-group">
          <label class="form-label">节点类型</label>
          <input
            type="text"
            class="form-input"
            :value="selectedNode.type"
            readonly
          />
        </div>

        <template v-if="selectedNode.type === 'dialogue'">
          <div class="form-group">
            <label class="form-label">说话人 ID</label>
            <input
              type="text"
              class="form-input"
              :value="selectedNode.data.speakerId || ''"
              @input="updateData('speakerId', $event.target.value)"
            />
          </div>

          <div class="form-group">
            <label class="form-label">对话文本</label>
            <textarea
              class="form-textarea"
              :value="selectedNode.data.textKey || ''"
              @input="updateData('textKey', $event.target.value)"
            ></textarea>
          </div>

          <div class="form-group">
            <label class="form-label">语速</label>
            <input
              type="number"
              class="form-input"
              :value="selectedNode.data.speechRate || 1.0"
              step="0.1"
              @input="updateData('speechRate', parseFloat($event.target.value))"
            />
          </div>

          <div class="form-group">
            <label class="form-label">语音资产</label>
            <input
              type="text"
              class="form-input"
              :value="selectedNode.data.voiceAsset || ''"
              @input="updateData('voiceAsset', $event.target.value)"
            />
          </div>
        </template>

        <p class="dev-notice">其他节点类型的属性编辑功能开发中...</p>
      </div>
      <div v-else class="property-panel-empty">
        点击节点以编辑属性
      </div>
    </div>
  </div>
</template>

<script setup>
const props = defineProps({
  selectedNode: {
    type: Object,
    default: null
  }
});

const emit = defineEmits(['update']);

function updateNodeId(event) {
  if (!props.selectedNode) return;
  const updated = { ...props.selectedNode, id: event.target.value };
  emit('update', updated);
}

function updateData(key, value) {
  if (!props.selectedNode) return;
  const updated = {
    ...props.selectedNode,
    data: {
      ...props.selectedNode.data,
      [key]: value
    }
  };
  emit('update', updated);
}
</script>

<style scoped>
.property-panel {
  width: 300px;
  background: white;
  border-left: 1px solid #ddd;
  display: flex;
  flex-direction: column;
  overflow: hidden;
}

.property-panel-header {
  padding: 15px;
  background: #34495e;
  color: white;
  font-weight: 600;
}

.property-panel-content {
  flex: 1;
  padding: 15px;
  overflow-y: auto;
}

.property-panel-empty {
  color: #95a5a6;
  text-align: center;
  padding: 40px 20px;
}

.property-form {
  display: flex;
  flex-direction: column;
  gap: 15px;
}

.form-group {
  display: flex;
  flex-direction: column;
  gap: 5px;
}

.form-label {
  font-size: 13px;
  font-weight: 500;
  color: #2c3e50;
}

.form-input,
.form-textarea {
  width: 100%;
  padding: 8px;
  border: 1px solid #ddd;
  border-radius: 4px;
  font-size: 13px;
  font-family: inherit;
}

.form-textarea {
  min-height: 80px;
  resize: vertical;
}

.form-input:focus,
.form-textarea:focus {
  outline: none;
  border-color: #3498db;
}

.dev-notice {
  color: #95a5a6;
  font-size: 12px;
  margin-top: 10px;
  font-style: italic;
}
</style>
