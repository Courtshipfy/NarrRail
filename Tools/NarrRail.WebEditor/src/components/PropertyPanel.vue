<template>
  <div class="property-panel-wrapper">
    <!-- 触发热区（始终存在，透明） -->
    <div
      class="panel-hotzone"
      @mouseenter="handleMouseEnter"
    ></div>

    <!-- 收起时的箭头按钮 -->
    <div class="panel-trigger" :class="{ 'hidden': isExpanded }">
      <span class="material-symbols-outlined">chevron_left</span>
    </div>

    <!-- 展开的面板 -->
    <div
      class="property-panel glass-morphism-strong"
      :class="{ 'expanded': isExpanded }"
      @mouseleave="handleMouseLeave"
    >
      <div class="property-panel-header">
        <h2 class="panel-title">属性面板</h2>
        <p class="panel-subtitle">Inspector</p>
      </div>

      <div class="property-panel-content">
        <div v-if="selectedNode" class="property-form">
          <div class="form-group glass-input">
            <label class="form-label">节点 ID</label>
            <input
              type="text"
              class="form-input"
              :value="selectedNode.id"
              @input="updateNodeId"
            />
          </div>

          <div class="form-group glass-input">
            <label class="form-label">节点类型</label>
            <input
              type="text"
              class="form-input"
              :value="selectedNode.type"
              readonly
            />
          </div>

          <template v-if="selectedNode.type === 'dialogue'">
            <div class="form-group glass-input">
              <label class="form-label">说话人 ID</label>
              <input
                type="text"
                class="form-input"
                :value="selectedNode.data.speakerId || ''"
                @input="updateData('speakerId', $event.target.value)"
              />
            </div>

            <div class="form-group glass-input">
              <label class="form-label">对话文本</label>
              <textarea
                class="form-textarea"
                :value="selectedNode.data.textKey || ''"
                @input="updateData('textKey', $event.target.value)"
              ></textarea>
            </div>

            <div class="form-group glass-input">
              <label class="form-label">语速</label>
              <input
                type="number"
                class="form-input"
                :value="selectedNode.data.speechRate || 1.0"
                step="0.1"
                @input="updateData('speechRate', parseFloat($event.target.value))"
              />
            </div>

            <div class="form-group glass-input">
              <label class="form-label">语音资产</label>
              <input
                type="text"
                class="form-input"
                :value="selectedNode.data.voiceAsset || ''"
                @input="updateData('voiceAsset', $event.target.value)"
              />
            </div>
          </template>

          <button class="update-button bouncy-feedback spring-animation">
            <span class="material-symbols-outlined">check_circle</span>
            <span>更新属性</span>
          </button>

          <p class="dev-notice">其他节点类型开发中...</p>
        </div>
        <div v-else class="property-panel-empty">
          <span class="material-symbols-outlined empty-icon">touch_app</span>
          <p>点击节点以编辑属性</p>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue';

const props = defineProps({
  selectedNode: {
    type: Object,
    default: null
  }
});

const emit = defineEmits(['update']);

const isExpanded = ref(false);

function handleMouseEnter() {
  isExpanded.value = true;
}

function handleMouseLeave() {
  isExpanded.value = false;
}

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
.property-panel-wrapper {
  position: fixed;
  right: 0;
  top: 0;
  bottom: 0;
  z-index: 40;
  pointer-events: none;
}

/* 触发热区 - 始终存在，透明 */
.panel-hotzone {
  position: absolute;
  right: 0;
  top: 0;
  bottom: 0;
  width: 48px;
  pointer-events: auto;
  cursor: pointer;
}

/* 箭头按钮 */
.panel-trigger {
  position: absolute;
  right: 0;
  top: 50%;
  transform: translateY(-50%);
  width: 48px;
  height: 96px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(255, 255, 255, 0.2);
  backdrop-filter: blur(16px);
  border-radius: 16px 0 0 16px;
  border: 1px solid rgba(255, 255, 255, 0.6);
  border-right: none;
  cursor: pointer;
  transition: all 0.3s ease;
  box-shadow: -4px 0 16px rgba(0, 0, 0, 0.05);
  pointer-events: none;
}

