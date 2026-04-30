<template>
    <div class="script-library-page">
        <header class="top-bar glass-morphism">
            <div class="brand">
                <div class="logo">NR</div>
                <div class="title-wrap">
                    <h1>NarrRail Script Library</h1>
                    <p class="subtitle">从 GitHub 仓库读取脚本并进入编辑器</p>
                </div>
            </div>

            <div class="top-actions">
                <span
                    class="status-badge"
                    :class="usingMockData ? 'mock' : 'auth-ok'"
                >
                    {{
                        usingMockData ? "Mock Repository" : "GitHub Repository"
                    }}
                </span>

                <span
                    v-if="authState?.authenticated"
                    class="status-badge auth-ok"
                    :title="authState.user?.login || ''"
                >
                    {{ authState.user?.login || "GitHub 已登录" }}
                </span>
                <span v-else class="status-badge auth-off">未登录</span>

                <span v-if="createScriptStatus" class="status-badge auth-ok">
                    {{ createScriptStatus }}
                </span>

                <button
                    v-if="!authState?.authenticated"
                    class="btn secondary"
                    @click="emit('login-github')"
                    :disabled="authState?.loading"
                >
                    {{ authState?.loading ? "检查登录中..." : "GitHub 登录" }}
                </button>
                <button v-else class="btn secondary" @click="emit('logout')">
                    退出登录
                </button>

                <button class="btn secondary" @click="emit('toggle-theme')">
                    {{ isDarkMode ? "切换浅色" : "切换深色" }}
                </button>
                <button
                    class="btn secondary"
                    @click="createNewScript"
                    :disabled="isCreatingScript"
                >
                    {{ isCreatingScript ? "创建中..." : "新建脚本" }}
                </button>
            </div>
        </header>

        <section class="filters glass-morphism">
            <div class="field repo-field">
                <label>仓库</label>
                <div class="repo-row">
                    <div class="summary inline-summary">
                        共 <strong>{{ filteredScripts.length }}</strong> 个脚本
                    </div>
                    <select
                        v-model="selectedRepoFullName"
                        :disabled="!authState?.authenticated || loadingRepos"
                    >
                        <option value="">请选择仓库</option>
                        <option
                            v-for="repo in repoOptions"
                            :key="repo.fullName"
                            :value="repo.fullName"
                        >
                            {{ repo.fullName }}
                        </option>
                    </select>
                </div>
            </div>

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
        </section>

        <section class="global-config glass-morphism">
            <div class="global-config-header">
                <div class="header-main">
                    <h2>全局配置</h2>
                </div>
                <div class="header-actions">
                    <span class="config-file-chip">{{
                        globalConfigFileName
                    }}</span>
                </div>
            </div>

            <div class="global-config-columns">
                <div class="config-card">
                    <div class="config-card-head">
                        <h3>全局变量</h3>
                        <button
                            class="icon-btn add"
                            @click="showAddVariableForm = !showAddVariableForm"
                            :title="showAddVariableForm ? '收起' : '添加变量'"
                            :aria-label="
                                showAddVariableForm
                                    ? '收起添加变量表单'
                                    : '添加变量'
                            "
                        >
                            +
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
                                class="icon-btn danger"
                                @click="removeVariable(index)"
                                title="删除变量"
                                aria-label="删除变量"
                            >
                                ×
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
                            class="icon-btn add"
                            @click="showAddSpeakerForm = !showAddSpeakerForm"
                            :title="showAddSpeakerForm ? '收起' : '添加说话人'"
                            :aria-label="
                                showAddSpeakerForm
                                    ? '收起添加说话人表单'
                                    : '添加说话人'
                            "
                        >
                            +
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
                                class="icon-btn danger"
                                @click="removeSpeaker(index)"
                                title="删除说话人"
                                aria-label="删除说话人"
                            >
                                ×
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
            <article
                v-for="script in filteredScripts"
                :key="script.id"
                class="script-card glass-morphism"
            >
                <div class="card-head">
                    <h3 :title="script.fileName">
                        <button
                            class="file-open-link"
                            @click="openScript(script)"
                            :title="`打开 ${script.fileName}`"
                        >
                            {{ script.fileName }}
                        </button>
                    </h3>
                    <button
                        class="delete-script-btn"
                        @click="deleteScript(script)"
                        title="删除脚本"
                        aria-label="删除脚本"
                    >
                        ×
                    </button>
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
import { computed, onMounted, reactive, ref, watch } from "vue";
import {
    parseGlobalConfigFromYAML,
    serializeGlobalConfigToYAML,
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
    authState: {
        type: Object,
        default: () => ({ loading: false, authenticated: false, user: null }),
    },
});

