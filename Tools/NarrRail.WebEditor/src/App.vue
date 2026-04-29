<template>
    <ScriptLibraryPage
        v-if="currentView === 'library'"
        :is-dark-mode="darkModeEnabled"
        :variables="variables"
        :preset-speakers="presetSpeakers"
        :auth-state="authState"
        @toggle-theme="handleToggleDarkMode"
        @open-script="handleOpenScriptFromLibrary"
        @open-empty="handleOpenEmptyEditor"
        @update-variables="handleVariablesUpdate"
        @update-speakers="handlePresetSpeakersUpdate"
        @login-github="handleLoginGithub"
        @logout="handleLogout"
    />

    <div v-else class="editor-container">
        <div class="main-content">
            <div class="graph-editor-wrapper">
                <GraphEditorWrapper
                    :nodes="nodes"
                    :edges="edges"
                    :edge-render-mode="edgeRenderMode"
                    :preset-speakers="presetSpeakers"
                />
            </div>
        </div>

        <Toolbar
            :focus-mode-enabled="focusModeEnabled"
            :is-dark-mode="darkModeEnabled"
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
            @toggle-dark-mode="handleToggleDarkMode"
            @go-library="handleGoLibrary"
            @help="handleHelp"
        />

        <StatusBar
            :node-count="nodes.length"
            :edge-count="edges.length"
            :story-id="storyMeta.storyId"
            :entry-node-id="storyMeta.entryNodeId"
            :error-count="validationResult.errors.length"
            :warning-count="validationResult.warnings.length"
            :autosave-interval-sec="30"
            :autosave-target="`localStorage:narrrail_${storyMeta.storyId || 'NewStory'}`"
        />

        <PropertyPanel
            :selected-node="selectedNode"
            :entry-node-id="storyMeta.entryNodeId"
            :preset-speakers="presetSpeakers"
            @update="handleNodeUpdate"
            @set-entry-node="handleSetEntryNode"
        />

        <VariablePanel
            :variables="variables"
            :preset-speakers="presetSpeakers"
            @update="handleVariablesUpdate"
            @update-speakers="handlePresetSpeakersUpdate"
        />

        <div v-if="selectedEdge" class="edge-editor glass-morphism-strong">
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

            <div class="form-group">
                <label>条件逻辑 (condition.logic)</label>
                <select class="form-input" v-model="edgeDraft.logic">
                    <option value="All">All</option>
                    <option value="Any">Any</option>
                    <option value="Not">Not</option>
                </select>
            </div>

            <div class="form-group">
                <label>条件项 (condition.terms)</label>
                <div class="condition-editor">
                    <div
                        v-if="edgeDraft.terms.length === 0"
                        class="condition-empty"
                    >
                        暂无条件项（空数组表示恒真条件）
                    </div>

                    <div
                        v-for="(term, index) in edgeDraft.terms"
                        :key="`term-${index}`"
                        class="condition-term-card"
                    >
                        <div class="condition-term-header">
                            <span>Term #{{ index + 1 }}</span>
                            <button
                                class="mini-btn danger"
                                @click="removeEdgeConditionTerm(index)"
                            >
                                删除
                            </button>
                        </div>

                        <div class="condition-grid">
                            <input
                                class="form-input"
                                v-model="term.variable.name"
                                placeholder="变量名 (variable.name)"
                            />
                            <select
                                class="form-input"
                                v-model="term.variable.type"
                            >
                                <option
                                    v-for="type in conditionVariableTypes"
                                    :key="type"
                                    :value="type"
                                >
                                    {{ type }}
                                </option>
                            </select>
                            <select
                                class="form-input"
                                v-model="term.variable.scope"
                            >
                                <option
                                    v-for="scope in conditionVariableScopes"
                                    :key="scope"
                                    :value="scope"
                                >
                                    {{ scope }}
                                </option>
                            </select>
                            <select class="form-input" v-model="term.operator">
                                <option
                                    v-for="op in conditionOperators"
                                    :key="op"
                                    :value="op"
                                >
                                    {{ op }}
                                </option>
                            </select>
                            <input
                                class="form-input"
                                v-model="term.compareValue"
                                placeholder="比较值 (compareValue)"
                            />
                        </div>
                    </div>

                    <div class="condition-actions">
                        <button
                            class="mini-btn primary"
                            @click="addEdgeConditionTerm"
                        >
                            + 添加条件项
                        </button>
                    </div>

                    <p v-if="edgeDraftError" class="condition-error">
                        {{ edgeDraftError }}
                    </p>
                </div>
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
            accept=".yaml,.yml"
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
import ScriptLibraryPage from "./components/ScriptLibraryPage.vue";
import { buildYAMLString, exportToYAML } from "./utils/yaml-exporter.js";
import { importFromYAML } from "./utils/yaml-importer.js";
import { validateStory } from "./utils/validation.js";
import { serializeGlobalConfigToYAML } from "./utils/global-config-yaml.js";
import storage from "./utils/storage.js";

