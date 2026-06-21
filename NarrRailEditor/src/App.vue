<template>
    <ScriptLibraryPage
        v-if="currentView === 'library'"
        :is-dark-mode="darkModeEnabled"
        :variables="variables"
        :preset-speakers="presetSpeakers"
        :auth-state="authState"
        @toggle-theme="handleToggleDarkMode"
        @open-script="handleOpenScriptFromLibrary"
        @open-rail="handleOpenRailFromLibrary"
        @open-empty="handleOpenEmptyEditor"
        @open-overview="handleOpenOverview"
        @update-variables="handleVariablesUpdate"
        @update-speakers="handlePresetSpeakersUpdate"
        @login-github="handleLoginGithub"
        @logout="handleLogout"
    />

    <OverviewPage
        v-else-if="currentView === 'overview'"
        :is-dark-mode="darkModeEnabled"
        @back-library="handleGoLibrary"
        @start-editor="handleOpenEmptyEditor"
        @toggle-theme="handleToggleDarkMode"
    />

    <div v-else class="editor-container">
        <div class="main-content">
            <div class="graph-editor-wrapper">
                <GraphEditorWrapper
                    v-if="editorMode === 'graph'"
                    :nodes="activeNodes"
                    :edges="activeEdges"
                    :edge-render-mode="edgeRenderMode"
                    :preset-speakers="presetSpeakers"
                    :graph-mode="assetMode"
                />
                <ReadModePanel
                    v-else
                    :preview-mode="assetMode"
                    :nodes="assetMode === 'story' ? nodes : railNodes"
                    :edges="assetMode === 'story' ? edges : railEdges"
                    :variables="variables"
                    :entry-node-id="
                        assetMode === 'story'
                            ? storyMeta.entryNodeId
                            : railMeta.entryNodeId
                    "
                    :rail="{ meta: railMeta, nodes: railNodes, edges: railEdges }"
                    :resolve-story-by-id="resolveStoryById"
                    :is-dark-mode="darkModeEnabled"
                />
            </div>
        </div>

        <Toolbar
            :focus-mode-enabled="focusModeEnabled"
            :is-dark-mode="darkModeEnabled"
            :read-mode-enabled="editorMode === 'read'"
            :edge-style="edgeRenderMode"
            @new="handleNew"
            @import="handleImport"
            @export="handleExport"
            @undo="handleUndo"
            @redo="handleRedo"
            @validate="handleValidate"
            @auto-layout="handleAutoLayout"
            @toggle-edge-style="handleToggleEdgeRenderMode"
            @toggle-focus-mode="handleToggleFocusMode"
            @toggle-read-mode="handleToggleReadMode"
            @toggle-dark-mode="handleToggleDarkMode"
            @go-library="handleGoLibrary"
            @help="handleHelp"
        />

        <StatusBar
            :node-count="activeNodes.length"
            :edge-count="activeEdges.length"
            :story-id="activeAssetId"
            :entry-node-id="activeEntryNodeId"
            :last-saved-at="lastSavedAt"
        />

        <PropertyPanel
            v-if="editorMode === 'graph'"
            :selected-node="selectedNode"
            :entry-node-id="activeEntryNodeId"
            :variables="variables"
            :preset-speakers="presetSpeakers"
            :asset-mode="assetMode"
            :story-entries="availableStoryEntries"
            :is-dark-mode="darkModeEnabled"
            :open-multi-dialogue-request="multiDialogueOpenRequest"
            :open-dialogue-request="dialogueOpenRequest"
            :open-choice-request="choiceOpenRequest"
            :open-condition-request="conditionOpenRequest"
            @update="handleNodeUpdate"
            @set-entry-node="handleSetEntryNode"
        />

        <VariablePanel
            v-if="editorMode === 'graph' && assetMode === 'story'"
            :variables="variables"
            :preset-speakers="presetSpeakers"
            @update="handleVariablesUpdate"
            @update-speakers="handlePresetSpeakersUpdate"
        />

        <div
            v-if="editorMode === 'graph' && selectedEdge"
            class="edge-editor glass-morphism-strong"
        >
            <div class="edge-editor-header">
                <h3>边属性</h3>
                <button class="close-btn" @click="clearEdgeSelection">✕</button>
            </div>

            <div class="form-group">
                <label>Edge ID</label>
                <input class="form-input" :value="selectedEdge.id" readonly />
            </div>

            <div class="form-group">
                <label>Source → Target</label>
                <input
                    class="form-input"
                    :value="`${selectedEdge.source} → ${selectedEdge.target}`"
                    readonly
                />
            </div>

            <div class="form-group">
                <label>优先级 (priority)</label>
                <input
                    type="number"
                    class="form-input"
                    v-model.number="edgeDraft.priority"
                />
            </div>

            <div class="edge-editor-actions">
                <button class="action-btn primary" @click="applyEdgeDraft">
                    应用
                </button>
                <button class="action-btn" @click="resetEdgeDraft">重置</button>
            </div>
        </div>

        <input
            ref="fileInput"
            type="file"
            accept=".nrstory,.nrrail"
            style="display: none"
            @change="handleFileChange"
        />
    </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted, onUnmounted, watch } from "vue";
import Toolbar from "./components/Toolbar.vue";
import GraphEditorWrapper from "./components/GraphEditorWrapper.vue";
import PropertyPanel from "./components/PropertyPanel.vue";
import VariablePanel from "./components/VariablePanel.vue";
import StatusBar from "./components/StatusBar.vue";
import ReadModePanel from "./components/ReadModePanel.vue";
import ScriptLibraryPage from "./components/ScriptLibraryPage.vue";
import OverviewPage from "./components/OverviewPage.vue";
import { buildYAMLString, exportToYAML } from "./utils/yaml-exporter.js";
import { importFromYAML } from "./utils/yaml-importer.js";
import {
    buildEmptyRailData,
    buildRailYAMLString,
    exportRailToYAML,
    importRailFromYAML,
} from "./utils/rail-yaml.js";
import { validateRail } from "./utils/rail-validation.js";
import { validateStory } from "./utils/validation.js";
import { serializeGlobalConfigToYAML } from "./utils/global-config-yaml.js";
import storage from "./utils/storage.js";
import {
    getDialogueLineTexts,
    normalizeEditorDialogueNode,
    normalizeEditorStoryNodes,
} from "./utils/dialogue-node.js";

const nodes = ref([
    {
        id: "node-1",
        type: "dialogue",
        position: { x: 220, y: 120 },
        data: {
            speakerId: "Alice",
            lines: [{ textKey: "你好！欢迎使用 NarrRail 编辑器。" }],
        },
    },
    {
        id: "node-2",
        type: "choice",
        position: { x: 560, y: 120 },
        data: {
            choices: [{ textKey: "继续对话" }, { textKey: "结束对话" }],
            choiceMode: "SinglePass",
            choiceTimer: {
                enabled: false,
                durationSeconds: 8,
                timeoutChoiceTextKey: "超时",
            },
        },
    },
    {
        id: "node-3",
        type: "dialogue",
        position: { x: 900, y: 40 },
        data: { speakerId: "Bob", lines: [{ textKey: "很高兴认识你！" }] },
    },
    {
        id: "node-4",
        type: "end",
        position: { x: 900, y: 220 },
        data: {},
    },
]);

const edges = ref([
    {
        id: "e1-2",
        source: "node-1",
        target: "node-2",
        type: "straight",
        animated: false,
        data: { priority: 0 },
    },
    {
        id: "e2-3",
        source: "node-2",
        sourceHandle: "choice-0",
        target: "node-3",
        type: "straight",
        animated: false,
        data: { priority: 0 },
    },
    {
        id: "e2-4",
        source: "node-2",
        sourceHandle: "choice-1",
        target: "node-4",
        type: "straight",
        animated: false,
        data: { priority: 0 },
    },
]);

const selectedNode = ref(null);
const selectedEdge = ref(null);
const multiDialogueOpenRequest = ref(null);
const dialogueOpenRequest = ref(null);
const choiceOpenRequest = ref(null);
const conditionOpenRequest = ref(null);
const lastNodeClickMeta = ref({ id: "", time: 0 });
const recentDragSuppressUntil = ref(0);
const focusModeEnabled = ref(false);
const darkModeEnabled = ref(false);
const editorMode = ref("graph");
const assetMode = ref("story");
const edgeRenderMode = ref("straight");
const DARK_MODE_STORAGE_KEY = "narrrail_editor_theme";
const EVER_LOGGED_IN_STORAGE_KEY = "narrrail_ever_logged_in";
const LOCAL_SCRIPT_CONTENT_PREFIX = "narrrail_local_script_content_";

const authState = ref({
    loading: true,
    authenticated: false,
    user: null,
});

const IS_LOCAL_DEV_HOST =
    typeof window !== "undefined" &&
    ["localhost", "127.0.0.1"].includes(window.location.hostname);

const currentView = ref(
    IS_LOCAL_DEV_HOST ||
        localStorage.getItem(EVER_LOGGED_IN_STORAGE_KEY) === "1"
        ? "library"
        : "overview",
);
const selectedScriptEntry = ref(null);
const selectedRailEntry = ref(null);
const selectedGithubFileContext = ref(null);
const isSavingToGithub = ref(false);
const isSyncingGlobalConfigFromEditor = ref(false);
const GLOBAL_CONFIG_CANDIDATE_PATHS = [
    "globalconfig.nrstory",
    "global-config.nrstory",
];
const LOCAL_GLOBAL_CONFIG_STORAGE_KEY = "narrrail_local_global_config";
const globalConfigRepoPathFromEditor = ref(GLOBAL_CONFIG_CANDIDATE_PATHS[0]);

const undoStack = ref([]);
const redoStack = ref([]);
const isApplyingHistory = ref(false);
const isNodeDragInProgress = ref(false);
const MAX_HISTORY_SIZE = 100;

const storyMeta = ref({
    storyId: "DemoStory",
    entryNodeId: "node-1",
    schemaVersion: 1,
});

const emptyRail = buildEmptyRailData("main_story");
const railMeta = ref(safeClone(emptyRail.meta));
const railNodes = ref(safeClone(emptyRail.nodes));
const railEdges = ref(safeClone(emptyRail.edges));
const railValidationResult = ref({ errors: [], warnings: [] });

const activeNodes = computed(() =>
    assetMode.value === "rail" ? railNodes.value : nodes.value,
);
const activeEdges = computed(() =>
    assetMode.value === "rail" ? railEdges.value : edges.value,
);
const activeAssetId = computed(() =>
    assetMode.value === "rail"
        ? railMeta.value.railId || "main_story"
        : storyMeta.value.storyId || "NewStory",
);
const activeEntryNodeId = computed(() =>
    assetMode.value === "rail"
        ? railMeta.value.entryNodeId || ""
        : storyMeta.value.entryNodeId || "",
);

const variables = ref([]);
const presetSpeakers = ref([]);
const fileInput = ref(null);

const validationResult = ref({ errors: [], warnings: [] });
const lastSavedAt = ref("");
const hasErrors = computed(() => validationResult.value.errors.length > 0);
const hasWarnings = computed(() => validationResult.value.warnings.length > 0);

const edgeDraft = reactive({
    priority: 0,
});

const validationDebounceTimer = ref(null);
const libraryRefreshKey = ref(0);

let autoSaveTimer = null;
let autoSaveDirty = false;
let globalConfigSyncTimer = null;
let pendingValidationAfterDrag = false;
let pendingMockRepoPersistAfterDrag = false;
let pendingEdgeStyleAfterDrag = false;
let pendingDragGraphStateDetail = null;
let dragGraphStateRafId = 0;

function safeClone(obj) {
    return JSON.parse(JSON.stringify(obj));
}

function getLocalScriptStorageKeyByPath(path) {
    return `${LOCAL_SCRIPT_CONTENT_PREFIX}${String(path || "")}`;
}

function getStoredLibraryEntries() {
    try {
        const raw = localStorage.getItem("narrrail_mock_script_list");
        const parsed = raw ? JSON.parse(raw) : [];
        return Array.isArray(parsed) ? parsed : [];
    } catch {
        return [];
    }
}

const availableStoryEntries = computed(() =>
    (libraryRefreshKey.value, getStoredLibraryEntries())
        .filter((entry) => String(entry?.extension || ".nrstory") === ".nrstory")
        .map((entry) => ({
            ...entry,
            storyId:
                entry.storyId ||
                String(entry.fileName || "").replace(/\.nrstory$/i, ""),
        })),
);

