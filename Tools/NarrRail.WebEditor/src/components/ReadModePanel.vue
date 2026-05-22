<template>
    <div class="preview-mode-root" :class="{ 'is-dark-mode': isDarkMode }">
        <div class="preview-mode-body">
            <div class="preview-header">
                <div class="preview-title-wrap">
                    <span class="material-symbols-outlined">smart_toy</span>
                    <h3>预览模式</h3>
                </div>
                <div class="preview-actions">
                    <button class="action-btn" @click="handleRestart">
                        重新开始
                    </button>
                </div>
            </div>

            <div v-if="state.error" class="preview-error">
                {{ state.error }}
            </div>

            <div class="preview-main">
                <div
                    ref="timelineRef"
                    class="preview-timeline"
                    @click="handleTimelineAdvance"
                >
                    <div class="timeline-title">阅读预览（点击此区域推进）</div>
                    <div class="timeline-content">
                        <div v-if="state.timeline.length === 0" class="hint">
                            暂无执行内容
                        </div>
                        <div v-else class="timeline-list">
                            <div
                                v-for="(item, index) in state.timeline"
                                :key="`${item.nodeId}-${item.kind}-${index}`"
                                class="timeline-item"
                            >
                                <template
                                    v-if="
                                        item.kind === 'dialogue' ||
                                        item.kind === 'multidialogue'
                                    "
                                >
                                    <div class="line-row">
                                        <span class="speaker">{{
                                            item.speaker
                                        }}</span>
                                        <span class="sep">：</span>
                                        <span class="text">{{
                                            item.text
                                        }}</span>
                                    </div>
                                </template>

                                <template v-else-if="item.kind === 'choice'">
                                    <div class="line-row choice-row">
                                        <span class="speaker"></span>
                                        <span class="sep"></span>
                                        <span class="text choice-content">
                                            <span class="choice-arrow">→</span>
                                            <span class="choice-text">{{
                                                item.text
                                            }}</span>
                                        </span>
                                    </div>
                                </template>

                                <template
                                    v-else-if="item.kind === 'setvariable'"
                                >
                                    <div class="event-row">
                                        <span class="tag tag-variable"
                                            >变量</span
                                        >
                                        <span class="sep">：</span>
                                        <span class="text">{{
                                            item.text
                                        }}</span>
                                    </div>
                                </template>

                                <template v-else-if="item.kind === 'emitevent'">
                                    <div class="event-row">
                                        <span class="tag tag-event">事件</span>
                                        <span class="sep">：</span>
                                        <span class="text">{{
                                            item.text
                                        }}</span>
                                    </div>
                                </template>

                                <template v-else-if="item.kind === 'jump'">
                                    <div class="event-row">
                                        <span class="tag tag-jump">跳转</span>
                                        <span class="sep">：</span>
                                        <span class="text">{{
                                            item.text
                                        }}</span>
                                    </div>
                                </template>

                                <template v-else-if="item.kind === 'end'">
                                    <div class="event-row">
                                        <span class="tag tag-end">结束</span>
                                        <span class="sep">：</span>
                                        <span class="text">{{
                                            item.text
                                        }}</span>
                                    </div>
                                </template>
                            </div>
                        </div>

                        <div
                            v-if="state.status === 'await-choice'"
                            class="timeline-choice-box"
                            @click.stop
                        >
                            <p class="hint">请选择一个选项继续：</p>
                            <div class="choice-inline-list">
                                <button
                                    v-for="choice in state.pendingChoices"
                                    :key="choice.handle"
                                    class="choice-inline-btn"
                                    @click.stop="handleChoose(choice.handle)"
                                >
                                    {{ choice.text || "（空选项）" }}
                                </button>
                            </div>
                        </div>

                        <div
                            v-else-if="state.status === 'ended'"
                            class="timeline-end-hint"
                        >
                            已执行到 End 节点或无可用后续连线。
                        </div>
                    </div>
                </div>

                <div class="preview-vars">
                    <div class="timeline-title">会话变量快照</div>
                    <div v-if="state.varList.length === 0" class="hint">
                        暂无变量
                    </div>
                    <div v-else class="vars-list">
                        <div
                            v-for="v in state.varList"
                            :key="v.name"
                            class="var-row"
                        >
                            <span class="var-name">{{ v.name }}</span>
                            <span class="var-value">{{
                                formatVarValue(v.value)
                            }}</span>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>