const nodes = ref([
    {
        id: "node-1",
        type: "dialogue",
        position: { x: 220, y: 120 },
        data: {
            speakerId: "Alice",
            textKey: "你好！欢迎使用 NarrRail 编辑器。",
        },
    },
    {
        id: "node-2",
        type: "choice",
        position: { x: 560, y: 120 },
        data: { choices: [{ textKey: "继续对话" }, { textKey: "结束对话" }] },
    },
    {
        id: "node-3",
        type: "dialogue",
        position: { x: 900, y: 40 },
        data: { speakerId: "Bob", textKey: "很高兴认识你！" },
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
        data: { priority: 0, condition: { logic: "All", terms: [] } },
    },
    {
        id: "e2-3",
        source: "node-2",
        sourceHandle: "choice-0",
        target: "node-3",
        type: "straight",
        animated: false,
        data: { priority: 0, condition: { logic: "All", terms: [] } },
    },
    {
        id: "e2-4",
        source: "node-2",
        sourceHandle: "choice-1",
        target: "node-4",
        type: "straight",
        animated: false,
        data: { priority: 0, condition: { logic: "All", terms: [] } },
    },
]);

const selectedNode = ref(null);
const selectedEdge = ref(null);
const focusModeEnabled = ref(false);
const darkModeEnabled = ref(false);
const edgeRenderMode = ref("straight");
const DARK_MODE_STORAGE_KEY = "narrrail_editor_theme";

const authState = ref({
    loading: false,
    authenticated: false,
    user: null,
});

const currentView = ref("library");
const selectedScriptEntry = ref(null);
const selectedGithubFileContext = ref(null);
const isSavingToGithub = ref(false);
const isSyncingGlobalConfigFromEditor = ref(false);
const GLOBAL_CONFIG_CANDIDATE_PATHS = [
    "globalconfig.narrrail.yaml",
    "global-config.narrrail.yaml",
    "globalconfig.narrrail.yml",
    "global-config.narrrail.yml",
];
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

const variables = ref([]);
const presetSpeakers = ref([]);
const fileInput = ref(null);

const validationResult = ref({ errors: [], warnings: [] });
const hasErrors = computed(() => validationResult.value.errors.length > 0);
const hasWarnings = computed(() => validationResult.value.warnings.length > 0);

const edgeDraft = reactive({
    priority: 0,
    logic: "All",
    terms: [],
});

const edgeDraftError = ref("");
const validationDebounceTimer = ref(null);
const conditionOperators = ["==", "!=", ">", ">=", "<", "<="];
const conditionVariableTypes = ["Bool", "Int", "Float", "String"];
const conditionVariableScopes = ["Session", "Global"];
const VALID_LOGICS = new Set(["All", "Any", "Not"]);

let autoSaveTimer = null;
let globalConfigSyncTimer = null;

function safeClone(obj) {
    return JSON.parse(JSON.stringify(obj));
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

    nodes.value = safeClone(snapshot.nodes || []);
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
}

function handleNodeDragStop() {
    isNodeDragInProgress.value = false;
}

function isDeepEqual(a, b) {
    return JSON.stringify(a) === JSON.stringify(b);
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
            condition: {
                logic: edge?.data?.condition?.logic || "All",
                terms: Array.isArray(edge?.data?.condition?.terms)
                    ? edge.data.condition.terms
                    : [],
            },
        },
    };
}

function isValidChoiceHandle(node, sourceHandle) {
    if (!node || node.type !== "choice") return true;
    if (!sourceHandle) return true;
    const match = /^choice-(\d+)$/.exec(sourceHandle);
    if (!match) return false;
    const index = Number(match[1]);
    const count = Array.isArray(node.data?.choices)
        ? node.data.choices.length
        : 0;
    return Number.isInteger(index) && index >= 0 && index < count;
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

        return isValidChoiceHandle(sourceNode, edge.sourceHandle);
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
        .filter((node) => node?.type === "dialogue")
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
    const sourceFanOutOffsets = buildSourceFanOutOffsetMap(edges.value);
    const parallelOffsets = buildParallelEdgeOffsetMap(edges.value);
    const isBezier = edgeRenderMode.value === "bezier";
    const edgeType = isBezier ? "default" : "straight";

    const next = edges.value.map((edge) => {
        const sourceOffset = sourceFanOutOffsets.get(edge.id) ?? 0;
        const parallelOffset = parallelOffsets.get(edge.id) ?? 0;
        const visualOffset = sourceOffset + parallelOffset;

        const { style, animated, pathOptions, zIndex } = deriveEdgeStyle(
            edge,
            visualOffset,
        );

        return {
            ...edge,
            style,
            animated,
            type: edgeType,
            ...(isBezier ? { pathOptions } : { pathOptions: undefined }),
            zIndex,
        };
    });

    if (!isDeepEqual(next, edges.value)) {
        edges.value = safeClone(next);
    }
}

