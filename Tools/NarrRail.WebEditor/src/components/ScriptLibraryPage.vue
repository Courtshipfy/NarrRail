<template>
    <div class="script-library-page">
        <header class="top-bar glass-morphism">
            <div class="brand">
                <div class="logo">NR</div>
                <div class="title-wrap">
                    <h1>NarrRail Script Library</h1>
                    <p class="subtitle">
                        从仓库脚本列表中选择并进入编辑器（当前为 Mock 数据）
                    </p>
                </div>
            </div>

            <div class="top-actions">
                <span class="status-badge mock">Mock Repository</span>
                <button class="btn secondary" @click="emit('toggle-theme')">
                    {{ isDarkMode ? "切换浅色" : "切换深色" }}
                </button>
                <button class="btn secondary" @click="emit('open-empty')">
                    空白进入编辑器
                </button>
            </div>
        </header>

        <section class="filters glass-morphism">
            <div class="field">
                <label>搜索脚本</label>
                <input
                    v-model.trim="keyword"
                    type="text"
                    placeholder="按文件名 / storyId / 路径搜索"
                />
            </div>

            <div class="field">
                <label>目录</label>
                <select v-model="selectedFolder">
                    <option value="all">全部目录</option>
                    <option
                        v-for="folder in folders"
                        :key="folder"
                        :value="folder"
                    >
                        {{ folder }}
                    </option>
                </select>
            </div>

            <div class="field">
                <label>排序</label>
                <select v-model="sortBy">
                    <option value="updatedDesc">最近更新优先</option>
                    <option value="nameAsc">文件名 A-Z</option>
                    <option value="sizeDesc">文件大小（大到小）</option>
                </select>
            </div>

            <div class="summary">
                共 <strong>{{ filteredScripts.length }}</strong> 个脚本
            </div>
        </section>

        <main class="grid">
            <article
                v-for="script in filteredScripts"
                :key="script.id"
                class="script-card glass-morphism"
            >
                <div class="card-head">
                    <h3 :title="script.fileName">{{ script.fileName }}</h3>
                    <span class="ext">{{ script.extension }}</span>
                </div>

                <p class="path">{{ script.path }}</p>

                <div class="meta">
                    <span>Story: {{ script.storyId }}</span>
                    <span>{{ formatSize(script.size) }}</span>
                    <span>{{ formatDate(script.updatedAt) }}</span>
                </div>

                <div class="tags">
                    <span
                        v-for="tag in script.tags"
                        :key="`${script.id}-${tag}`"
                        class="tag"
                    >
                        {{ tag }}
                    </span>
                </div>

                <div class="actions">
                    <button class="btn primary" @click="openScript(script)">
                        打开并进入编辑器
                    </button>
                </div>
            </article>

            <div
                v-if="filteredScripts.length === 0"
                class="empty glass-morphism"
            >
                <h3>没有匹配的脚本</h3>
                <p>请调整搜索条件后重试。</p>
            </div>
        </main>
    </div>
</template>

<script setup>
import { computed, ref } from "vue";

defineProps({
    isDarkMode: {
        type: Boolean,
        default: false,
    },
});

const emit = defineEmits(["open-script", "open-empty", "toggle-theme"]);

const keyword = ref("");
const selectedFolder = ref("all");
const sortBy = ref("updatedDesc");

const mockScripts = ref([
    {
        id: "s1",
        fileName: "chapter_01_intro.narrrail.yaml",
        extension: ".yaml",
        path: "Stories/Chapter01/chapter_01_intro.narrrail.yaml",
        storyId: "chapter_01_intro",
        size: 8421,
        updatedAt: "2026-04-10T10:24:00Z",
        tags: ["Main", "Dialogue"],
    },
    {
        id: "s2",
        fileName: "chapter_01_choice.narrrail.yaml",
        extension: ".yaml",
        path: "Stories/Chapter01/chapter_01_choice.narrrail.yaml",
        storyId: "chapter_01_choice",
        size: 12544,
        updatedAt: "2026-04-11T03:17:00Z",
        tags: ["Choice", "Branch"],
    },
    {
        id: "s3",
        fileName: "affinity_demo.narrrail.yaml",
        extension: ".yaml",
        path: "Stories/Demo/affinity_demo.narrrail.yaml",
        storyId: "affinity_demo",
        size: 23310,
        updatedAt: "2026-04-12T14:40:00Z",
        tags: ["Demo", "Variable"],
    },
    {
        id: "s4",
        fileName: "new_story_temp.narrrail.yml",
        extension: ".yml",
        path: "Stories/Sandbox/new_story_temp.narrrail.yml",
        storyId: "new_story_temp",
        size: 5112,
        updatedAt: "2026-04-09T21:55:00Z",
        tags: ["Sandbox"],
    },
    {
        id: "s5",
        fileName: "ending_route_a.narrrail.yaml",
        extension: ".yaml",
        path: "Stories/Ending/ending_route_a.narrrail.yaml",
        storyId: "ending_route_a",
        size: 9688,
        updatedAt: "2026-04-12T01:03:00Z",
        tags: ["Ending", "RouteA"],
    },
]);

const folders = computed(() => {
    const set = new Set(
        mockScripts.value.map((s) => s.path.split("/")[1] || "Unknown"),
    );
    return Array.from(set).sort((a, b) => a.localeCompare(b));
});

