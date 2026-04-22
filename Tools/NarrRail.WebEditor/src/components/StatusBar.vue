<template>
  <div class="status-bar">
    <div class="status-item">
      <span>节点数: {{ nodeCount }}</span>
    </div>
    <div class="status-item">
      <span>边数: {{ edgeCount }}</span>
    </div>
    <div class="status-item">
      <span>剧情 ID: {{ storyId }}</span>
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
  height: 30px;
  background: #34495e;
  color: white;
  display: flex;
  align-items: center;
  padding: 0 20px;
  font-size: 12px;
  gap: 20px;
}

.status-item {
  display: flex;
  align-items: center;
  gap: 5px;
}
</style>