function normalizeRailPayload(payload) {
    if (payload?.yaml) {
        return importRailFromYAML(String(payload.yaml || ""));
    }
    return {
        meta: payload?.meta || buildEmptyRailData("main_story").meta,
        nodes: Array.isArray(payload?.nodes) ? payload.nodes : [],
        edges: Array.isArray(payload?.edges) ? payload.edges : [],
        hasNodePositions: Array.isArray(payload?.nodes)
            ? payload.nodes.some(
                  (node) =>
                      Number.isFinite(Number(node?.position?.x)) &&
                      Number.isFinite(Number(node?.position?.y)),
              )
            : false,
    };
}

function persistCurrentRailToLocal() {
    const entry = selectedRailEntry.value;
    if (!entry || entry.source !== "mock-repository") return;

    const storageKey =
        entry.localStorageKey || getLocalScriptStorageKeyByPath(entry.path);
    if (!storageKey) return;

    saveLocalScriptContent(storageKey, {
        meta: safeClone(railMeta.value),
        nodes: safeClone(railNodes.value),
        edges: safeClone(railEdges.value),
        yaml: buildRailYAMLString(
            railNodes.value,
            railEdges.value,
            railMeta.value,
        ),
    });
}

async function resolveStoryById(storyId) {
    const normalized = String(storyId || "").trim();
    if (!normalized) return null;

    const localEntries = getStoredLibraryEntries();
    const localMatch = localEntries.find(
        (entry) =>
            String(entry?.extension || ".nrstory") === ".nrstory" &&
            String(entry?.storyId || "").trim() === normalized,
    );

    if (localMatch?.source === "mock-repository" || localMatch?.localStorageKey) {
        const storageKey =
            localMatch.localStorageKey ||
            getLocalScriptStorageKeyByPath(localMatch.path);
        const localData = loadLocalScriptContent(storageKey);
        if (localData?.nodes || localData?.edges) {
            return {
                meta: {
                    schemaVersion: localData?.meta?.schemaVersion ?? 1,
                    storyId: localData?.meta?.storyId || normalized,
                    entryNodeId: localData?.meta?.entryNodeId || "",
                },
                nodes: normalizeEditorStoryNodes(safeClone(localData.nodes || [])),
                edges: safeClone(localData.edges || []),
            };
        }
    }

    const githubContext =
        selectedRailEntry.value?.source === "github"
            ? selectedRailEntry.value
            : selectedGithubFileContext.value;
    if (githubContext?.owner && githubContext?.repo) {
        const listUrl = new URL(
            window.location.origin + "/api/github/file-content",
        );
        listUrl.searchParams.set("mode", "list");
        listUrl.searchParams.set("owner", githubContext.owner);
        listUrl.searchParams.set("repo", githubContext.repo);
        listUrl.searchParams.set("branch", githubContext.branch || "main");
        const listRes = await fetch(listUrl.toString(), {
            method: "GET",
            credentials: "include",
        });
        const listData = await listRes.json();
        if (!listRes.ok) throw new Error(listData?.error || "读取脚本列表失败");
        const match = (Array.isArray(listData?.files) ? listData.files : []).find(
            (entry) =>
                String(entry?.extension || "") === ".nrstory" &&
                String(entry?.storyId || "") === normalized,
        );
        if (!match?.path) return null;

        const contentUrl = new URL(
            window.location.origin + "/api/github/file-content",
        );
        contentUrl.searchParams.set("owner", githubContext.owner);
        contentUrl.searchParams.set("repo", githubContext.repo);
        contentUrl.searchParams.set("branch", githubContext.branch || "main");
        contentUrl.searchParams.set("path", match.path);
        const res = await fetch(contentUrl.toString(), {
            method: "GET",
            credentials: "include",
        });
        const data = await res.json();
        if (!res.ok) throw new Error(data?.error || "读取脚本内容失败");
        return importFromYAML(String(data?.content || ""));
    }

    return null;
}

function getVariableNameSet(variableList) {
    return new Set(
        (Array.isArray(variableList) ? variableList : [])
            .map((variable) => String(variable?.name || "").trim())
            .filter(Boolean),
    );
}

function clearDeletedVariableReferencesFromNode(node, deletedVariableNames) {
    if (!node || deletedVariableNames.size === 0) return node;
    const nextNode = safeClone(node);

    if (
        nextNode.type === "setvariable" &&
        deletedVariableNames.has(String(nextNode.data?.variableName || ""))
    ) {
        nextNode.data = nextNode.data || {};
        nextNode.data.variableName = "";
    }

    if (nextNode.type === "condition") {
        const branches = Array.isArray(nextNode.data?.condition?.branches)
            ? nextNode.data.condition.branches
            : Array.isArray(nextNode.data?.condition?.terms)
              ? [nextNode.data.condition]
              : [];

        branches.forEach((branch) => {
            const terms = Array.isArray(branch?.terms) ? branch.terms : [];
            terms.forEach((term) => {
                const name = String(term?.variable?.name || "").trim();
                if (deletedVariableNames.has(name)) {
                    term.variable = {
                        ...(term.variable || {}),
                        name: "",
                    };
                }
            });
        });
    }

    return nextNode;
}

function clearDeletedVariableReferences(nextVariables) {
    const nextNames = getVariableNameSet(nextVariables);
    const currentNames = getVariableNameSet(variables.value);
    const deletedNames = new Set(
        [...currentNames].filter((name) => !nextNames.has(name)),
    );
    if (deletedNames.size === 0) return;

    const nextNodes = nodes.value.map((node) =>
        clearDeletedVariableReferencesFromNode(node, deletedNames),
    );
    if (!isDeepEqual(nextNodes, nodes.value)) {
        nodes.value = nextNodes;
    }

    if (selectedNode.value) {
        const latestSelected = nextNodes.find(
            (node) => node.id === selectedNode.value.id,
        );
        selectedNode.value = latestSelected ? safeClone(latestSelected) : null;
    }
}

function loadLocalScriptContent(storageKey) {
    try {
        const raw = localStorage.getItem(String(storageKey || ""));
        if (!raw) return null;
        const parsed = JSON.parse(raw);
        return parsed && typeof parsed === "object" ? parsed : null;
    } catch {
        return null;
    }
}

function saveLocalScriptContent(storageKey, payload) {
    try {
        localStorage.setItem(
            String(storageKey || ""),
            JSON.stringify({
                ...(payload || {}),
                updatedAt: new Date().toISOString(),
            }),
        );
    } catch {
        // ignore local storage failures
    }
}

function persistCurrentMockRepoScriptToLocal() {
    const entry = selectedScriptEntry.value;
    if (!entry || entry.source !== "mock-repository") return;

    const storageKey =
        entry.localStorageKey || getLocalScriptStorageKeyByPath(entry.path);
    if (!storageKey) return;

    saveLocalScriptContent(storageKey, {
        meta: safeClone(storyMeta.value),
        variables: safeClone(variables.value),
        nodes: safeClone(nodes.value),
        edges: safeClone(edges.value),
    });
}

function percentile(sortedValues, ratio) {
    if (!Array.isArray(sortedValues) || sortedValues.length === 0) return 0;
    const clamped = Math.min(Math.max(ratio, 0), 1);
    const index = Math.floor((sortedValues.length - 1) * clamped);
    return sortedValues[index];
}

async function runGraphSyncBenchmark(options = {}) {
    const iterations = Math.max(30, Number(options.iterations || 240));
    const moveEvery = Math.max(1, Number(options.moveEvery || 3));
    const shiftPx = Number(options.shiftPx || 1.5);

    const originalNodes = safeClone(nodes.value);
    const originalEdges = safeClone(edges.value);
    const previousDragState = isNodeDragInProgress.value;

    const samples = [];
    isNodeDragInProgress.value = true;
    clearPendingDragGraphStateFlush();
    pendingDragGraphStateDetail = null;

    try {
        for (let i = 0; i < iterations; i++) {
            const direction = i % 2 === 0 ? 1 : -1;
            const nextNodes = nodes.value.map((node, index) => {
                if (index % moveEvery !== 0) return node;
                const pos = node.position || { x: 0, y: 0 };
                return {
                    ...node,
                    position: {
                        x: Number(pos.x || 0) + shiftPx * direction,
                        y: Number(pos.y || 0),
                    },
                };
            });

            const start = performance.now();
            applyGraphStateSnapshot(nextNodes, edges.value);
            const end = performance.now();
            samples.push(end - start);

            await new Promise((resolve) => requestAnimationFrame(resolve));
        }
    } finally {
        isNodeDragInProgress.value = previousDragState;
        clearPendingDragGraphStateFlush();
        pendingDragGraphStateDetail = null;
        pendingValidationAfterDrag = false;
        pendingMockRepoPersistAfterDrag = false;
        pendingEdgeStyleAfterDrag = false;

        nodes.value = originalNodes;
        edges.value = originalEdges;
        scheduleRealtimeValidation();
        applyEdgeVisualStyles();
    }

    const sorted = [...samples].sort((a, b) => a - b);
    const total = samples.reduce((sum, value) => sum + value, 0);
    const result = {
        iterations,
        averageMs: total / samples.length,
        p95Ms: percentile(sorted, 0.95),
        p99Ms: percentile(sorted, 0.99),
        minMs: sorted[0] || 0,
        maxMs: sorted[sorted.length - 1] || 0,
    };

    if (typeof window !== "undefined") {
        console.table(result);
    }

    return result;
}

function clearPendingDragGraphStateFlush() {
    if (dragGraphStateRafId) {
        cancelAnimationFrame(dragGraphStateRafId);
        dragGraphStateRafId = 0;
    }
}

function scheduleDragGraphStateFlush() {
    if (dragGraphStateRafId) return;

    dragGraphStateRafId = requestAnimationFrame(() => {
        dragGraphStateRafId = 0;
        const detail = pendingDragGraphStateDetail;
        pendingDragGraphStateDetail = null;
        if (!detail) return;
        applyGraphStateSnapshot(detail.nodes || [], detail.edges || []);
    });
}

function createStateSnapshot() {
    return {
        nodes: safeClone(nodes.value),
        edges: safeClone(edges.value),
        storyMeta: safeClone(storyMeta.value),
        variables: safeClone(variables.value),
        presetSpeakers: safeClone(presetSpeakers.value),
        selectedNode: selectedNode.value ? safeClone(selectedNode.value) : null,
        selectedEdge: selectedEdge.value ? safeClone(selectedEdge.value) : null,
    };
}

function applyStateSnapshot(snapshot) {
    if (!snapshot) return;

    isApplyingHistory.value = true;

    nodes.value = normalizeEditorStoryNodes(safeClone(snapshot.nodes || []));
    edges.value = safeClone(snapshot.edges || []);
    storyMeta.value = safeClone(
        snapshot.storyMeta || {
            storyId: "NewStory",
            entryNodeId: "",
            schemaVersion: 1,
        },
    );
    variables.value = safeClone(snapshot.variables || []);
    presetSpeakers.value = safeClone(snapshot.presetSpeakers || []);
    selectedNode.value = snapshot.selectedNode
        ? safeClone(snapshot.selectedNode)
        : null;
    selectedEdge.value = snapshot.selectedEdge
        ? safeClone(snapshot.selectedEdge)
        : null;

    scheduleRealtimeValidation();
    applyEdgeVisualStyles();
    syncEdgeDraftFromSelection();

    isApplyingHistory.value = false;
}

function pushHistorySnapshot() {
    if (isApplyingHistory.value) return;

    undoStack.value.push(createStateSnapshot());
    if (undoStack.value.length > MAX_HISTORY_SIZE) {
        undoStack.value.shift();
    }
    redoStack.value = [];
}

function handleNodeDragStart() {
    if (isNodeDragInProgress.value) return;
    isNodeDragInProgress.value = true;
    pushHistorySnapshot();
    pendingDragGraphStateDetail = null;
    clearPendingDragGraphStateFlush();
}

function handleNodeDragStop() {
    isNodeDragInProgress.value = false;
    recentDragSuppressUntil.value = Date.now() + 280;

    clearPendingDragGraphStateFlush();
    if (pendingDragGraphStateDetail) {
        const detail = pendingDragGraphStateDetail;
        pendingDragGraphStateDetail = null;
        applyGraphStateSnapshot(detail.nodes || [], detail.edges || []);
    }

    if (pendingValidationAfterDrag) {
        pendingValidationAfterDrag = false;
        scheduleRealtimeValidation();
    }

    if (pendingMockRepoPersistAfterDrag) {
        pendingMockRepoPersistAfterDrag = false;
        if (assetMode.value === "rail") persistCurrentRailToLocal();
        else persistCurrentMockRepoScriptToLocal();
    }

    if (pendingEdgeStyleAfterDrag) {
        pendingEdgeStyleAfterDrag = false;
        applyEdgeVisualStyles();
    }

    markAutoSaveDirty();
}

