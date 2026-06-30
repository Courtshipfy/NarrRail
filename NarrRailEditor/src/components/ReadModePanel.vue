<template>
    <div
        class="preview-mode-root"
        :class="{ 'is-dark-mode': isDarkMode }"
        @click="handleTimelineAdvance"
    >
        <div class="preview-mode-body">
            <div class="preview-header">
                <div class="preview-title-wrap">
                    <IconGlyph name="smart_toy" />
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
                <div ref="timelineRef" class="preview-timeline">
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
                                :class="{
                                    'timeline-item--chapter':
                                        item.kind === 'railnote' ||
                                        item.kind === 'railbranch' ||
                                        item.kind === 'railend',
                                }"
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
                                    <div
                                        class="line-row meta-row meta-row--choice"
                                    >
                                        <span class="speaker"></span>
                                        <span
                                            class="sep meta-arrow meta-arrow--choice"
                                        ></span>
                                        <span class="text meta-content">
                                            <span
                                                class="meta-type meta-type--choice"
                                                >[选择]</span
                                            >
                                            <span class="meta-text">{{
                                                item.text
                                            }}</span>
                                        </span>
                                    </div>
                                </template>

                                <template v-else-if="item.kind === 'condition'">
                                    <div
                                        class="line-row meta-row meta-row--condition"
                                    >
                                        <span class="speaker"></span>
                                        <span
                                            class="sep meta-arrow meta-arrow--condition"
                                        ></span>
                                        <span class="text meta-content">
                                            <span
                                                class="meta-type meta-type--condition"
                                                >[条件]</span
                                            >
                                            <span class="meta-text">{{
                                                item.text
                                            }}</span>
                                        </span>
                                    </div>
                                </template>

                                <template
                                    v-else-if="item.kind === 'setvariable'"
                                >
                                    <div
                                        class="line-row meta-row meta-row--setvariable"
                                    >
                                        <span class="speaker"></span>
                                        <span
                                            class="sep meta-arrow meta-arrow--setvariable"
                                        ></span>
                                        <span class="text meta-content">
                                            <span
                                                class="meta-type meta-type--setvariable"
                                                >[变量]</span
                                            >
                                            <span class="meta-text">{{
                                                item.text
                                            }}</span>
                                        </span>
                                    </div>
                                </template>

                                <template v-else-if="item.kind === 'emitevent'">
                                    <div
                                        class="line-row meta-row meta-row--emitevent"
                                    >
                                        <span class="speaker"></span>
                                        <span
                                            class="sep meta-arrow meta-arrow--emitevent"
                                        ></span>
                                        <span class="text meta-content">
                                            <span
                                                class="meta-type meta-type--emitevent"
                                                >[事件]</span
                                            >
                                            <span class="meta-text">
                                                {{ item.text }}
                                                <span
                                                    v-if="getEventParamRows(item).length"
                                                    class="meta-param-list"
                                                >
                                                    <span
                                                        v-for="param in getEventParamRows(item)"
                                                        :key="param.key"
                                                        class="meta-param-row"
                                                    >
                                                        <span class="meta-param-key">{{ param.key }}</span>
                                                        <span class="meta-param-value">{{ param.value }}</span>
                                                    </span>
                                                </span>
                                            </span>
                                        </span>
                                    </div>
                                </template>

                                <template v-else-if="item.kind === 'jump'">
                                    <div
                                        class="line-row meta-row meta-row--jump"
                                    >
                                        <span class="speaker"></span>
                                        <span
                                            class="sep meta-arrow meta-arrow--jump"
                                        ></span>
                                        <span class="text meta-content">
                                            <span
                                                class="meta-type meta-type--jump"
                                                >[跳转]</span
                                            >
                                            <span class="meta-text">{{
                                                item.text
                                            }}</span>
                                        </span>
                                    </div>
                                </template>

                                <template v-else-if="item.kind === 'end'">
                                    <div
                                        class="line-row meta-row meta-row--end"
                                    >
                                        <span class="speaker"></span>
                                        <span
                                            class="sep meta-arrow meta-arrow--end"
                                        ></span>
                                        <span class="text meta-content">
                                            <span
                                                class="meta-type meta-type--end"
                                                >[结束]</span
                                            >
                                            <span class="meta-text">{{
                                                item.text
                                            }}</span>
                                        </span>
                                    </div>
                                </template>

                                <template
                                    v-else-if="
                                        item.kind === 'railnote' ||
                                        item.kind === 'railbranch' ||
                                        item.kind === 'railend'
                                    "
                                >
                                    <div
                                        class="chapter-marker"
                                        :class="{
                                            'chapter-marker--branch':
                                                item.kind === 'railbranch',
                                            'chapter-marker--end':
                                                item.kind === 'railend',
                                        }"
                                    >
                                        <div class="chapter-line"></div>
                                        <div class="chapter-content">
                                            <div class="chapter-title">
                                                {{
                                                    item.text ||
                                                    (item.kind === "railbranch"
                                                        ? "总纲分支"
                                                        : item.kind ===
                                                            "railend"
                                                          ? "总纲结束"
                                                          : "章节标记")
                                                }}
                                            </div>
                                            <div
                                                v-if="item.summary"
                                                class="chapter-summary"
                                            >
                                                {{ item.summary }}
                                            </div>
                                        </div>
                                        <div class="chapter-line"></div>
                                    </div>
                                </template>
                            </div>
                        </div>

                        <div
                            v-if="state.status === 'await-choice'"
                            class="timeline-choice-box"
                            @click.stop
                        >
                            <div
                                v-if="state.choiceTimer.enabled"
                                class="choice-countdown"
                                aria-live="polite"
                            >
                                <span>倒计时</span>
                                <strong>{{
                                    formatCountdown(
                                        state.choiceTimer.remainingSeconds,
                                    )
                                }}</strong>
                            </div>
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
import { nextTick, onUnmounted, reactive, ref, watch } from "vue";
import { createRailPreviewRunner } from "../utils/rail-preview-runner.js";
import { normalizeVariableValue } from "../utils/story-preview-runner.js";

