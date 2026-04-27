<template>
    <div class="status-bar glass-morphism">
        <div class="status-left">
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
        </div>

        <div class="status-corner-panel glass-morphism-strong">
            <div class="corner-item validation" :class="validationClass">
                <span class="material-symbols-outlined">{{
                    validationIcon
                }}</span>
                <span class="corner-label">{{ validationMessage }}</span>
                <span class="validation-count error">E: {{ errorCount }}</span>
                <span class="validation-count warning"
                    >W: {{ warningCount }}</span
                >
            </div>

            <div
                class="corner-item autosave"
                :title="`自动保存到${autosaveTarget}，每 ${autosaveIntervalSec} 秒一次`"
            >
                <span class="material-symbols-outlined pulse">cloud_done</span>
                <span class="corner-label">自动保存中</span>
                <span class="autosave-meta">{{ autosaveIntervalSec }}s</span>
            </div>
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
    autosaveIntervalSec: {
        type: Number,
        default: 30,
    },
    autosaveTarget: {
        type: String,
        default: "浏览器本地存储",
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
    left: 16px;
    right: 16px;
    bottom: 16px;
    min-height: 48px;
    border-radius: 20px;
    z-index: 50;
    border: none;
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 0 14px 0 24px;
    gap: 16px;
}

.status-left {
    min-width: 0;
    display: flex;
    align-items: center;
    gap: 22px;
    flex-wrap: wrap;
}

.status-item {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    font-size: 13px;
    font-weight: 500;
    color: #1d1d1f;
    opacity: 0.78;
    white-space: nowrap;
    font-family:
        "SF Pro Text",
        -apple-system,
        BlinkMacSystemFont,
        sans-serif;
}

.status-item .material-symbols-outlined {
    font-size: 18px;
    opacity: 0.68;
}

.status-item.warning {
    color: #ff9500;
    opacity: 1;
}

.status-item.warning .material-symbols-outlined {
    color: #ff9500;
    opacity: 1;
}

.status-corner-panel {
    display: flex;
    align-items: center;
    gap: 8px;
    border-radius: 14px;
    padding: 6px 8px;
    background: rgba(255, 255, 255, 0.48);
    border: 0.5px solid rgba(255, 255, 255, 0.45);
    box-shadow: 0 4px 12px rgba(15, 23, 42, 0.08);
}

.corner-item {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    border-radius: 10px;
    padding: 6px 10px;
    font-size: 12px;
    font-weight: 650;
    white-space: nowrap;
    background: rgba(255, 255, 255, 0.45);
    border: 0.5px solid rgba(255, 255, 255, 0.5);
}

.corner-item .material-symbols-outlined {
    font-size: 17px;
}

.corner-label {
    font-size: 12px;
    font-weight: 700;
    letter-spacing: 0.01em;
}

/* Validation variants */
.corner-item.validation.is-valid {
    color: #34c759;
}

.corner-item.validation.has-warning {
    color: #ff9500;
}

.corner-item.validation.has-error {
    color: #ff3b30;
}

.validation-count {
    font-size: 11px;
    font-weight: 800;
    letter-spacing: 0.01em;
    border-radius: 999px;
    padding: 2px 6px;
    background: rgba(255, 255, 255, 0.58);
}

.validation-count.error {
    color: #ff3b30;
}

.validation-count.warning {
    color: #ff9500;
}

/* Autosave */
.corner-item.autosave {
    color: #34c759;
}

.autosave-meta {
    font-size: 11px;
    font-weight: 800;
    padding: 2px 6px;
    border-radius: 999px;
    background: rgba(255, 255, 255, 0.58);
    color: #1f9a4a;
}

@keyframes pulse {
    0%,
    100% {
        opacity: 1;
        transform: translateY(0);
    }
    50% {
        opacity: 0.55;
        transform: translateY(-0.5px);
    }
}

.pulse {
    animation: pulse 2s ease-in-out infinite;
}
</style>
