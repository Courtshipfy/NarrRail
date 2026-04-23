<template>
  <div class="property-panel-wrapper">
    <!-- 展开的面板 -->
    <div
      class="property-panel glass-morphism-strong"
      :class="{ 'expanded': isExpanded }"
    >
      <div class="property-panel-header">
        <h2 class="panel-title">属性面板</h2>
        <p class="panel-subtitle">Inspector</p>
      </div>

      <div class="property-panel-content">
        <div v-if="localNode" class="property-form">
          <div class="form-group glass-input">
            <label class="form-label">节点 ID</label>
            <input
              type="text"
              class="form-input"
              v-model="localNode.id"
              @compositionstart="handleCompositionStart"
              @compositionend="handleCompositionEnd"
              @blur="handleInputChange"
            />
          </div>

          <div class="form-group glass-input">
            <label class="form-label">节点类型</label>
            <input
              type="text"
              class="form-input"
              :value="localNode.type"
              readonly
            />
          </div>

          <template v-if="localNode.type === 'dialogue'">
            <div class="form-group glass-input">
              <label class="form-label">说话人 ID</label>
              <input
                type="text"
                class="form-input"
                v-model="localNode.data.speakerId"
                @compositionstart="handleCompositionStart"
                @compositionend="handleCompositionEnd"
                @blur="handleInputChange"
              />
            </div>

            <div class="form-group glass-input">
              <label class="form-label">对话文本</label>
              <textarea
                class="form-textarea"
                v-model="localNode.data.textKey"
                @compositionstart="handleCompositionStart"
                @compositionend="handleCompositionEnd"
                @blur="handleInputChange"
              ></textarea>
            </div>

            <div class="form-group glass-input">
              <label class="form-label">语速</label>
              <input
                type="number"
                class="form-input"
                v-model.number="localNode.data.speechRate"
                step="0.1"
                @blur="handleUpdate"
              />
            </div>

            <div class="form-group glass-input">
              <label class="form-label">语音资产</label>
              <input
                type="text"
                class="form-input"
                v-model="localNode.data.voiceAsset"
                @compositionstart="handleCompositionStart"
                @compositionend="handleCompositionEnd"
                @blur="handleInputChange"
              />
            </div>
          </template>

          <template v-else-if="localNode.type === 'choice'">
            <div class="form-group glass-input">
              <label class="form-label">选项列表</label>
              <div v-for="(choice, index) in localNode.data.choices" :key="index" class="choice-item">
                <input
                  type="text"
                  class="form-input"
                  v-model="choice.textKey"
                  placeholder="选项文本"
                  @compositionstart="handleCompositionStart"
                  @compositionend="handleCompositionEnd"
                  @blur="handleInputChange"
                />
                <input
                  type="text"
                  class="form-input"
                  v-model="choice.targetNodeId"
                  placeholder="目标节点ID"
                  @compositionstart="handleCompositionStart"
                  @compositionend="handleCompositionEnd"
                  @blur="handleInputChange"
                />
                <button class="remove-choice-btn" @click="removeChoice(index)">
                  <span class="material-symbols-outlined">close</span>
                </button>
              </div>
              <button class="add-choice-btn" @click="addChoice">
                <span class="material-symbols-outlined">add</span>
                <span>添加选项</span>
              </button>
            </div>
          </template>

          <template v-else-if="localNode.type === 'jump'">
            <div class="form-group glass-input">
              <label class="form-label">目标节点 ID</label>
              <input
                type="text"
                class="form-input"
                v-model="localNode.data.targetNodeId"
                @compositionstart="handleCompositionStart"
                @compositionend="handleCompositionEnd"
                @blur="handleInputChange"
              />
            </div>
          </template>

          <template v-else-if="localNode.type === 'setvariable'">
            <div class="form-group glass-input">
              <label class="form-label">变量名</label>
              <input
                type="text"
                class="form-input"
                v-model="localNode.data.variableName"
                @compositionstart="handleCompositionStart"
                @compositionend="handleCompositionEnd"
                @blur="handleInputChange"
              />
            </div>

            <div class="form-group glass-input">
              <label class="form-label">操作</label>
              <select
                class="form-input"
                v-model="localNode.data.operation"
                @change="handleUpdate"
              >
                <option value="Set">Set (设置)</option>
                <option value="Add">Add (增加)</option>
                <option value="Subtract">Subtract (减少)</option>
                <option value="Multiply">Multiply (乘以)</option>
                <option value="Divide">Divide (除以)</option>
              </select>
            </div>

            <div class="form-group glass-input">
              <label class="form-label">值</label>
              <input
                type="text"
                class="form-input"
                v-model="localNode.data.value"
                @compositionstart="handleCompositionStart"
                @compositionend="handleCompositionEnd"
                @blur="handleInputChange"
              />
            </div>
          </template>

          <template v-else-if="localNode.type === 'emitevent'">
            <div class="form-group glass-input">
              <label class="form-label">事件 ID</label>
              <input
                type="text"
                class="form-input"
                v-model="localNode.data.eventId"
                @compositionstart="handleCompositionStart"
                @compositionend="handleCompositionEnd"
                @blur="handleInputChange"
              />
            </div>

            <div class="form-group glass-input">
              <label class="form-label">参数 (JSON)</label>
              <textarea
                class="form-textarea"
                :value="JSON.stringify(localNode.data.parameters || {}, null, 2)"
                @input="updateParameters($event.target.value)"
                @compositionstart="handleCompositionStart"
                @compositionend="handleCompositionEnd"
                @blur="handleInputChange"
                placeholder='{"key": "value"}'
              ></textarea>
            </div>
          </template>

          <button class="update-button bouncy-feedback spring-animation" @click="handleUpdate">
            <span class="material-symbols-outlined">check_circle</span>
            <span>更新属性</span>
          </button>
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
import { ref, watch } from 'vue';

