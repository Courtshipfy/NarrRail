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

        <section class="global-config glass-morphism">
            <div class="global-config-header">
                <div class="header-main">
                    <h2>全局配置</h2>
                    <p>这里设置的变量与预设说话人将对所有脚本编辑器生效</p>
                </div>
                <div class="header-actions">
                    <span class="config-file-chip">{{
                        globalConfigFileName
                    }}</span>
                    <button
                        class="btn secondary small"
                        @click="importGlobalConfigYaml"
                    >
                        导入 YAML
                    </button>
                    <button
                        class="btn primary small"
                        @click="exportGlobalConfigYaml"
                    >
                        导出 YAML
                    </button>
                </div>
            </div>

            <div class="global-config-columns">
                <div class="config-card">
                    <div class="config-card-head">
                        <h3>全局变量</h3>
                        <button
                            class="btn secondary small"
                            @click="showAddVariableForm = !showAddVariableForm"
                        >
                            {{ showAddVariableForm ? "收起" : "添加变量" }}
                        </button>
                    </div>

                    <div v-if="variables.length > 0" class="config-list">
                        <div
                            v-for="(variable, index) in variables"
                            :key="`global-var-${index}`"
                            class="config-item"
                        >
                            <div class="config-item-main">
                                <strong>{{ variable.name }}</strong>
                                <span>{{ variable.type }}</span>
                                <span>{{ formatVariableValue(variable) }}</span>
                            </div>
                            <button
                                class="btn danger tiny"
                                @click="removeVariable(index)"
                            >
                                删除
                            </button>
                        </div>
                    </div>
                    <div v-else class="config-empty">暂无全局变量</div>

                    <div v-if="showAddVariableForm" class="inline-form">
                        <input
                            v-model.trim="newVariable.name"
                            type="text"
                            placeholder="变量名"
                        />
                        <select v-model="newVariable.type">
                            <option value="Bool">Bool</option>
                            <option value="Int">Int</option>
                            <option value="Float">Float</option>
                            <option value="String">String</option>
                        </select>

                        <input
                            v-if="newVariable.type === 'Bool'"
                            v-model="newVariable.boolValue"
                            type="checkbox"
                            class="checkbox"
                        />
                        <input
                            v-else-if="newVariable.type === 'Int'"
                            v-model.number="newVariable.intValue"
                            type="number"
                            placeholder="初始值"
                        />
                        <input
                            v-else-if="newVariable.type === 'Float'"
                            v-model.number="newVariable.floatValue"
                            type="number"
                            step="0.01"
                            placeholder="初始值"
                        />
                        <input
                            v-else
                            v-model="newVariable.stringValue"
                            type="text"
                            placeholder="初始值"
                        />

                        <div class="inline-actions">
                            <button
                                class="btn primary tiny"
                                @click="addVariable"
                            >
                                确认
                            </button>
                            <button
                                class="btn secondary tiny"
                                @click="resetVariableForm"
                            >
                                取消
                            </button>
                        </div>
                    </div>
                </div>

                <div class="config-card">
                    <div class="config-card-head">
                        <h3>预设说话人</h3>
                        <button
                            class="btn secondary small"
                            @click="showAddSpeakerForm = !showAddSpeakerForm"
                        >
                            {{ showAddSpeakerForm ? "收起" : "添加说话人" }}
                        </button>
                    </div>

                    <div v-if="presetSpeakers.length > 0" class="config-list">
                        <div
                            v-for="(speaker, index) in presetSpeakers"
                            :key="`global-speaker-${index}`"
                            class="config-item"
                        >
                            <div class="config-item-main">
                                <strong>{{
                                    formatSpeakerLabel(speaker)
                                }}</strong>
                            </div>
                            <button
                                class="btn danger tiny"
                                @click="removeSpeaker(index)"
                            >
                                删除
                            </button>
                        </div>
                    </div>
                    <div v-else class="config-empty">暂无预设说话人</div>

                    <div v-if="showAddSpeakerForm" class="inline-form">
                        <input
                            v-model.trim="newSpeaker.id"
                            type="text"
                            placeholder="说话人 ID"
                        />
                        <input
                            v-model.trim="newSpeaker.displayName"
                            type="text"
                            placeholder="显示名（可选）"
                        />
                        <div class="inline-actions">
                            <button
                                class="btn primary tiny"
                                @click="addSpeaker"
                            >
                                确认
                            </button>
                            <button
                                class="btn secondary tiny"
                                @click="resetSpeakerForm"
                            >
                                取消
                            </button>
                        </div>
                    </div>
                </div>
            </div>
        </section>

        <main class="grid">
            <article class="script-card pinned glass-morphism">
                <div class="card-head">
                    <h3 :title="globalConfigFileName">
                        {{ globalConfigFileName }}
                    </h3>
                    <span class="ext">.yaml</span>
                </div>

                <p class="path">Repository/Config/{{ globalConfigFileName }}</p>

                <div class="meta">
                    <span>Global Config</span>
                    <span>variables: {{ variables.length }}</span>
                    <span>speakers: {{ presetSpeakers.length }}</span>
                </div>

                <div class="tags">
                    <span class="tag">Pinned</span>
                    <span class="tag">YAML</span>
                    <span class="tag">Shared</span>
                </div>

                <div class="actions">
                    <button
                        class="btn secondary"
                        @click="importGlobalConfigYaml"
                    >
                        导入 YAML
                    </button>
                    <button class="btn primary" @click="exportGlobalConfigYaml">
                        导出 YAML
                    </button>
                </div>
            </article>

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

        <input
            ref="globalConfigFileInput"
            type="file"
            accept=".yaml,.yml"
            style="display: none"
            @change="handleGlobalConfigFileChange"
        />
    </div>