<script setup>
import { nextTick, reactive, ref, watch } from "vue";

const props = defineProps({
    nodes: { type: Array, default: () => [] },
    edges: { type: Array, default: () => [] },
    variables: { type: Array, default: () => [] },
    entryNodeId: { type: String, default: "" },
    isDarkMode: { type: Boolean, default: false },
});

const timelineRef = ref(null);

const state = reactive({
    status: "idle", // idle | running | await-choice | ended
    currentNodeId: "",
    pendingChoices: [],
    timeline: [],
    vars: {},
    varList: [],
    error: "",
    multiLineCursor: {
        nodeId: "",
        nextIndex: 0,
    },
});

function toNodeType(node) {
    return String(node?.type || "").toLowerCase();
}

function toSpeaker(rawSpeaker) {
    const speaker = String(rawSpeaker || "").trim();
    return speaker || "旁白";
}

function toText(rawText) {
    return String(rawText || "").trim();
}

function toNumberLike(value) {
    if (typeof value === "number") return value;
    const n = Number(value);
    return Number.isFinite(n) ? n : 0;
}

function normalizeVariableValue(variable) {
    const type = String(variable?.type || "");
    if (type === "Bool") return !!variable?.boolValue;
    if (type === "Int") return Math.trunc(toNumberLike(variable?.intValue));
    if (type === "Float") return toNumberLike(variable?.floatValue);
    if (type === "String") return String(variable?.stringValue || "");
    return variable?.stringValue ?? "";
}

function refreshVarList() {
    state.varList = Object.keys(state.vars)
        .sort((a, b) => a.localeCompare(b))
        .map((name) => ({ name, value: state.vars[name] }));
}

function initSessionVars() {
    const next = {};
    for (const variable of props.variables || []) {
        const name = String(variable?.name || "").trim();
        if (!name) continue;
        next[name] = normalizeVariableValue(variable);
    }
    state.vars = next;
    refreshVarList();
}

function getNodeById(nodeId) {
    return (props.nodes || []).find((node) => node?.id === nodeId) || null;
}

function getOutgoingEdges(sourceId) {
    return (props.edges || [])
        .filter((edge) => edge?.source === sourceId)
        .sort((a, b) => {
            const pa = Number(a?.data?.priority ?? 0);
            const pb = Number(b?.data?.priority ?? 0);
            if (pa !== pb) return pa - pb;
            return String(a?.id || "").localeCompare(String(b?.id || ""));
        });
}

function compareValues(left, operator, right) {
    switch (operator) {
        case "==":
            return left == right;
        case "!=":
            return left != right;
        case ">":
            return left > right;
        case ">=":
            return left >= right;
        case "<":
            return left < right;
        case "<=":
            return left <= right;
        default:
            return false;
    }
}

function coerceCompareValue(raw) {
    if (typeof raw === "boolean" || typeof raw === "number") return raw;
    const text = String(raw ?? "").trim();
    if (text === "true") return true;
    if (text === "false") return false;
    const n = Number(text);
    if (Number.isFinite(n) && text !== "") return n;
    return text;
}

function evaluateCondition(condition) {
    if (!condition || typeof condition !== "object") return true;

    const logic = String(condition.logic || "All");
    const terms = Array.isArray(condition.terms) ? condition.terms : [];
    if (terms.length === 0) return true;

    const checks = terms.map((term) => {
        const variableName = String(term?.variable?.name || "").trim();
        const operator = String(term?.operator || "==");
        const compareValue = coerceCompareValue(term?.compareValue);
        const left = state.vars[variableName];
        return compareValues(left, operator, compareValue);
    });

    if (logic === "Any") return checks.some(Boolean);
    if (logic === "Not") return !checks[0];
    return checks.every(Boolean);
}

function firstNextTarget(nodeId) {
    const outgoing = getOutgoingEdges(nodeId);
    const next = outgoing.find((edge) =>
        evaluateCondition(edge?.data?.condition),
    );
    return next?.target ? String(next.target) : "";
}

