<template>
    <div class="read-mode-root">
        <div class="read-mode-body">
            <div v-if="lines.length === 0" class="read-empty">
                <span class="material-symbols-outlined">menu_book</span>
                <p>当前没有可阅读的对话内容</p>
            </div>

            <div v-else class="read-content">
                <div class="read-lines">
                    <div
                        v-for="(line, index) in lines"
                        :key="`${line.nodeId}-${line.lineIndex ?? 0}-${index}`"
                        class="read-line"
                    >
                        <span
                            class="speaker"
                            :style="{
                                color: isDarkMode ? '#d9e2ec' : '#1e3a8a',
                            }"
                        >
                            {{ line.speaker }}
                        </span>
                        <span
                            class="sep"
                            :style="{
                                color: isDarkMode ? '#7f8ea3' : '#475569',
                            }"
                            >：</span
                        >
                        <span
                            class="text"
                            :style="{
                                color: isDarkMode ? '#b8c4d4' : '#111827',
                            }"
                            >{{ line.text }}</span
                        >
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>

<script setup>
defineProps({
    lines: {
        type: Array,
        default: () => [],
    },
    isDarkMode: {
        type: Boolean,
        default: false,
    },
});
</script>

<style scoped>
.read-mode-root {
    position: absolute;
    inset: 96px 24px 84px 24px;
    z-index: 15;
}

.read-mode-body {
    width: 100%;
    height: 100%;
    padding: 6px 4px;
    overflow: auto;
    background: transparent;
    border: none;
    box-shadow: none;
    backdrop-filter: none;
    -webkit-backdrop-filter: none;
}

.read-content {
    min-height: 100%;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 14px 0;
}

.read-empty {
    height: 100%;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    gap: 10px;
    color: #94a3b8;
}

.read-empty .material-symbols-outlined {
    font-size: 40px;
    opacity: 0.6;
}

.read-lines {
    width: min(920px, 96%);
    margin: 0 auto;
    display: flex;
    flex-direction: column;
    gap: 12px;
}

.read-line {
    display: grid;
    grid-template-columns: 11ch auto 1fr;
    align-items: start;
    column-gap: 2px;
    line-height: 1.7;
    font-size: 16px;
    color: #0f172a;
}

.speaker {
    font-weight: 700;
    color: #1e3a8a;
    text-align: right;
    overflow: hidden;
    white-space: nowrap;
    text-overflow: ellipsis;
}

.sep {
    color: #475569;
}

.text {
    color: #111827;
    word-break: break-word;
    white-space: pre-wrap;
}

:global(body.dark-theme) .read-mode-body,
:global(body[data-theme="dark"]) .read-mode-body {
    background: linear-gradient(
        180deg,
        rgba(15, 23, 42, 0.34),
        rgba(15, 23, 42, 0.26)
    );
}

:global(body.dark-theme) .read-line,
:global(body[data-theme="dark"]) .read-line {
    color: #e5e7eb;
}

:global(body.dark-theme) .read-line .speaker,
:global(body[data-theme="dark"]) .read-line .speaker {
    font-weight: 800;
}

:global(body.dark-theme) .sep,
:global(body[data-theme="dark"]) .sep {
    color: #9ca3af;
    opacity: 1;
}

:global(body.dark-theme) .text,
:global(body[data-theme="dark"]) .text {
    color: #e2e8f0;
}
</style>