const props = defineProps({
  selectedNode: {
    type: Object,
    default: null
  }
});

const emit = defineEmits(['update']);

const isExpanded = ref(false);
const localNode = ref(null);
const isComposing = ref(false);

// 监听 selectedNode 变化，同步到本地副本
watch(() => props.selectedNode, (newNode) => {
  if (newNode) {
    localNode.value = JSON.parse(JSON.stringify(newNode));
    isExpanded.value = true; // 选中节点时自动展开
  } else {
    localNode.value = null;
    isExpanded.value = false; // 取消选中时自动收起
  }
}, { immediate: true, deep: true });

// 处理中文输入法
function handleCompositionStart() {
  isComposing.value = true;
}

function handleCompositionEnd() {
  isComposing.value = false;
}

function handleInputChange() {
  // 只在非输入法状态下保存
  if (!isComposing.value) {
    handleUpdate();
  }
}

function updateNodeId(event) {
  if (!localNode.value) return;
  localNode.value.id = event.target.value;
}

function updateData(key, value) {
  if (!localNode.value) return;
  localNode.value.data = {
    ...localNode.value.data,
    [key]: value
  };
}

function addChoice() {
  if (!localNode.value || localNode.value.type !== 'choice') return;
  const choices = [...(localNode.value.data.choices || [])];
  choices.push({ textKey: '', targetNodeId: '' });
  localNode.value.data.choices = choices;
}

function removeChoice(index) {
  if (!localNode.value || localNode.value.type !== 'choice') return;
  const choices = [...(localNode.value.data.choices || [])];
  choices.splice(index, 1);
  localNode.value.data.choices = choices;
}

function updateChoice(index, key, value) {
  if (!localNode.value || localNode.value.type !== 'choice') return;
  const choices = [...(localNode.value.data.choices || [])];
  choices[index] = { ...choices[index], [key]: value };
  localNode.value.data.choices = choices;
}

function updateParameters(jsonString) {
  if (!localNode.value) return;
  try {
    const params = JSON.parse(jsonString);
    localNode.value.data.parameters = params;
  } catch (e) {
    // 忽略无效的 JSON
  }
}

function handleUpdate() {
  // 提交本地修改到父组件
  if (localNode.value) {
    emit('update', localNode.value);
  }
}
</script>

<style scoped>
.property-panel-wrapper {
  position: fixed;
  right: 0;
  top: 0;
  bottom: 0;
  z-index: 30;
  pointer-events: none;
}

/* 面板本体 */
.property-panel {
  position: absolute;
  right: 16px;
  top: 88px;
  bottom: 92px;
  width: 320px;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  border-radius: 16px;
  transform: translateX(calc(100% + 32px));
  opacity: 0;
  transition: all 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
  pointer-events: none;
}

.property-panel.expanded {
  transform: translateX(0);
  opacity: 1;
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

.choice-item {
  display: flex;
  flex-direction: column;
  gap: 8px;
  padding: 12px;
  background: rgba(255, 255, 255, 0.3);
  border-radius: 8px;
  margin-bottom: 8px;
  position: relative;
}

.remove-choice-btn {
  position: absolute;
  top: 8px;
  right: 8px;
  width: 24px;
  height: 24px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(239, 68, 68, 0.8);
  border: none;
  border-radius: 50%;
  cursor: pointer;
  transition: all 0.2s ease;
}

.remove-choice-btn:hover {
  background: rgba(239, 68, 68, 1);
  transform: scale(1.1);
}

.remove-choice-btn .material-symbols-outlined {
  font-size: 16px;
  color: white;
}

.add-choice-btn {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 6px;
  width: 100%;
  padding: 10px;
  background: rgba(59, 130, 246, 0.1);
  border: 1px dashed rgba(59, 130, 246, 0.4);
  border-radius: 8px;
  cursor: pointer;
  font-size: 13px;
  font-weight: 600;
  color: #3b82f6;
  transition: all 0.2s ease;
}

.add-choice-btn:hover {
  background: rgba(59, 130, 246, 0.2);
  border-color: rgba(59, 130, 246, 0.6);
}

.add-choice-btn .material-symbols-outlined {
  font-size: 18px;
}
</style>