const props = defineProps({
    previewMode: { type: String, default: "story" },
    nodes: { type: Array, default: () => [] },
    edges: { type: Array, default: () => [] },
    variables: { type: Array, default: () => [] },
    entryNodeId: { type: String, default: "" },
    rail: { type: Object, default: null },
    resolveStoryById: { type: Function, default: null },
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
    exhaustiveSelectedByNode: {}, // { [nodeId]: { [choiceHandle]: true } }
    exhaustiveReturnStack: [], // string[]
    choiceTimer: {
        enabled: false,
        remainingSeconds: 0,
        durationSeconds: 0,
        timeoutChoiceTextKey: "超时",
    },
});

let choiceTimerId = null;
let railRunner = null;

function isRailPreviewMode() {
    return props.previewMode === "rail";
}

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

function getEventParamRows(item) {
    return formatEventParamRows(item?.payload?.params);
}

function formatEventParamRows(params) {
    if (!params || typeof params !== "object") return [];

    return Object.entries(params)
        .filter(([key]) => String(key || "").trim())
        .map(([key, value]) => ({
            key: String(key),
            value: formatEventParamValue(value),
        }));
}

function formatEventParamValue(value) {
    if (value === null) return "null";
    if (value === undefined) return "";
    if (typeof value === "string") return value.trim() || "\"\"";
    if (typeof value === "number" || typeof value === "boolean") return String(value);

    try {
        return JSON.stringify(value);
    } catch (error) {
        return String(value);
    }
}

function toNumberLike(value) {
    if (typeof value === "number") return value;
    const n = Number(value);
    return Number.isFinite(n) ? n : 0;
}

function refreshVarList() {
    state.varList = Object.keys(state.vars)
        .sort((a, b) => a.localeCompare(b))
        .map((name) => ({ name, value: state.vars[name] }));
}