function getChoiceEdges(nodeId, handle) {
    return getOutgoingEdges(nodeId).filter(
        (edge) => String(edge?.sourceHandle || "") === String(handle || ""),
    );
}

function applySetVariable(node) {
    const name = String(node?.data?.variableName || "").trim();
    if (!name) return "变量名为空，已跳过";

    const op = String(node?.data?.operation || "Set");
    const rawValue = node?.data?.value;

    const current = state.vars[name];
    const value = coerceCompareValue(rawValue);

    let nextValue = current;
    if (op === "Set") nextValue = value;
    else if (op === "Add")
        nextValue = toNumberLike(current) + toNumberLike(value);
    else if (op === "Subtract")
        nextValue = toNumberLike(current) - toNumberLike(value);
    else if (op === "Multiply")
        nextValue = toNumberLike(current) * toNumberLike(value);
    else if (op === "Divide") {
        const div = toNumberLike(value);
        nextValue =
            div === 0 ? toNumberLike(current) : toNumberLike(current) / div;
    } else nextValue = value;

    state.vars[name] = nextValue;
    refreshVarList();
    return `${name} ${op} ${String(value)} => ${String(nextValue)}`;
}

function pushTimeline(item) {
    state.timeline.push(item);
}

function scrollTimelineToBottom() {
    const el = timelineRef.value;
    if (!el) return;

    nextTick(() => {
        el.scrollTop = el.scrollHeight;
    });
}

function setEnded(errorText = "") {
    state.status = "ended";
    state.currentNodeId = "";
    state.pendingChoices = [];
    state.error = errorText || "";
}

function handleBranchEnd() {
    setEnded();
}

function restartPreview() {
    state.timeline = [];
    state.pendingChoices = [];
    state.error = "";
    state.multiLineCursor = {
        nodeId: "",
        nextIndex: 0,
    };
    initSessionVars();

    const entryId = String(props.entryNodeId || "").trim();
    if (!entryId) {
        state.status = "idle";
        state.currentNodeId = "";
        state.error = "入口节点为空，请先设置 Entry Node。";
        return;
    }

    if (!getNodeById(entryId)) {
        state.status = "idle";
        state.currentNodeId = "";
        state.error = `入口节点不存在: ${entryId}`;
        return;
    }

    state.status = "running";
    state.currentNodeId = entryId;
}