const filteredScripts = computed(() => {
    const kw = keyword.value.toLowerCase();
    let result = mockScripts.value.filter((s) => {
        const folder = s.path.split("/")[1] || "Unknown";
        const inFolder =
            selectedFolder.value === "all" || folder === selectedFolder.value;
        const inKeyword =
            !kw ||
            s.fileName.toLowerCase().includes(kw) ||
            s.storyId.toLowerCase().includes(kw) ||
            s.path.toLowerCase().includes(kw);
        return inFolder && inKeyword;
    });

    switch (sortBy.value) {
        case "nameAsc":
            result = result.sort((a, b) =>
                a.fileName.localeCompare(b.fileName),
            );
            break;
        case "sizeDesc":
            result = result.sort((a, b) => b.size - a.size);
            break;
        case "updatedDesc":
        default:
            result = result.sort(
                (a, b) =>
                    new Date(b.updatedAt).getTime() -
                    new Date(a.updatedAt).getTime(),
            );
            break;
    }

    return result;
});

function openScript(script) {
    emit("open-script", {
        id: script.id,
        storyId: script.storyId,
        fileName: script.fileName,
        path: script.path,
        source: "mock-repository",
    });
}

function formatSize(bytes) {
    if (bytes < 1024) return `${bytes} B`;
    if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)} KB`;
    return `${(bytes / (1024 * 1024)).toFixed(2)} MB`;
}

function formatDate(iso) {
    return new Date(iso).toLocaleString();
}
</script>

<style scoped>
.script-library-page {
    min-height: 100vh;
    padding: 20px;
    color: var(--nr-text);
    background: transparent;
}

.top-bar {
    display: flex;
    justify-content: space-between;
    gap: 12px;
    align-items: center;
    padding: 16px 18px;
    margin-bottom: 14px;
}

.brand {
    display: flex;
    align-items: center;
    gap: 12px;
}

.logo {
    width: 36px;
    height: 36px;
    border-radius: 10px;
    display: grid;
    place-items: center;
    font-weight: 700;
    background: linear-gradient(135deg, #7c3aed, #3b82f6);
}

.title-wrap h1 {
    margin: 0;
    font-size: 18px;
}

.subtitle {
    margin: 2px 0 0;
    font-size: 12px;
    color: color-mix(in srgb, var(--nr-text) 52%, transparent);
}

.top-actions {
    display: flex;
    align-items: center;
    gap: 10px;
}

.status-badge {
    font-size: 12px;
    border-radius: 999px;
    padding: 4px 10px;
    border: 1px solid transparent;
}

.status-badge.mock {
    color: #92400e;
    background: rgba(217, 119, 6, 0.16);
    border-color: rgba(217, 119, 6, 0.35);
}

.filters {
    display: grid;
    grid-template-columns: 1.8fr 1fr 1fr auto;
    gap: 10px;
    padding: 12px;
    margin-bottom: 14px;
}

.field {
    display: flex;
    flex-direction: column;
    gap: 6px;
}

.field label {
    font-size: 12px;
    color: color-mix(in srgb, var(--nr-text) 72%, #3b82f6 28%);
}

.field input,
.field select {
    background: color-mix(in srgb, var(--nr-bg) 70%, #ffffff 30%);
    border: 1px solid color-mix(in srgb, var(--nr-text) 18%, transparent);
    color: var(--nr-text);
    border-radius: 10px;
    padding: 9px 10px;
    outline: none;
}

.summary {
    display: flex;
    align-items: end;
    justify-content: end;
    color: color-mix(in srgb, var(--nr-text) 78%, transparent);
    font-size: 13px;
    padding-bottom: 8px;
}

.grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(320px, 1fr));
    gap: 12px;
}

.script-card {
    padding: 12px;
}

.card-head {
    display: flex;
    justify-content: space-between;
    gap: 8px;
    align-items: start;
}

.card-head h3 {
    margin: 0;
    font-size: 15px;
    line-height: 1.3;
    word-break: break-all;
}

.ext {
    font-size: 11px;
    color: #93c5fd;
    background: rgba(37, 99, 235, 0.2);
    border: 1px solid rgba(37, 99, 235, 0.5);
    border-radius: 999px;
    padding: 2px 8px;
    white-space: nowrap;
}

.path {
    margin: 8px 0;
    font-size: 12px;
    color: color-mix(in srgb, var(--nr-text) 55%, transparent);
    word-break: break-all;
}

.meta {
    display: flex;
    flex-wrap: wrap;
    gap: 8px;
    font-size: 12px;
    color: color-mix(in srgb, var(--nr-text) 78%, transparent);
    margin-bottom: 8px;
}

.tags {
    display: flex;
    gap: 6px;
    flex-wrap: wrap;
    margin-bottom: 10px;
}

.tag {
    font-size: 11px;
    color: color-mix(in srgb, var(--nr-text) 68%, #2563eb 32%);
    background: color-mix(in srgb, #3b82f6 16%, transparent);
    border: 1px solid color-mix(in srgb, #60a5fa 38%, transparent);
    border-radius: 999px;
    padding: 2px 8px;
}

.actions {
    display: flex;
    justify-content: flex-end;
}

.btn {
    border: none;
    border-radius: 10px;
    padding: 9px 12px;
    cursor: pointer;
    font-weight: 600;
}

.btn.primary {
    color: #f8fafc;
    background: linear-gradient(135deg, #2563eb, #7c3aed);
}

.btn.secondary {
    color: var(--nr-text);
    background: color-mix(in srgb, var(--nr-bg) 68%, #ffffff 32%);
    border: 1px solid color-mix(in srgb, var(--nr-text) 20%, transparent);
}

.empty {
    padding: 28px;
    grid-column: 1 / -1;
    text-align: center;
    color: color-mix(in srgb, var(--nr-text) 55%, transparent);
}

:global(body[data-theme="dark"]) .script-library-page .title-wrap h1,
:global(body.dark-theme) .script-library-page .title-wrap h1,
:global(body[data-theme="dark"]) .script-library-page .card-head h3,
:global(body.dark-theme) .script-library-page .card-head h3 {
    color: #e5e7eb;
}
</style>
