<template>
    <div class="status-bar glass-morphism">
        <div class="status-left">
            <div class="status-item">
                <IconGlyph name="account_tree" />
                <span>节点: {{ nodeCount }}</span>
            </div>

            <div class="status-item">
                <IconGlyph name="share" />
                <span>边: {{ edgeCount }}</span>
            </div>

            <div class="status-item" :class="{ warning: !entryNodeId }">
                <IconGlyph name="flag" />
                <span>入口: {{ entryNodeId || "未设置" }}</span>
            </div>

            <div class="status-item">
                <IconGlyph name="label" />
                <span>{{ storyId }}</span>
            </div>
        </div>

        <div class="status-item status-right">
            <IconGlyph name="schedule" />
            <span>{{ lastSavedText }}</span>
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
    lastSavedAt: {
        type: String,
        default: "",
    },
});

const lastSavedText = computed(() => {
    const raw = String(props.lastSavedAt || "").trim();
    if (!raw) return "上次保存：未触发";

    const ts = new Date(raw).getTime();
    if (!Number.isFinite(ts)) return "上次保存：未触发";
    return `上次保存 ${new Date(ts).toLocaleTimeString()}`;
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
    line-height: 1;
    color: #1d1d1f;
    opacity: 0.78;
    white-space: nowrap;
    font-family:
        "SF Pro Text",
        -apple-system,
        BlinkMacSystemFont,
        sans-serif;
}

.status-item :deep(.icon-glyph) {
    font-size: 18px;
    line-height: 1;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    transform: translateY(0.5px);
    opacity: 0.68;
}

.status-item.warning {
    color: #ff9500;
    opacity: 1;
}

.status-item.warning :deep(.icon-glyph) {
    color: #ff9500;
    opacity: 1;
}

.status-right {
    margin-left: auto;
}
</style>