function runRealtimeValidation() {
    validationResult.value = validateStory(
        nodes.value,
        edges.value,
        storyMeta.value,
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

function setupAutoSave() {
    if (autoSaveTimer) clearInterval(autoSaveTimer);
    autoSaveTimer = storage.setupAutoSave(
        storyMeta.value.storyId || "NewStory",
        () => ({
            nodes: nodes.value,
            edges: edges.value,
            meta: storyMeta.value,
            variables: variables.value,
            presetSpeakers: presetSpeakers.value,
        }),
    );
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

function normalizeConditionTerm(term = {}) {
    return {
        variable: {
            name: String(term?.variable?.name || ""),
            type: conditionVariableTypes.includes(term?.variable?.type)
                ? term.variable.type
                : "String",
            scope: conditionVariableScopes.includes(term?.variable?.scope)
                ? term.variable.scope
                : "Session",
        },
        operator: conditionOperators.includes(term?.operator)
            ? term.operator
            : "==",
        compareValue:
            term?.compareValue == null ? "" : String(term.compareValue),
    };
}

function normalizeConditionTerms(terms) {
    if (!Array.isArray(terms)) return [];
    return terms.map((term) => normalizeConditionTerm(term));
}

function syncEdgeDraftFromSelection() {
    if (!selectedEdge.value) return;
    edgeDraft.priority = selectedEdge.value.data?.priority ?? 0;
    edgeDraft.logic = selectedEdge.value.data?.condition?.logic || "All";
    edgeDraft.terms = normalizeConditionTerms(
        selectedEdge.value.data?.condition?.terms || [],
    );
    edgeDraftError.value = "";
}

function applyEdgeDraft() {
    if (!selectedEdge.value) return;

    const logic = VALID_LOGICS.has(edgeDraft.logic) ? edgeDraft.logic : "All";
    const parsedTerms = normalizeConditionTerms(edgeDraft.terms);

    for (let i = 0; i < parsedTerms.length; i++) {
        const term = parsedTerms[i];
        if (!term.variable.name.trim()) {
            edgeDraftError.value = `Term #${i + 1} 的 variable.name 不能为空`;
            return;
        }
    }

    if (logic === "Not" && parsedTerms.length !== 1) {
        edgeDraftError.value =
            "logic=Not 时 condition.terms 建议且通常应为 1 项";
        return;
    }

    edgeDraftError.value = "";
    handleEdgeUpdate({
        ...selectedEdge.value,
        data: {
            ...(selectedEdge.value.data || {}),
            priority: Number.isFinite(edgeDraft.priority)
                ? edgeDraft.priority
                : 0,
            condition: { logic, terms: parsedTerms },
        },
    });
}

function addEdgeConditionTerm() {
    edgeDraft.terms.push(
        normalizeConditionTerm({
            variable: { name: "", type: "String", scope: "Session" },
            operator: "==",
            compareValue: "",
        }),
    );
    edgeDraftError.value = "";
}

function removeEdgeConditionTerm(index) {
    edgeDraft.terms.splice(index, 1);
    edgeDraftError.value = "";
}

function resetEdgeDraft() {
    syncEdgeDraftFromSelection();
}

function clearEdgeSelection() {
    selectedEdge.value = null;
    applyEdgeVisualStyles();
}

function handleNodeClick(event) {
    selectedNode.value = event.detail ? safeClone(event.detail) : null;
    if (selectedNode.value) selectedEdge.value = null;
    applyEdgeVisualStyles();
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
    const nextNodes = Array.isArray(nextNodesInput) ? nextNodesInput : [];
    const nextEdgesRaw = Array.isArray(nextEdgesInput) ? nextEdgesInput : [];
    const sanitizedEdges = sanitizeEdges(nextEdgesRaw, nextNodes);

    const shouldUpdateNodes = !isDeepEqual(nextNodes, nodes.value);
    const shouldUpdateEdges = !isDeepEqual(sanitizedEdges, edges.value);

    if (shouldUpdateNodes || shouldUpdateEdges) {
        if (!isNodeDragInProgress.value) {
            pushHistorySnapshot();
        }

        if (shouldUpdateNodes) {
            nodes.value = safeClone(nextNodes);
        }

        if (shouldUpdateEdges) {
            edges.value = safeClone(sanitizedEdges);
        }
    }

    if (selectedNode.value) {
        const latestNode = nodes.value.find(
            (n) => n.id === selectedNode.value.id,
        );
        selectedNode.value = latestNode ? safeClone(latestNode) : null;
    }

    if (selectedEdge.value) {
        const latestEdge = edges.value.find(
            (e) => e.id === selectedEdge.value.id,
        );
        selectedEdge.value = latestEdge
            ? normalizeEdge(safeClone(latestEdge))
            : null;
        syncEdgeDraftFromSelection();
    }

    applyEdgeVisualStyles();
}

function handleNodesChange(event) {
    const nextNodes = Array.isArray(event.detail) ? event.detail : [];
    applyGraphStateSnapshot(nextNodes, edges.value);
}

function handleEdgesChange(event) {
    const incomingEdges = Array.isArray(event.detail) ? event.detail : [];
    applyGraphStateSnapshot(nodes.value, incomingEdges);
}

function handleGraphStateChange(event) {
    const detail = event?.detail || {};
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

    const choiceNodes = byY.filter((n) => n.type === "choice");
    const endNodes = byY.filter((n) => n.type === "end");
    const regularNodes = byY.filter(
        (n) => n.type !== "choice" && n.type !== "end",
    );

    const centered = [...regularNodes];
    const centerIndex = Math.floor(centered.length / 2);
    centered.splice(centerIndex, 0, ...choiceNodes);

    return [...centered, ...endNodes];
}

function getChoiceHandleIndex(sourceHandle) {
    if (!sourceHandle) return null;
    const match = /^choice-(\d+)$/.exec(sourceHandle);
    if (!match) return null;
    return Number(match[1]);
}

function computeIncomingDesiredY(node, incomingEdges, assignedYById, nodeMap) {
    if (!incomingEdges || incomingEdges.length === 0) return null;

    const fanOutSpread = 90;
    let total = 0;
    let count = 0;

    incomingEdges.forEach((edge) => {
        const sourceY = assignedYById.get(edge.source);
        if (sourceY == null) return;

        const sourceNode = nodeMap.get(edge.source);
        const handleIndex = getChoiceHandleIndex(edge.sourceHandle);

        if (handleIndex != null && sourceNode?.type === "choice") {
            const choiceCount = Array.isArray(sourceNode.data?.choices)
                ? sourceNode.data.choices.length
                : 0;
            const center = (Math.max(choiceCount, 1) - 1) / 2;
            const offset = (handleIndex - center) * fanOutSpread;
            total += sourceY + offset;
        } else {
            total += sourceY;
        }

        count += 1;
    });

    if (count === 0) return null;
    return total / count;
}

function resolveLayerYCollisions(orderedNodes, desiredYById, startY, minGap) {
    const result = new Map();
    if (orderedNodes.length === 0) return result;

    let cursor = startY;
    orderedNodes.forEach((node) => {
        const desiredY = desiredYById.get(node.id) ?? cursor;
        const y = Math.max(desiredY, cursor);
        result.set(node.id, y);
        cursor = y + minGap;
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

function handleAutoLayout() {
    if (nodes.value.length === 0) return;

    const rankSep = 360;
    const nodeSep = 185;
    const startX = 140;
    const startY = 110;

    const layerMap = buildLayers(
        nodes.value,
        edges.value,
        storyMeta.value.entryNodeId,
    );
    const nodeMap = new Map(nodes.value.map((n) => [n.id, n]));
    const incomingByTarget = new Map();

    edges.value.forEach((edge) => {
        if (!incomingByTarget.has(edge.target))
            incomingByTarget.set(edge.target, []);
        incomingByTarget.get(edge.target).push(edge);
    });

    const grouped = new Map();
    nodes.value.forEach((node) => {
        const l = layerMap.get(node.id) ?? 0;
        if (!grouped.has(l)) grouped.set(l, []);
        grouped.get(l).push(node);
    });

    const layers = [...grouped.keys()].sort((a, b) => a - b);
    const assignedYById = new Map();

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

            const incomingDesiredY = computeIncomingDesiredY(
                node,
                incomingEdges,
                assignedYById,
                nodeMap,
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
        });
    });

    const nextNodes = nodes.value.map((node) => {
        const l = layerMap.get(node.id) ?? 0;
        const y = assignedYById.get(node.id) ?? startY;
        return {
            ...node,
            position: { x: startX + l * rankSep, y },
        };
    });

    if (!isDeepEqual(nextNodes, nodes.value)) {
        pushHistorySnapshot();
        nodes.value = safeClone(nextNodes);
    }
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
        authState.value = {
            loading: false,
            authenticated: !!data?.authenticated,
            user: data?.user || null,
        };
    } catch {
        authState.value = {
            loading: false,
            authenticated: false,
            user: null,
        };
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

function handleOpenEmptyEditor() {
    selectedScriptEntry.value = null;
    currentView.value = "editor";
}

async function handleOpenScriptFromLibrary(scriptEntry) {
    selectedScriptEntry.value = safeClone(scriptEntry || null);

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
            const importedNodes = safeClone(imported.nodes || []);
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
            variables.value = safeClone(imported.variables || []);
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

    if (scriptEntry?.storyId) {
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
    window.addEventListener("nodes-change", handleNodesChange);
    window.addEventListener("edges-change", handleEdgesChange);
    window.addEventListener("graph-state-change", handleGraphStateChange);
    window.addEventListener("node-drag-start", handleNodeDragStart);
    window.addEventListener("node-drag-stop", handleNodeDragStop);
    window.addEventListener("keydown", handleGlobalKeyDown);

    setupAutoSave();
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
    window.removeEventListener("node-click", handleNodeClick);
    window.removeEventListener("edge-click", handleEdgeClick);
    window.removeEventListener("nodes-change", handleNodesChange);
    window.removeEventListener("edges-change", handleEdgesChange);
    window.removeEventListener("graph-state-change", handleGraphStateChange);
    window.removeEventListener("node-drag-start", handleNodeDragStart);
    window.removeEventListener("node-drag-stop", handleNodeDragStop);
    window.removeEventListener("keydown", handleGlobalKeyDown);
});

watch(
    () => [nodes.value, edges.value, storyMeta.value.entryNodeId],
    () => scheduleRealtimeValidation(),
    { deep: true, immediate: true },
);

watch(
    () => storyMeta.value.storyId,
    () => setupAutoSave(),
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
            const importedNodes = safeClone(imported.nodes || []);
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
            variables.value = safeClone(imported.variables || []);
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
            "- 点击“自动排布”：按层级整理节点\n" +
            "- 点击“连线样式”：直线/曲线切换\n" +
            "- 点击“焦点模式”：弱化非相关连线\n" +
            "- 点击边：编辑边的优先级和条件\n" +
            "- Delete键：删除选中的节点或边",
    );
}

function handleNodeUpdate(updatedNode) {
    if (!updatedNode) return;

    const prevId = selectedNode.value?.id;
    let index = -1;
    if (prevId) index = nodes.value.findIndex((n) => n.id === prevId);
    if (index === -1)
        index = nodes.value.findIndex((n) => n.id === updatedNode.id);
    if (index === -1) return;

    const oldId = nodes.value[index].id;
    const nextNode = safeClone(updatedNode);
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
    if (storyMeta.value.entryNodeId === nodeId) return;
    pushHistorySnapshot();
    storyMeta.value.entryNodeId = nodeId;
}

function scheduleGlobalConfigSyncFromEditor() {
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

.condition-editor {
    display: flex;
    flex-direction: column;
    gap: 10px;
}

.condition-empty {
    font-size: 12px;
    color: #64748b;
    background: rgba(148, 163, 184, 0.12);
    border: 1px dashed rgba(148, 163, 184, 0.35);
    border-radius: 8px;
    padding: 8px 10px;
}

.condition-term-card {
    background: rgba(255, 255, 255, 0.42);
    border: 1px solid rgba(148, 163, 184, 0.25);
    border-radius: 10px;
    padding: 10px;
    display: flex;
    flex-direction: column;
    gap: 8px;
}

.condition-term-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-size: 12px;
    font-weight: 700;
    color: #334155;
}

.condition-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 8px;
}

.condition-actions {
    display: flex;
    justify-content: flex-start;
}

.mini-btn {
    border: none;
    border-radius: 8px;
    padding: 6px 10px;
    font-size: 12px;
    font-weight: 700;
    cursor: pointer;
    background: rgba(15, 23, 42, 0.08);
    color: #1e293b;
}

.mini-btn.primary {
    background: rgba(59, 130, 246, 0.2);
    color: #1d4ed8;
}

.mini-btn.danger {
    background: rgba(239, 68, 68, 0.14);
    color: #b91c1c;
}

.condition-error {
    margin: 0;
    font-size: 12px;
    color: #b91c1c;
    background: rgba(254, 226, 226, 0.9);
    border-radius: 8px;
    padding: 6px 8px;
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