function isObjectLike(value) {
    return value !== null && typeof value === "object";
}

function isPlainObject(value) {
    if (!isObjectLike(value)) return false;
    const proto = Object.getPrototypeOf(value);
    return proto === Object.prototype || proto === null;
}

function isDeepEqual(a, b) {
    if (Object.is(a, b)) return true;

    if (Array.isArray(a) || Array.isArray(b)) {
        if (!Array.isArray(a) || !Array.isArray(b)) return false;
        if (a.length !== b.length) return false;
        for (let i = 0; i < a.length; i++) {
            if (!isDeepEqual(a[i], b[i])) return false;
        }
        return true;
    }

    if (isObjectLike(a) || isObjectLike(b)) {
        if (!isPlainObject(a) || !isPlainObject(b)) return false;

        const keysA = Object.keys(a);
        const keysB = Object.keys(b);
        if (keysA.length !== keysB.length) return false;

        for (const key of keysA) {
            if (!Object.prototype.hasOwnProperty.call(b, key)) return false;
            if (!isDeepEqual(a[key], b[key])) return false;
        }
        return true;
    }

    return false;
}

function hasNodePositionChanged(nextNodes, currentNodes) {
    if (nextNodes.length !== currentNodes.length) return true;

    const byId = new Map();
    currentNodes.forEach((node) => {
        byId.set(node.id, node);
    });

    for (const nextNode of nextNodes) {
        const currentNode = byId.get(nextNode.id);
        if (!currentNode) return true;
        if (nextNode.type !== currentNode.type) return true;

        const nextPos = nextNode.position || {};
        const curPos = currentNode.position || {};
        if (nextPos.x !== curPos.x || nextPos.y !== curPos.y) return true;
    }

    return false;
}

function applyDraggingNodePositions(nextNodes) {
    if (!Array.isArray(nextNodes) || nextNodes.length === 0) return false;

    const indexById = new Map();
    nodes.value.forEach((node, index) => {
        indexById.set(node.id, index);
    });

    const updatedNodes = [...nodes.value];
    let changed = false;

    for (const nextNode of nextNodes) {
        const index = indexById.get(nextNode.id);
        if (index == null) return false;

        const currentNode = updatedNodes[index];
        if (!currentNode) return false;

        const nextPos = nextNode.position || {};
        const curPos = currentNode.position || {};
        if (nextPos.x === curPos.x && nextPos.y === curPos.y) {
            continue;
        }

        updatedNodes[index] = {
            ...currentNode,
            position: { x: nextPos.x, y: nextPos.y },
        };
        changed = true;
    }

    if (changed) {
        nodes.value = updatedNodes;
    }

    return changed;
}

function normalizeEdge(edge) {
    return {
        ...edge,
        type: edge?.type || "straight",
        animated: edge?.animated ?? false,
        style:
            edge?.style ||
            "stroke: rgba(168, 85, 247, 0.55); stroke-width: 2px;",
        data: {
            priority: edge?.data?.priority ?? 0,
        },
    };
}

function isValidChoiceHandle(node, sourceHandle) {
    if (!node || node.type !== "choice") return true;
    if (!sourceHandle) return true;

    if (sourceHandle === "choice-complete") {
        return node.data?.choiceMode === "ExhaustiveUntilComplete";
    }

    if (sourceHandle === "choice-timeout") {
        return Boolean(node.data?.choiceTimer?.enabled);
    }

    const match = /^choice-(\d+)$/.exec(sourceHandle);
    if (!match) return false;
    const index = Number(match[1]);
    const count = Array.isArray(node.data?.choices)
        ? node.data.choices.length
        : 0;
    return Number.isInteger(index) && index >= 0 && index < count;
}

function isValidConditionHandle(node, sourceHandle) {
    if (!node || node.type !== "condition") return true;
    if (!sourceHandle) return false;
    if (sourceHandle === "condition-fallback") return true;
    const match = /^condition-(\d+)$/.exec(sourceHandle);
    if (!match) return false;
    const index = Number(match[1]);
    const branches = Array.isArray(node.data?.condition?.branches)
        ? node.data.condition.branches
        : Array.isArray(node.data?.condition?.terms)
          ? [node.data.condition]
          : [];
    return Number.isInteger(index) && index >= 0 && index < branches.length;
}

function isValidRailHandle(node, sourceHandle) {
    if (!node || node.type !== "railbranch") return true;
    if (!sourceHandle) return false;
    if (sourceHandle === "branch-fallback") return true;
    const match = /^branch-(\d+)$/.exec(sourceHandle);
    if (!match) return false;
    const index = Number(match[1]);
    const branches = Array.isArray(node.data?.branches)
        ? node.data.branches
        : [];
    return Number.isInteger(index) && index >= 0 && index < branches.length;
}

function sanitizeEdges(rawEdges, nodeList = nodes.value) {
    const seenIds = new Set();
    const nodeMap = new Map((nodeList || []).map((n) => [n.id, n]));

    return (rawEdges || []).map(normalizeEdge).filter((edge) => {
        if (!edge?.id || seenIds.has(edge.id)) return false;
        seenIds.add(edge.id);

        const sourceNode = nodeMap.get(edge.source);
        const targetNode = nodeMap.get(edge.target);
        if (!sourceNode || !targetNode) return false;

        return (
            isValidChoiceHandle(sourceNode, edge.sourceHandle) &&
            isValidConditionHandle(sourceNode, edge.sourceHandle)
        );
    });
}

function sanitizeRailEdges(rawEdges, nodeList = railNodes.value) {
    const seenIds = new Set();
    const nodeMap = new Map((nodeList || []).map((n) => [n.id, n]));

    return (rawEdges || []).map(normalizeEdge).filter((edge) => {
        if (!edge?.id || seenIds.has(edge.id)) return false;
        seenIds.add(edge.id);

        const sourceNode = nodeMap.get(edge.source);
        const targetNode = nodeMap.get(edge.target);
        if (!sourceNode || !targetNode) return false;

        return isValidRailHandle(sourceNode, edge.sourceHandle);
    });
}

function normalizePresetSpeakers(input) {
    const safeList = Array.isArray(input) ? input : [];
    const seen = new Set();
    const normalized = [];

    safeList.forEach((speaker) => {
        const id =
            typeof speaker === "string"
                ? speaker.trim()
                : String(speaker?.id || "").trim();
        if (!id || seen.has(id)) return;
        seen.add(id);

        const displayName =
            typeof speaker === "object" && speaker?.displayName
                ? String(speaker.displayName).trim()
                : "";

        normalized.push(displayName ? { id, displayName } : { id });
    });

    return normalized;
}

function derivePresetSpeakersFromNodes(nodeList) {
    const sourceNodes = Array.isArray(nodeList) ? nodeList : [];
    const collected = sourceNodes
        .filter(
            (node) => node?.type === "dialogue" || node?.type === "multidialogue",
        )
        .map((node) => String(node?.data?.speakerId || "").trim())
        .filter((id) => id.length > 0)
        .map((id) => ({ id }));

    return normalizePresetSpeakers(collected);
}

function syncChoiceEdgesForNode(choiceNode, currentEdges) {
    if (!choiceNode || choiceNode.type !== "choice") return currentEdges;
    return (currentEdges || []).filter((edge) => {
        if (edge.source !== choiceNode.id) return true;
        return isValidChoiceHandle(choiceNode, edge.sourceHandle);
    });
}

function syncConditionEdgesForNode(conditionNode, currentEdges) {
    if (!conditionNode || conditionNode.type !== "condition")
        return currentEdges;
    return (currentEdges || []).filter((edge) => {
        if (edge.source !== conditionNode.id) return true;
        return isValidConditionHandle(conditionNode, edge.sourceHandle);
    });
}

function isEdgeFocused(edge) {
    if (selectedEdge.value) return selectedEdge.value.id === edge.id;
    if (selectedNode.value) {
        return (
            edge.source === selectedNode.value.id ||
            edge.target === selectedNode.value.id
        );
    }
    return false;
}

function deriveEdgeStyle(edge, visualOffset = 0) {
    const focused = isEdgeFocused(edge);
    const baseColor = "168, 85, 247";

    const pathOffset = Math.max(8, 26 + visualOffset);
    const borderRadius = 12 + Math.min(Math.abs(visualOffset) * 0.35, 10);

    if (focusModeEnabled.value) {
        if (focused) {
            return {
                style: "stroke: rgba(168, 85, 247, 0.95); stroke-width: 3.5px; stroke-linecap: round;",
                animated: true,
                pathOptions: { offset: pathOffset, borderRadius },
                zIndex: 30,
            };
        }
        return {
            style: "stroke: rgba(168, 85, 247, 0.12); stroke-width: 1.5px; stroke-linecap: round;",
            animated: false,
            pathOptions: { offset: pathOffset, borderRadius },
            zIndex: 3,
        };
    }

    if (focused) {
        return {
            style: `stroke: rgba(${baseColor}, 0.88); stroke-width: 3px; stroke-linecap: round;`,
            animated: true,
            pathOptions: { offset: pathOffset, borderRadius },
            zIndex: 25,
        };
    }

    return {
        style: `stroke: rgba(${baseColor}, 0.52); stroke-width: 2px; stroke-linecap: round;`,
        animated: false,
        pathOptions: { offset: pathOffset, borderRadius },
        zIndex: 5,
    };
}

function buildSourceFanOutOffsetMap(edgesList) {
    const bySource = new Map();
    edgesList.forEach((edge) => {
        if (!bySource.has(edge.source)) bySource.set(edge.source, []);
        bySource.get(edge.source).push(edge);
    });

    const offsetMap = new Map();

    bySource.forEach((list) => {
        const sorted = [...list].sort((a, b) => {
            const ai = getChoiceHandleIndex(a.sourceHandle);
            const bi = getChoiceHandleIndex(b.sourceHandle);

            if (ai != null && bi != null && ai !== bi) return ai - bi;
            if (ai != null && bi == null) return -1;
            if (ai == null && bi != null) return 1;
            return String(a.target).localeCompare(String(b.target));
        });

        const center = (sorted.length - 1) / 2;
        sorted.forEach((edge, idx) => {
            offsetMap.set(edge.id, (idx - center) * 12);
        });
    });

    return offsetMap;
}

function buildParallelEdgeOffsetMap(edgesList) {
    const byPair = new Map();

    edgesList.forEach((edge) => {
        const key = `${edge.source}->${edge.target}`;
        if (!byPair.has(key)) byPair.set(key, []);
        byPair.get(key).push(edge);
    });

    const offsetMap = new Map();

    byPair.forEach((list) => {
        if (list.length === 1) {
            offsetMap.set(list[0].id, 0);
            return;
        }

        const center = (list.length - 1) / 2;
        list.forEach((edge, idx) => {
            offsetMap.set(edge.id, (idx - center) * 16);
        });
    });

    return offsetMap;
}

function applyEdgeVisualStyles() {
    const currentEdges = activeEdges.value;
    const sourceFanOutOffsets = buildSourceFanOutOffsetMap(currentEdges);
    const parallelOffsets = buildParallelEdgeOffsetMap(currentEdges);
    const isBezier = edgeRenderMode.value === "bezier";
    const edgeType = isBezier ? "default" : "straight";

    const next = currentEdges.map((edge) => {
        const sourceOffset = sourceFanOutOffsets.get(edge.id) ?? 0;
        const parallelOffset = parallelOffsets.get(edge.id) ?? 0;
        const visualOffset = sourceOffset + parallelOffset;

        const { style, animated, pathOptions, zIndex } = deriveEdgeStyle(
            edge,
            visualOffset,
        );

        const nextPathOptions = isBezier ? pathOptions : undefined;
        const currentPathOptions = edge.pathOptions;
        const pathOptionsEqual = isBezier
            ? !!currentPathOptions &&
              currentPathOptions.offset === nextPathOptions.offset &&
              currentPathOptions.borderRadius === nextPathOptions.borderRadius
            : currentPathOptions == null;

        const unchanged =
            edge.style === style &&
            edge.animated === animated &&
            edge.type === edgeType &&
            edge.zIndex === zIndex &&
            pathOptionsEqual;

        if (unchanged) {
            return edge;
        }

        return {
            ...edge,
            style,
            animated,
            type: edgeType,
            ...(isBezier
                ? { pathOptions: nextPathOptions }
                : { pathOptions: undefined }),
            zIndex,
        };
    });

    let changed = false;
    for (let i = 0; i < next.length; i++) {
        if (next[i] !== currentEdges[i]) {
            changed = true;
            break;
        }
    }

    if (changed) {
        if (assetMode.value === "rail") railEdges.value = next;
        else edges.value = next;
    }
}