function advanceUntilPause(maxSteps = 4000) {
    let steps = 0;

    while (state.status === "running" && steps < maxSteps) {
        steps += 1;

        const node = getNodeById(state.currentNodeId);
        if (!node) {
            setEnded(`节点不存在: ${state.currentNodeId}`);
            return;
        }

        const kind = toNodeType(node);

        if (kind === "dialogue") {
            const text = toText(node?.data?.textKey);
            if (text) {
                pushTimeline({
                    kind: "dialogue",
                    nodeId: node.id,
                    speaker: toSpeaker(node?.data?.speakerId),
                    text,
                });
            }
            const next = firstNextTarget(node.id);
            if (!next) {
                handleBranchEnd();
                return;
            }
            state.currentNodeId = next;
            return;
        }

        if (kind === "multidialogue") {
            const speaker = toSpeaker(node?.data?.speakerId);
            const lines = Array.isArray(node?.data?.lines)
                ? node.data.lines
                      .map((line) =>
                          toText(
                              typeof line === "string" ? line : line?.textKey,
                          ),
                      )
                      .filter((text) => text.length > 0)
                : [];

            if (lines.length === 0) {
                const next = firstNextTarget(node.id);
                if (!next) {
                    handleBranchEnd();
                    return;
                }
                state.currentNodeId = next;
                continue;
            }

            const isSameNode = state.multiLineCursor.nodeId === node.id;
            const currentLineIndex = isSameNode
                ? state.multiLineCursor.nextIndex
                : 0;

            if (currentLineIndex >= lines.length) {
                state.multiLineCursor = {
                    nodeId: "",
                    nextIndex: 0,
                };
                const next = firstNextTarget(node.id);
                if (!next) {
                    handleBranchEnd();
                    return;
                }
                state.currentNodeId = next;
                continue;
            }

            pushTimeline({
                kind: "multidialogue",
                nodeId: node.id,
                speaker,
                text: lines[currentLineIndex],
            });

            const nextLineIndex = currentLineIndex + 1;
            if (nextLineIndex < lines.length) {
                state.multiLineCursor = {
                    nodeId: node.id,
                    nextIndex: nextLineIndex,
                };
                return;
            }

            state.multiLineCursor = {
                nodeId: "",
                nextIndex: 0,
            };
            const next = firstNextTarget(node.id);
            if (!next) {
                handleBranchEnd();
                return;
            }
            state.currentNodeId = next;
            return;
        }

        if (kind === "choice") {
            const choices = Array.isArray(node?.data?.choices)
                ? node.data.choices
                : [];
            const pending = choices
                .map((choice, idx) => ({
                    handle: `choice-${idx}`,
                    text: toText(choice?.textKey),
                }))
                .filter((choice) => {
                    const edges = getChoiceEdges(node.id, choice.handle);
                    return edges.some((edge) =>
                        evaluateCondition(edge?.data?.condition),
                    );
                });

            if (pending.length <= 0) {
                const fallback = firstNextTarget(node.id);
                if (!fallback) {
                    setEnded(
                        "该 Choice 节点没有可用分支。\n请检查选项连线或条件。",
                    );
                    return;
                }
                state.currentNodeId = fallback;
                continue;
            }

            state.pendingChoices = pending;
            state.status = "await-choice";
            return;
        }

        if (kind === "setvariable") {
            const desc = applySetVariable(node);
            pushTimeline({
                kind: "setvariable",
                nodeId: node.id,
                text: desc,
            });
            const next = firstNextTarget(node.id);
            if (!next) {
                handleBranchEnd();
                return;
            }
            state.currentNodeId = next;
            continue;
        }

        if (kind === "emitevent") {
            const eventId = toText(node?.data?.eventId) || "(未命名事件)";
            pushTimeline({
                kind: "emitevent",
                nodeId: node.id,
                text: `${eventId}`,
            });
            const next = firstNextTarget(node.id);
            if (!next) {
                handleBranchEnd();
                return;
            }
            state.currentNodeId = next;
            continue;
        }

        if (kind === "jump") {
            const target = String(node?.data?.targetNodeId || "").trim();
            if (!target) {
                setEnded(`Jump 节点 ${node.id} 未配置 targetNodeId`);
                return;
            }
            pushTimeline({
                kind: "jump",
                nodeId: node.id,
                text: `${node.id} -> ${target}`,
            });
            state.currentNodeId = target;
            continue;
        }

        if (kind === "end") {
            pushTimeline({
                kind: "end",
                nodeId: node.id,
                text: `${node.id}`,
            });
            handleBranchEnd();
            return;
        }

        const next = firstNextTarget(node.id);
        if (!next) {
            handleBranchEnd();
            return;
        }
        state.currentNodeId = next;
    }

    if (steps >= maxSteps && state.status === "running") {
        setEnded("执行超过最大步数，疑似存在循环。已停止预览。");
    }
}

function handleRestart() {
    restartPreview();
}

function handleTimelineAdvance() {
    if (state.status === "await-choice") return;

    if (state.status === "idle") {
        restartPreview();
        if (state.status === "running") {
            advanceUntilPause();
            scrollTimelineToBottom();
        }
        return;
    }

    if (state.status === "running") {
        advanceUntilPause();
        scrollTimelineToBottom();
        return;
    }

    if (state.status === "ended") {
        return;
    }
}

function handleChoose(handle) {
    if (state.status !== "await-choice") return;

    const currentNode = getNodeById(state.currentNodeId);
    if (!currentNode) {
        setEnded("当前 Choice 节点不存在");
        return;
    }

    const idxMatch = /^choice-(\d+)$/.exec(String(handle || ""));
    const choiceIndex = idxMatch ? Number(idxMatch[1]) : -1;
    const choiceText =
        choiceIndex >= 0
            ? toText(currentNode?.data?.choices?.[choiceIndex]?.textKey)
            : "";

    const targetEdge = getChoiceEdges(currentNode.id, handle).find((edge) =>
        evaluateCondition(edge?.data?.condition),
    );

    if (!targetEdge?.target) {
        setEnded("所选分支没有可用目标节点");
        return;
    }

    pushTimeline({
        kind: "choice",
        nodeId: currentNode.id,
        text: choiceText || `选项 ${choiceIndex + 1}`,
    });

    state.pendingChoices = [];
    state.status = "running";
    state.currentNodeId = String(targetEdge.target);
    advanceUntilPause();
    scrollTimelineToBottom();
}

