<template>
    <div class="script-library-page">
        <header class="top-bar glass-morphism">
            <div class="brand">
                <div class="title-wrap">
                    <h1>NarrRail Script Library</h1>
                    <div
                        v-if="canAccessLibraryContent"
                        class="library-context"
                    >
                        <span class="summary inline-summary">
                            共
                            <strong>{{ filteredScripts.length }}</strong>
                            个脚本
                        </span>
                        <div class="repo-picker">
                            <button
                                class="repo-picker-button"
                                :disabled="
                                    !authState?.authenticated || loadingRepos
                                "
                                @click="toggleRepoPopover"
                                :title="selectedRepoLabel"
                                aria-label="选择仓库"
                            >
                                <span>{{ selectedRepoLabel }}</span>
                                <IconGlyph name="chevron_right" />
                            </button>
                            <div
                                v-if="showRepoPopover"
                                class="repo-popover glass-morphism-strong"
                            >
                                <button
                                    v-for="repo in repoOptions"
                                    :key="repo.fullName"
                                    class="repo-option"
                                    :class="{
                                        selected:
                                            repo.fullName ===
                                            selectedRepoFullName,
                                    }"
                                    @click="selectRepo(repo.fullName)"
                                >
                                    {{ repo.fullName }}
                                </button>
                                <div
                                    v-if="repoOptions.length === 0"
                                    class="repo-empty"
                                >
                                    暂无可选仓库
                                </div>
                            </div>
                        </div>
                    </div>
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

                <div
                    v-if="canAccessLibraryContent"
                    class="search-action"
                >
                    <button
                        class="btn secondary top-icon-btn"
                        :class="{ active: showSearchPopover || keyword }"
                        @click="toggleSearchPopover"
                        title="搜索脚本"
                        aria-label="搜索脚本"
                    >
                        <IconGlyph name="search" />
                    </button>
                    <div
                        v-if="showSearchPopover"
                        class="search-popover"
                    >
                        <input
                            ref="searchInputRef"
                            v-model.trim="keyword"
                            type="text"
                            placeholder="按文件名 / ID / 路径搜索"
                            @keydown.esc.prevent="closeSearchPopover"
                            @keydown.stop
                        />
                        <button
                            v-if="keyword"
                            class="search-clear-btn"
                            @click="keyword = ''"
                            title="清空搜索"
                            aria-label="清空搜索"
                        >
                            <IconGlyph name="close" />
                        </button>
                    </div>
                </div>

                <button
                    class="btn secondary top-icon-btn"
                    @click="emit('open-overview')"
                    title="产品介绍"
                    aria-label="产品介绍"
                >
                    <IconGlyph name="info" />
                </button>

                <button
                    class="btn secondary top-icon-btn"
                    @click="emit('toggle-theme')"
                    :title="isDarkMode ? '切换浅色' : '切换深色'"
                    :aria-label="isDarkMode ? '切换浅色' : '切换深色'"
                >
                    <IconGlyph :name="isDarkMode ? 'light_mode' : 'dark_mode'" />
                </button>
                <button
                    class="btn secondary top-icon-btn"
                    @click="createNewScript"
                    :disabled="
                        (!authState?.authenticated && !isOffline) ||
                        isCreatingScript
                    "
                    :title="
                        !authState?.authenticated && !isOffline
                            ? '请先登录后新建脚本'
                            : isCreatingScript
                              ? '创建中...'
                              : isOffline
                                ? '离线新建本地脚本'
                                : '新建脚本'
                    "
                    :aria-label="
                        !authState?.authenticated && !isOffline
                            ? '请先登录后新建脚本'
                            : isCreatingScript
                              ? '创建中...'
                              : isOffline
                                ? '离线新建本地脚本'
                                : '新建脚本'
                    "
                >
                    <IconGlyph
                        :name="
                            isCreatingScript
                                ? 'progress_activity'
                                : 'add_circle'
                        "
                    />
                </button>

                <button
                    class="btn secondary top-icon-btn"
                    @click="openOutlineRail"
                    title="打开剧情总纲"
                    aria-label="打开剧情总纲"
                >
                    <IconGlyph name="account_tree" />
                </button>

                <button
                    class="btn secondary top-icon-btn"
                    @click="
                        authState?.authenticated
                            ? emit('logout')
                            : emit('login-github')
                    "
                    :disabled="authState?.loading"
                    :title="
                        authState?.loading
                            ? '检查登录中...'
                            : authState?.authenticated
                              ? '退出登录'
                              : 'GitHub 登录'
                    "
                    :aria-label="
                        authState?.loading
                            ? '检查登录中...'
                            : authState?.authenticated
                              ? '退出登录'
                              : 'GitHub 登录'
                    "
                >
                    <IconGlyph
                        :name="authState?.authenticated ? 'logout' : 'login'"
                    />
                </button>
            </div>
        </header>

        <section
            v-if="canAccessLibraryContent"
            class="global-config glass-morphism"
        >
            <div class="global-config-header">
                <div class="header-main">
                    <h2>全局配置</h2>
                </div>
                <div class="header-actions">
                    <span
                        v-if="loadingScripts"
                        class="subtle-loading-dot"
                        aria-live="polite"
                        title="全局配置加载中"
                        aria-label="全局配置加载中"
                    ></span>
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
                            :disabled="loadingScripts"
                            :title="
                                loadingScripts
                                    ? '全局配置加载中，请稍候'
                                    : showAddVariableForm
                                      ? '收起'
                                      : '添加变量'
                            "
                            :aria-label="
                                loadingScripts
                                    ? '全局配置加载中，请稍候'
                                    : showAddVariableForm
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
                        <h3>预设角色</h3>
                        <button
                            class="icon-btn add"
                            @click="showAddSpeakerForm = !showAddSpeakerForm"
                            :disabled="loadingScripts"
                            :title="
                                loadingScripts
                                    ? '全局配置加载中，请稍候'
                                    : showAddSpeakerForm
                                      ? '收起'
                                      : '添加角色'
                            "
                            :aria-label="
                                loadingScripts
                                    ? '全局配置加载中，请稍候'
                                    : showAddSpeakerForm
                                      ? '收起添加角色表单'
                                      : '添加角色'
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
                                title="删除角色"
                                aria-label="删除角色"
                            >
                                ×
                            </button>
                        </div>
                    </div>
                    <div v-else class="config-empty">暂无预设角色</div>

                    <div v-if="showAddSpeakerForm" class="inline-form">
                        <input
                            v-model.trim="newSpeaker.id"
                            type="text"
                            placeholder="角色 ID"
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

        <section
            v-if="!canAccessLibraryContent"
            class="empty glass-morphism auth-empty"
        >
            <h3>你还没有登录 GitHub</h3>
            <p>请点击右上角最右侧按钮登录后查看脚本列表与全局配置。</p>
        </section>

        <main v-if="canAccessLibraryContent" class="grid">
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
                            {{ formatScriptDisplayName(script.fileName) }}
                        </button>
                    </h3>
                    <div class="card-actions">
                        <button
                            class="rename-script-btn"
                            @click="renameScript(script)"
                            title="重命名脚本"
                            aria-label="重命名脚本"
                        >
                            <svg
                                class="card-action-icon"
                                viewBox="0 0 24 24"
                                fill="none"
                                aria-hidden="true"
                            >
                                <path
                                    d="M4 20h4l10-10a2 2 0 0 0-4-4L4 16v4z"
                                    stroke="currentColor"
                                    stroke-width="1.8"
                                    stroke-linecap="round"
                                    stroke-linejoin="round"
                                />
                            </svg>
                        </button>
                        <button
                            class="delete-script-btn"
                            @click="deleteScript(script)"
                            title="删除脚本"
                            aria-label="删除脚本"
                        >
                            <svg
                                class="card-action-icon"
                                viewBox="0 0 24 24"
                                fill="none"
                                aria-hidden="true"
                            >
                                <path
                                    d="M6 6l12 12M18 6L6 18"
                                    stroke="currentColor"
                                    stroke-width="1.8"
                                    stroke-linecap="round"
                                    stroke-linejoin="round"
                                />
                            </svg>
                        </button>
                    </div>
                </div>

                <div class="card-bottom">
                    <div class="card-meta-spacer">
                        <div class="middle-stats">
                            <span
                                >Nodes:
                                {{ formatCount(script.nodeCount) }}</span
                            >
                            <span
                                >Edges:
                                {{ formatCount(script.edgeCount) }}</span
                            >
                            <span>{{ formatSize(script.size) }}</span>
                        </div>
                    </div>

                    <div class="meta">
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
import {
    computed,
    nextTick,
    onMounted,
    onUnmounted,
    reactive,
    ref,
    watch,
} from "vue";
import {
    parseGlobalConfigFromYAML,
    serializeGlobalConfigToYAML,
} from "../utils/global-config-yaml.js";
import {
    formatSpeakerLabel,
    validateSpeakerId,
    validateVariableName,
} from "../utils/editor-form-utils.js";

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
    "open-rail",
    "open-overview",
    "toggle-theme",
    "update-variables",
    "update-speakers",
    "login-github",
    "logout",
]);