const emit = defineEmits([
    "open-script",
    "toggle-theme",
    "update-variables",
    "update-speakers",
    "login-github",
    "logout",
]);

const keyword = ref("");
const selectedFolder = ref("all");

const showAddVariableForm = ref(false);
const showAddSpeakerForm = ref(false);
const GLOBAL_CONFIG_CANDIDATE_PATHS = [
    "globalconfig.narrrail.yaml",
    "global-config.narrrail.yaml",
    "globalconfig.narrrail.yml",
    "global-config.narrrail.yml",
];
const globalConfigRepoPath = ref(GLOBAL_CONFIG_CANDIDATE_PATHS[0]);
const globalConfigFileName = computed(() => globalConfigRepoPath.value);
const globalConfigRepoSha = ref("");
const isSyncingGlobalConfig = ref(false);

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

const SCRIPT_LIST_STORAGE_KEY = "narrrail_mock_script_list";
const REPO_SELECTION_STORAGE_KEY = "narrrail_selected_repo";

const DEFAULT_MOCK_SCRIPTS = [];

const mockScripts = ref([...DEFAULT_MOCK_SCRIPTS]);
const repoOptions = ref([]);
const selectedRepoFullName = ref("");
const loadingRepos = ref(false);
const loadingScripts = ref(false);
const usingMockData = ref(true);
const isCreatingScript = ref(false);
const createScriptStatus = ref("");

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

    result = result.sort(
        (a, b) =>
            new Date(b.updatedAt).getTime() - new Date(a.updatedAt).getTime(),
    );

    return result;
});

async function addVariable() {
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

    const nextVariables = [...props.variables, variable];
    emit("update-variables", nextVariables);
    resetVariableForm();
    await syncGlobalConfigToRepo({
        variables: nextVariables,
        presetSpeakers: props.presetSpeakers,
    });
}