function formatVarValue(value) {
    if (typeof value === "string") return `"${value}"`;
    if (typeof value === "boolean") return value ? "true" : "false";
    return String(value);
}

watch(
    () => [props.nodes, props.edges, props.variables, props.entryNodeId],
    () => {
        restartPreview();
        if (state.status === "running") {
            advanceUntilPause();
            scrollTimelineToBottom();
        }
    },
    { deep: true, immediate: true },
);
</script>

<style scoped>
.preview-mode-root {
    position: absolute;
    inset: 104px 24px 96px 24px;
    z-index: 15;
}

.preview-mode-body {
    width: 100%;
    height: 100%;
    padding: 4px 4px;
    display: flex;
    flex-direction: column;
    gap: 6px;
    overflow: hidden;
    background: transparent;
    border: none;
    box-shadow: none;
    backdrop-filter: none;
    -webkit-backdrop-filter: none;
}

.preview-header {
    display: none;
}

.preview-title-wrap {
    display: none;
}

.preview-actions {
    display: none;
}

.action-btn {
    display: none;
}

.action-btn:disabled {
    opacity: 0.5;
    cursor: not-allowed;
}

.preview-main {
    min-height: 0;
    flex: 1;
    display: flex;
    justify-content: center;
    align-items: center;
}

.preview-timeline {
    width: min(780px, 92%);
    max-height: min(56vh, 500px);
    margin: 0 auto;
    border: none;
    border-radius: 0;
    padding: 4px 2px;
    overflow: auto;
    background: transparent;
    scrollbar-width: thin;
    scrollbar-color: rgba(148, 163, 184, 0.28) transparent;
}

.preview-timeline::-webkit-scrollbar {
    width: 4px;
}

.preview-timeline::-webkit-scrollbar-track {
    background: transparent;
}

.preview-timeline::-webkit-scrollbar-thumb {
    background: rgba(148, 163, 184, 0.28);
    border-radius: 4px;
}

.preview-timeline::-webkit-scrollbar-thumb:hover {
    background: rgba(148, 163, 184, 0.46);
}

.timeline-content {
    min-height: 100%;
    display: flex;
    flex-direction: column;
    justify-content: center;
    padding: 6px 0;
    position: relative;
}

.timeline-content > .hint,
.timeline-content > .timeline-list,
.timeline-content > .timeline-choice-box,
.timeline-content > .timeline-end-hint {
    width: max-content;
    max-width: min(920px, 96%);
    margin-left: auto;
    margin-right: auto;
}

.preview-vars {
    display: none;
}

.timeline-title {
    display: none;
}

.hint {
    color: #94a3b8;
    line-height: 1.7;
    font-size: 14px;
}

.choice-inline-list {
    display: flex;
    flex-direction: column;
    gap: 8px;
}

.choice-inline-btn {
    text-align: left;
    border-radius: 10px;
    border: 1px solid rgba(59, 130, 246, 0.35);
    background: rgba(59, 130, 246, 0.08);
    padding: 8px 10px;
    cursor: pointer;
}

.choice-inline-btn:hover {
    background: rgba(59, 130, 246, 0.14);
    border-color: rgba(59, 130, 246, 0.48);
}

.timeline-list {
    display: flex;
    flex-direction: column;
    gap: 6px;
}

.timeline-choice-box {
    margin-top: 8px;
    border-top: 1px dashed rgba(148, 163, 184, 0.28);
    padding-top: 8px;
}

.timeline-end-hint {
    margin-top: 10px;
    color: #94a3b8;
    font-size: 12px;
}

.timeline-item {
    border: none;
    border-radius: 0;
    padding: 1px 2px;
    width: max-content;
    max-width: 100%;
    line-height: 1.65;
    font-size: 15px;
    color: #111827;
}

.line-row,
.event-row {
    display: grid;
    grid-template-columns: 9ch auto minmax(0, 1fr);
    align-items: start;
    column-gap: 2px;
}