const keyword = ref("");
const showSearchPopover = ref(false);
const searchInputRef = ref(null);
const showRepoPopover = ref(false);

const showAddVariableForm = ref(false);
const showAddSpeakerForm = ref(false);
const GLOBAL_CONFIG_CANDIDATE_PATHS = [
    "globalconfig.nrstory",
    "global-config.nrstory",
];
const globalConfigRepoPath = ref(GLOBAL_CONFIG_CANDIDATE_PATHS[0]);
const LOCAL_GLOBAL_CONFIG_STORAGE_KEY = "narrrail_local_global_config";
const globalConfigFileName = computed(() =>
    props.authState?.authenticated
        ? globalConfigRepoPath.value
        : "localStorage",
);
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
const LOCAL_SCRIPT_CONTENT_PREFIX = "narrrail_local_script_content_";

const DEFAULT_MOCK_SCRIPTS = [];

const mockScripts = ref([...DEFAULT_MOCK_SCRIPTS]);
const repoOptions = ref([]);
const selectedRepoFullName = ref("");
const loadingRepos = ref(false);
const loadingScripts = ref(false);
const usingMockData = ref(true);
const isCreatingScript = ref(false);
const createScriptStatus = ref("");
let globalConfigMutationVersion = 0;
let pendingGlobalConfigSync = null;

const isLocalDevHost =
    typeof window !== "undefined" &&
    ["localhost", "127.0.0.1"].includes(window.location.hostname);

const isOffline = ref(
    isLocalDevHost || (typeof navigator !== "undefined" && !navigator.onLine),
);

const canAccessLibraryContent = computed(
    () => !!props.authState?.authenticated || isOffline.value,
);

function syncOfflineState() {
    isOffline.value =
        isLocalDevHost ||
        (typeof navigator !== "undefined" && !navigator.onLine);
}

function toSortableTime(value) {
    const timestamp = new Date(value).getTime();
    return Number.isFinite(timestamp) ? timestamp : Number.NEGATIVE_INFINITY;
}

const filteredScripts = computed(() => {
    const kw = keyword.value.toLowerCase();
    let result = mockScripts.value.filter((s) => {
        const isRail = String(s.extension || "").toLowerCase() === ".nrrail";
        if (isRail) return false;
        const inKeyword =
            !kw ||
            s.fileName.toLowerCase().includes(kw) ||
            s.storyId.toLowerCase().includes(kw) ||
            s.path.toLowerCase().includes(kw);
        return inKeyword;
    });

    result = result.sort((a, b) => {
        const diff = toSortableTime(b.updatedAt) - toSortableTime(a.updatedAt);
        if (diff !== 0) return diff;
        return String(a.fileName || "").localeCompare(String(b.fileName || ""));
    });

    return result;
});

