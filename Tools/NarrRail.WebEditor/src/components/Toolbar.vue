<template>
  <div class="toolbar glass-morphism">
    <div class="toolbar-title gradient-text">NarrRail Story Editor</div>
    <button class="toolbar-button bouncy-feedback spring-animation" @click="$emit('new')">
      <span class="material-symbols-outlined">add_circle</span>
      <span>新建</span>
    </button>
    <button class="toolbar-button bouncy-feedback spring-animation" @click="$emit('import')">
      <span class="material-symbols-outlined">upload_file</span>
      <span>导入</span>
    </button>
    <button class="toolbar-button bouncy-feedback spring-animation" @click="$emit('export')">
      <span class="material-symbols-outlined">download</span>
      <span>导出</span>
    </button>
    <button class="toolbar-button secondary bouncy-feedback spring-animation" @click="$emit('validate')">
      <span class="material-symbols-outlined">verified</span>
      <span>验证</span>
    </button>
    <button class="toolbar-button secondary bouncy-feedback spring-animation" @click="$emit('help')">
      <span class="material-symbols-outlined">help</span>
      <span>帮助</span>
    </button>
  </div>
</template>

<script setup>
defineEmits(['new', 'import', 'export', 'validate', 'help']);
</script>

<style scoped>
.toolbar {
  position: fixed;
  top: 16px;
  left: 16px;
  right: 16px;
  height: 72px;
  display: flex;
  align-items: center;
  padding: 0 32px;
  gap: 16px;
  border-radius: 20px;
  z-index: 50;
  border: none;
}

.toolbar-title {
  font-family: 'Plus Jakarta Sans', sans-serif;
  font-size: 22px;
  font-weight: 800;
  margin-right: 32px;
  letter-spacing: -0.02em;
}

.toolbar-button {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 10px 20px;
  background: rgba(255, 255, 255, 0.6);
  color: #1d1d1f;
  border: 0.5px solid rgba(255, 255, 255, 0.3);
  border-radius: 12px;
  cursor: pointer;
  font-size: 14px;
  font-weight: 600;
  font-family: 'SF Pro Text', -apple-system, BlinkMacSystemFont, sans-serif;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08),
              inset 0 1px 0 rgba(255, 255, 255, 0.9);
  backdrop-filter: blur(20px);
  transition: all 0.2s ease;
}

.toolbar-button:hover {
  background: rgba(255, 255, 255, 0.8);
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.12),
              inset 0 1px 0 rgba(255, 255, 255, 1);
}

.toolbar-button:active {
  transform: translateY(0);
  background: rgba(255, 255, 255, 0.7);
}

.toolbar-button.secondary {
  background: rgba(255, 255, 255, 0.4);
}

.toolbar-button.secondary:hover {
  background: rgba(255, 255, 255, 0.6);
}

.toolbar-button .material-symbols-outlined {
  font-size: 20px;
  opacity: 0.8;
}
</style>