.var-row {
    display: flex;
    align-items: flex-start;
    gap: 4px;
}

.speaker {
    display: inline-block;
    width: 9ch;
    max-width: 9ch;
    text-align: right;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
    font-weight: 700;
    font-size: 0.95em;
    color: #1e3a8a;
}

.choice-arrow {
    display: inline-block;
    position: relative;
    left: -0.72em;
    align-self: center;
    margin-right: 0;
    font-size: 0.78em;
    font-weight: 700;
    color: rgba(110, 162, 216, 0.55);
}

.sep {
    color: #64748b;
}

.text {
    white-space: pre-wrap;
    word-break: break-word;
    color: #111827;
}

.preview-mode-root.is-dark-mode .timeline-item {
    color: #b8cde0 !important;
}

.preview-mode-root.is-dark-mode .speaker {
    color: #a8c7e6 !important;
    font-weight: 800;
}

.preview-mode-root.is-dark-mode .choice-arrow {
    color: rgba(141, 183, 226, 0.62) !important;
}

.preview-mode-root.is-dark-mode .sep {
    color: #8ea6bc !important;
}

.preview-mode-root.is-dark-mode .text {
    color: #b8cde0 !important;
}

:global(body.dark-theme) .preview-mode-root .timeline-item,
:global(body[data-theme="dark"]) .preview-mode-root .timeline-item {
    color: #b8cde0 !important;
}

:global(body.dark-theme) .preview-mode-root .speaker,
:global(body[data-theme="dark"]) .preview-mode-root .speaker {
    color: #a8c7e6 !important;
    font-weight: 800;
}

:global(body.dark-theme) .preview-mode-root .choice-arrow,
:global(body[data-theme="dark"]) .preview-mode-root .choice-arrow {
    color: rgba(141, 183, 226, 0.62) !important;
}

:global(body.dark-theme) .preview-mode-root .sep,
:global(body[data-theme="dark"]) .preview-mode-root .sep {
    color: #8ea6bc !important;
}

:global(body.dark-theme) .preview-mode-root .text,
:global(body[data-theme="dark"]) .preview-mode-root .text {
    color: #b8cde0 !important;
}

.tag {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    justify-self: end;
    align-self: start;
    min-width: 2.4em;
    max-width: 11ch;
    white-space: nowrap;
    font-size: 10px;
    font-weight: 800;
    letter-spacing: 0.04em;
    line-height: 1;
    border-radius: 6px;
    padding: 3px 0.45em;
    color: #334155;
    border: 1px solid rgba(148, 163, 184, 0.3);
    background: rgba(148, 163, 184, 0.12);
}

.tag-choice {
    color: #6ea2d8;
    border: none;
    background: transparent;
    padding: 0;
    min-width: 2.2em;
}

.tag-variable {
    color: #6d28d9;
    border-color: rgba(124, 58, 237, 0.32);
    background: rgba(124, 58, 237, 0.14);
}

.tag-event {
    color: #9a3412;
    border-color: rgba(234, 88, 12, 0.3);
    background: rgba(234, 88, 12, 0.14);
}

.tag-jump {
    color: #0f766e;
    border-color: rgba(13, 148, 136, 0.3);
    background: rgba(13, 148, 136, 0.14);
}

.tag-end {
    color: #9f1239;
    border-color: rgba(190, 24, 93, 0.3);
    background: rgba(190, 24, 93, 0.12);
}

.event-row .sep {
    align-self: start;
}

.event-row .text {
    align-self: start;
}

.choice-content {
    grid-column: 3;
    margin-left: 0;
    display: inline-flex;
    align-items: center;
    gap: 0.28em;
    min-width: 0;
    font-size: 0.94em;
    font-weight: 650;
    letter-spacing: 0.005em;
}

.choice-text {
    min-width: 0;
    flex: 1;
}

.vars-list {
    display: flex;
    flex-direction: column;
    gap: 6px;
}

.var-name {
    font-weight: 700;
}

.var-value {
    margin-left: auto;
    color: #334155;
}

.preview-error {
    color: #b91c1c;
    background: rgba(254, 226, 226, 0.75);
    border: 1px solid rgba(248, 113, 113, 0.5);
    border-radius: 8px;
    padding: 6px 10px;
}
</style>