function syncRunnerState(runnerState) {
    clearChoiceTimer();
    state.status = runnerState?.status || "idle";
    state.currentNodeId =
        runnerState?.currentNodeId || runnerState?.currentRailNodeId || "";
    state.pendingChoices = Array.isArray(runnerState?.pendingChoices)
        ? [...runnerState.pendingChoices]
        : [];
    state.timeline = Array.isArray(runnerState?.timeline)
        ? [...runnerState.timeline]
        : [];
    state.vars = runnerState?.vars || {};
    state.error = runnerState?.error || "";
    state.choiceTimer =
        runnerState?.choiceTimer && runnerState.choiceTimer.enabled
            ? { ...runnerState.choiceTimer }
            : {
                  enabled: false,
                  remainingSeconds: 0,
                  durationSeconds: 0,
                  timeoutChoiceTextKey: "超时",
              };
    refreshVarList();

    if (state.status === "await-choice" && state.choiceTimer.enabled) {
        startExternalChoiceTimer(state.choiceTimer);
    }
}

function restartRailPreview() {
    clearChoiceTimer();
    railRunner = createRailPreviewRunner(
        props.rail || {
            meta: { entryNodeId: props.entryNodeId },
            nodes: props.nodes,
            edges: props.edges,
        },
        {
            variables: props.variables,
            resolveStoryById:
                typeof props.resolveStoryById === "function"
                    ? props.resolveStoryById
                    : () => null,
        },
    );
    syncRunnerState(railRunner.state);

    if (!String(props.entryNodeId || props.rail?.meta?.entryNodeId || "").trim()) {
        state.status = "idle";
        state.error = "入口节点为空，请先设置 Entry Node。";
    }
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

function getConditionBranches(condition) {
    if (Array.isArray(condition?.branches)) {
        return condition.branches;
    }
    if (Array.isArray(condition?.terms)) {
        return [
            {
                label: "Condition 1",
                logic: condition?.logic || "All",
                terms: condition.terms,
            },
        ];
    }
    return [];
}

function firstNextTarget(nodeId) {
    const outgoing = getOutgoingEdges(nodeId);
    const next = outgoing[0];
    return next?.target ? String(next.target) : "";
}

function getChoiceEdges(nodeId, handle) {
    return getOutgoingEdges(nodeId).filter(
        (edge) => String(edge?.sourceHandle || "") === String(handle || ""),
    );
}

function isExhaustiveChoiceNode(node) {
    return String(node?.data?.choiceMode || "") === "ExhaustiveUntilComplete";
}

function isChoiceHandleSelected(nodeId, handle) {
    const selectedMap = state.exhaustiveSelectedByNode[String(nodeId)];
    return !!selectedMap?.[String(handle)];
}

function markChoiceHandleSelected(nodeId, handle) {
    const key = String(nodeId);
    const selectedMap = state.exhaustiveSelectedByNode[key] || {};
    selectedMap[String(handle)] = true;
    state.exhaustiveSelectedByNode[key] = selectedMap;
}

function resolveChoiceCompleteTarget(nodeId) {
    const edge = getChoiceEdges(nodeId, "choice-complete")[0];
    return edge?.target ? String(edge.target) : "";
}

function normalizeChoiceTimer(timer) {
    const durationSeconds = Number(timer?.durationSeconds);
    return {
        enabled: Boolean(timer?.enabled),
        durationSeconds:
            Number.isFinite(durationSeconds) && durationSeconds > 0
                ? durationSeconds
                : 8,
        timeoutChoiceTextKey: String(timer?.timeoutChoiceTextKey || "超时"),
    };
}

function resetChoiceTimerState() {
    state.choiceTimer = {
        enabled: false,
        remainingSeconds: 0,
        durationSeconds: 0,
        timeoutChoiceTextKey: "超时",
    };
}

function clearChoiceTimer() {
    if (choiceTimerId) {
        clearInterval(choiceTimerId);
        choiceTimerId = null;
    }
    resetChoiceTimerState();
}

function startChoiceTimerForNode(node, pendingChoices) {
    clearChoiceTimer();
    const timer = normalizeChoiceTimer(node?.data?.choiceTimer);
    if (!timer.enabled || pendingChoices.length <= 0) return;

    state.choiceTimer = {
        ...timer,
        remainingSeconds: timer.durationSeconds,
    };

    const startedAt = Date.now();
    choiceTimerId = setInterval(() => {
        if (state.status !== "await-choice" || state.currentNodeId !== node.id) {
            clearChoiceTimer();
            return;
        }

        const elapsedSeconds = (Date.now() - startedAt) / 1000;
        const remaining = Math.max(0, timer.durationSeconds - elapsedSeconds);
        state.choiceTimer.remainingSeconds = remaining;

        if (remaining > 0) return;

        clearChoiceTimer();
        handleChoiceTimeout(node.id, timer);
    }, 100);
}

function startExternalChoiceTimer(timer) {
    clearChoiceTimer();
    const durationSeconds = Number(timer?.durationSeconds);
    if (
        !timer?.enabled ||
        !Number.isFinite(durationSeconds) ||
        durationSeconds <= 0
    ) {
        resetChoiceTimerState();
        return;
    }

    state.choiceTimer = {
        enabled: true,
        durationSeconds,
        remainingSeconds: durationSeconds,
        timeoutChoiceTextKey: String(timer?.timeoutChoiceTextKey || "超时"),
    };

    const startedAt = Date.now();
    choiceTimerId = setInterval(() => {
        if (state.status !== "await-choice") {
            clearChoiceTimer();
            return;
        }

        const elapsedSeconds = (Date.now() - startedAt) / 1000;
        const remaining = Math.max(0, durationSeconds - elapsedSeconds);
        state.choiceTimer.remainingSeconds = remaining;

        if (remaining > 0) return;

        clearChoiceTimer();
        handleChoose("choice-timeout", { timedOut: true });
    }, 100);
}

function handleChoiceTimeout(nodeId, timer) {
    if (state.status !== "await-choice" || state.currentNodeId !== nodeId) {
        return;
    }

    const currentNode = getNodeById(nodeId);
    if (!currentNode) {
        setEnded("当前 Choice 节点不存在");
        return;
    }

    handleChoose("choice-timeout", { timedOut: true });
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
    clearChoiceTimer();
    state.status = "ended";
    state.currentNodeId = "";
    state.pendingChoices = [];
    state.error = errorText || "";
}

function handleBranchEnd() {
    if (state.exhaustiveReturnStack.length > 0) {
        const returnNodeId = String(
            state.exhaustiveReturnStack.pop() || "",
        ).trim();
        if (returnNodeId && getNodeById(returnNodeId)) {
            state.status = "running";
            state.currentNodeId = returnNodeId;
            return;
        }
    }

    setEnded();
}

function restartPreview() {
    if (isRailPreviewMode()) {
        restartRailPreview();
        return;
    }

    clearChoiceTimer();
    state.timeline = [];
    state.pendingChoices = [];
    state.error = "";
    state.multiLineCursor = {
        nodeId: "",
        nextIndex: 0,
    };
    state.exhaustiveSelectedByNode = {};
    state.exhaustiveReturnStack = [];
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

        if (kind === "dialogue" || kind === "multidialogue") {
            const speaker = toSpeaker(node?.data?.speakerId);
            const lines = Array.isArray(node?.data?.lines)
                ? node.data.lines
                      .map((line) =>
                          toText(
                              typeof line === "string" ? line : line?.textKey,
                          ),
                      )
                      .filter((text) => text.length > 0)
                : [toText(node?.data?.textKey)].filter(
                      (text) => text.length > 0,
                  );

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
                kind: "dialogue",
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
            const exhaustiveMode = isExhaustiveChoiceNode(node);
            const pending = choices
                .map((choice, idx) => ({
                    handle: `choice-${idx}`,
                    text: toText(choice?.textKey),
                }))
                .filter((choice) => {
                    if (
                        exhaustiveMode &&
                        isChoiceHandleSelected(node.id, choice.handle)
                    ) {
                        return false;
                    }

                    return getChoiceEdges(node.id, choice.handle).length > 0;
                });

            if (pending.length <= 0) {
                if (exhaustiveMode) {
                    const completeTarget = resolveChoiceCompleteTarget(node.id);
                    if (!completeTarget) {
                        setEnded(
                            "穷举 Choice 节点已耗尽选项，但未找到 choice-complete 出口。",
                        );
                        return;
                    }
                    state.currentNodeId = completeTarget;
                    continue;
                }

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
            startChoiceTimerForNode(node, pending);
            return;
        }

        if (kind === "condition") {
            const branches = getConditionBranches(node?.data?.condition);
            const matchedIndex = branches.findIndex((branch) =>
                evaluateCondition(branch),
            );
            const matchedBranch =
                matchedIndex >= 0 ? branches[matchedIndex] : null;
            const handle =
                matchedIndex >= 0
                    ? `condition-${matchedIndex}`
                    : "condition-fallback";
            const targetEdge = getChoiceEdges(node.id, handle)[0];

            pushTimeline({
                kind: "condition",
                nodeId: node.id,
                text: `${node.id} => ${matchedBranch?.label || handle}`,
            });

            if (!targetEdge?.target) {
                setEnded(
                    `Condition 节点 ${node.id} 缺少 ${handle} 出口。`,
                );
                return;
            }

            state.currentNodeId = String(targetEdge.target);
            continue;
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
            const eventLabel =
                toText(node?.data?.eventType) ||
                "(未命名事件)";
            pushTimeline({
                kind: "emitevent",
                nodeId: node.id,
                text: `${eventLabel}`,
                payload: {
                    nodeId: node.id,
                    phase: "node",
                    eventType: toText(node?.data?.eventType),
                    params: node?.data?.params || {},
                },
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

async function advanceRailPreview() {
    if (!railRunner) {
        restartRailPreview();
    }
    if (!railRunner || state.status === "await-choice") return;
    await railRunner.advance();
    syncRunnerState(railRunner.state);
    scrollTimelineToBottom();
}

async function handleTimelineAdvance() {
    if (isRailPreviewMode()) {
        if (state.status === "idle") {
            restartRailPreview();
        }
        if (state.status === "running") {
            await advanceRailPreview();
        }
        return;
    }

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

async function handleChoose(handle, options = {}) {
    if (isRailPreviewMode()) {
        if (state.status !== "await-choice" || !railRunner) return;
        await railRunner.choose(handle);
        syncRunnerState(railRunner.state);
        scrollTimelineToBottom();
        return;
    }

    if (state.status !== "await-choice") return;
    clearChoiceTimer();

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
            : String(handle || "") === "choice-timeout"
              ? toText(currentNode?.data?.choiceTimer?.timeoutChoiceTextKey)
            : "";

    const targetEdge = getChoiceEdges(currentNode.id, handle)[0];

    if (!targetEdge?.target) {
        setEnded("所选分支没有可用目标节点");
        return;
    }

    pushTimeline({
        kind: "choice",
        nodeId: currentNode.id,
        text: `${options.timedOut ? "超时选择：" : ""}${choiceText || `选项 ${choiceIndex + 1}`}`,
    });

    const exhaustiveMode = isExhaustiveChoiceNode(currentNode);
    if (exhaustiveMode && handle !== "choice-timeout") {
        markChoiceHandleSelected(currentNode.id, handle);
        state.exhaustiveReturnStack.push(String(currentNode.id));
    }

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

function formatCountdown(value) {
    const seconds = Number(value);
    if (!Number.isFinite(seconds)) return "0.0s";
    return `${Math.max(0, seconds).toFixed(1)}s`;
}

onUnmounted(() => {
    clearChoiceTimer();
});

watch(
    () => [
        props.previewMode,
        props.nodes,
        props.edges,
        props.variables,
        props.entryNodeId,
        props.rail,
        props.resolveStoryById,
    ],
    async () => {
        restartPreview();
        if (state.status === "running") {
            if (isRailPreviewMode()) {
                await advanceRailPreview();
            } else {
                advanceUntilPause();
            }
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
    user-select: none;
    -webkit-user-select: none;
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
    gap: 2px;
}

.choice-inline-btn {
    text-align: left;
    border: none;
    background: transparent;
    color: inherit;
    padding: 3px 2px;
    cursor: pointer;
    border-radius: 4px;
    transition:
        background-color 0.12s ease,
        color 0.12s ease;
}

.choice-inline-btn:hover {
    background: rgba(148, 163, 184, 0.12);
    color: #0ea5e9;
}

.choice-inline-btn:focus-visible {
    outline: 1px solid rgba(14, 165, 233, 0.55);
    outline-offset: 1px;
}

.timeline-list {
    display: flex;
    flex-direction: column;
    gap: 6px;
}

.timeline-choice-box {
    margin-top: 8px;
    border-top: 1px dashed rgba(148, 163, 184, 0.28);
    padding-top: 6px;
}

.choice-countdown {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 16px;
    margin-bottom: 6px;
    color: #92400e;
    font-size: 12px;
    font-weight: 700;
}

.choice-countdown strong {
    font-variant-numeric: tabular-nums;
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

.timeline-item--chapter {
    width: 100%;
    max-width: 100%;
    padding: 8px 2px;
}

.chapter-marker {
    --chapter-marker-color: #10b981;
    --chapter-marker-text: #047857;
    --chapter-marker-line: rgba(16, 185, 129, 0.38);
    display: grid;
    grid-template-columns: minmax(24px, 1fr) auto minmax(24px, 1fr);
    align-items: center;
    gap: 12px;
    width: 100%;
    color: var(--chapter-marker-text);
}

.chapter-marker--branch {
    --chapter-marker-color: #8b5cf6;
    --chapter-marker-text: #7c3aed;
    --chapter-marker-line: rgba(139, 92, 246, 0.42);
}

.chapter-marker--end {
    --chapter-marker-color: #f43f5e;
    --chapter-marker-text: #e11d48;
    --chapter-marker-line: rgba(244, 63, 94, 0.42);
}

.chapter-line {
    height: 1px;
    background: linear-gradient(
        90deg,
        transparent,
        var(--chapter-marker-line),
        transparent
    );
}

.chapter-content {
    max-width: min(520px, 70vw);
    text-align: center;
}

.chapter-title {
    font-size: 13px;
    font-weight: 800;
    letter-spacing: 0.04em;
}

.chapter-summary {
    margin-top: 2px;
    color: #64748b;
    font-size: 12px;
    font-weight: 600;
    letter-spacing: 0;
}

.line-row,
.event-row {
    --speaker-col-width: 9ch;
    --sep-col-width: 1.2ch;
    display: grid;
    grid-template-columns: var(--speaker-col-width) var(--sep-col-width) minmax(
            0,
            1fr
        );
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

.sep {
    color: #64748b;
    display: inline-block;
    width: var(--sep-col-width);
    text-align: right;
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

.preview-mode-root.is-dark-mode .meta-arrow,
.preview-mode-root.is-dark-mode .meta-type {
    color: rgba(141, 183, 226, 0.62) !important;
}

.preview-mode-root.is-dark-mode .meta-arrow--choice,
.preview-mode-root.is-dark-mode .meta-type--choice {
    color: #ffcc00 !important;
}

.preview-mode-root.is-dark-mode .meta-arrow--condition,
.preview-mode-root.is-dark-mode .meta-type--condition {
    color: #38bdf8 !important;
}

.preview-mode-root.is-dark-mode .meta-arrow--setvariable,
.preview-mode-root.is-dark-mode .meta-type--setvariable {
    color: #af52de !important;
}

.preview-mode-root.is-dark-mode .meta-arrow--emitevent,
.preview-mode-root.is-dark-mode .meta-type--emitevent {
    color: #ff9500 !important;
}

.preview-mode-root.is-dark-mode .meta-arrow--jump,
.preview-mode-root.is-dark-mode .meta-type--jump {
    color: #007aff !important;
}

.preview-mode-root.is-dark-mode .meta-arrow--end,
.preview-mode-root.is-dark-mode .meta-type--end {
    color: #ff3b30 !important;
}

.preview-mode-root.is-dark-mode .sep {
    color: #8ea6bc !important;
}

.preview-mode-root.is-dark-mode .text {
    color: #b8cde0 !important;
}

.preview-mode-root.is-dark-mode .meta-param-key {
    color: #9fb3c8 !important;
}

.preview-mode-root.is-dark-mode .meta-param-value {
    color: #d6e4f0 !important;
}

.preview-mode-root.is-dark-mode .chapter-marker {
    --chapter-marker-text: #34d399;
    --chapter-marker-line: rgba(52, 211, 153, 0.42);
    color: var(--chapter-marker-text) !important;
}

.preview-mode-root.is-dark-mode .chapter-marker--branch {
    --chapter-marker-text: #a78bfa;
    --chapter-marker-line: rgba(167, 139, 250, 0.48);
}

.preview-mode-root.is-dark-mode .chapter-marker--end {
    --chapter-marker-text: #fb7185;
    --chapter-marker-line: rgba(251, 113, 133, 0.5);
}

.preview-mode-root.is-dark-mode .chapter-line {
    background: linear-gradient(
        90deg,
        transparent,
        var(--chapter-marker-line),
        transparent
    ) !important;
}

.preview-mode-root.is-dark-mode .chapter-summary {
    color: #9fb3c8 !important;
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

:global(body.dark-theme) .preview-mode-root .meta-arrow,
:global(body.dark-theme) .preview-mode-root .meta-type,
:global(body[data-theme="dark"]) .preview-mode-root .meta-arrow,
:global(body[data-theme="dark"]) .preview-mode-root .meta-type {
    color: rgba(141, 183, 226, 0.62) !important;
}

:global(body.dark-theme) .preview-mode-root .meta-arrow--choice,
:global(body.dark-theme) .preview-mode-root .meta-type--choice,
:global(body[data-theme="dark"]) .preview-mode-root .meta-arrow--choice,
:global(body[data-theme="dark"]) .preview-mode-root .meta-type--choice {
    color: #ffcc00 !important;
}

:global(body.dark-theme) .preview-mode-root .meta-arrow--condition,
:global(body.dark-theme) .preview-mode-root .meta-type--condition,
:global(body[data-theme="dark"]) .preview-mode-root .meta-arrow--condition,
:global(body[data-theme="dark"]) .preview-mode-root .meta-type--condition {
    color: #38bdf8 !important;
}

:global(body.dark-theme) .preview-mode-root .meta-arrow--setvariable,
:global(body.dark-theme) .preview-mode-root .meta-type--setvariable,
:global(body[data-theme="dark"]) .preview-mode-root .meta-arrow--setvariable,
:global(body[data-theme="dark"]) .preview-mode-root .meta-type--setvariable {
    color: #af52de !important;
}

:global(body.dark-theme) .preview-mode-root .meta-arrow--emitevent,
:global(body.dark-theme) .preview-mode-root .meta-type--emitevent,
:global(body[data-theme="dark"]) .preview-mode-root .meta-arrow--emitevent,
:global(body[data-theme="dark"]) .preview-mode-root .meta-type--emitevent {
    color: #ff9500 !important;
}

:global(body.dark-theme) .preview-mode-root .meta-arrow--jump,
:global(body.dark-theme) .preview-mode-root .meta-type--jump,
:global(body[data-theme="dark"]) .preview-mode-root .meta-arrow--jump,
:global(body[data-theme="dark"]) .preview-mode-root .meta-type--jump {
    color: #007aff !important;
}

:global(body.dark-theme) .preview-mode-root .meta-arrow--end,
:global(body.dark-theme) .preview-mode-root .meta-type--end,
:global(body[data-theme="dark"]) .preview-mode-root .meta-arrow--end,
:global(body[data-theme="dark"]) .preview-mode-root .meta-type--end {
    color: #ff3b30 !important;
}

:global(body.dark-theme) .preview-mode-root .sep,
:global(body[data-theme="dark"]) .preview-mode-root .sep {
    color: #8ea6bc !important;
}

:global(body.dark-theme) .preview-mode-root .text,
:global(body[data-theme="dark"]) .preview-mode-root .text {
    color: #b8cde0 !important;
}

:global(body.dark-theme) .preview-mode-root .chapter-marker,
:global(body[data-theme="dark"]) .preview-mode-root .chapter-marker {
    --chapter-marker-text: #34d399;
    --chapter-marker-line: rgba(52, 211, 153, 0.42);
    color: var(--chapter-marker-text) !important;
}

:global(body.dark-theme) .preview-mode-root .chapter-marker--branch,
:global(body[data-theme="dark"]) .preview-mode-root .chapter-marker--branch {
    --chapter-marker-text: #a78bfa;
    --chapter-marker-line: rgba(167, 139, 250, 0.48);
}

:global(body.dark-theme) .preview-mode-root .chapter-marker--end,
:global(body[data-theme="dark"]) .preview-mode-root .chapter-marker--end {
    --chapter-marker-text: #fb7185;
    --chapter-marker-line: rgba(251, 113, 133, 0.5);
}

:global(body.dark-theme) .preview-mode-root .chapter-line,
:global(body[data-theme="dark"]) .preview-mode-root .chapter-line {
    background: linear-gradient(
        90deg,
        transparent,
        var(--chapter-marker-line),
        transparent
    ) !important;
}

:global(body.dark-theme) .preview-mode-root .chapter-summary,
:global(body[data-theme="dark"]) .preview-mode-root .chapter-summary {
    color: #9fb3c8 !important;
}

:global(body.dark-theme) .preview-mode-root .meta-param-key,
:global(body[data-theme="dark"]) .preview-mode-root .meta-param-key {
    color: #9fb3c8 !important;
}

:global(body.dark-theme) .preview-mode-root .meta-param-value,
:global(body[data-theme="dark"]) .preview-mode-root .meta-param-value {
    color: #d6e4f0 !important;
}

.meta-content {
    grid-column: 3;
    margin-left: 0;
    display: grid;
    grid-template-columns: auto minmax(0, 1fr);
    align-items: start;
    column-gap: 0.34em;
    min-width: 0;
    font-size: 0.94em;
    font-weight: 650;
    letter-spacing: 0.005em;
}

.meta-arrow {
    white-space: nowrap;
    justify-self: end;
    width: auto;
    color: rgba(110, 162, 216, 0.62);
    font-weight: 700;
}

.meta-type {
    white-space: nowrap;
    color: rgba(110, 162, 216, 0.62);
    font-weight: 700;
}

.meta-arrow--choice,
.meta-type--choice {
    color: #ffcc00;
}

.meta-arrow--condition,
.meta-type--condition {
    color: #0ea5e9;
}

.meta-arrow--setvariable,
.meta-type--setvariable {
    color: #af52de;
}

.meta-arrow--emitevent,
.meta-type--emitevent {
    color: #ff9500;
}

.meta-arrow--jump,
.meta-type--jump {
    color: #007aff;
}

.meta-arrow--end,
.meta-type--end {
    color: #ff3b30;
}

.meta-text {
    min-width: 0;
    white-space: pre-wrap;
    overflow-wrap: anywhere;
    word-break: break-word;
}

.meta-param-list {
    display: flex;
    flex-direction: column;
    gap: 3px;
    margin-top: 5px;
    font-size: 0.92em;
    font-weight: 500;
    line-height: 1.35;
}

.meta-param-row {
    display: grid;
    grid-template-columns: minmax(72px, max-content) minmax(0, 1fr);
    column-gap: 8px;
    min-width: 0;
}

.meta-param-key {
    color: rgba(100, 116, 139, 0.9);
    font-weight: 700;
    overflow-wrap: anywhere;
}

.meta-param-key::after {
    content: ":";
}

.meta-param-value {
    min-width: 0;
    color: #334155;
    overflow-wrap: anywhere;
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