function runRealtimeValidation() {
    if (assetMode.value === "rail") {
        railValidationResult.value = validateRail(
            railNodes.value,
            railEdges.value,
            railMeta.value,
            {
                storyEntries: availableStoryEntries.value,
                variables: variables.value,
            },
        );
        validationResult.value = {
            errors: railValidationResult.value.errors.map((message) => ({
                message,
            })),
            warnings: railValidationResult.value.warnings.map((message) => ({
                message,
            })),
        };
        return;
    }

    validationResult.value = validateStory(
        nodes.value,
        edges.value,
        storyMeta.value,
        variables.value,
    );
}

function scheduleRealtimeValidation() {
    if (validationDebounceTimer.value) {
        clearTimeout(validationDebounceTimer.value);
    }
    validationDebounceTimer.value = setTimeout(() => {
        runRealtimeValidation();
        validationDebounceTimer.value = null;
    }, 180);
}

function flushAutoSaveIfDirty() {
    if (!autoSaveDirty) return false;

    storage.save(storyMeta.value.storyId || "NewStory", {
        nodes: nodes.value,
        edges: edges.value,
        meta: storyMeta.value,
        variables: variables.value,
        presetSpeakers: presetSpeakers.value,
    });

    autoSaveDirty = false;
    lastSavedAt.value = new Date().toISOString();
    return true;
}

function setupAutoSave() {
    if (autoSaveTimer) {
        clearInterval(autoSaveTimer);
        autoSaveTimer = null;
    }

    autoSaveTimer = setInterval(() => {
        const didSave = flushAutoSaveIfDirty();
        if (!didSave) {
            clearInterval(autoSaveTimer);
            autoSaveTimer = null;
        }
    }, 120000);
}

function markAutoSaveDirty() {
    autoSaveDirty = true;
    if (!autoSaveTimer) {
        setupAutoSave();
    }
}

function applyThemeMode() {
    const theme = darkModeEnabled.value ? "dark" : "light";
    const body = document.body;
    if (!body) return;

    body.setAttribute("data-theme", theme);
    body.classList.toggle("dark-theme", theme === "dark");
    body.classList.toggle("light-theme", theme === "light");
}

function loadDarkModePreference() {
    const saved = localStorage.getItem(DARK_MODE_STORAGE_KEY);
    if (saved === "dark") {
        darkModeEnabled.value = true;
    } else if (saved === "light") {
        darkModeEnabled.value = false;
    }
    applyThemeMode();
}

function persistDarkModePreference() {
    localStorage.setItem(
        DARK_MODE_STORAGE_KEY,
        darkModeEnabled.value ? "dark" : "light",
    );
}

function syncEdgeDraftFromSelection() {
    if (!selectedEdge.value) return;
    edgeDraft.priority = selectedEdge.value.data?.priority ?? 0;
}

function applyEdgeDraft() {
    if (!selectedEdge.value) return;

    handleEdgeUpdate({
        ...selectedEdge.value,
        data: {
            ...(selectedEdge.value.data || {}),
            priority: Number.isFinite(edgeDraft.priority)
                ? edgeDraft.priority
                : 0,
        },
    });
}

function resetEdgeDraft() {
    syncEdgeDraftFromSelection();
}

function clearEdgeSelection() {
    selectedEdge.value = null;
    applyEdgeVisualStyles();
}

function handleNodeClick(event) {
    const now = Date.now();
    if (now < recentDragSuppressUntil.value) {
        return;
    }

    const node = event.detail ? safeClone(event.detail) : null;

    if (!node) {
        selectedNode.value = null;
        applyEdgeVisualStyles();
        lastNodeClickMeta.value = { id: "", time: now };
        return;
    }

    const isSameSelectedNode = selectedNode.value?.id === node.id;
    if (!isSameSelectedNode) {
        selectedNode.value = node;
    }
    selectedEdge.value = null;
    applyEdgeVisualStyles();

    const isDoubleClickOnSameNode =
        lastNodeClickMeta.value.id === node.id &&
        now - Number(lastNodeClickMeta.value.time || 0) <= 280;

    lastNodeClickMeta.value = { id: node.id, time: now };

    if (!isDoubleClickOnSameNode) {
        return;
    }

    if (node.type === "dialogue") {
        multiDialogueOpenRequest.value = {
            nodeId: node.id,
            nonce: now,
        };
        return;
    }

    if (node.type === "choice") {
        choiceOpenRequest.value = {
            nodeId: node.id,
            nonce: now,
        };
        return;
    }

    if (node.type === "condition") {
        conditionOpenRequest.value = {
            nodeId: node.id,
            nonce: now,
        };
    }
}

function handleEdgeClick(event) {
    selectedEdge.value = event.detail
        ? normalizeEdge(safeClone(event.detail))
        : null;
    if (selectedEdge.value) selectedNode.value = null;
    syncEdgeDraftFromSelection();
    applyEdgeVisualStyles();
}

function applyGraphStateSnapshot(nextNodesInput, nextEdgesInput) {
    if (assetMode.value === "rail") {
        const nextNodes = Array.isArray(nextNodesInput) ? nextNodesInput : [];
        const nextEdgesRaw = Array.isArray(nextEdgesInput) ? nextEdgesInput : [];
        const sanitizedEdges = isNodeDragInProgress.value
            ? railEdges.value
            : sanitizeRailEdges(nextEdgesRaw, nextNodes);

        const shouldUpdateNodes = isNodeDragInProgress.value
            ? hasNodePositionChanged(nextNodes, railNodes.value)
            : !isDeepEqual(nextNodes, railNodes.value);
        const shouldUpdateEdges = isNodeDragInProgress.value
            ? false
            : !isDeepEqual(sanitizedEdges, railEdges.value);

        if (shouldUpdateNodes) {
            railNodes.value = safeClone(nextNodes);
        }
        if (shouldUpdateEdges) {
            railEdges.value = safeClone(sanitizedEdges);
        }

        if (!isNodeDragInProgress.value && selectedNode.value) {
            const latestNode = railNodes.value.find(
                (n) => n.id === selectedNode.value.id,
            );
            selectedNode.value = latestNode ? safeClone(latestNode) : null;
        }

        if (!isNodeDragInProgress.value && selectedEdge.value) {
            const latestEdge = railEdges.value.find(
                (e) => e.id === selectedEdge.value.id,
            );
            selectedEdge.value = latestEdge
                ? normalizeEdge(safeClone(latestEdge))
                : null;
            syncEdgeDraftFromSelection();
        }

        if (!isNodeDragInProgress.value) {
            applyEdgeVisualStyles();
            persistCurrentRailToLocal();
        }
        return;
    }

    const nextNodes = normalizeEditorStoryNodes(
        Array.isArray(nextNodesInput) ? nextNodesInput : [],
    );
    const nextEdgesRaw = Array.isArray(nextEdgesInput) ? nextEdgesInput : [];
    const isDragging = isNodeDragInProgress.value;
    const sanitizedEdges = isDragging
        ? edges.value
        : sanitizeEdges(nextEdgesRaw, nextNodes);

    const shouldUpdateNodes = isDragging
        ? hasNodePositionChanged(nextNodes, nodes.value)
        : !isDeepEqual(nextNodes, nodes.value);
    const shouldUpdateEdges = isDragging
        ? false
        : !isDeepEqual(sanitizedEdges, edges.value);

    if (shouldUpdateNodes || shouldUpdateEdges) {
        if (!isDragging) {
            pushHistorySnapshot();
        }

        if (shouldUpdateNodes) {
            if (isDragging) {
                const applied = applyDraggingNodePositions(nextNodes);
                if (!applied) {
                    nodes.value = nextNodes.map((node) => ({
                        ...node,
                        position: { ...(node.position || {}) },
                    }));
                }
            } else {
                nodes.value = normalizeEditorStoryNodes(safeClone(nextNodes));
            }
        }

        if (shouldUpdateEdges) {
            edges.value = safeClone(sanitizedEdges);
        }
    }

    if (!isDragging && selectedNode.value) {
        const latestNode = nodes.value.find(
            (n) => n.id === selectedNode.value.id,
        );
        selectedNode.value = latestNode ? safeClone(latestNode) : null;
    }

    if (!isDragging && selectedEdge.value) {
        const latestEdge = edges.value.find(
            (e) => e.id === selectedEdge.value.id,
        );
        selectedEdge.value = latestEdge
            ? normalizeEdge(safeClone(latestEdge))
            : null;
        syncEdgeDraftFromSelection();
    }

    if (isDragging) {
        pendingEdgeStyleAfterDrag = true;
    } else {
        applyEdgeVisualStyles();
    }
}

function handleGraphStateChange(event) {
    const detail = event?.detail || {};
    if (isNodeDragInProgress.value) {
        pendingDragGraphStateDetail = detail;
        scheduleDragGraphStateFlush();
        return;
    }

    pendingDragGraphStateDetail = null;
    clearPendingDragGraphStateFlush();
    applyGraphStateSnapshot(detail.nodes || [], detail.edges || []);
}

function buildLayers(nodesInput, edgesInput, entryNodeId) {
    const nodeIds = nodesInput.map((n) => n.id);
    const inDegree = new Map(nodeIds.map((id) => [id, 0]));
    const outgoing = new Map(nodeIds.map((id) => [id, []]));

    edgesInput.forEach((e) => {
        if (inDegree.has(e.target) && outgoing.has(e.source)) {
            inDegree.set(e.target, inDegree.get(e.target) + 1);
            outgoing.get(e.source).push(e.target);
        }
    });

    const roots = nodeIds.filter((id) => inDegree.get(id) === 0);
    const orderedRoots = [
        ...(entryNodeId && roots.includes(entryNodeId) ? [entryNodeId] : []),
        ...roots.filter((id) => id !== entryNodeId),
    ];

    const queue = [...orderedRoots];
    const layer = new Map(nodeIds.map((id) => [id, 0]));
    const visited = new Set();

    while (queue.length > 0) {
        const cur = queue.shift();
        if (visited.has(cur)) continue;
        visited.add(cur);

        const curLayer = layer.get(cur) ?? 0;
        for (const next of outgoing.get(cur) || []) {
            layer.set(next, Math.max(layer.get(next) ?? 0, curLayer + 1));
            inDegree.set(next, inDegree.get(next) - 1);
            if (inDegree.get(next) <= 0) queue.push(next);
        }
    }

    // 处理有环或未访问节点
    let maxLayer = 0;
    layer.forEach((v) => {
        if (v > maxLayer) maxLayer = v;
    });

    nodeIds.forEach((id) => {
        if (!visited.has(id)) {
            maxLayer += 1;
            layer.set(id, maxLayer);
        }
    });

    return layer;
}

function orderNodesWithinLayer(list, desiredYById = null) {
    const byY = [...list].sort((a, b) => {
        const ay = desiredYById?.get(a.id) ?? a.position?.y ?? 0;
        const by = desiredYById?.get(b.id) ?? b.position?.y ?? 0;
        return ay - by;
    });

    const choiceNodes = byY.filter(
        (n) => n.type === "choice" || n.type === "railbranch",
    );
    const endNodes = byY.filter(
        (n) => n.type === "end" || n.type === "railend",
    );
    const regularNodes = byY.filter(
        (n) =>
            n.type !== "choice" &&
            n.type !== "railbranch" &&
            n.type !== "end" &&
            n.type !== "railend",
    );

    const centered = [...regularNodes];
    const centerIndex = Math.floor(centered.length / 2);
    centered.splice(centerIndex, 0, ...choiceNodes);

    return [...centered, ...endNodes];
}

function getChoiceHandleIndex(sourceHandle) {
    if (!sourceHandle) return null;
    const match = /^(choice|branch)-(\d+)$/.exec(sourceHandle);
    if (!match) return null;
    return Number(match[2]);
}

function estimateNodeHeight(node) {
    if (!node) return 140;

    if (node.type === "dialogue" || node.type === "multidialogue") {
        const lines = getDialogueLineTexts(node.data).filter((text) =>
            String(text || "").trim(),
        );

        const baseHeight = 110;
        const perLineHeight = 22;
        return baseHeight + lines.length * perLineHeight;
    }

    if (node.type === "choice") {
        const choiceTexts = Array.isArray(node.data?.choices)
            ? node.data.choices.map((choice) =>
                  String(choice?.textKey || "").trim(),
              )
            : [];

        const choiceCount = choiceTexts.length;
        const approxCharsPerLine = 20;
        const visualLineCount = Math.max(
            1,
            choiceTexts.reduce((sum, text) => {
                const len = text.length;
                const lines = Math.max(1, Math.ceil(len / approxCharsPerLine));
                return sum + lines;
            }, 0),
        );

        const baseHeight = 120;
        const perVisualLineHeight = 18;
        const perChoiceSpacing = 8;
        return (
            baseHeight +
            visualLineCount * perVisualLineHeight +
            choiceCount * perChoiceSpacing
        );
    }

    return 140;
}