async function removeVariable(index) {
    const updated = [...props.variables];
    updated.splice(index, 1);
    emit("update-variables", updated);
    await syncGlobalConfigToRepo({
        variables: updated,
        presetSpeakers: props.presetSpeakers,
    });
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

async function addSpeaker() {
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
    const nextSpeakers = [...props.presetSpeakers, payload];
    emit("update-speakers", nextSpeakers);
    resetSpeakerForm();
    await syncGlobalConfigToRepo({
        variables: props.variables,
        presetSpeakers: nextSpeakers,
    });
}

async function removeSpeaker(index) {
    const updated = [...props.presetSpeakers];
    updated.splice(index, 1);
    emit("update-speakers", updated);
    await syncGlobalConfigToRepo({
        variables: props.variables,
        presetSpeakers: updated,
    });
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

function hasSelectedGithubRepo() {
    return (
        !usingMockData.value &&
        !!selectedOwner.value &&
        !!selectedRepoName.value
    );
}

async function syncGlobalConfigToRepo(overrideConfig = null) {
    if (!hasSelectedGithubRepo()) return;
    if (isSyncingGlobalConfig.value) return;

    isSyncingGlobalConfig.value = true;
    try {
        const sourceConfig = overrideConfig || {
            variables: props.variables,
            presetSpeakers: props.presetSpeakers,
        };

        const content = serializeGlobalConfigToYAML(sourceConfig);

        const payload = {
            owner: selectedOwner.value,
            repo: selectedRepoName.value,
            branch: selectedRepoBranch.value,
            path: globalConfigRepoPath.value,
            content,
            message: `chore(config): sync ${globalConfigRepoPath.value}`,
        };

        if (globalConfigRepoSha.value) {
            payload.sha = globalConfigRepoSha.value;
        }

        const response = await fetch("/api/github/commit-file", {
            method: "POST",
            credentials: "include",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(payload),
        });
        const data = await response.json();
        if (!response.ok) {
            throw new Error(data?.error || "同步全局配置到仓库失败");
        }

        globalConfigRepoSha.value =
            data?.content?.sha || globalConfigRepoSha.value || "";
    } catch (error) {
        alert(`同步全局配置失败: ${error.message}`);
    } finally {
        isSyncingGlobalConfig.value = false;
    }
}

async function deleteScript(script) {
    const path = String(script?.path || "");
    if (!path) return;

    const ok = confirm(`确认删除脚本？\n${path}`);
    if (!ok) return;

    if (!usingMockData.value && selectedOwner.value && selectedRepoName.value) {
        try {
            const readUrl = new URL(
                window.location.origin + "/api/github/file-content",
            );
            readUrl.searchParams.set("mode", "content");
            readUrl.searchParams.set("owner", selectedOwner.value);
            readUrl.searchParams.set("repo", selectedRepoName.value);
            readUrl.searchParams.set("branch", selectedRepoBranch.value);
            readUrl.searchParams.set("path", path);

            const readRes = await fetch(readUrl.toString(), {
                method: "GET",
                credentials: "include",
            });
            const readData = await readRes.json();
            if (!readRes.ok) {
                throw new Error(readData?.error || "读取脚本信息失败");
            }

            const delRes = await fetch("/api/github/delete-file", {
                method: "POST",
                credentials: "include",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({
                    owner: selectedOwner.value,
                    repo: selectedRepoName.value,
                    branch: selectedRepoBranch.value,
                    path,
                    sha: readData?.sha,
                    message: `chore(script): delete ${path}`,
                }),
            });
            const delData = await delRes.json();
            if (!delRes.ok) {
                throw new Error(delData?.error || "删除仓库脚本失败");
            }

            await reloadScriptsFromRepo();
            return;
        } catch (error) {
            alert(`删除脚本失败: ${error.message}`);
            return;
        }
    }

    mockScripts.value = mockScripts.value.filter((s) => s.path !== path);
}

function openScript(script) {
    emit("open-script", {
        id: script.id,
        storyId: script.storyId,
        fileName: script.fileName,
        path: script.path,
        source:
            script.source ||
            (usingMockData.value ? "mock-repository" : "github"),
        owner: script.owner || selectedOwner.value,
        repo: script.repo || selectedRepoName.value,
        branch: script.branch || selectedRepoBranch.value,
    });
}

const selectedRepo = computed(
    () =>
        repoOptions.value.find(
            (r) => r.fullName === selectedRepoFullName.value,
        ) || null,
);

const selectedOwner = computed(() => selectedRepo.value?.owner || "");
const selectedRepoName = computed(() => selectedRepo.value?.name || "");
const selectedRepoBranch = computed(
    () => selectedRepo.value?.defaultBranch || "main",
);

async function loadRepos() {
    if (!props.authState?.authenticated) return;
    loadingRepos.value = true;
    try {
        const res = await fetch("/api/github/repos", {
            method: "GET",
            credentials: "include",
        });
        const data = await res.json();
        if (!res.ok) throw new Error(data?.error || "加载仓库失败");

        repoOptions.value = Array.isArray(data?.repos) ? data.repos : [];

        const cached = localStorage.getItem(REPO_SELECTION_STORAGE_KEY) || "";
        const matched = repoOptions.value.some((r) => r.fullName === cached);
        if (matched) {
            selectedRepoFullName.value = cached;
        } else if (repoOptions.value.length > 0) {
            selectedRepoFullName.value = repoOptions.value[0].fullName;
        }
    } catch (error) {
        alert(`读取仓库列表失败: ${error.message}`);
    } finally {
        loadingRepos.value = false;
    }
}

async function loadGlobalConfigFromRepo() {
    if (!selectedOwner.value || !selectedRepoName.value) return;

    try {
        const url = new URL(
            window.location.origin + "/api/github/file-content",
        );
        url.searchParams.set("mode", "content");
        url.searchParams.set("owner", selectedOwner.value);
        url.searchParams.set("repo", selectedRepoName.value);
        url.searchParams.set("branch", selectedRepoBranch.value);
        let loadedData = null;
        let foundPath = "";

        for (const candidatePath of GLOBAL_CONFIG_CANDIDATE_PATHS) {
            url.searchParams.set("path", candidatePath);
            const res = await fetch(url.toString(), {
                method: "GET",
                credentials: "include",
            });
            const data = await res.json();

            if (res.ok) {
                loadedData = data;
                foundPath = candidatePath;
                break;
            }

            const notFound =
                res.status === 404 ||
                String(data?.error || "")
                    .toLowerCase()
                    .includes("not found");
            if (!notFound) {
                throw new Error(data?.error || "读取全局配置失败");
            }
        }

        if (!loadedData) {
            globalConfigRepoPath.value = GLOBAL_CONFIG_CANDIDATE_PATHS[0];
            globalConfigRepoSha.value = "";
            await syncGlobalConfigToRepo({
                variables: props.variables,
                presetSpeakers: props.presetSpeakers,
            });
            return;
        }

        const parsed = parseGlobalConfigFromYAML(
            String(loadedData?.content || ""),
        );
        emit("update-variables", parsed.variables || []);
        emit("update-speakers", parsed.presetSpeakers || []);
        globalConfigRepoPath.value = foundPath;
        globalConfigRepoSha.value = loadedData?.sha || "";
    } catch (error) {
        alert(`读取全局配置失败: ${error.message}`);
    }
}

async function reloadScriptsFromRepo() {
    if (!selectedOwner.value || !selectedRepoName.value) return;
    loadingScripts.value = true;
    try {
        const url = new URL(
            window.location.origin + "/api/github/file-content",
        );
        url.searchParams.set("mode", "list");
        url.searchParams.set("owner", selectedOwner.value);
        url.searchParams.set("repo", selectedRepoName.value);
        url.searchParams.set("branch", selectedRepoBranch.value);

        const res = await fetch(url.toString(), {
            method: "GET",
            credentials: "include",
        });
        const data = await res.json();
        if (!res.ok) throw new Error(data?.error || "读取脚本列表失败");

        const files = Array.isArray(data?.files) ? data.files : [];
        mockScripts.value = files.filter((file) =>
            String(file?.path || "").startsWith("Stories/"),
        );
        usingMockData.value = false;

        await loadGlobalConfigFromRepo();
    } catch (error) {
        alert(`读取仓库脚本失败: ${error.message}`);
    } finally {
        loadingScripts.value = false;
    }
}

function loadScriptListFromStorage() {
    try {
        const raw = localStorage.getItem(SCRIPT_LIST_STORAGE_KEY);
        if (!raw) return;
        const parsed = JSON.parse(raw);
        if (!Array.isArray(parsed)) return;

        const legacyPresetNames = new Set([
            "chapter_01_intro.narrrail.yaml",
            "chapter_01_choice.narrrail.yaml",
            "affinity_demo.narrrail.yaml",
            "new_story_temp.narrrail.yml",
            "ending_route_a.narrrail.yaml",
        ]);

        const filtered = parsed.filter(
            (item) => !legacyPresetNames.has(String(item?.fileName || "")),
        );

        mockScripts.value = filtered;
    } catch {
        // ignore invalid local data
    }
}

function saveScriptListToStorage() {
    try {
        localStorage.setItem(
            SCRIPT_LIST_STORAGE_KEY,
            JSON.stringify(mockScripts.value || []),
        );
    } catch {
        // ignore storage failures
    }
}

function buildNewStoryYaml(storyId) {
    return `meta:\n  schemaVersion: 1\n  storyId: ${storyId}\n  entryNodeId: \"\"\nvariables: []\nnodes: []\nedges: []\n`;
}

function sleep(ms) {
    return new Promise((resolve) => setTimeout(resolve, ms));
}

async function createNewScript() {
    const baseName = prompt("请输入新脚本名称（不含扩展名）", "new_story");
    if (!baseName) return;

    const normalized = String(baseName)
        .trim()
        .replace(/\s+/g, "_")
        .replace(/[^a-zA-Z0-9_\-]/g, "_")
        .toLowerCase();

    if (!normalized) {
        alert("脚本名称不能为空");
        return;
    }

    const fileName = `${normalized}.narrrail.yaml`;
    const existing = mockScripts.value.some((s) => s.fileName === fileName);
    if (existing) {
        alert("同名脚本已存在，请换一个名称");
        return;
    }

    const createdPath = `Stories/${fileName}`;

    if (!usingMockData.value && selectedOwner.value && selectedRepoName.value) {
        isCreatingScript.value = true;
        createScriptStatus.value = "正在提交到仓库...";
        try {
            const response = await fetch("/api/github/commit-file", {
                method: "POST",
                credentials: "include",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({
                    owner: selectedOwner.value,
                    repo: selectedRepoName.value,
                    branch: selectedRepoBranch.value,
                    path: createdPath,
                    content: buildNewStoryYaml(normalized),
                    message: `feat(script): create ${createdPath}`,
                }),
            });
            const data = await response.json();
            if (!response.ok) {
                throw new Error(data?.error || "创建仓库脚本失败");
            }

            selectedFolder.value = "all";
            keyword.value = "";

            let found = false;
            for (let i = 0; i < 6; i++) {
                createScriptStatus.value = `仓库同步中... (${i + 1}/6)`;
                await reloadScriptsFromRepo();
                found = mockScripts.value.some((s) => s.path === createdPath);
                if (found) break;
                await sleep(800);
            }

            createScriptStatus.value = found
                ? "新脚本已创建"
                : "已提交，仓库列表可能稍后刷新";
            setTimeout(() => {
                createScriptStatus.value = "";
            }, 2200);
            return;
        } catch (error) {
            createScriptStatus.value = "";
            alert(`创建仓库脚本失败: ${error.message}`);
            return;
        } finally {
            isCreatingScript.value = false;
        }
    }

    const created = {
        id: `s-${Date.now()}`,
        fileName,
        extension: ".yaml",
        path: createdPath,
        storyId: normalized,
        size: 0,
        updatedAt: new Date().toISOString(),
        tags: ["New", "Sandbox"],
        source: "mock-repository",
    };

    mockScripts.value = [created, ...mockScripts.value];
}

function formatSize(bytes) {
    if (bytes < 1024) return `${bytes} B`;
    if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)} KB`;
    return `${(bytes / (1024 * 1024)).toFixed(2)} MB`;
}

function formatDate(iso) {
    return new Date(iso).toLocaleString();
}

onMounted(async () => {
    loadScriptListFromStorage();
    if (props.authState?.authenticated) {
        await loadRepos();
        if (selectedRepoFullName.value) {
            await reloadScriptsFromRepo();
        }
    }
});

watch(
    () => mockScripts.value,
    () => {
        if (usingMockData.value) saveScriptListToStorage();
    },
    { deep: true },
);

watch(
    () => props.authState?.authenticated,
    async (authed) => {
        if (authed) {
            await loadRepos();
            if (selectedRepoFullName.value) {
                await reloadScriptsFromRepo();
            }
        } else {
            usingMockData.value = true;
            repoOptions.value = [];
            selectedRepoFullName.value = "";
            loadScriptListFromStorage();
        }
    },
);

watch(
    () => selectedRepoFullName.value,
    async (fullName) => {
        if (!fullName) return;
        localStorage.setItem(REPO_SELECTION_STORAGE_KEY, fullName);
        if (props.authState?.authenticated) {
            await reloadScriptsFromRepo();
        }
    },
);
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

.status-badge.auth-ok {
    color: #166534;
    background: rgba(34, 197, 94, 0.16);
    border-color: rgba(34, 197, 94, 0.38);
}

.status-badge.auth-off {
    color: #9f1239;
    background: rgba(244, 63, 94, 0.16);
    border-color: rgba(244, 63, 94, 0.36);
}

.filters {
    display: grid;
    grid-template-columns: minmax(540px, 2.4fr) minmax(220px, 1.2fr) minmax(
            180px,
            0.8fr
        );
    gap: 10px;
    padding: 12px;
    margin-bottom: 14px;
    align-items: end;
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

.repo-row {
    display: flex;
    align-items: center;
    gap: 10px;
}

.repo-row > select {
    flex: 1;
    min-width: 240px;
}

.summary {
    display: flex;
    align-items: center;
    gap: 8px;
    color: color-mix(in srgb, var(--nr-text) 78%, transparent);
    font-size: 13px;
    white-space: nowrap;
}

.inline-summary {
    margin-right: 2px;
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

.icon-btn {
    width: 26px;
    height: 26px;
    border-radius: 8px;
    border: 1px solid color-mix(in srgb, var(--nr-text) 18%, transparent);
    background: color-mix(in srgb, var(--nr-bg) 72%, #ffffff 28%);
    color: var(--nr-text);
    display: inline-flex;
    align-items: center;
    justify-content: center;
    font-size: 16px;
    font-weight: 700;
    line-height: 1;
    cursor: pointer;
    transition: all 0.15s ease;
}

.icon-btn:hover {
    transform: translateY(-1px);
}

.icon-btn.add {
    color: #4f46e5;
    border-color: color-mix(in srgb, #6366f1 42%, transparent);
    background: color-mix(in srgb, #6366f1 16%, transparent);
}

.icon-btn.danger {
    color: #dc2626;
    border-color: color-mix(in srgb, #ef4444 46%, transparent);
    background: color-mix(in srgb, #ef4444 16%, transparent);
}

.config-list {
    display: flex;
    flex-wrap: wrap;
    gap: 8px;
    margin-bottom: 8px;
}

.config-item {
    display: inline-flex;
    align-items: center;
    justify-content: flex-start;
    gap: 6px;
    width: auto;
    max-width: 100%;
    border: 1px solid color-mix(in srgb, var(--nr-text) 14%, transparent);
    border-radius: 999px;
    padding: 6px 8px;
    background: color-mix(in srgb, var(--nr-bg) 74%, #ffffff 26%);
    transition:
        padding-right 0.16s ease,
        border-color 0.16s ease,
        background 0.16s ease;
}

.config-item:hover {
    padding-right: 10px;
    border-color: color-mix(in srgb, var(--nr-text) 24%, transparent);
    background: color-mix(in srgb, var(--nr-bg) 68%, #ffffff 32%);
}

.config-item-main {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    font-size: 12px;
    white-space: nowrap;
}

.config-item .icon-btn.danger {
    width: 0;
    height: 20px;
    min-width: 0;
    padding: 0;
    border-width: 0;
    opacity: 0;
    overflow: hidden;
    pointer-events: none;
    font-size: 13px;
    border-radius: 999px;
    margin-left: 0;
    transform: scale(0.85);
    transition:
        width 0.16s ease,
        opacity 0.12s ease,
        margin-left 0.16s ease,
        border-width 0.16s ease,
        transform 0.16s ease;
}

.config-item:hover .icon-btn.danger {
    width: 20px;
    min-width: 20px;
    border-width: 1px;
    opacity: 1;
    pointer-events: auto;
    margin-left: 2px;
    transform: scale(1);
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

.delete-script-btn {
    opacity: 0;
    pointer-events: none;
    border: 1px solid color-mix(in srgb, #ef4444 46%, transparent);
    background: color-mix(in srgb, #ef4444 14%, transparent);
    color: #b91c1c;
    border-radius: 8px;
    width: 24px;
    height: 24px;
    padding: 0;
    font-size: 16px;
    font-weight: 700;
    line-height: 1;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    cursor: pointer;
    transition: all 0.15s ease;
}

.script-card:hover .delete-script-btn {
    opacity: 1;
    pointer-events: auto;
}

.delete-script-btn:hover {
    transform: translateY(-1px);
    background: color-mix(in srgb, #ef4444 20%, transparent);
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

.btn {
    border: none;
    border-radius: 10px;
    padding: 9px 12px;
    cursor: pointer;
    font-weight: 600;
}

.file-open-link {
    border: none;
    background: transparent;
    padding: 0;
    margin: 0;
    font: inherit;
    color: inherit;
    font-weight: 700;
    text-align: left;
    cursor: pointer;
}

.file-open-link:hover {
    color: #2563eb;
    text-decoration: underline;
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
