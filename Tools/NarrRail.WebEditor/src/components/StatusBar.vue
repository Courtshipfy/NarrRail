<template>
  <div class="status-bar glass-morphism">
    <div class="status-item">
      <span class="material-symbols-outlined">account_tree</span>
      <span>节点: {{ nodeCount }}</span>
    </div>
    <div class="status-item">
      <span class="material-symbols-outlined">share</span>
      <span>边: {{ edgeCount }}</span>
    </div>
    <div class="status-item">
      <span class="material-symbols-outlined">label</span>
      <span>{{ storyId }}</span>
    </div>
    <div class="status-item auto-save">
      <span class="material-symbols-outlined pulse">cloud_done</span>
      <span>自动保存</span>
    </div>
  </div>
</template>

<script setup>
defineProps({
  nodeCount: {
    type: Number,
    default: 0
  },
  edgeCount: {
    type: Number,
    default: 0
  },
  storyId: {
    type: String,
    default: 'NewStory'
  }
});
</script>

<style scoped>
.status-bar {
  position: fixed;
  bottom: 16px;
  left: 16px;
  right: 16px;
  height: 44px;
  display: flex;
  align-items: center;
  padding: 0 32px;
  gap: 32px;
  border-radius: 20px;
  border: none;
  z-index: 50;
}

.status-item {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 13px;
  font-weight: 500;
  color: #1d1d1f;
  font-family: 'SF Pro Text', -apple-system, BlinkMacSystemFont, sans-serif;
  opacity: 0.7;
}

.status-item .material-symbols-outlined {
  font-size: 18px;
  opacity: 0.6;
}

.status-item.auto-save {
  margin-left: auto;
  color: #34c759;
  opacity: 1;
}

.status-item.auto-save .material-symbols-outlined {
  color: #34c759;
  opacity: 1;
}

@keyframes pulse {
  0%, 100% {
    opacity: 1;
  }
  50% {
    opacity: 0.5;
  }
}

.pulse {
  animation: pulse 2s ease-in-out infinite;
}
</style>