function estimateNodeWidth(node) {
    if (!node) return 230;

    if (node.type === "dialogue" || node.type === "multidialogue") {
        const lines = getDialogueLineTexts(node.data)
            .map((text) => String(text || "").trim())
            .filter((text) => text.length > 0);

        const maxLineLen = lines.reduce(
            (max, text) => Math.max(max, text.length),
            0,
        );
        const avgLineLen =
            lines.length > 0
                ? lines.reduce((sum, text) => sum + text.length, 0) /
                  lines.length
                : 0;

        const estimated = 260 + maxLineLen * 8.2 + avgLineLen * 1.1;
        return Math.min(1300, Math.max(320, estimated));
    }

    if (node.type === "choice") {
        const choiceTexts = Array.isArray(node.data?.choices)
            ? node.data.choices
                  .map((choice) => String(choice?.textKey || "").trim())
                  .filter((text) => text.length > 0)
            : [];

        const maxChoiceLen = choiceTexts.reduce(
            (max, text) => Math.max(max, text.length),
            0,
        );

        return Math.min(900, Math.max(280, 250 + maxChoiceLen * 6.8));
    }

    return 230;
}

function getNodeCenterY(node, topY) {
    return topY + estimateNodeHeight(node) * 0.5;
}

function computeIncomingDesiredY(
    node,
    incomingEdges,
    assignedYById,
    nodeMap,
    options = {},
) {
    if (!incomingEdges || incomingEdges.length === 0) return null;

    const fanOutSpread = 90;
    const preferEnvelope = !!options.preferEnvelope;
    let total = 0;
    let count = 0;
    let minTop = Number.POSITIVE_INFINITY;
    let maxBottom = Number.NEGATIVE_INFINITY;

    incomingEdges.forEach((edge) => {
        const sourceTopY = assignedYById.get(edge.source);
        if (sourceTopY == null) return;

        const sourceNode = nodeMap.get(edge.source);
        const sourceHeight = estimateNodeHeight(sourceNode);
        const sourceBottomY = sourceTopY + sourceHeight;

        minTop = Math.min(minTop, sourceTopY);
        maxBottom = Math.max(maxBottom, sourceBottomY);

        const sourceCenterY = getNodeCenterY(sourceNode, sourceTopY);
        const handleIndex = getChoiceHandleIndex(edge.sourceHandle);

        if (
            handleIndex != null &&
            (sourceNode?.type === "choice" || sourceNode?.type === "railbranch")
        ) {
            const sourceItems =
                sourceNode.type === "railbranch"
                    ? sourceNode.data?.branches
                    : sourceNode.data?.choices;
            const choiceCount = Array.isArray(sourceItems)
                ? sourceItems.length
                : 0;
            const center = (Math.max(choiceCount, 1) - 1) / 2;
            const offset = (handleIndex - center) * fanOutSpread;
            total += sourceCenterY + offset;
        } else {
            total += sourceCenterY;
        }

        count += 1;
    });

    if (count === 0) return null;

    const targetHeight = estimateNodeHeight(node);
    const meanCenterY = total / count;
    const envelopeCenterY = (minTop + maxBottom) * 0.5;
    const desiredCenterY = preferEnvelope
        ? envelopeCenterY * 0.72 + meanCenterY * 0.28
        : meanCenterY;

    return desiredCenterY - targetHeight * 0.5;
}

function resolveLayerYCollisions(orderedNodes, desiredYById, startY, minGap) {
    const result = new Map();
    if (orderedNodes.length === 0) return result;

    let cursor = startY;
    orderedNodes.forEach((node) => {
        const desiredY = desiredYById.get(node.id) ?? cursor;
        const y = Math.max(desiredY, cursor);
        result.set(node.id, y);

        const nodeHeight = estimateNodeHeight(node);
        cursor = y + Math.max(minGap, nodeHeight + 26);
    });

    const assignedValues = orderedNodes.map((n) => result.get(n.id));
    const desiredValues = orderedNodes.map(
        (n) => desiredYById.get(n.id) ?? result.get(n.id),
    );
    const avgAssigned =
        assignedValues.reduce((sum, v) => sum + v, 0) /
        Math.max(assignedValues.length, 1);
    const avgDesired =
        desiredValues.reduce((sum, v) => sum + v, 0) /
        Math.max(desiredValues.length, 1);

    const shiftUpLimit = Math.max(Math.min(...assignedValues) - startY, 0);
    const shift = Math.max(Math.min(avgDesired - avgAssigned, shiftUpLimit), 0);

    if (shift > 0) {
        orderedNodes.forEach((node) => {
            result.set(node.id, result.get(node.id) - shift);
        });
    }

    return result;
}

function enforceForwardLayers(initialLayerMap, edgeList, nodeList) {
    const adjusted = new Map(initialLayerMap || []);
    const nodeIds = new Set((nodeList || []).map((n) => n.id));

    nodeIds.forEach((id) => {
        if (!adjusted.has(id)) adjusted.set(id, 0);
    });

    const maxIterations = Math.max(nodeIds.size * 2, 12);
    for (let i = 0; i < maxIterations; i += 1) {
        let changed = false;

        (edgeList || []).forEach((edge) => {
            if (!nodeIds.has(edge.source) || !nodeIds.has(edge.target)) return;

            const sourceLayer = adjusted.get(edge.source) ?? 0;
            const targetLayer = adjusted.get(edge.target) ?? 0;
            const required = sourceLayer + 1;

            if (targetLayer < required) {
                adjusted.set(edge.target, required);
                changed = true;
            }
        });

        if (!changed) break;
    }

    return adjusted;
}

function computeAutoLayoutNodes(graphNodes, graphEdges, entryNodeId) {
    const rankSep = 180;
    const nodeSep = 210;
    const startX = 140;
    const startY = 110;

    const rawLayerMap = buildLayers(
        graphNodes,
        graphEdges,
        entryNodeId,
    );
    const layerMap = enforceForwardLayers(
        rawLayerMap,
        graphEdges,
        graphNodes,
    );
    const nodeMap = new Map(graphNodes.map((n) => [n.id, n]));
    const incomingByTarget = new Map();
    const outgoingBySource = new Map();

    graphEdges.forEach((edge) => {
        if (!incomingByTarget.has(edge.target))
            incomingByTarget.set(edge.target, []);
        incomingByTarget.get(edge.target).push(edge);

        if (!outgoingBySource.has(edge.source)) {
            outgoingBySource.set(edge.source, []);
        }
        outgoingBySource.get(edge.source).push(edge);
    });

    const grouped = new Map();
    graphNodes.forEach((node) => {
        const l = layerMap.get(node.id) ?? 0;
        if (!grouped.has(l)) grouped.set(l, []);
        grouped.get(l).push(node);
    });

    const layers = [...grouped.keys()].sort((a, b) => a - b);
    const assignedYById = new Map();

    const layerMaxWidth = new Map();
    layers.forEach((layer) => {
        const layerNodes = grouped.get(layer) || [];
        const maxWidth = layerNodes.reduce(
            (max, node) => Math.max(max, estimateNodeWidth(node)),
            230,
        );
        layerMaxWidth.set(layer, maxWidth);
    });

    const layerX = new Map();
    const nodeXById = new Map();
    let cursorX = startX;
    layers.forEach((layer, index) => {
        if (index === 0) {
            layerX.set(layer, cursorX);
            return;
        }

        const prevLayer = layers[index - 1];
        const prevWidth = layerMaxWidth.get(prevLayer) ?? 230;
        cursorX += prevWidth + rankSep;
        layerX.set(layer, cursorX);
    });

    const sinkRightOffset = 190;

    layers.forEach((layer) => {
        const layerNodes = grouped.get(layer) || [];
        const desiredYById = new Map();

        const fallbackOrder = orderNodesWithinLayer(layerNodes);
        fallbackOrder.forEach((node, idx) => {
            desiredYById.set(node.id, startY + idx * nodeSep);
        });

        layerNodes.forEach((node) => {
            const incomingEdges = (incomingByTarget.get(node.id) || []).filter(
                (edge) => {
                    const sourceLayer = layerMap.get(edge.source) ?? 0;
                    return sourceLayer < layer;
                },
            );

            const outgoingCount = (outgoingBySource.get(node.id) || []).length;
            const isFanInSink =
                incomingEdges.length >= 2 && outgoingCount === 0;

            const incomingDesiredY = computeIncomingDesiredY(
                node,
                incomingEdges,
                assignedYById,
                nodeMap,
                { preferEnvelope: isFanInSink },
            );

            if (incomingDesiredY != null) {
                desiredYById.set(node.id, incomingDesiredY);
            }
        });

        const ordered = orderNodesWithinLayer(layerNodes, desiredYById);
        const resolvedY = resolveLayerYCollisions(
            ordered,
            desiredYById,
            startY,
            nodeSep,
        );

        ordered.forEach((node) => {
            assignedYById.set(node.id, resolvedY.get(node.id) ?? startY);

            const incomingEdges = (incomingByTarget.get(node.id) || []).filter(
                (edge) => {
                    const sourceLayer = layerMap.get(edge.source) ?? 0;
                    return sourceLayer < layer;
                },
            );
            const outgoingCount = (outgoingBySource.get(node.id) || []).length;
            const isFanInSink =
                incomingEdges.length >= 2 && outgoingCount === 0;

            const baseX = layerX.get(layer) ?? startX;

            if (!isFanInSink) {
                nodeXById.set(node.id, baseX);
                return;
            }

            const sourceRightMost = incomingEdges.reduce((maxRight, edge) => {
                const sourceNode = nodeMap.get(edge.source);
                const sourceLayer = layerMap.get(edge.source) ?? 0;
                const sourceX =
                    nodeXById.get(edge.source) ??
                    layerX.get(sourceLayer) ??
                    startX;
                const sourceWidth = estimateNodeWidth(sourceNode);
                return Math.max(maxRight, sourceX + sourceWidth);
            }, Number.NEGATIVE_INFINITY);

            const safeForwardGap = 230;
            const sinkX = Math.max(
                baseX + sinkRightOffset,
                sourceRightMost + safeForwardGap,
            );
            nodeXById.set(node.id, sinkX);
        });
    });

    return graphNodes.map((node) => {
        const l = layerMap.get(node.id) ?? 0;
        const y = assignedYById.get(node.id) ?? startY;
        const x = nodeXById.get(node.id) ?? layerX.get(l) ?? startX;
        return {
            ...node,
            position: { x, y },
        };
    });
}

function handleAutoLayout() {
    const graphNodes = assetMode.value === "rail" ? railNodes.value : nodes.value;
    const graphEdges = assetMode.value === "rail" ? railEdges.value : edges.value;
    const entryNodeId =
        assetMode.value === "rail"
            ? railMeta.value.entryNodeId
            : storyMeta.value.entryNodeId;

    if (graphNodes.length === 0) return;

    const nextNodes = computeAutoLayoutNodes(
        graphNodes,
        graphEdges,
        entryNodeId,
    );

    if (!isDeepEqual(nextNodes, graphNodes)) {
        pushHistorySnapshot();
        if (assetMode.value === "rail") {
            railNodes.value = safeClone(nextNodes);
            persistCurrentRailToLocal();
        } else {
            nodes.value = normalizeEditorStoryNodes(safeClone(nextNodes));
        }
    }

    applyEdgeVisualStyles();
}

function getNodeById(nodeId) {
    return nodes.value.find((node) => node.id === nodeId) || null;
}

function getOutgoingEdgesBySource(sourceId) {
    return edges.value
        .filter((edge) => edge.source === sourceId)
        .sort((a, b) => {
            const pa = Number(a?.data?.priority ?? 0);
            const pb = Number(b?.data?.priority ?? 0);
            if (pa !== pb) return pa - pb;
            return String(a?.id || "").localeCompare(String(b?.id || ""));
        });
}

function handleToggleFocusMode() {
    focusModeEnabled.value = !focusModeEnabled.value;
    applyEdgeVisualStyles();
}

function handleToggleDarkMode() {
    darkModeEnabled.value = !darkModeEnabled.value;
    applyThemeMode();
    persistDarkModePreference();
}