.panel-trigger.hidden {
  opacity: 0;
}

.panel-hotzone:hover ~ .panel-trigger:not(.hidden) {
  background: rgba(255, 255, 255, 0.3);
  width: 56px;
}

.panel-trigger .material-symbols-outlined {
  font-size: 28px;
  color: #3b82f6;
  animation: pulse-arrow 2s ease-in-out infinite;
}

@keyframes pulse-arrow {
  0%, 100% {
    transform: translateX(0);
  }
  50% {
    transform: translateX(-4px);
  }
}

/* 面板本体 */
.property-panel {
  position: absolute;
  right: 0;
  top: 80px;
  bottom: 48px;
  width: 320px;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  border-radius: 16px 0 0 16px;
  transform: translateX(100%);
  transition: transform 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
  pointer-events: none;
}

.property-panel.expanded {
  transform: translateX(0);
  pointer-events: auto;
}

.property-panel-header {
  padding: 24px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.3);
}

.panel-title {
  font-family: 'Plus Jakarta Sans', sans-serif;
  font-size: 24px;
  font-weight: 700;
  color: #3b82f6;
  margin-bottom: 4px;
}

.panel-subtitle {
  font-size: 12px;
  font-weight: 600;
  color: #94a3b8;
  text-transform: uppercase;
  letter-spacing: 0.05em;
}

.property-panel-content {
  flex: 1;
  padding: 24px;
  overflow-y: auto;
}

.property-panel-empty {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 100%;
  color: #94a3b8;
  text-align: center;
  gap: 16px;
}

.empty-icon {
  font-size: 64px;
  opacity: 0.3;
}

.property-form {
  display: flex;
  flex-direction: column;
  gap: 20px;
}

.form-group {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.glass-input {
  background: rgba(147, 197, 253, 0.15);
  backdrop-filter: blur(16px);
  padding: 16px;
  border-radius: 12px;
  border: 1px solid rgba(255, 255, 255, 0.6);
}

.form-label {
  font-size: 10px;
  font-weight: 800;
  color: #3b82f6;
  text-transform: uppercase;
  letter-spacing: 0.1em;
  font-family: 'Plus Jakarta Sans', sans-serif;
}

.form-input,
.form-textarea {
  width: 100%;
  padding: 12px 16px;
  background: rgba(255, 255, 255, 0.4);
  border: none;
  border-radius: 8px;
  font-size: 14px;
  font-weight: 600;
  color: #1e293b;
  font-family: 'Be Vietnam Pro', sans-serif;
  transition: all 0.3s ease;
}

.form-input:focus,
.form-textarea:focus {
  outline: none;
  background: rgba(255, 255, 255, 0.6);
  box-shadow: 0 0 0 2px rgba(59, 130, 246, 0.3);
}

.form-textarea {
  min-height: 100px;
  resize: vertical;
}

.update-button {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
  width: 100%;
  padding: 16px;
  background: linear-gradient(135deg, rgba(59, 130, 246, 0.8), rgba(168, 85, 247, 0.8));
  color: white;
  border: 1px solid rgba(255, 255, 255, 0.4);
  border-radius: 9999px;
  cursor: pointer;
  font-size: 14px;
  font-weight: 800;
  font-family: 'Plus Jakarta Sans', sans-serif;
  box-shadow: 0 12px 32px rgba(59, 130, 246, 0.3),
              inset 0 1px 2px rgba(255, 255, 255, 0.5);
}

.update-button:hover {
  transform: scale(1.02);
}

.update-button .material-symbols-outlined {
  font-size: 20px;
}

.dev-notice {
  color: #94a3b8;
  font-size: 11px;
  text-align: center;
  font-style: italic;
  margin-top: 8px;
}
</style>