async function toggleSearchPopover() {
    showSearchPopover.value = !showSearchPopover.value;
    if (showSearchPopover.value) {
        showRepoPopover.value = false;
        await nextTick();
        searchInputRef.value?.focus();
        searchInputRef.value?.select();
    }
}

function closeSearchPopover() {
    showSearchPopover.value = false;
}

function handleLibraryPointerDown(event) {
    const target = event?.target;
    if (!(target instanceof Element)) return;

    if (!target.closest(".repo-picker")) {
        showRepoPopover.value = false;
    }
    if (!target.closest(".search-action")) {
        showSearchPopover.value = false;
    }
}

async function addVariable() {
    const variableValidation = validateVariableName(
        props.variables,
        newVariable.name,
        { duplicate: "变量名已存在" },
    );
    if (!variableValidation.ok) {
        alert(variableValidation.message);
        return;
    }

    const variable = {
        name: variableValidation.value,
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
    globalConfigMutationVersion += 1;
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
    globalConfigMutationVersion += 1;
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
    const speakerValidation = validateSpeakerId(
        props.presetSpeakers,
        newSpeaker.id,
    );
    if (!speakerValidation.ok) {
        alert(speakerValidation.message);
        return;
    }

    const id = speakerValidation.value;
    const displayName = String(newSpeaker.displayName || "").trim();

    const payload = displayName ? { id, displayName } : { id };
    const nextSpeakers = [...props.presetSpeakers, payload];
    globalConfigMutationVersion += 1;
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
    globalConfigMutationVersion += 1;
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

function loadGlobalConfigFromLocal() {
    try {
        const raw = localStorage.getItem(LOCAL_GLOBAL_CONFIG_STORAGE_KEY);
        if (!raw) {
            emit("update-variables", []);
            emit("update-speakers", []);
            return;
        }

        const parsed = JSON.parse(raw);
        emit(
            "update-variables",
            Array.isArray(parsed?.variables) ? parsed.variables : [],
        );
        emit(
            "update-speakers",
            Array.isArray(parsed?.presetSpeakers) ? parsed.presetSpeakers : [],
        );
    } catch {
        emit("update-variables", []);
        emit("update-speakers", []);
    }
}

function syncGlobalConfigToLocal(config) {
    try {
        localStorage.setItem(
            LOCAL_GLOBAL_CONFIG_STORAGE_KEY,
            JSON.stringify({
                variables: Array.isArray(config?.variables)
                    ? config.variables
                    : [],
                presetSpeakers: Array.isArray(config?.presetSpeakers)
                    ? config.presetSpeakers
                    : [],
                updatedAt: new Date().toISOString(),
            }),
        );
    } catch {
        // ignore
    }
}

async function syncGlobalConfigToRepo(overrideConfig = null) {
    const sourceConfig = overrideConfig || {
        variables: props.variables,
        presetSpeakers: props.presetSpeakers,
    };
    syncGlobalConfigToLocal(sourceConfig);

    if (!props.authState?.authenticated || !hasSelectedGithubRepo()) {
        return;
    }

    if (isSyncingGlobalConfig.value) {
        pendingGlobalConfigSync = sourceConfig;
        return;
    }

    isSyncingGlobalConfig.value = true;
    try {
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
        if (pendingGlobalConfigSync) {
            const nextConfig = pendingGlobalConfigSync;
            pendingGlobalConfigSync = null;
            await syncGlobalConfigToRepo(nextConfig);
        }
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

    try {
        localStorage.removeItem(getLocalScriptStorageKey(path));
    } catch {
        // ignore storage failures
    }
}

function openScript(script) {
    if (String(script?.extension || "").toLowerCase() === ".nrrail") {
        emit("open-rail", {
            id: script.id,
            railId: script.railId || script.storyId,
            fileName: script.fileName,
            path: script.path,
            source:
                script.source ||
                (usingMockData.value ? "mock-repository" : "github"),
            owner: script.owner || selectedOwner.value,
            repo: script.repo || selectedRepoName.value,
            branch: script.branch || selectedRepoBranch.value,
            localStorageKey:
                script.localStorageKey ||
                (script.source === "mock-repository"
                    ? getLocalScriptStorageKey(script.path)
                    : ""),
        });
        return;
    }

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
        localStorageKey:
            script.localStorageKey ||
            (script.source === "mock-repository"
                ? getLocalScriptStorageKey(script.path)
                : ""),
    });
}

const selectedRepo = computed(
    () =>
        repoOptions.value.find(
            (r) => r.fullName === selectedRepoFullName.value,
        ) || null,
);

const selectedRepoLabel = computed(() => {
    if (loadingRepos.value) return "仓库加载中...";
    return selectedRepoFullName.value || "请选择仓库";
});

const selectedOwner = computed(() => selectedRepo.value?.owner || "");
const selectedRepoName = computed(() => selectedRepo.value?.name || "");
const selectedRepoBranch = computed(
    () => selectedRepo.value?.defaultBranch || "main",
);

function toggleRepoPopover() {
    if (!props.authState?.authenticated || loadingRepos.value) return;
    showRepoPopover.value = !showRepoPopover.value;
    if (showRepoPopover.value) {
        showSearchPopover.value = false;
    }
}

function selectRepo(fullName) {
    selectedRepoFullName.value = fullName;
    showRepoPopover.value = false;
}

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

    const repoSnapshot = `${selectedOwner.value}/${selectedRepoName.value}@${selectedRepoBranch.value}`;
    const requestMutationVersion = globalConfigMutationVersion;

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

        if (
            `${selectedOwner.value}/${selectedRepoName.value}@${selectedRepoBranch.value}` !==
            repoSnapshot
        ) {
            return;
        }
        if (requestMutationVersion !== globalConfigMutationVersion) {
            return;
        }

        if (!loadedData) {
            const emptyConfig = {
                variables: [],
                presetSpeakers: [],
            };
            emit("update-variables", emptyConfig.variables);
            emit("update-speakers", emptyConfig.presetSpeakers);
            globalConfigRepoPath.value = GLOBAL_CONFIG_CANDIDATE_PATHS[0];
            globalConfigRepoSha.value = "";
            await syncGlobalConfigToRepo(emptyConfig);
            return;
        }

        const parsed = parseGlobalConfigFromYAML(
            String(loadedData?.content || ""),
        );
        if (requestMutationVersion !== globalConfigMutationVersion) {
            return;
        }
        emit("update-variables", parsed.variables || []);
        emit("update-speakers", parsed.presetSpeakers || []);
        syncGlobalConfigToLocal(parsed);
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

function getLocalScriptStorageKey(path) {
    return `${LOCAL_SCRIPT_CONTENT_PREFIX}${String(path || "")}`;
}

function buildInitialLocalScriptData(storyId) {
    return {
        meta: {
            schemaVersion: 1,
            storyId: String(storyId || "NewStory"),
            entryNodeId: "",
        },
        variables: [],
        nodes: [],
        edges: [],
        updatedAt: new Date().toISOString(),
    };
}

function saveLocalScriptContent(path, payload) {
    try {
        localStorage.setItem(
            getLocalScriptStorageKey(path),
            JSON.stringify({
                ...(payload || {}),
                updatedAt: new Date().toISOString(),
            }),
        );
    } catch {
        // ignore storage failures
    }
}

function loadScriptListFromStorage() {
    try {
        const raw = localStorage.getItem(SCRIPT_LIST_STORAGE_KEY);
        if (!raw) return;
        const parsed = JSON.parse(raw);
        if (!Array.isArray(parsed)) return;

        const legacyPresetNames = new Set([
            "chapter_01_intro.nrstory",
            "chapter_01_choice.nrstory",
            "affinity_demo.nrstory",
            "new_story_temp.nrstory",
            "ending_route_a.nrstory",
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

function toYamlDoubleQuoted(value) {
    return `"${String(value).replace(/\\/g, "\\\\").replace(/"/g, '\\"')}"`;
}

function buildNewStoryYaml(storyId) {
    return `meta:\n  schemaVersion: 1\n  storyId: ${toYamlDoubleQuoted(storyId)}\n  entryNodeId: \"\"\nvariables: []\nnodes: []\nedges: []\n`;
}

function buildNewRailYaml(railId) {
    return `meta:\n  schemaVersion: 1\n  railId: ${toYamlDoubleQuoted(railId)}\n  title: ${toYamlDoubleQuoted("剧情总纲")}\n  entryNodeId: "rail-start"\nnodes:\n  - nodeId: rail-start\n    nodeType: Note\n    title: "总纲开始"\n    summary: ""\n  - nodeId: rail-end\n    nodeType: End\n    title: "总纲结束"\n    summary: ""\nedges:\n  - sourceNodeId: rail-start\n    sourceHandle: ""\n    targetNodeId: rail-end\n    priority: 0\n`;
}

function sleep(ms) {
    return new Promise((resolve) => setTimeout(resolve, ms));
}

function sanitizeScriptStem(name) {
    return String(name || "")
        .trim()
        .replace(/\s+/g, "_")
        .replace(/[<>:"/\\|?*\x00-\x1F]/g, "_")
        .replace(/^\.+|\.+$/g, "")
        .replace(/_+/g, "_");
}

function replaceStoryIdInYaml(yamlText, nextStoryId) {
    const normalized = String(nextStoryId || "").trim();
    const escaped = normalized.replace(/\\/g, "\\\\").replace(/"/g, '\\"');

    const source = String(yamlText || "");
    const replaced = source.replace(
        /^([ \t]*storyId\s*:\s*)(.*)$/m,
        `$1"${escaped}"`,
    );

    if (replaced !== source) return replaced;

    if (/^meta\s*:/m.test(source)) {
        return source.replace(
            /^meta\s*:[ \t]*$/m,
            `meta:\n  storyId: "${escaped}"`,
        );
    }

    return `meta:\n  storyId: "${escaped}"\n${source}`;
}

async function renameScript(script) {
    const oldPath = String(script?.path || "");
    const oldFileName = String(script?.fileName || "");
    const oldStem = formatScriptDisplayName(oldFileName);
    const isRail = String(script?.extension || "").toLowerCase() === ".nrrail";
    if (!oldPath || !oldFileName) return;

    const baseName = prompt("请输入新的脚本名称（不含扩展名）", oldStem);
    if (!baseName) return;

    const safeStem = sanitizeScriptStem(baseName);
    if (!safeStem) {
        alert("脚本名称不能为空");
        return;
    }

    const newFileName = `${safeStem}${isRail ? ".nrrail" : ".nrstory"}`;
    const newPath = `Stories/${newFileName}`;

    if (newPath === oldPath) return;

    const pathExists = mockScripts.value.some((s) => s.path === newPath);
    if (pathExists) {
        alert("目标名称已存在，请换一个名称");
        return;
    }

    if (!usingMockData.value && selectedOwner.value && selectedRepoName.value) {
        try {
            const readUrl = new URL(
                window.location.origin + "/api/github/file-content",
            );
            readUrl.searchParams.set("mode", "content");
            readUrl.searchParams.set("owner", selectedOwner.value);
            readUrl.searchParams.set("repo", selectedRepoName.value);
            readUrl.searchParams.set("branch", selectedRepoBranch.value);
            readUrl.searchParams.set("path", oldPath);

            const readRes = await fetch(readUrl.toString(), {
                method: "GET",
                credentials: "include",
            });
            const readData = await readRes.json();
            if (!readRes.ok) {
                throw new Error(readData?.error || "读取原脚本失败");
            }

            const updatedContent = isRail
                ? String(readData?.content || "").replace(
                      /^([ \t]*railId\s*:\s*)(.*)$/m,
                      `$1"${safeStem.replace(/\\/g, "\\\\").replace(/"/g, '\\"')}"`,
                  )
                : replaceStoryIdInYaml(String(readData?.content || ""), safeStem);

            const createRes = await fetch("/api/github/commit-file", {
                method: "POST",
                credentials: "include",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({
                    owner: selectedOwner.value,
                    repo: selectedRepoName.value,
                    branch: selectedRepoBranch.value,
                    path: newPath,
                    content: updatedContent,
                    message: `${isRail ? "feat(rail)" : "feat(script)"}: rename ${oldPath} -> ${newPath}`,
                }),
            });
            const createData = await createRes.json();
            if (!createRes.ok) {
                throw new Error(createData?.error || "创建新名称脚本失败");
            }

            const delRes = await fetch("/api/github/delete-file", {
                method: "POST",
                credentials: "include",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({
                    owner: selectedOwner.value,
                    repo: selectedRepoName.value,
                    branch: selectedRepoBranch.value,
                    path: oldPath,
                    sha: readData?.sha,
                    message: `${isRail ? "chore(rail)" : "chore(script)"}: delete old name ${oldPath}`,
                }),
            });
            const delData = await delRes.json();
            if (!delRes.ok) {
                throw new Error(delData?.error || "删除旧名称脚本失败");
            }

            await reloadScriptsFromRepo();
            alert(`重命名成功：\n${oldFileName} -> ${newFileName}`);
            return;
        } catch (error) {
            alert(`重命名脚本失败: ${error.message}`);
            return;
        }
    }

    const targetStorageKey = getLocalScriptStorageKey(newPath);
    const sourceStorageKey = getLocalScriptStorageKey(oldPath);

    let localData = null;
    try {
        const raw = localStorage.getItem(sourceStorageKey);
        localData = raw ? JSON.parse(raw) : null;
    } catch {
        localData = null;
    }

    if (localData && localData.meta && typeof localData.meta === "object") {
        if (isRail) localData.meta.railId = safeStem;
        else localData.meta.storyId = safeStem;
    }
    if (isRail && localData?.yaml) {
        localData.yaml = String(localData.yaml).replace(
            /^([ \t]*railId\s*:\s*)(.*)$/m,
            `$1"${safeStem.replace(/\\/g, "\\\\").replace(/"/g, '\\"')}"`,
        );
    } else if (localData && localData.meta && typeof localData.meta === "object") {
        localData.meta.storyId = safeStem;
    }

    saveLocalScriptContent(
        newPath,
        localData || buildInitialLocalScriptData(safeStem),
    );

    try {
        localStorage.removeItem(sourceStorageKey);
    } catch {
        // ignore storage failures
    }

    mockScripts.value = mockScripts.value.map((entry) => {
        if (entry.path !== oldPath) return entry;
        return {
            ...entry,
            id: newPath,
            fileName: newFileName,
            path: newPath,
            storyId: safeStem,
            railId: isRail ? safeStem : entry.railId,
            extension: isRail ? ".nrrail" : entry.extension,
            updatedAt: new Date().toISOString(),
            localStorageKey: targetStorageKey,
        };
    });

    alert(`重命名成功：\n${oldFileName} -> ${newFileName}`);
}

function createLocalScriptEntry({ safeStem, fileName, createdPath }) {
    const created = {
        id: `s-${Date.now()}`,
        fileName,
        extension: ".nrstory",
        path: createdPath,
        storyId: safeStem,
        size: 0,
        nodeCount: 0,
        edgeCount: 0,
        updatedAt: new Date().toISOString(),
        tags: ["Local", "Offline"],
        source: "mock-repository",
        localStorageKey: getLocalScriptStorageKey(createdPath),
    };

    saveLocalScriptContent(createdPath, buildInitialLocalScriptData(safeStem));

    usingMockData.value = true;
    keyword.value = "";
    mockScripts.value = [created, ...mockScripts.value];
    saveScriptListToStorage();
}

function createLocalRailEntry({ safeStem, fileName, createdPath }) {
    const created = {
        id: `r-${Date.now()}`,
        fileName,
        extension: ".nrrail",
        path: createdPath,
        storyId: safeStem,
        railId: safeStem,
        size: 0,
        nodeCount: 2,
        edgeCount: 1,
        updatedAt: new Date().toISOString(),
        tags: ["Outline", "Local"],
        source: "mock-repository",
        localStorageKey: getLocalScriptStorageKey(createdPath),
    };

    saveLocalScriptContent(createdPath, {
        yaml: buildNewRailYaml(safeStem),
        updatedAt: new Date().toISOString(),
    });

    usingMockData.value = true;
    keyword.value = "";
    mockScripts.value = [created, ...mockScripts.value];
    saveScriptListToStorage();
    return created;
}

async function openOutlineRail() {
    const existing =
        mockScripts.value.find(
            (s) => String(s.extension || "").toLowerCase() === ".nrrail",
        ) || null;

    if (existing) {
        openScript(existing);
        return;
    }

    const safeStem = "main_story";
    const fileName = "main_story.nrrail";
    const createdPath = "Stories/main_story.nrrail";

    if (!usingMockData.value && selectedOwner.value && selectedRepoName.value) {
        isCreatingScript.value = true;
        createScriptStatus.value = "正在创建剧情总纲...";
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
                    content: buildNewRailYaml(safeStem),
                    message: `feat(rail): create ${createdPath}`,
                }),
            });
            const data = await response.json();
            if (!response.ok) {
                throw new Error(data?.error || "创建仓库总纲失败");
            }

            const created = {
                id: createdPath,
                fileName,
                extension: ".nrrail",
                path: createdPath,
                storyId: safeStem,
                railId: safeStem,
                nodeCount: 2,
                edgeCount: 1,
                updatedAt: new Date().toISOString(),
                tags: ["GitHub"],
                source: "github",
                owner: selectedOwner.value,
                repo: selectedRepoName.value,
                branch: selectedRepoBranch.value,
            };

            keyword.value = "";
            mockScripts.value = [created, ...mockScripts.value];
            createScriptStatus.value = "剧情总纲已创建";

            await reloadScriptsFromRepo();
            const latest =
                mockScripts.value.find((s) => s.path === createdPath) ||
                created;
            openScript(latest);

            setTimeout(() => {
                createScriptStatus.value = "";
            }, 2200);
            return;
        } catch (error) {
            createScriptStatus.value = "";
            alert(`创建仓库总纲失败: ${error.message}`);
            return;
        } finally {
            isCreatingScript.value = false;
        }
    }

    const created = createLocalRailEntry({
        safeStem,
        fileName,
        createdPath,
    });
    openScript(created);
}

async function createNewScript() {
    const baseName = prompt("请输入新脚本名称（不含扩展名）", "new_story");
    if (!baseName) return;

    const normalized = String(baseName).trim();

    const safeStem = sanitizeScriptStem(normalized);

    if (!safeStem) {
        alert("脚本名称不能为空");
        return;
    }

    const fileName = `${safeStem}.nrstory`;
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
                    content: buildNewStoryYaml(safeStem),
                    message: `feat(script): create ${createdPath}`,
                }),
            });
            const data = await response.json();
            if (!response.ok) {
                throw new Error(data?.error || "创建仓库脚本失败");
            }

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

            const useOffline = confirm(
                `创建仓库脚本失败：${error.message}\n\n是否改为离线本地创建？`,
            );
            if (useOffline) {
                createLocalScriptEntry({ safeStem, fileName, createdPath });
                alert("已在本地离线创建脚本。联网后可继续同步到仓库。");
                return;
            }

            alert(`创建仓库脚本失败: ${error.message}`);
            return;
        } finally {
            isCreatingScript.value = false;
        }
    }

    createLocalScriptEntry({ safeStem, fileName, createdPath });
}