function handleToggleReadMode() {
    editorMode.value = editorMode.value === "graph" ? "read" : "graph";
    if (editorMode.value === "read") {
        selectedNode.value = null;
        selectedEdge.value = null;
        applyEdgeVisualStyles();
    }
}

function handleLoginGithub() {
    window.location.href = "/api/auth/github-start";
}

async function fetchAuthState() {
    authState.value.loading = true;
    try {
        const response = await fetch("/api/auth/me", {
            method: "GET",
            credentials: "include",
        });

        const data = await response.json();
        const authenticated = !!data?.authenticated;

        authState.value = {
            loading: false,
            authenticated,
            user: data?.user || null,
        };

        if (authenticated) {
            localStorage.setItem(EVER_LOGGED_IN_STORAGE_KEY, "1");
        }

        const everLoggedIn =
            localStorage.getItem(EVER_LOGGED_IN_STORAGE_KEY) === "1";
        currentView.value =
            IS_LOCAL_DEV_HOST || authenticated || everLoggedIn
                ? "library"
                : "overview";
    } catch {
        authState.value = {
            loading: false,
            authenticated: false,
            user: null,
        };

        const everLoggedIn =
            localStorage.getItem(EVER_LOGGED_IN_STORAGE_KEY) === "1";
        currentView.value =
            IS_LOCAL_DEV_HOST || everLoggedIn ? "library" : "overview";
    }
}

async function handleLogout() {
    try {
        await fetch("/api/auth/logout", {
            method: "POST",
            credentials: "include",
        });
    } catch {
        // ignore and refresh auth state anyway
    }

    await fetchAuthState();
}

function handleToggleEdgeRenderMode() {
    edgeRenderMode.value =
        edgeRenderMode.value === "straight" ? "bezier" : "straight";
    applyEdgeVisualStyles();
}

function handleGoLibrary() {
    currentView.value = "library";
}

function handleOpenOverview() {
    currentView.value = "overview";
}

function handleOpenEmptyEditor() {
    selectedScriptEntry.value = null;
    selectedRailEntry.value = null;
    selectedGithubFileContext.value = null;
    assetMode.value = "story";
    editorMode.value = "graph";
    currentView.value = "editor";
}

function openRailData(imported, railEntry = null) {
    selectedRailEntry.value = railEntry ? safeClone(railEntry) : null;
    selectedScriptEntry.value = null;
    if (!railEntry || railEntry.source !== "github") {
        selectedGithubFileContext.value = null;
    }
    railMeta.value = {
        schemaVersion: imported.meta?.schemaVersion ?? 1,
        railId: imported.meta?.railId || railEntry?.railId || "main_story",
        title: imported.meta?.title || "",
        entryNodeId: imported.meta?.entryNodeId || "",
    };
    railNodes.value = safeClone(imported.nodes || []);
    railEdges.value = safeClone(imported.edges || []);
    if (!imported.hasNodePositions) {
        const layoutNodes = computeAutoLayoutNodes(
            railNodes.value,
            railEdges.value,
            railMeta.value.entryNodeId,
        );
        if (!isDeepEqual(layoutNodes, railNodes.value)) {
            railNodes.value = safeClone(layoutNodes);
        }
    }
    railValidationResult.value = validateRail(
        railNodes.value,
        railEdges.value,
        railMeta.value,
        { storyEntries: availableStoryEntries.value, variables: variables.value },
    );
    libraryRefreshKey.value += 1;
    assetMode.value = "rail";
    editorMode.value = "graph";
    selectedNode.value = null;
    selectedEdge.value = null;
    currentView.value = "editor";
}

async function handleOpenRailFromLibrary(railEntry) {
    selectedRailEntry.value = safeClone(railEntry || null);
    selectedScriptEntry.value = null;

    if (
        railEntry?.source === "github" &&
        railEntry?.owner &&
        railEntry?.repo &&
        railEntry?.path
    ) {
        try {
            const url = new URL(
                window.location.origin + "/api/github/file-content",
            );
            url.searchParams.set("owner", railEntry.owner);
            url.searchParams.set("repo", railEntry.repo);
            url.searchParams.set("branch", railEntry.branch || "main");
            url.searchParams.set("path", railEntry.path);

            const response = await fetch(url.toString(), {
                method: "GET",
                credentials: "include",
            });
            const data = await response.json();
            if (!response.ok) {
                throw new Error(data?.error || "读取总纲内容失败");
            }

            selectedGithubFileContext.value = {
                owner: railEntry.owner,
                repo: railEntry.repo,
                branch: railEntry.branch || "main",
                path: railEntry.path,
                sha: data?.sha || "",
            };

            openRailData(importRailFromYAML(String(data?.content || "")), railEntry);
            return;
        } catch (error) {
            alert(`打开 GitHub 总纲失败: ${error.message}`);
            return;
        }
    }

    if (railEntry?.source === "mock-repository") {
        const storageKey =
            railEntry.localStorageKey ||
            getLocalScriptStorageKeyByPath(railEntry.path || "");
        const localData = loadLocalScriptContent(storageKey);
        if (localData) {
            openRailData(normalizeRailPayload(localData), railEntry);
            return;
        }
    }

    openRailData(buildEmptyRailData(railEntry?.railId || "main_story"), railEntry);
}

async function handleOpenScriptFromLibrary(scriptEntry) {
    selectedScriptEntry.value = safeClone(scriptEntry || null);
    selectedRailEntry.value = null;
    assetMode.value = "story";
    editorMode.value = "graph";

    if (
        scriptEntry?.source === "github" &&
        scriptEntry?.owner &&
        scriptEntry?.repo &&
        scriptEntry?.path
    ) {
        try {
            const url = new URL(
                window.location.origin + "/api/github/file-content",
            );
            url.searchParams.set("owner", scriptEntry.owner);
            url.searchParams.set("repo", scriptEntry.repo);
            url.searchParams.set("branch", scriptEntry.branch || "main");
            url.searchParams.set("path", scriptEntry.path);

            const response = await fetch(url.toString(), {
                method: "GET",
                credentials: "include",
            });
            const data = await response.json();
            if (!response.ok) {
                throw new Error(data?.error || "读取脚本内容失败");
            }

            const imported = importFromYAML(String(data?.content || ""));
            const importedNodes = normalizeEditorStoryNodes(
                safeClone(imported.nodes || []),
            );
            const importedEdges = safeClone(
                sanitizeEdges(imported.edges || [], importedNodes),
            );

            pushHistorySnapshot();
            nodes.value = importedNodes;
            edges.value = importedEdges;
            storyMeta.value = {
                schemaVersion: imported.meta?.schemaVersion ?? 1,
                storyId:
                    imported.meta?.storyId ||
                    scriptEntry.storyId ||
                    "ImportedStory",
                entryNodeId: imported.meta?.entryNodeId || "",
            };
            selectedNode.value = null;
            selectedEdge.value = null;

            selectedGithubFileContext.value = {
                owner: scriptEntry.owner,
                repo: scriptEntry.repo,
                branch: scriptEntry.branch || "main",
                path: scriptEntry.path,
                sha: data?.sha || "",
            };

            handleAutoLayout();
            scheduleRealtimeValidation();
            applyEdgeVisualStyles();
            currentView.value = "editor";
            return;
        } catch (error) {
            alert(`打开 GitHub 脚本失败: ${error.message}`);
            return;
        }
    }

    if (scriptEntry?.source === "mock-repository") {
        selectedGithubFileContext.value = null;
        const storageKey =
            scriptEntry?.localStorageKey ||
            getLocalScriptStorageKeyByPath(scriptEntry?.path || "");
        const localData = loadLocalScriptContent(storageKey);

        if (localData) {
            const importedNodes = normalizeEditorStoryNodes(
                safeClone(localData.nodes || []),
            );
            const importedEdges = safeClone(
                sanitizeEdges(localData.edges || [], importedNodes),
            );

            pushHistorySnapshot();
            nodes.value = importedNodes;
            edges.value = importedEdges;
            storyMeta.value = {
                schemaVersion: localData?.meta?.schemaVersion ?? 1,
                storyId:
                    localData?.meta?.storyId ||
                    scriptEntry.storyId ||
                    "LocalStory",
                entryNodeId: localData?.meta?.entryNodeId || "",
            };
            selectedNode.value = null;
            selectedEdge.value = null;
        } else if (scriptEntry?.storyId) {
            storyMeta.value = {
                ...storyMeta.value,
                storyId: scriptEntry.storyId,
                entryNodeId:
                    storyMeta.value.entryNodeId || nodes.value[0]?.id || "",
                schemaVersion: storyMeta.value.schemaVersion || 1,
            };
        }

        currentView.value = "editor";
        scheduleRealtimeValidation();
        applyEdgeVisualStyles();
        return;
    }

    if (scriptEntry?.storyId) {
        selectedGithubFileContext.value = null;
        storyMeta.value = {
            ...storyMeta.value,
            storyId: scriptEntry.storyId,
            entryNodeId:
                storyMeta.value.entryNodeId || nodes.value[0]?.id || "",
            schemaVersion: storyMeta.value.schemaVersion || 1,
        };
    }

    currentView.value = "editor";
    scheduleRealtimeValidation();
    applyEdgeVisualStyles();
}

function handleUndo() {
    if (undoStack.value.length === 0) return;

    const current = createStateSnapshot();
    const prev = undoStack.value.pop();

    redoStack.value.push(current);
    applyStateSnapshot(prev);
}

function handleRedo() {
    if (redoStack.value.length === 0) return;

    const current = createStateSnapshot();
    const next = redoStack.value.pop();

    undoStack.value.push(current);
    applyStateSnapshot(next);
}

function handleGlobalKeyDown(event) {
    const activeElement = document.activeElement;
    const isInputFocused =
        activeElement?.tagName === "INPUT" ||
        activeElement?.tagName === "TEXTAREA" ||
        activeElement?.isContentEditable;

    if (isInputFocused) return;

    const key = String(event.key || "").toLowerCase();
    const isCtrlOrMeta = event.ctrlKey || event.metaKey;

    if (isCtrlOrMeta && !event.shiftKey && key === "z") {
        event.preventDefault();
        handleUndo();
        return;
    }

    if (
        (isCtrlOrMeta && key === "y") ||
        (isCtrlOrMeta && event.shiftKey && key === "z")
    ) {
        event.preventDefault();
        handleRedo();
    }
}

onMounted(() => {
    loadDarkModePreference();
    fetchAuthState();

    window.addEventListener("node-click", handleNodeClick);
    window.addEventListener("edge-click", handleEdgeClick);
    window.addEventListener("graph-state-change", handleGraphStateChange);
    window.addEventListener("node-drag-start", handleNodeDragStart);
    window.addEventListener("node-drag-stop", handleNodeDragStop);
    window.addEventListener("keydown", handleGlobalKeyDown);

    if (IS_LOCAL_DEV_HOST && typeof window !== "undefined") {
        window.__narrrailBench = {
            runGraphSyncBenchmark,
        };
    }

    runRealtimeValidation();
    applyEdgeVisualStyles();
});

onUnmounted(() => {
    if (autoSaveTimer) clearInterval(autoSaveTimer);
    if (globalConfigSyncTimer) {
        clearTimeout(globalConfigSyncTimer);
        globalConfigSyncTimer = null;
    }
    if (validationDebounceTimer.value) {
        clearTimeout(validationDebounceTimer.value);
        validationDebounceTimer.value = null;
    }
    clearPendingDragGraphStateFlush();
    pendingDragGraphStateDetail = null;
    if (IS_LOCAL_DEV_HOST && typeof window !== "undefined") {
        delete window.__narrrailBench;
    }
    window.removeEventListener("node-click", handleNodeClick);
    window.removeEventListener("edge-click", handleEdgeClick);
    window.removeEventListener("graph-state-change", handleGraphStateChange);
    window.removeEventListener("node-drag-start", handleNodeDragStart);
    window.removeEventListener("node-drag-stop", handleNodeDragStop);
    window.removeEventListener("keydown", handleGlobalKeyDown);
});

watch(
    () => [
        assetMode.value,
        nodes.value,
        edges.value,
        storyMeta.value.entryNodeId,
        railNodes.value,
        railEdges.value,
        railMeta.value.entryNodeId,
        variables.value,
    ],
    () => {
        if (isNodeDragInProgress.value) {
            pendingValidationAfterDrag = true;
            return;
        }
        scheduleRealtimeValidation();
    },
    { deep: true, immediate: true },
);

