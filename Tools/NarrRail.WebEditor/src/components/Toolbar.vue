<template>
    <div class="toolbar glass-morphism">
        <div class="toolbar-title gradient-text">NarrRail Story Editor</div>

        <button
            class="toolbar-button bouncy-feedback spring-animation"
            @click="$emit('new')"
        >
            <span class="material-symbols-outlined">add_circle</span>
            <span>新建</span>
        </button>

        <button
            class="toolbar-button bouncy-feedback spring-animation"
            @click="$emit('import')"
        >
            <span class="material-symbols-outlined">upload_file</span>
            <span>导入</span>
        </button>

        <button
            class="toolbar-button bouncy-feedback spring-animation"
            @click="$emit('export')"
        >
            <span class="material-symbols-outlined">download</span>
            <span>导出</span>
        </button>

        <button
            class="toolbar-button secondary bouncy-feedback spring-animation"
            @click="$emit('undo')"
            title="撤销 (Ctrl+Z)"
        >
            <span class="material-symbols-outlined">undo</span>
            <span>撤销</span>
        </button>

        <button
            class="toolbar-button secondary bouncy-feedback spring-animation"
            @click="$emit('redo')"
            title="重做 (Ctrl+Y)"
        >
            <span class="material-symbols-outlined">redo</span>
            <span>重做</span>
        </button>

        <button
            class="toolbar-button secondary bouncy-feedback spring-animation"
            @click="$emit('validate')"
        >
            <span class="material-symbols-outlined">verified</span>
            <span>验证</span>
        </button>

        <button
            class="toolbar-button secondary bouncy-feedback spring-animation"
            @click="$emit('auto-layout')"
            title="自动排布节点（从左到右）"
        >
            <span class="material-symbols-outlined">account_tree</span>
            <span>自动排布</span>
        </button>

        <button
            class="toolbar-button secondary bouncy-feedback spring-animation"
            @click="$emit('toggle-edge-style')"
            :title="
                edgeStyle === 'straight'
                    ? '当前为直线，点击切换为曲线'
                    : '当前为曲线，点击切换为直线'
            "
        >
            <span class="material-symbols-outlined">
                {{ edgeStyle === "straight" ? "timeline" : "device_hub" }}
            </span>
            <span>{{
                edgeStyle === "straight" ? "连线: 直线" : "连线: 曲线"
            }}</span>
        </button>

        <button
            class="toolbar-button secondary bouncy-feedback spring-animation"
            :class="{ active: focusModeEnabled }"
            @click="$emit('toggle-focus-mode')"
            :title="
                focusModeEnabled
                    ? '关闭焦点模式'
                    : '开启焦点模式（弱化非相关连线）'
            "
        >
            <span class="material-symbols-outlined">
                {{
                    focusModeEnabled
                        ? "center_focus_strong"
                        : "center_focus_weak"
                }}
            </span>
            <span>{{
                focusModeEnabled ? "焦点模式: 开" : "焦点模式: 关"
            }}</span>
        </button>

        <button
            class="toolbar-button secondary compact-icon bouncy-feedback spring-animation"
            :class="{ active: isDarkMode }"
            @click="$emit('toggle-dark-mode')"
            :title="isDarkMode ? '切换为浅色模式' : '切换为深色模式'"
            :aria-label="isDarkMode ? '切换为浅色模式' : '切换为深色模式'"
        >
            <span class="material-symbols-outlined">
                {{ isDarkMode ? "light_mode" : "dark_mode" }}
            </span>
        </button>

        <button
            class="toolbar-button secondary compact-icon bouncy-feedback spring-animation"
            @click="$emit('go-library')"
            title="返回脚本库页面"
            aria-label="返回脚本库页面"
        >
            <span class="material-symbols-outlined">library_books</span>
        </button>

        <button
            class="toolbar-button secondary compact-icon bouncy-feedback spring-animation"
            @click="$emit('help')"
            title="帮助"
            aria-label="帮助"
        >
            <span class="material-symbols-outlined">help</span>
        </button>
    </div>
</template>

<script setup>
defineProps({
    focusModeEnabled: {
        type: Boolean,
        default: false,
    },
    edgeStyle: {
        type: String,
        default: "straight",
    },
    isDarkMode: {
        type: Boolean,
        default: false,
    },
});

defineEmits([
    "new",
    "import",
    "export",
    "undo",
    "redo",
    "validate",
    "auto-layout",
    "toggle-edge-style",
    "toggle-focus-mode",
    "toggle-dark-mode",
    "go-library",
    "help",
]);
</script>

<style scoped>
.toolbar {
    position: fixed;
    top: 16px;
    left: 16px;
    right: 16px;
    min-height: 72px;
    display: flex;
    align-items: center;
    flex-wrap: wrap;
    padding: 12px 24px;
    gap: 12px;
    border-radius: 20px;
    z-index: 50;
    border: none;
}

.toolbar-title {
    font-family: "Outfit", sans-serif;
    font-size: 22px;
    font-weight: 800;
    margin-right: 20px;
    letter-spacing: -0.02em;
    white-space: nowrap;
}

.toolbar-button {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    padding: 10px 16px;
    background: rgba(255, 255, 255, 0.62);
    color: #1d1d1f;
    border: 0.5px solid rgba(255, 255, 255, 0.35);
    border-radius: 12px;
    cursor: pointer;
    font-size: 13px;
    font-weight: 650;
    font-family:
        "SF Pro Text",
        -apple-system,
        BlinkMacSystemFont,
        sans-serif;
    box-shadow:
        0 2px 8px rgba(0, 0, 0, 0.08),
        inset 0 1px 0 rgba(255, 255, 255, 0.9);
    backdrop-filter: blur(20px);
    transition: all 0.2s ease;
    white-space: nowrap;
}

.toolbar-button:hover {
    background: rgba(255, 255, 255, 0.82);
    transform: translateY(-1px);
    box-shadow:
        0 4px 12px rgba(0, 0, 0, 0.12),
        inset 0 1px 0 rgba(255, 255, 255, 1);
}

.toolbar-button:active {
    transform: translateY(0);
    background: rgba(255, 255, 255, 0.72);
}

.toolbar-button.secondary {
    background: rgba(255, 255, 255, 0.45);
}

.toolbar-button.secondary:hover {
    background: rgba(255, 255, 255, 0.66);
}

.toolbar-button.active {
    background: rgba(168, 85, 247, 0.18);
    border-color: rgba(168, 85, 247, 0.35);
    color: #7e22ce;
    box-shadow:
        0 4px 14px rgba(168, 85, 247, 0.2),
        inset 0 1px 0 rgba(255, 255, 255, 0.95);
}

.toolbar-button .material-symbols-outlined {
    font-size: 19px;
    opacity: 0.85;
}

.toolbar-button.compact-icon {
    width: 38px;
    height: 38px;
    padding: 0;
    justify-content: center;
    gap: 0;
    border-radius: 10px;
    background: color-mix(in srgb, var(--nr-bg) 72%, #ffffff 28%);
    border-color: color-mix(in srgb, var(--nr-text) 16%, transparent);
    box-shadow:
        0 1px 6px rgba(0, 0, 0, 0.08),
        inset 0 1px 0 rgba(255, 255, 255, 0.65);
}

.toolbar-button.compact-icon:hover {
    background: color-mix(in srgb, var(--nr-bg) 66%, #ffffff 34%);
    box-shadow:
        0 2px 8px rgba(0, 0, 0, 0.1),
        inset 0 1px 0 rgba(255, 255, 255, 0.72);
}

.toolbar-button.compact-icon .material-symbols-outlined {
    font-size: 20px;
}
</style>