function formatSize(bytes) {
    if (bytes < 1024) return `${bytes} B`;
    if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)} KB`;
    return `${(bytes / (1024 * 1024)).toFixed(2)} MB`;
}

function formatCount(value) {
    const n = Number(value);
    return Number.isFinite(n) && n >= 0 ? n : "--";
}

function formatDate(iso) {
    const timestamp = new Date(iso).getTime();
    if (!Number.isFinite(timestamp)) return "--";
    return new Date(timestamp).toLocaleString();
}

function formatScriptDisplayName(fileName) {
    return String(fileName || "").replace(/\.(nrstory|nrrail)$/i, "");
}

onMounted(async () => {
    syncOfflineState();
    window.addEventListener("pointerdown", handleLibraryPointerDown, true);
    window.addEventListener("online", syncOfflineState);
    window.addEventListener("offline", syncOfflineState);

    loadScriptListFromStorage();
    if (props.authState?.loading) {
        return;
    }

    if (props.authState?.authenticated) {
        await loadRepos();
        if (selectedRepoFullName.value) {
            await reloadScriptsFromRepo();
        }
    } else {
        loadGlobalConfigFromLocal();
    }
});

onUnmounted(() => {
    window.removeEventListener("pointerdown", handleLibraryPointerDown, true);
    window.removeEventListener("online", syncOfflineState);
    window.removeEventListener("offline", syncOfflineState);
});

watch(
    () => mockScripts.value,
    () => {
        if (usingMockData.value) saveScriptListToStorage();
    },
    { deep: true },
);

watch(
    () => [props.authState?.loading, props.authState?.authenticated],
    async ([loading, authed]) => {
        if (loading) return;

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
            loadGlobalConfigFromLocal();
        }
    },
);

watch(
    () => selectedRepoFullName.value,
    async (fullName) => {
        if (!fullName) return;
        localStorage.setItem(REPO_SELECTION_STORAGE_KEY, fullName);

        globalConfigRepoPath.value = GLOBAL_CONFIG_CANDIDATE_PATHS[0];
        globalConfigRepoSha.value = "";

        if (props.authState?.authenticated) {
            await reloadScriptsFromRepo();
        }
    },
);
</script>

<style scoped>
.script-library-page {
    display: flex;
    flex-direction: column;
    height: 100vh;
    min-height: 0;
    overflow: hidden;
    overscroll-behavior: contain;
    padding: 20px;
    color: var(--nr-text);
    background: transparent;
    position: relative;
    z-index: 2;
    pointer-events: auto;
}

:global(body.dark-theme) .script-library-page,
:global(body[data-theme="dark"]) .script-library-page {
    background:
        linear-gradient(
            180deg,
            rgba(2, 6, 23, 0.72),
            rgba(2, 6, 23, 0.86)
        ),
        radial-gradient(
            circle at 50% 28%,
            rgba(14, 165, 233, 0.08),
            transparent 42%
        );
}

:global(body.dark-theme) .script-library-page .glass-morphism,
:global(body.dark-theme) .script-library-page .glass-morphism-strong,
:global(body[data-theme="dark"]) .script-library-page .glass-morphism,
:global(body[data-theme="dark"]) .script-library-page .glass-morphism-strong {
    background: rgba(15, 23, 42, 0.82) !important;
    border-color: rgba(148, 163, 184, 0.22) !important;
    box-shadow:
        0 14px 32px rgba(0, 0, 0, 0.28),
        inset 0 1px 0 rgba(255, 255, 255, 0.04) !important;
}

:global(body.dark-theme) .script-library-page .top-bar,
:global(body[data-theme="dark"]) .script-library-page .top-bar {
    background: rgba(15, 23, 42, 0.78) !important;
}

.top-bar {
    display: flex;
    justify-content: space-between;
    gap: 12px;
    align-items: center;
    padding: 16px 18px;
    margin-bottom: 14px;
    border-bottom: none;
    position: relative;
    z-index: 20;
    overflow: visible;
}

.brand {
    display: flex;
    align-items: center;
    gap: 12px;
    min-width: 0;
    flex: 1;
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
    font-size: 24px;
    font-weight: 700;
}

.title-wrap {
    display: flex;
    align-items: center;
    gap: 18px;
    min-width: 0;
}

.library-context {
    display: flex;
    align-items: center;
    gap: 12px;
    min-width: 0;
}

.repo-picker {
    position: relative;
    min-width: 260px;
    width: clamp(260px, 28vw, 420px);
}

.repo-picker-button {
    min-width: 260px;
    max-width: 420px;
    width: clamp(260px, 28vw, 420px);
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 10px;
    background: color-mix(in srgb, var(--nr-bg) 70%, #ffffff 30%);
    border: 1px solid color-mix(in srgb, var(--nr-text) 18%, transparent);
    color: var(--nr-text);
    border-radius: 999px;
    padding: 8px 12px 8px 14px;
    outline: none;
    font-size: 13px;
    font-weight: 700;
    cursor: pointer;
}

.repo-picker-button span {
    min-width: 0;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
}

.repo-picker-button :deep(.icon-glyph) {
    flex: 0 0 auto;
    width: 16px;
    height: 16px;
    transform: rotate(90deg);
    opacity: 0.74;
}

.repo-picker-button:focus,
.repo-picker-button:hover {
    border-color: color-mix(in srgb, #60a5fa 58%, var(--nr-text) 18%);
    box-shadow: 0 0 0 3px rgba(96, 165, 250, 0.16);
}

.repo-picker-button:disabled {
    opacity: 0.72;
    cursor: not-allowed;
}

.repo-popover {
    position: absolute;
    top: calc(100% + 8px);
    left: 0;
    width: min(420px, 78vw);
    max-height: 300px;
    overflow: auto;
    padding: 8px;
    border-radius: 14px;
    z-index: 30;
}

.repo-option {
    width: 100%;
    border: none;
    border-radius: 10px;
    padding: 9px 10px;
    background: transparent;
    color: var(--nr-text);
    cursor: pointer;
    text-align: left;
    font-size: 13px;
    font-weight: 700;
    overflow-wrap: anywhere;
}

.repo-option:hover,
.repo-option.selected {
    background: rgba(96, 165, 250, 0.14);
    color: #60a5fa;
}

.repo-empty {
    padding: 10px;
    color: color-mix(in srgb, var(--nr-text) 56%, transparent);
    font-size: 12px;
    text-align: center;
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
    flex-shrink: 0;
    position: relative;
}

.top-actions .top-icon-btn,
.top-actions .top-icon-btn.btn,
.top-actions .top-icon-btn.btn.secondary {
    width: 30px;
    height: 30px;
    padding: 0;
    border-radius: 8px;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    background: transparent !important;
    border: none !important;
    box-shadow: none !important;
    color: color-mix(in srgb, var(--nr-text) 72%, transparent);
}

.top-icon-btn :deep(.icon-glyph) {
    font-size: 20px;
    opacity: 0.9;
}

.top-actions .top-icon-btn:hover,
.top-actions .top-icon-btn.btn:hover,
.top-actions .top-icon-btn.btn.secondary:hover {
    background: transparent !important;
    border: none !important;
    box-shadow: none !important;
    color: color-mix(in srgb, var(--nr-text) 94%, transparent);
}

.top-actions .top-icon-btn.active {
    color: #60a5fa;
}

.top-icon-btn:disabled {
    opacity: 0.7;
    cursor: not-allowed;
}

.search-action {
    position: relative;
    display: inline-flex;
    align-items: center;
}

.search-popover {
    position: absolute;
    top: calc(100% + 10px);
    right: -8px;
    width: 300px;
    max-width: min(300px, calc(100vw - 32px));
    padding: 9px 10px;
    border-radius: 14px;
    display: grid;
    grid-template-columns: minmax(0, 1fr) auto;
    align-items: center;
    gap: 4px;
    z-index: 80;
    background: color-mix(in srgb, var(--nr-bg) 58%, #ffffff 42%);
    border: 1px solid color-mix(in srgb, var(--nr-text) 18%, transparent);
    box-shadow:
        0 18px 42px rgba(15, 23, 42, 0.22),
        inset 0 1px 0 rgba(255, 255, 255, 0.16);
    backdrop-filter: blur(18px);
}

.search-popover input {
    min-width: 0;
    background: transparent;
    border: none;
    color: var(--nr-text);
    border-radius: 0;
    padding: 6px 4px;
    outline: none;
    font-size: 13px;
    font-weight: 700;
}

.search-popover input:focus {
    box-shadow: none;
}

.search-clear-btn {
    width: 34px;
    height: 34px;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    border: none;
    border-radius: 9px;
    color: color-mix(in srgb, var(--nr-text) 70%, transparent);
    background: transparent;
    cursor: pointer;
}

.search-clear-btn:hover {
    color: var(--nr-text);
    background: rgba(148, 163, 184, 0.14);
}

@media (max-width: 1180px) {
    .top-bar {
        align-items: flex-start;
        flex-direction: column;
    }

    .title-wrap {
        flex-wrap: wrap;
        row-gap: 10px;
    }

    .top-actions {
        align-self: flex-end;
        flex-wrap: wrap;
        justify-content: flex-end;
    }
}

@media (max-width: 720px) {
    .title-wrap,
    .library-context {
        width: 100%;
        align-items: flex-start;
        flex-direction: column;
    }

    .repo-picker,
    .repo-picker-button {
        width: 100%;
        min-width: 0;
        max-width: none;
    }

    .search-popover {
        right: auto;
        left: 50%;
        width: min(300px, calc(100vw - 32px));
        transform: translateX(-50%);
    }
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

.subtle-loading-dot {
    width: 8px;
    height: 8px;
    border-radius: 999px;
    background: color-mix(in srgb, #3b82f6 70%, var(--nr-text) 30%);
    opacity: 0.6;
    animation: subtleDotPulse 1.2s ease-in-out infinite;
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
    line-height: 1.2;
}

.config-item-main > * {
    display: inline-flex;
    align-items: center;
    line-height: 1.2;
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

@keyframes subtleDotPulse {
    0%,
    100% {
        opacity: 0.35;
        transform: scale(1);
    }
    50% {
        opacity: 0.75;
        transform: scale(1.08);
    }
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
    align-content: start;
    gap: 12px;
    flex: 1;
    min-height: 0;
    overflow-x: hidden;
    overflow-y: auto;
    padding: 0 14px 12px 0;
    scrollbar-gutter: stable;
    scrollbar-width: thin;
    scrollbar-color: color-mix(in srgb, var(--nr-text) 34%, transparent)
        color-mix(in srgb, var(--nr-bg) 42%, transparent);
    -webkit-overflow-scrolling: touch;
}

.grid::-webkit-scrollbar {
    width: 10px;
}

.grid::-webkit-scrollbar-track {
    background: color-mix(in srgb, var(--nr-bg) 42%, transparent);
    border-radius: 999px;
}

.grid::-webkit-scrollbar-thumb {
    background: color-mix(in srgb, var(--nr-text) 30%, transparent);
    border: 3px solid transparent;
    border-radius: 999px;
    background-clip: content-box;
}

.grid::-webkit-scrollbar-thumb:hover {
    background: color-mix(in srgb, var(--nr-text) 44%, transparent);
    border: 3px solid transparent;
    background-clip: content-box;
}

.script-card {
    padding: 12px;
    display: flex;
    flex-direction: column;
}

.card-bottom {
    margin-top: auto;
}

.card-actions {
    display: inline-flex;
    align-items: center;
    gap: 6px;
}

.rename-script-btn,
.delete-script-btn {
    opacity: 0;
    pointer-events: none;
    border-radius: 8px;
    width: 24px;
    height: 24px;
    padding: 0;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    cursor: pointer;
    transition: all 0.15s ease;
}

.card-action-icon {
    width: 14px;
    height: 14px;
    display: block;
    flex: 0 0 auto;
}

.rename-script-btn {
    border: 1px solid color-mix(in srgb, #3b82f6 46%, transparent);
    background: color-mix(in srgb, #3b82f6 12%, transparent);
    color: #1d4ed8;
}

.delete-script-btn {
    border: 1px solid color-mix(in srgb, #ef4444 46%, transparent);
    background: color-mix(in srgb, #ef4444 14%, transparent);
    color: #b91c1c;
    font-size: 16px;
}

.script-card:hover .rename-script-btn,
.script-card:hover .delete-script-btn {
    opacity: 1;
    pointer-events: auto;
}

.rename-script-btn:hover {
    transform: translateY(-1px);
    background: color-mix(in srgb, #3b82f6 18%, transparent);
}

.delete-script-btn:hover {
    transform: translateY(-1px);
    background: color-mix(in srgb, #ef4444 20%, transparent);
}

.card-head {
    display: flex;
    justify-content: space-between;
    gap: 8px;
    align-items: center;
}

.card-head h3 {
    margin: 0;
    font-size: 15px;
    line-height: 1.3;
    word-break: break-all;
}

.card-actions {
    align-self: center;
}

.path {
    margin: 8px 0;
    font-size: 12px;
    color: color-mix(in srgb, var(--nr-text) 55%, transparent);
    word-break: break-all;
}

.card-meta-spacer {
    min-height: 56px;
    display: flex;
    align-items: flex-end;
}

.middle-stats {
    display: flex;
    flex-wrap: wrap;
    gap: 8px;
    font-size: 12px;
    line-height: 1.4;
    color: color-mix(in srgb, var(--nr-text) 74%, transparent);
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
    margin-bottom: 0;
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

:global(body[data-theme="dark"]) .script-library-page .repo-picker-button,
:global(body[data-theme="dark"]) .script-library-page .inline-form input,
:global(body[data-theme="dark"]) .script-library-page .inline-form select,
:global(body.dark-theme) .script-library-page .repo-picker-button,
:global(body.dark-theme) .script-library-page .inline-form input,
:global(body.dark-theme) .script-library-page .inline-form select {
    background: rgba(15, 23, 42, 0.86);
    border-color: rgba(148, 163, 184, 0.22);
    color: #e5e7eb;
}

:global(body[data-theme="dark"]) .script-library-page .search-popover,
:global(body.dark-theme) .script-library-page .search-popover {
    background: rgba(15, 23, 42, 0.72);
    border-color: rgba(148, 163, 184, 0.22);
}

:global(body[data-theme="dark"]) .script-library-page .search-popover input,
:global(body.dark-theme) .script-library-page .search-popover input {
    color: #e5e7eb;
}

:global(body[data-theme="dark"]) .script-library-page .config-card,
:global(body[data-theme="dark"]) .script-library-page .script-card,
:global(body.dark-theme) .script-library-page .config-card,
:global(body.dark-theme) .script-library-page .script-card {
    background: rgba(15, 23, 42, 0.68);
    border-color: rgba(148, 163, 184, 0.18);
}
</style>