watch(
    () => [assetMode.value, storyMeta.value.storyId, railMeta.value.railId],
    () => {
        if (autoSaveTimer) {
            clearInterval(autoSaveTimer);
            autoSaveTimer = null;
        }
        autoSaveDirty = false;
        markAutoSaveDirty();
    },
);

watch(
    () => [
        assetMode.value,
        nodes.value,
        edges.value,
        storyMeta.value,
        railNodes.value,
        railEdges.value,
        railMeta.value,
        variables.value,
        presetSpeakers.value,
    ],
    () => {
        if (isNodeDragInProgress.value) return;
        markAutoSaveDirty();
    },
    { deep: true },
);

watch(
    () => [
        selectedScriptEntry.value?.path,
        selectedScriptEntry.value?.source,
        storyMeta.value,
        variables.value,
        nodes.value,
        edges.value,
    ],
    () => {
        if (isNodeDragInProgress.value) {
            pendingMockRepoPersistAfterDrag = true;
            return;
        }
        persistCurrentMockRepoScriptToLocal();
    },
    { deep: true },
);

watch(
    () => [
        selectedRailEntry.value?.path,
        selectedRailEntry.value?.source,
        railMeta.value,
        railNodes.value,
        railEdges.value,
    ],
    () => {
        if (assetMode.value !== "rail") return;
        if (isNodeDragInProgress.value) {
            pendingMockRepoPersistAfterDrag = true;
            return;
        }
        persistCurrentRailToLocal();
    },
    { deep: true },
);

watch(
    () => [
        selectedNode.value?.id,
        selectedEdge.value?.id,
        focusModeEnabled.value,
        edgeRenderMode.value,
    ],
    () => applyEdgeVisualStyles(),
);

watch(
    () => darkModeEnabled.value,
    () => {
        applyThemeMode();
        persistDarkModePreference();
    },
);

function handleNew() {
    if (assetMode.value === "rail") {
        if (
            (railNodes.value.length > 0 || railEdges.value.length > 0) &&
            !confirm("创建新总纲将清空当前内容，是否继续？")
        ) {
            return;
        }
        const empty = buildEmptyRailData("main_story");
        railMeta.value = safeClone(empty.meta);
        railNodes.value = safeClone(empty.nodes);
        railEdges.value = safeClone(empty.edges);
        railValidationResult.value = validateRail(
            railNodes.value,
            railEdges.value,
            railMeta.value,
            { storyEntries: availableStoryEntries.value, variables: variables.value },
        );
        persistCurrentRailToLocal();
        return;
    }

    if (
        nodes.value.length > 0 &&
        !confirm("创建新剧情将清空当前内容，是否继续？")
    )
        return;

    if (
        nodes.value.length > 0 ||
        edges.value.length > 0 ||
        variables.value.length > 0 ||
        presetSpeakers.value.length > 0
    ) {
        pushHistorySnapshot();
    }

    nodes.value = [];
    edges.value = [];
    selectedNode.value = null;
    selectedEdge.value = null;
    storyMeta.value = {
        storyId: "NewStory",
        entryNodeId: "",
        schemaVersion: 1,
    };
    variables.value = [];
    presetSpeakers.value = [];
    scheduleRealtimeValidation();
}

function handleImport() {
    fileInput.value?.click();
}

function handleFileChange(event) {
    const file = event.target.files?.[0];
    if (!file) return;

    if (String(file.name || "").toLowerCase().endsWith(".nrrail")) {
        const hasRailData = railNodes.value.length > 0 || railEdges.value.length > 0;
        if (
            assetMode.value === "rail" &&
            hasRailData &&
            !confirm("导入总纲将覆盖当前总纲内容，是否继续？")
        ) {
            event.target.value = "";
            return;
        }

        const reader = new FileReader();
        reader.onload = (e) => {
            try {
                const imported = importRailFromYAML(String(e.target?.result || ""));
                openRailData(imported, null);
                alert(
                    `总纲导入成功！\n节点数: ${railNodes.value.length}\n边数: ${railEdges.value.length}`,
                );
            } catch (error) {
                alert(`总纲导入失败: ${error.message}`);
            }
        };
        reader.readAsText(file);
        event.target.value = "";
        return;
    }

    assetMode.value = "story";
    editorMode.value = "graph";

    const hasCurrentData =
        nodes.value.length > 0 ||
        edges.value.length > 0 ||
        variables.value.length > 0 ||
        presetSpeakers.value.length > 0;
    if (hasCurrentData && !confirm("导入将覆盖当前编辑内容，是否继续？")) {
        event.target.value = "";
        return;
    }

    const reader = new FileReader();
    reader.onload = (e) => {
        try {
            const imported = importFromYAML(String(e.target?.result || ""));
            const importedNodes = normalizeEditorStoryNodes(
                safeClone(imported.nodes || []),
            );
            const importedEdges = safeClone(
                sanitizeEdges(imported.edges || [], importedNodes),
            );
            const importedMeta = {
                schemaVersion: imported.meta?.schemaVersion ?? 1,
                storyId: imported.meta?.storyId || "ImportedStory",
                entryNodeId: imported.meta?.entryNodeId || "",
            };

            if (importedMeta.schemaVersion !== 1) {
                const proceed = confirm(
                    `检测到 schemaVersion=${importedMeta.schemaVersion}（当前编辑器按 v1 处理）。是否继续导入？`,
                );
                if (!proceed) return;
            }

            const precheck = validateStory(
                importedNodes,
                importedEdges,
                importedMeta,
                variables.value,
            );
            if (precheck.errors.length > 0) {
                const preview = precheck.errors
                    .slice(0, 8)
                    .map((err) => `- ${err.message}`)
                    .join("\n");
                const proceed = confirm(
                    `导入内容存在 ${precheck.errors.length} 个错误：\n${preview}\n\n仍然继续导入吗？`,
                );
                if (!proceed) return;
            }

            const importedPresetSpeakers = normalizePresetSpeakers(
                imported.presetSpeakers,
            );
            pushHistorySnapshot();
            nodes.value = importedNodes;
            edges.value = importedEdges;
            storyMeta.value = importedMeta;
            presetSpeakers.value =
                importedPresetSpeakers.length > 0
                    ? importedPresetSpeakers
                    : derivePresetSpeakersFromNodes(importedNodes);
            selectedNode.value = null;
            selectedEdge.value = null;

            // 导入成功后默认执行一次自动排布，提升可读性
            handleAutoLayout();

            runRealtimeValidation();
            applyEdgeVisualStyles();
            alert(
                `导入成功！\n节点数: ${nodes.value.length}\n边数: ${edges.value.length}`,
            );
        } catch (error) {
            alert(`导入失败: ${error.message}`);
        }
    };

    reader.readAsText(file);
    event.target.value = "";
}

async function handleExport() {
    if (assetMode.value === "rail") {
        if (railNodes.value.length === 0) {
            alert("没有可导出的总纲内容");
            return;
        }

        const result = validateRail(railNodes.value, railEdges.value, railMeta.value, {
            storyEntries: availableStoryEntries.value,
            variables: variables.value,
        });
        railValidationResult.value = result;
        if (result.errors.length > 0) {
            alert(
                `总纲导出已阻止：存在 ${result.errors.length} 个错误。\n\n${result.errors
                    .slice(0, 8)
                    .map((err) => `- ${err}`)
                    .join("\n")}`,
            );
            return;
        }
        if (result.warnings.length > 0) {
            const proceed = confirm(
                `当前总纲有 ${result.warnings.length} 个警告，是否仍然导出？`,
            );
            if (!proceed) return;
        }

        try {
            const yamlString = buildRailYAMLString(
                railNodes.value,
                railEdges.value,
                railMeta.value,
            );
            if (
                selectedGithubFileContext.value?.owner &&
                selectedGithubFileContext.value?.repo &&
                selectedGithubFileContext.value?.path?.endsWith(".nrrail")
            ) {
                if (isSavingToGithub.value) return;
                isSavingToGithub.value = true;
                const response = await fetch("/api/github/commit-file", {
                    method: "POST",
                    credentials: "include",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({
                        owner: selectedGithubFileContext.value.owner,
                        repo: selectedGithubFileContext.value.repo,
                        branch: selectedGithubFileContext.value.branch || "main",
                        path: selectedGithubFileContext.value.path,
                        sha: selectedGithubFileContext.value.sha || undefined,
                        content: yamlString,
                        message: `feat(rail): update ${selectedGithubFileContext.value.path}`,
                    }),
                });
                const data = await response.json();
                if (!response.ok) {
                    throw new Error(data?.error || "提交总纲到 GitHub 失败");
                }
                selectedGithubFileContext.value = {
                    ...selectedGithubFileContext.value,
                    sha: data?.content?.sha || selectedGithubFileContext.value.sha,
                };
                alert(`总纲已提交到 GitHub！\ncommit: ${data?.commit?.sha || "(unknown)"}`);
                return;
            }

            exportRailToYAML(railNodes.value, railEdges.value, railMeta.value);
            persistCurrentRailToLocal();
            alert(
                `总纲导出成功！\n节点数: ${railNodes.value.length}\n边数: ${railEdges.value.length}`,
            );
        } catch (error) {
            alert(`总纲导出失败: ${error.message}`);
        } finally {
            isSavingToGithub.value = false;
        }
        return;
    }

    if (nodes.value.length === 0) {
        alert("没有可导出的内容");
        return;
    }

    runRealtimeValidation();
    const result = validationResult.value;

    if (result.errors.length > 0) {
        const preview = result.errors
            .slice(0, 8)
            .map((err) => `- ${err.message}`)
            .join("\n");
        alert(
            `导出已阻止：存在 ${result.errors.length} 个验证错误。\n\n${preview}`,
        );
        return;
    }

    if (result.warnings.length > 0) {
        const proceed = confirm(
            `当前有 ${result.warnings.length} 个警告，是否仍然导出？`,
        );
        if (!proceed) return;
    }

    try {
        const yamlString = buildYAMLString(
            nodes.value,
            edges.value,
            variables.value,
            storyMeta.value,
        );

        if (
            selectedGithubFileContext.value?.owner &&
            selectedGithubFileContext.value?.repo &&
            selectedGithubFileContext.value?.path
        ) {
            if (isSavingToGithub.value) return;
            isSavingToGithub.value = true;

            const payload = {
                owner: selectedGithubFileContext.value.owner,
                repo: selectedGithubFileContext.value.repo,
                branch: selectedGithubFileContext.value.branch || "main",
                path: selectedGithubFileContext.value.path,
                sha: selectedGithubFileContext.value.sha || undefined,
                content: yamlString,
                message: `feat(script): update ${selectedGithubFileContext.value.path}`,
            };

            const response = await fetch("/api/github/commit-file", {
                method: "POST",
                credentials: "include",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(payload),
            });

            const data = await response.json();
            if (!response.ok) {
                throw new Error(data?.error || "提交到 GitHub 失败");
            }

            selectedGithubFileContext.value = {
                ...selectedGithubFileContext.value,
                sha: data?.content?.sha || selectedGithubFileContext.value.sha,
            };

            lastSavedAt.value = new Date().toISOString();
            autoSaveDirty = false;
            alert(
                `已提交到 GitHub！\ncommit: ${data?.commit?.sha || "(unknown)"}`,
            );
            return;
        }

        exportToYAML(
            nodes.value,
            edges.value,
            variables.value,
            storyMeta.value,
        );
        lastSavedAt.value = new Date().toISOString();
        autoSaveDirty = false;
        alert(
            `导出成功！\n节点数: ${nodes.value.length}\n边数: ${edges.value.length}`,
        );
    } catch (error) {
        alert(`导出失败: ${error.message}`);
    } finally {
        isSavingToGithub.value = false;
    }
}

function handleValidate() {
    if (assetMode.value === "rail") {
        const result = validateRail(railNodes.value, railEdges.value, railMeta.value, {
            storyEntries: availableStoryEntries.value,
            variables: variables.value,
        });
        railValidationResult.value = result;
        if (result.errors.length === 0 && result.warnings.length === 0) {
            alert("总纲验证通过！没有发现错误。");
            return;
        }

        let message = "总纲验证结果：\n\n";
        if (result.errors.length > 0) {
            message += `错误 (${result.errors.length}):\n`;
            result.errors.forEach((err) => (message += `- ${err}\n`));
        }
        if (result.warnings.length > 0) {
            message += `\n警告 (${result.warnings.length}):\n`;
            result.warnings.forEach((warn) => (message += `- ${warn}\n`));
        }
        alert(message);
        return;
    }

    runRealtimeValidation();
    const result = validationResult.value;
    if (result.errors.length === 0 && result.warnings.length === 0) {
        alert("验证通过！没有发现错误。");
        return;
    }

    let message = "验证结果：\n\n";
    if (result.errors.length > 0) {
        message += `错误 (${result.errors.length}):\n`;
        result.errors.forEach((err) => (message += `- ${err.message}\n`));
    }
    if (result.warnings.length > 0) {
        message += `\n警告 (${result.warnings.length}):\n`;
        result.warnings.forEach((warn) => (message += `- ${warn.message}\n`));
    }
    alert(message);
}

