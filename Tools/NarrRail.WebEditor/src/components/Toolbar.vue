<template>
  <div class="toolbar">
    <div class="toolbar-title">NarrRail Story Editor</div>
    <button class="toolbar-button" @click="$emit('new')">新建</button>
    <button class="toolbar-button" @click="$emit('import')">导入</button>
    <button class="toolbar-button" @click="$emit('export')">导出</button>
    <button class="toolbar-button secondary" @click="$emit('validate')">验证</button>
    <button class="toolbar-button secondary" @click="$emit('help')">帮助</button>
  </div>
</template>

<script setup>
defineEmits(['new', 'import', 'export', 'validate', 'help']);
</script>

<style scoped>
.toolbar {
  height: 50px;
  background: #2c3e50;
  color: white;
  display: flex;
  align-items: center;
  padding: 0 20px;
  gap: 10px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
}

.toolbar-title {
  font-size: 18px;
  font-weight: 600;
  margin-right: 20px;
}

.toolbar-button {
  padding: 6px 16px;
  background: #3498db;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
  transition: background 0.2s;
}

.toolbar-button:hover {
  background: #2980b9;
}

.toolbar-button:active {
  background: #21618c;
}

.toolbar-button.secondary {
  background: #95a5a6;
}

.toolbar-button.secondary:hover {
  background: #7f8c8d;
}
</style>