</template>

<script setup>
import { computed, reactive, ref } from "vue";
import {
    downloadGlobalConfigYAML,
    parseGlobalConfigFromYAML,
} from "../utils/global-config-yaml.js";

const props = defineProps({
    isDarkMode: {
        type: Boolean,
        default: false,
    },
    variables: {
        type: Array,
        default: () => [],
    },
    presetSpeakers: {
        type: Array,
        default: () => [],
    },
});

const emit = defineEmits([
    "open-script",
    "open-empty",
    "toggle-theme",
    "update-variables",
    "update-speakers",
]);

const keyword = ref("");
const selectedFolder = ref("all");
const sortBy = ref("updatedDesc");

const showAddVariableForm = ref(false);
const showAddSpeakerForm = ref(false);
const globalConfigFileInput = ref(null);
const globalConfigFileName = "global-config.narrrail.yaml";

const newVariable = reactive({
    name: "",
    type: "Bool",
    boolValue: false,
    intValue: 0,
    floatValue: 0,
    stringValue: "",
});

const newSpeaker = reactive({
    id: "",
    displayName: "",
});

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

function addVariable() {
    const name = String(newVariable.name || "").trim();
    if (!name) {
        alert("请输入变量名");
        return;
    }

    const exists = props.variables.some(
        (v) => String(v?.name || "").trim() === name,
    );
    if (exists) {
        alert("变量名已存在");
        return;
    }

    const variable = {
        name,
        type: newVariable.type,
    };

    if (newVariable.type === "Bool")
        variable.boolValue = !!newVariable.boolValue;
    else if (newVariable.type === "Int")
        variable.intValue = Number(newVariable.intValue || 0);
    else if (newVariable.type === "Float")
        variable.floatValue = Number(newVariable.floatValue || 0);
    else variable.stringValue = String(newVariable.stringValue || "");

    emit("update-variables", [...props.variables, variable]);
    resetVariableForm();
}

function removeVariable(index) {
    const updated = [...props.variables];
    updated.splice(index, 1);
    emit("update-variables", updated);
}

function resetVariableForm() {
    newVariable.name = "";
    newVariable.type = "Bool";
    newVariable.boolValue = false;
    newVariable.intValue = 0;
    newVariable.floatValue = 0;
    newVariable.stringValue = "";
    showAddVariableForm.value = false;
}

function addSpeaker() {
    const id = String(newSpeaker.id || "").trim();
    const displayName = String(newSpeaker.displayName || "").trim();

    if (!id) {
        alert("请输入说话人 ID");
        return;
    }

    const normalized = props.presetSpeakers.map((speaker) =>
        typeof speaker === "string"
            ? speaker
            : String(speaker?.id || "").trim(),
    );

    if (normalized.includes(id)) {
        alert("该说话人 ID 已存在");
        return;
    }

    const payload = displayName ? { id, displayName } : { id };
    emit("update-speakers", [...props.presetSpeakers, payload]);
    resetSpeakerForm();
}

function removeSpeaker(index) {
    const updated = [...props.presetSpeakers];
    updated.splice(index, 1);
    emit("update-speakers", updated);
}

function resetSpeakerForm() {
    newSpeaker.id = "";
    newSpeaker.displayName = "";
    showAddSpeakerForm.value = false;
}

