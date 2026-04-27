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

        <div class="status-item" :class="{ warning: !entryNodeId }">
            <span class="material-symbols-outlined">flag</span>
            <span>入口: {{ entryNodeId || "未设置" }}</span>
        </div>

        <div class="status-item">
            <span class="material-symbols-outlined">label</span>
            <span>{{ storyId }}</span>
        </div>

        <div class="status-item validation" :class="validationClass">
            <span class="material-symbols-outlined">{{ validationIcon }}</span>
            <span class="validation-message">{{ validationMessage }}</span>
            <span class="validation-count error">E: {{ errorCount }}</span>
            <span class="validation-count warning">W: {{ warningCount }}</span>
        </div>

        <div class="status-item auto-save">
            <span class="material-symbols-outlined pulse">cloud_done</span>
            <span>自动保存</span>
        </div>
    </div>
</template>

<script setup>
import { computed } from "vue";

const props = defineProps({
    nodeCount: {
        type: Number,
        default: 0,
    },
    edgeCount: {
        type: Number,
        default: 0,
    },
    storyId: {
        type: String,
        default: "NewStory",
    },
    entryNodeId: {
        type: String,
        default: "",
    },
    errorCount: {
        type: Number,
        default: 0,
    },
    warningCount: {
        type: Number,
        default: 0,
    },
});

const validationIcon = computed(() => {
    if (props.errorCount > 0) return "error";
    if (props.warningCount > 0) return "warning";
    return "verified";
});

const validationClass = computed(() => {
    if (props.errorCount > 0) return "has-error";
    if (props.warningCount > 0) return "has-warning";
    return "is-valid";
});

const validationMessage = computed(() => {
    if (props.errorCount > 0) return `实时验证错误 ${props.errorCount}`;
    if (props.warningCount > 0) return `实时验证警告 ${props.warningCount}`;
    return "实时验证通过";
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
    gap: 24px;
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
    font-family:
        "SF Pro Text",
        -apple-system,
        BlinkMacSystemFont,
        sans-serif;
    opacity: 0.75;
    white-space: nowrap;
}

.status-item .material-symbols-outlined {
    font-size: 18px;
    opacity: 0.65;
}

.status-item.warning {
    color: #ff9500;
    opacity: 1;
}

.status-item.warning .material-symbols-outlined {
    color: #ff9500;
    opacity: 1;
}

.status-item.validation {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    padding: 4px 10px;
    border-radius: 999px;
    background: rgba(255, 255, 255, 0.35);
    border: 0.5px solid rgba(255, 255, 255, 0.4);
    opacity: 1;
}

.status-item.validation.is-valid {
    color: #34c759;
}

.status-item.validation.is-valid .material-symbols-outlined {
    color: #34c759;
    opacity: 1;
}

.status-item.validation.has-warning {
    color: #ff9500;
}

.status-item.validation.has-warning .material-symbols-outlined {
    color: #ff9500;
    opacity: 1;
}

.status-item.validation.has-error {
    color: #ff3b30;
}

.status-item.validation.has-error .material-symbols-outlined {
    color: #ff3b30;
    opacity: 1;
}

.validation-message {
    font-size: 12px;
    font-weight: 700;
    letter-spacing: 0.01em;
}

.validation-count {
    font-size: 12px;
    font-weight: 700;
    letter-spacing: 0.01em;
    padding: 2px 6px;
    border-radius: 999px;
    background: rgba(255, 255, 255, 0.5);
}

.validation-count.error {
    color: #ff3b30;
}

.validation-count.warning {
    color: #ff9500;
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
    0%,
    100% {
        opacity: 1;
    }
    50% {
        opacity: 0.45;
    }
}

.pulse {
    animation: pulse 2s ease-in-out infinite;
}
</style>