function handleHelp() {
    alert(
        "NarrRail Story Editor\n\n" +
            "快捷操作：\n" +
            "- 右键画布：添加节点\n" +
            "- 画布平移：鼠标中键拖拽 / 触控板双指滑动\n" +
            "- 点击“自动排布”：按层级整理节点\n" +
            "- 点击“连线样式”：直线/曲线切换\n" +
            "- 点击“焦点模式”：弱化非相关连线\n" +
            "- 点击边：编辑边的优先级和条件\n" +
            "- Delete键：删除选中的节点或边",
    );
}

function handleNodeUpdate(updatedNode) {
    if (!updatedNode) return;

    if (assetMode.value === "rail") {
        const prevId = selectedNode.value?.id;
        let index = -1;
        if (prevId) index = railNodes.value.findIndex((n) => n.id === prevId);
        if (index === -1)
            index = railNodes.value.findIndex((n) => n.id === updatedNode.id);
        if (index === -1) return;

        const oldId = railNodes.value[index].id;
        const nextNode = safeClone(updatedNode);
        if (isDeepEqual(railNodes.value[index], nextNode)) return;
        railNodes.value[index] = nextNode;

        let nextEdges = [...railEdges.value];
        if (oldId !== nextNode.id) {
            nextEdges = nextEdges.map((edge) => ({
                ...edge,
                source: edge.source === oldId ? nextNode.id : edge.source,
                target: edge.target === oldId ? nextNode.id : edge.target,
            }));

            if (railMeta.value.entryNodeId === oldId) {
                railMeta.value.entryNodeId = nextNode.id;
            }
        }

        railEdges.value = safeClone(sanitizeRailEdges(nextEdges));
        selectedNode.value = safeClone(nextNode);
        persistCurrentRailToLocal();
        applyEdgeVisualStyles();
        return;
    }

    const prevId = selectedNode.value?.id;
    let index = -1;
    if (prevId) index = nodes.value.findIndex((n) => n.id === prevId);
    if (index === -1)
        index = nodes.value.findIndex((n) => n.id === updatedNode.id);
    if (index === -1) return;

    const oldId = nodes.value[index].id;
    const nextNode = normalizeEditorDialogueNode(safeClone(updatedNode));
    if (isDeepEqual(nodes.value[index], nextNode)) return;
    pushHistorySnapshot();
    nodes.value[index] = nextNode;

    let nextEdges = [...edges.value];
    if (oldId !== nextNode.id) {
        nextEdges = nextEdges.map((edge) => ({
            ...edge,
            source: edge.source === oldId ? nextNode.id : edge.source,
            target: edge.target === oldId ? nextNode.id : edge.target,
        }));

        if (storyMeta.value.entryNodeId === oldId) {
            storyMeta.value.entryNodeId = nextNode.id;
        }
    }

    nextEdges = syncChoiceEdgesForNode(nextNode, nextEdges);
    nextEdges = syncConditionEdgesForNode(nextNode, nextEdges);
    edges.value = safeClone(sanitizeEdges(nextEdges));

    if (selectedEdge.value) {
        const latestEdge = edges.value.find(
            (e) => e.id === selectedEdge.value.id,
        );
        selectedEdge.value = latestEdge
            ? normalizeEdge(safeClone(latestEdge))
            : null;
        syncEdgeDraftFromSelection();
    }

    selectedNode.value = safeClone(nextNode);
    applyEdgeVisualStyles();
}

function handleEdgeUpdate(updatedEdge) {
    if (!updatedEdge) return;

    const edgeId = updatedEdge.id || selectedEdge.value?.id;
    if (!edgeId) return;

    const normalized = normalizeEdge({ ...updatedEdge, id: edgeId });
    if (assetMode.value === "rail") {
        const sourceNode = railNodes.value.find((n) => n.id === normalized.source);
        const targetNode = railNodes.value.find((n) => n.id === normalized.target);
        if (!sourceNode || !targetNode) {
            alert("边更新失败：源节点或目标节点不存在。");
            return;
        }

        if (!isValidRailHandle(sourceNode, normalized.sourceHandle)) {
            alert("边更新失败：总纲 Branch 节点 sourceHandle 无效。");
            return;
        }

        const index = railEdges.value.findIndex((e) => e.id === edgeId);
        const nextEdges = [...railEdges.value];
        if (index === -1) nextEdges.push(normalized);
        else nextEdges[index] = normalized;

        const sanitizedNext = sanitizeRailEdges(nextEdges);
        if (!isDeepEqual(sanitizedNext, railEdges.value)) {
            railEdges.value = safeClone(sanitizedNext);
            persistCurrentRailToLocal();
        }

        const latestEdge = railEdges.value.find((e) => e.id === edgeId);
        selectedEdge.value = latestEdge ? safeClone(latestEdge) : null;
        syncEdgeDraftFromSelection();
        applyEdgeVisualStyles();
        return;
    }

    const sourceNode = nodes.value.find((n) => n.id === normalized.source);
    const targetNode = nodes.value.find((n) => n.id === normalized.target);
    if (!sourceNode || !targetNode) {
        alert("边更新失败：源节点或目标节点不存在。");
        return;
    }

    if (!isValidChoiceHandle(sourceNode, normalized.sourceHandle)) {
        alert("边更新失败：Choice 节点 sourceHandle 无效。");
        return;
    }

    const index = edges.value.findIndex((e) => e.id === edgeId);
    const nextEdges = [...edges.value];
    if (index === -1) nextEdges.push(normalized);
    else nextEdges[index] = normalized;

    const sanitizedNext = sanitizeEdges(nextEdges);
    if (!isDeepEqual(sanitizedNext, edges.value)) {
        pushHistorySnapshot();
        edges.value = safeClone(sanitizedNext);
    }

    const latestEdge = edges.value.find((e) => e.id === edgeId);
    selectedEdge.value = latestEdge ? safeClone(latestEdge) : null;
    syncEdgeDraftFromSelection();
    applyEdgeVisualStyles();
}

function handleSetEntryNode(nodeId) {
    if (assetMode.value === "rail") {
        if (railMeta.value.entryNodeId === nodeId) return;
        railMeta.value.entryNodeId = nodeId;
        persistCurrentRailToLocal();
        return;
    }

    if (storyMeta.value.entryNodeId === nodeId) return;
    pushHistorySnapshot();
    storyMeta.value.entryNodeId = nodeId;
}

function syncGlobalConfigToLocalFallback() {
    try {
        localStorage.setItem(
            LOCAL_GLOBAL_CONFIG_STORAGE_KEY,
            JSON.stringify({
                variables: safeClone(variables.value || []),
                presetSpeakers: safeClone(presetSpeakers.value || []),
                updatedAt: new Date().toISOString(),
            }),
        );
    } catch {
        // ignore local storage failures
    }
}

function scheduleGlobalConfigSyncFromEditor() {
    syncGlobalConfigToLocalFallback();

    if (globalConfigSyncTimer) {
        clearTimeout(globalConfigSyncTimer);
        globalConfigSyncTimer = null;
    }

    globalConfigSyncTimer = setTimeout(async () => {
        globalConfigSyncTimer = null;
        await syncGlobalConfigToRepoFromEditor();
    }, 500);
}

async function syncGlobalConfigToRepoFromEditor() {
    if (currentView.value !== "editor") return;
    if (isSyncingGlobalConfigFromEditor.value) return;

    const owner = selectedGithubFileContext.value?.owner;
    const repo = selectedGithubFileContext.value?.repo;
    const branch = selectedGithubFileContext.value?.branch || "main";

    if (!owner || !repo) return;

    isSyncingGlobalConfigFromEditor.value = true;
    try {
        let sha;

        const readUrl = new URL(
            window.location.origin + "/api/github/file-content",
        );
        readUrl.searchParams.set("mode", "content");
        readUrl.searchParams.set("owner", owner);
        readUrl.searchParams.set("repo", repo);
        readUrl.searchParams.set("branch", branch);
        let detectedPath = "";
        for (const candidatePath of GLOBAL_CONFIG_CANDIDATE_PATHS) {
            readUrl.searchParams.set("path", candidatePath);
            const readResponse = await fetch(readUrl.toString(), {
                method: "GET",
                credentials: "include",
            });
            const readData = await readResponse.json();

            if (readResponse.ok) {
                detectedPath = candidatePath;
                sha = readData?.sha || undefined;
                break;
            }

            const isNotFound =
                readResponse.status === 404 ||
                String(readData?.error || "")
                    .toLowerCase()
                    .includes("not found");
            if (!isNotFound) {
                throw new Error(readData?.error || "读取全局配置失败");
            }
        }

        globalConfigRepoPathFromEditor.value =
            detectedPath || GLOBAL_CONFIG_CANDIDATE_PATHS[0];

        const content = serializeGlobalConfigToYAML({
            variables: variables.value,
            presetSpeakers: presetSpeakers.value,
        });

        const payload = {
            owner,
            repo,
            branch,
            path: globalConfigRepoPathFromEditor.value,
            content,
            message: `chore(config): sync ${globalConfigRepoPathFromEditor.value}`,
        };

        if (sha) payload.sha = sha;

        const commitResponse = await fetch("/api/github/commit-file", {
            method: "POST",
            credentials: "include",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(payload),
        });
        const commitData = await commitResponse.json();

        if (!commitResponse.ok) {
            throw new Error(commitData?.error || "同步全局配置失败");
        }
    } catch (error) {
        alert(`同步全局配置失败: ${error.message}`);
    } finally {
        isSyncingGlobalConfigFromEditor.value = false;
    }
}

function handleVariablesUpdate(updatedVariables) {
    const nextVariables = safeClone(updatedVariables || []);
    if (isDeepEqual(nextVariables, variables.value)) return;
    pushHistorySnapshot();
    clearDeletedVariableReferences(nextVariables);
    variables.value = nextVariables;
    scheduleGlobalConfigSyncFromEditor();
}

function handlePresetSpeakersUpdate(updatedSpeakers) {
    const nextSpeakers = normalizePresetSpeakers(updatedSpeakers || []);
    if (isDeepEqual(nextSpeakers, presetSpeakers.value)) return;
    pushHistorySnapshot();
    presetSpeakers.value = safeClone(nextSpeakers);
    scheduleGlobalConfigSyncFromEditor();
}
</script>

<style scoped>
.editor-container {
    width: 100vw;
    height: 100vh;
    display: flex;
    flex-direction: column;
    overflow: hidden;
    padding: 0;
    position: relative;
}

.main-content {
    position: absolute;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    display: flex;
    overflow: hidden;
}

.graph-editor-wrapper {
    flex: 1;
    position: relative;
}

.edge-editor {
    position: fixed;
    right: 16px;
    top: 100px;
    width: 360px;
    max-height: calc(100vh - 190px);
    overflow: auto;
    z-index: 60;
    padding: 16px;
    border-radius: 16px;
}

.edge-editor-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 12px;
}

.edge-editor-header h3 {
    margin: 0;
    font-size: 18px;
    font-weight: 700;
}

.close-btn {
    border: none;
    border-radius: 8px;
    width: 28px;
    height: 28px;
    cursor: pointer;
    background: rgba(0, 0, 0, 0.06);
}

.form-group {
    display: flex;
    flex-direction: column;
    gap: 6px;
    margin-bottom: 12px;
}

.form-group label {
    font-size: 12px;
    font-weight: 700;
    color: #475569;
}

.form-input,
.form-textarea {
    width: 100%;
    border: none;
    border-radius: 8px;
    background: rgba(255, 255, 255, 0.55);
    padding: 10px 12px;
    font-size: 14px;
    color: #1e293b;
}

.form-input:focus,
.form-textarea:focus {
    outline: 2px solid rgba(59, 130, 246, 0.35);
}

.form-textarea {
    min-height: 100px;
    resize: vertical;
}

.edge-editor-actions {
    display: flex;
    gap: 8px;
}

.action-btn {
    border: none;
    border-radius: 8px;
    padding: 10px 14px;
    cursor: pointer;
    background: rgba(0, 0, 0, 0.08);
    font-weight: 600;
}

.action-btn.primary {
    background: rgba(59, 130, 246, 0.22);
    color: #1d4ed8;
}
</style>