function formatSpeakerLabel(speaker) {
    if (typeof speaker === "string") return speaker;
    const id = String(speaker?.id || "").trim();
    const displayName = String(speaker?.displayName || "").trim();
    if (displayName && displayName !== id) return `${displayName} (${id})`;
    return id || "未命名说话人";
}

function formatVariableValue(variable) {
    if (!variable) return "";
    if (variable.type === "Bool") return variable.boolValue ? "true" : "false";
    if (variable.type === "Int") return String(variable.intValue ?? 0);
    if (variable.type === "Float") return String(variable.floatValue ?? 0);
    return `"${variable.stringValue ?? ""}"`;
}

function exportGlobalConfigYaml() {
    downloadGlobalConfigYAML(
        {
            variables: props.variables,
            presetSpeakers: props.presetSpeakers,
        },
        globalConfigFileName,
    );
}

function importGlobalConfigYaml() {
    globalConfigFileInput.value?.click();
}

function handleGlobalConfigFileChange(event) {
    const file = event.target.files?.[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = (e) => {
        try {
            const parsed = parseGlobalConfigFromYAML(
                String(e.target?.result || ""),
            );
            emit("update-variables", parsed.variables || []);
            emit("update-speakers", parsed.presetSpeakers || []);
            alert("全局配置导入成功");
        } catch (error) {
            alert(`全局配置导入失败: ${error.message}`);
        }
    };

    reader.readAsText(file);
    event.target.value = "";
}

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

.global-config {
    margin-bottom: 14px;
    padding: 14px;
    border-radius: 14px;
}

.global-config-header {
    margin-bottom: 10px;
    display: flex;
    justify-content: space-between;
    align-items: start;
    gap: 10px;
}

.global-config-header h2 {
    margin: 0 0 4px;
    font-size: 16px;
}

.global-config-header p {
    margin: 0;
    font-size: 12px;
    color: color-mix(in srgb, var(--nr-text) 60%, transparent);
}

.header-main {
    min-width: 0;
}

.header-actions {
    display: flex;
    align-items: center;
    gap: 8px;
    flex-wrap: wrap;
    justify-content: flex-end;
}

.config-file-chip {
    font-size: 11px;
    padding: 4px 8px;
    border-radius: 999px;
    background: color-mix(in srgb, #3b82f6 16%, transparent);
    border: 1px solid color-mix(in srgb, #60a5fa 40%, transparent);
    color: color-mix(in srgb, var(--nr-text) 72%, #2563eb 28%);
}

.global-config-columns {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 12px;
}

.config-card {
    background: color-mix(in srgb, var(--nr-bg) 65%, transparent);
    border: 1px solid color-mix(in srgb, var(--nr-text) 15%, transparent);
    border-radius: 12px;
    padding: 10px;
}

.config-card-head {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 8px;
    margin-bottom: 8px;
}

.config-card-head h3 {
    margin: 0;
    font-size: 14px;
}

.config-list {
    display: flex;
    flex-direction: column;
    gap: 8px;
    margin-bottom: 8px;
}

.config-item {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 8px;
    border: 1px solid color-mix(in srgb, var(--nr-text) 14%, transparent);
    border-radius: 10px;
    padding: 8px;
}

.config-item-main {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 8px;
    font-size: 12px;
}

.config-empty {
    font-size: 12px;
    color: color-mix(in srgb, var(--nr-text) 55%, transparent);
    margin-bottom: 8px;
}

.inline-form {
    display: flex;
    flex-direction: column;
    gap: 8px;
}

.inline-form input,
.inline-form select {
    background: color-mix(in srgb, var(--nr-bg) 70%, #ffffff 30%);
    border: 1px solid color-mix(in srgb, var(--nr-text) 18%, transparent);
    color: var(--nr-text);
    border-radius: 10px;
    padding: 8px 10px;
    outline: none;
}

.inline-form .checkbox {
    width: 18px;
    height: 18px;
}

.inline-actions {
    display: flex;
    gap: 8px;
}

.btn.small {
    padding: 6px 9px;
    font-size: 12px;
}

.btn.tiny {
    padding: 5px 8px;
    font-size: 11px;
}

.btn.danger {
    color: #fff;
    background: linear-gradient(135deg, #ef4444, #dc2626);
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
    gap: 8px;
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

.script-card.pinned {
    border-style: dashed;
    border-width: 1.5px;
    border-color: color-mix(in srgb, #60a5fa 45%, transparent);
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
