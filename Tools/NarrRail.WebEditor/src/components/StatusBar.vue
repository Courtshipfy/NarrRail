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
  height: 48px;
  display: flex;
  align-items: center;
  padding: 0 32px;
  gap: 32px;
  border-radius: 32px 32px 0 0;
}

.status-item {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 13px;
  font-weight: 600;
  color: #64748b;
  font-family: 'Plus Jakarta Sans', sans-serif;
}

.status-item .material-symbols-outlined {
  font-size: 18px;
  color: #94a3b8;
}

.status-item.auto-save {
  margin-left: auto;
  color: #10b981;
}

.status-item.auto-save .material-symbols-outlined {
  color: #10b981;
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
