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
  height: 80px;
  display: flex;
  align-items: center;
  padding: 0 32px;
  gap: 16px;
  border-radius: 0 0 32px 32px;
  position: relative;
  z-index: 50;
}

.toolbar-title {
  font-family: 'Plus Jakarta Sans', sans-serif;
  font-size: 24px;
  font-weight: 800;
  margin-right: 32px;
  letter-spacing: -0.02em;
}

.toolbar-button {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 10px 20px;
  background: linear-gradient(135deg, rgba(244, 114, 182, 0.8), rgba(168, 85, 247, 0.8));
  color: white;
  border: 1px solid rgba(255, 255, 255, 0.4);
  border-radius: 9999px;
  cursor: pointer;
  font-size: 14px;
  font-weight: 600;
  font-family: 'Plus Jakarta Sans', sans-serif;
  box-shadow: 0 8px 24px rgba(244, 114, 182, 0.3),
              inset 0 1px 2px rgba(255, 255, 255, 0.5);
}

.toolbar-button:hover {
  transform: scale(1.05);
  box-shadow: 0 12px 32px rgba(244, 114, 182, 0.4),
              inset 0 1px 2px rgba(255, 255, 255, 0.6);
}

.toolbar-button.secondary {
  background: linear-gradient(135deg, rgba(59, 130, 246, 0.7), rgba(147, 197, 253, 0.7));
  box-shadow: 0 8px 24px rgba(59, 130, 246, 0.2),
              inset 0 1px 2px rgba(255, 255, 255, 0.5);
}

.toolbar-button.secondary:hover {
  box-shadow: 0 12px 32px rgba(59, 130, 246, 0.3),
              inset 0 1px 2px rgba(255, 255, 255, 0.6);
}

.toolbar-button .material-symbols-outlined {
  font-size: 20px;
}
</style>
