<template>
    <div
        ref="containerRef"
        class="canvas-graph-editor"
        role="region"
        aria-label="NarrRail Graph Editor Canvas"
        tabindex="0"
        @contextmenu.prevent="openContextMenu"
    >
        <canvas
            ref="canvasRef"
            class="canvas-graph-surface"
            @pointerdown="handlePointerDown"
            @pointermove="handlePointerMove"
            @pointerup="handlePointerUp"
            @pointercancel="handlePointerCancel"
            @pointerleave="handlePointerLeave"
            @wheel.prevent="handleWheel"
        ></canvas>

        <div class="canvas-controls glass-morphism-strong" aria-label="Graph controls">
            <button type="button" class="canvas-control-btn" title="放大" @click="zoomBy(1.18)">
                +
            </button>
            <button type="button" class="canvas-control-btn" title="缩小" @click="zoomBy(0.85)">
                -
            </button>
            <button type="button" class="canvas-control-btn" title="适应画布" @click="fitView">
                ⤢
            </button>
        </div>

        <div
            v-if="contextMenu.show"
            ref="contextMenuRef"
            class="context-menu glass-morphism-strong"
            :style="{ left: `${contextMenu.x}px`, top: `${contextMenu.y}px` }"
        >
            <div class="context-menu-header">添加节点</div>
            <button
                v-for="item in contextMenuItems"
                :key="item.type"
                type="button"
                class="context-menu-item"
                @click="createNode(item.type)"
            >
                <IconGlyph :name="item.icon" />
                <span>{{ item.label }}</span>
            </button>
        </div>
    </div>
</template>

<script setup>
import { computed, nextTick, onMounted, onUnmounted, reactive, ref, watch } from "vue";
import IconGlyph from "./IconGlyph.vue";

const NODE_WIDTH = 236;
const NODE_HEADER_HEIGHT = 44;
const NODE_PADDING = 14;
const NODE_LINE_HEIGHT = 18;
const NODE_CONTENT_WIDTH = NODE_WIDTH - NODE_PADDING * 2;
const NODE_RADIUS = 16;
const HANDLE_RADIUS = 6;
const MIN_ZOOM = 0.2;
const MAX_ZOOM = 2.2;
const WHEEL_ZOOM_SENSITIVITY = 0.012;
const MIN_WHEEL_ZOOM_FACTOR = 0.35;
const MAX_WHEEL_ZOOM_FACTOR = 2.85;
const EDGE_HIT_RADIUS = 10;
const DRAG_THRESHOLD = 4;
const GRAPH_EDGE_COLOR = "#8b7aa8";
const GRAPH_EDGE_SELECTED_COLOR = "#a76f8f";

const NODE_META = {
    dialogue: { label: "对话", icon: "speaker_notes", color: "#5fae8b" },
    choice: { label: "选择", icon: "fork_right", color: "#c8945d" },
    jump: { label: "跳转", icon: "arrow_forward", color: "#6f9fc9" },
    setvariable: { label: "设置变量", icon: "edit_square", color: "#a785bd" },
    emitevent: { label: "触发事件", icon: "notifications_active", color: "#c77f95" },
    condition: { label: "条件", icon: "rule", color: "#8d8bc6" },
    end: { label: "结束", icon: "stop_circle", color: "#969aa3" },
    railstory: { label: "脚本", icon: "article", color: "#6f9fc9" },
    railbranch: { label: "总纲分支", icon: "rule", color: "#79aeb9" },
    railnote: { label: "章节标记", icon: "sticky_note_2", color: "#b9a35f" },
    railend: { label: "总纲结束", icon: "stop_circle", color: "#969aa3" },
};

const props = defineProps({
    nodes: {
        type: Array,
        default: () => [],
    },
    edges: {
        type: Array,
        default: () => [],
    },
    edgeRenderMode: {
        type: String,
        default: "straight",
    },
    presetSpeakers: {
        type: Array,
        default: () => [],
    },
    graphMode: {
        type: String,
        default: "story",
    },
});

const canvasRef = ref(null);
const containerRef = ref(null);
const contextMenuRef = ref(null);
const internalNodes = ref([]);
const internalEdges = ref([]);
const selectedNodeId = ref(null);
const selectedNodeIds = ref(new Set());
const selectedEdgeId = ref(null);
const viewport = reactive({ x: 0, y: 0, zoom: 1 });
const contextMenu = reactive({
    show: false,
    x: 0,
    y: 0,
    graphX: 0,
    graphY: 0,
});

let ctx = null;
let resizeObserver = null;
let animationFrame = 0;
let graphDispatchTimer = null;
let lastPointer = null;
let interaction = null;
let hasFitInitialView = false;
let themeIsDark = false;
let themeObserver = null;

const contextMenuItems = computed(() => {
    if (props.graphMode === "rail") {
        return [
            { type: "railstory", label: "脚本节点", icon: "article" },
            { type: "railbranch", label: "路线分支", icon: "rule" },
            { type: "railnote", label: "章节标记", icon: "sticky_note_2" },
            { type: "railend", label: "总纲结束", icon: "stop_circle" },
        ];
    }

    return [
        { type: "dialogue", label: "对话节点", icon: "chat" },
        { type: "choice", label: "选择节点", icon: "fork_right" },
        { type: "jump", label: "跳转节点", icon: "arrow_forward" },
        { type: "setvariable", label: "设置变量", icon: "edit_square" },
        { type: "emitevent", label: "触发事件", icon: "notifications_active" },
        { type: "condition", label: "条件判断", icon: "rule" },
        { type: "end", label: "结束节点", icon: "stop_circle" },
    ];
});

watch(
    () => props.nodes,
    (nodes) => {
        internalNodes.value = cloneArray(nodes);
        pruneSelection();
        if (!hasFitInitialView) nextTick(fitView);
        scheduleDraw();
    },
    { deep: true, immediate: true },
);

watch(
    () => props.edges,
    (edges) => {
        internalEdges.value = cloneArray(edges);
        scheduleDraw();
    },
    { deep: true, immediate: true },
);

watch(
    () => props.edgeRenderMode,
    () => scheduleDraw(),
);

watch(
    () => props.graphMode,
    () => {
        closeContextMenu();
        scheduleDraw();
    },
);

onMounted(() => {
    ctx = canvasRef.value?.getContext("2d", { alpha: true });
    resizeObserver = new ResizeObserver(resizeCanvas);
    resizeObserver.observe(containerRef.value);
    window.addEventListener("keydown", handleKeyDown);
    window.addEventListener("pointerdown", handleGlobalPointerDown, true);
    window.matchMedia?.("(prefers-color-scheme: dark)")?.addEventListener?.("change", handleThemeChange);
    themeObserver = new MutationObserver(handleThemeChange);
    themeObserver.observe(document.body, {
        attributes: true,
        attributeFilter: ["class", "data-theme"],
    });
    handleThemeChange();
    resizeCanvas();
    nextTick(fitView);
});

onUnmounted(() => {
    resizeObserver?.disconnect();
    resizeObserver = null;
    window.removeEventListener("keydown", handleKeyDown);
    window.removeEventListener("pointerdown", handleGlobalPointerDown, true);
    window.matchMedia?.("(prefers-color-scheme: dark)")?.removeEventListener?.("change", handleThemeChange);
    themeObserver?.disconnect();
    themeObserver = null;
    if (animationFrame) cancelAnimationFrame(animationFrame);
    if (graphDispatchTimer) clearTimeout(graphDispatchTimer);
});

function cloneArray(value) {
    return Array.isArray(value) ? value.map((item) => structuredCloneSafe(item)) : [];
}

function structuredCloneSafe(value) {
    if (typeof structuredClone === "function") {
        try {
            return structuredClone(value);
        } catch (error) {
            // Vue reactive proxies are not structured-cloneable in some browsers.
        }
    }
    return JSON.parse(JSON.stringify(value));
}

function handleThemeChange() {
    const body = document.body;
    themeIsDark =
        body.classList.contains("dark-theme") ||
        body.dataset.theme === "dark" ||
        (!body.classList.contains("light-theme") &&
            body.dataset.theme !== "light" &&
            window.matchMedia?.("(prefers-color-scheme: dark)")?.matches);
    scheduleDraw();
}

function resizeCanvas() {
    const canvas = canvasRef.value;
    const container = containerRef.value;
    if (!canvas || !container) return;

    const bounds = container.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;
    canvas.width = Math.max(1, Math.floor(bounds.width * dpr));
    canvas.height = Math.max(1, Math.floor(bounds.height * dpr));
    canvas.style.width = `${bounds.width}px`;
    canvas.style.height = `${bounds.height}px`;
    scheduleDraw();
}

function scheduleDraw() {
    if (animationFrame) return;
    animationFrame = requestAnimationFrame(() => {
        animationFrame = 0;
        draw();
    });
}

function draw() {
    const canvas = canvasRef.value;
    if (!ctx || !canvas) return;

    const dpr = window.devicePixelRatio || 1;
    const width = canvas.width / dpr;
    const height = canvas.height / dpr;
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, width, height);
    drawBackground(width, height);

    ctx.save();
    ctx.translate(viewport.x, viewport.y);
    ctx.scale(viewport.zoom, viewport.zoom);

    for (const edge of internalEdges.value) {
        drawEdge(edge, edge.id === selectedEdgeId.value);
    }

    if (interaction?.type === "connect" && interaction.current) {
        drawConnectionPreview(interaction);
    }

    for (const node of internalNodes.value) {
        drawNode(node, isNodeSelected(node.id));
    }

    if (interaction?.type === "select-box") {
        drawSelectionBox(interaction);
    }

    ctx.restore();
}

function drawBackground(width, height) {
    const grid = 20 * viewport.zoom;
    ctx.fillStyle = themeIsDark ? "rgba(11, 16, 32, 0.12)" : "rgba(255, 255, 255, 0.1)";
    ctx.fillRect(0, 0, width, height);

    if (grid < 5) return;
    ctx.fillStyle = themeIsDark ? "rgba(226, 232, 240, 0.1)" : "rgba(25, 28, 29, 0.06)";
    const startX = positiveModulo(viewport.x, grid);
    const startY = positiveModulo(viewport.y, grid);
    for (let x = startX; x < width; x += grid) {
        for (let y = startY; y < height; y += grid) {
            ctx.beginPath();
            ctx.arc(x, y, 1, 0, Math.PI * 2);
            ctx.fill();
        }
    }
}

function positiveModulo(value, modulo) {
    return ((value % modulo) + modulo) % modulo;
}

function drawNode(node, selected) {
    const layout = getNodeLayout(node);
    const meta = getNodeMeta(node.type);
    const theme = getCanvasTheme();
    const x = node.position?.x || 0;
    const y = node.position?.y || 0;

    ctx.save();
    ctx.shadowColor = selected
        ? hexToRgba(meta.color, theme.selectedShadowAlpha)
        : theme.nodeShadow;
    ctx.shadowBlur = selected ? 26 : 18;
    ctx.shadowOffsetY = selected ? 12 : 8;
    roundedRect(x, y, NODE_WIDTH, layout.height, NODE_RADIUS);
    ctx.fillStyle = theme.nodeFill;
    ctx.fill();
    ctx.shadowColor = "transparent";

    roundedRect(x, y, NODE_WIDTH, layout.height, NODE_RADIUS);
    ctx.lineWidth = selected ? 1.8 : 1;
    ctx.strokeStyle = selected ? hexToRgba(meta.color, 0.68) : theme.nodeBorder;
    ctx.stroke();

    ctx.save();
    roundedRect(x, y, NODE_WIDTH, layout.height, NODE_RADIUS);
    ctx.clip();
    const headerGradient = ctx.createLinearGradient(
        x,
        y,
        x,
        y + NODE_HEADER_HEIGHT + 18,
    );
    headerGradient.addColorStop(0, hexToRgba(meta.color, theme.headerTintAlpha));
    headerGradient.addColorStop(0.48, hexToRgba(meta.color, theme.headerTintMidAlpha));
    headerGradient.addColorStop(1, hexToRgba(meta.color, 0));
    ctx.fillStyle = headerGradient;
    ctx.fillRect(x, y, NODE_WIDTH, NODE_HEADER_HEIGHT + 18);
    ctx.restore();

    if (selected) {
        roundedRect(x - 3, y - 3, NODE_WIDTH + 6, layout.height + 6, NODE_RADIUS + 4);
        ctx.lineWidth = 1.5;
        ctx.strokeStyle = hexToRgba(meta.color, theme.selectedHaloAlpha);
        ctx.stroke();
    }

    drawPortRowGroups(x, y, layout, meta, theme);

    ctx.fillStyle = theme.titleText;
    ctx.font = "700 12px Outfit, Noto Sans SC, sans-serif";
    ctx.textBaseline = "middle";
    ctx.fillText(meta.label, x + NODE_PADDING, y + NODE_HEADER_HEIGHT / 2);

    ctx.textBaseline = "top";
    for (const line of layout.drawLines) {
        if (line.kind === "label") {
            ctx.fillStyle = theme.mutedText;
        } else {
            ctx.fillStyle = line.strong ? theme.strongText : theme.bodyText;
        }
        ctx.font = getRowFont(line);
        ctx.fillText(line.text, x + NODE_PADDING, y + line.y);
    }

    const ports = getNodePorts(node, layout);
    const densePorts = ports.filter((port) => port.kind === "source").length >= 5;
    for (const port of ports) {
        drawHandle(
            port.x,
            port.y,
            port.kind === "source" ? meta.color : theme.targetHandle,
            selected,
            densePorts && port.kind === "source",
        );
    }

    ctx.restore();
}

function drawSelectionBox(selection) {
    const rect = normalizeRect(selection.startGraph, selection.currentGraph || selection.startGraph);

    ctx.save();
    ctx.lineWidth = 1 / viewport.zoom;
    ctx.setLineDash([8 / viewport.zoom, 6 / viewport.zoom]);
    ctx.strokeStyle = themeIsDark ? "rgba(129, 140, 248, 0.9)" : "rgba(99, 102, 241, 0.88)";
    ctx.fillStyle = themeIsDark ? "rgba(129, 140, 248, 0.12)" : "rgba(99, 102, 241, 0.1)";
    ctx.beginPath();
    ctx.rect(rect.x, rect.y, rect.width, rect.height);
    ctx.fill();
    ctx.stroke();
    ctx.restore();
}

function drawPortRowGroups(x, y, layout, meta, theme) {
    const portRows = layout.rowRects.filter((row) => row.portId);
    if (portRows.length === 0) return;

    const groups = [];
    let currentGroup = [];
    for (const row of portRows) {
        const previous = currentGroup[currentGroup.length - 1];
        if (previous && row.y - (previous.y + previous.height) > 8) {
            groups.push(currentGroup);
            currentGroup = [];
        }
        currentGroup.push(row);
    }
    if (currentGroup.length) groups.push(currentGroup);

    for (const group of groups) {
        const groupX = x + NODE_PADDING - 6;
        const groupWidth = NODE_CONTENT_WIDTH + 12;

        for (let index = 1; index < group.length; index += 1) {
            const row = group[index];
            const separatorY = y + row.y - 2;
            ctx.beginPath();
            ctx.moveTo(groupX + 6, separatorY);
            ctx.lineTo(groupX + groupWidth - 18, separatorY);
            ctx.strokeStyle = theme.rowSeparator;
            ctx.stroke();
        }
    }
}

function drawHandle(x, y, color, selected, compact = false) {
    const theme = getCanvasTheme();
    const radius = compact ? HANDLE_RADIUS - 1.5 : HANDLE_RADIUS;
    ctx.beginPath();
    ctx.arc(x, y, radius, 0, Math.PI * 2);
    ctx.fillStyle = theme.handleFill;
    ctx.fill();
    ctx.lineWidth = selected ? 1.8 : 1.3;
    ctx.strokeStyle = hexToRgba(color, selected ? 0.84 : 0.58);
    ctx.stroke();
}

function drawEdge(edge, selected) {
    const sourceNode = getNodeById(edge.source);
    const targetNode = getNodeById(edge.target);
    if (!sourceNode || !targetNode) return;

    const source = getPortPoint(sourceNode, "source", edge.sourceHandle);
    const target = getPortPoint(targetNode, "target", edge.targetHandle);
    const color = selected
        ? hexToRgba(GRAPH_EDGE_SELECTED_COLOR, themeIsDark ? 0.88 : 0.82)
        : hexToRgba(GRAPH_EDGE_COLOR, themeIsDark ? 0.62 : 0.5);

    ctx.save();
    ctx.lineWidth = selected ? 2.8 : 1.8;
    ctx.strokeStyle = color;
    ctx.fillStyle = color;
    ctx.beginPath();
    if (props.edgeRenderMode === "bezier") {
        const dx = Math.max(80, Math.abs(target.x - source.x) * 0.48);
        ctx.moveTo(source.x, source.y);
        ctx.bezierCurveTo(source.x + dx, source.y, target.x - dx, target.y, target.x, target.y);
    } else {
        ctx.moveTo(source.x, source.y);
        ctx.lineTo(target.x, target.y);
    }
    ctx.stroke();
    drawArrowHead(source, target, color);
    ctx.restore();
}

function drawConnectionPreview(state) {
    const color = hexToRgba(GRAPH_EDGE_COLOR, themeIsDark ? 0.78 : 0.66);
    ctx.save();
    ctx.setLineDash([8, 6]);
    ctx.lineWidth = 2;
    ctx.strokeStyle = color;
    ctx.beginPath();
    ctx.moveTo(state.start.x, state.start.y);
    ctx.lineTo(state.current.x, state.current.y);
    ctx.stroke();
    ctx.restore();
}

function drawArrowHead(source, target, color) {
    const angle = Math.atan2(target.y - source.y, target.x - source.x);
    const size = 9;
    ctx.beginPath();
    ctx.moveTo(target.x, target.y);
    ctx.lineTo(target.x - size * Math.cos(angle - Math.PI / 6), target.y - size * Math.sin(angle - Math.PI / 6));
    ctx.lineTo(target.x - size * Math.cos(angle + Math.PI / 6), target.y - size * Math.sin(angle + Math.PI / 6));
    ctx.closePath();
    ctx.fillStyle = color;
    ctx.fill();
}

function getNodeLayout(node) {
    const rows = getNodeRows(node);
    const drawLines = [];
    const rowRects = [];
    const portAnchors = new Map();
    let cursorY = NODE_HEADER_HEIGHT + NODE_PADDING;

    for (const row of rows) {
        const font = getRowFont(row);
        const wrappedLines = row.wrap === false
            ? [String(row.text ?? "")]
            : wrapTextToWidth(row.text, NODE_CONTENT_WIDTH, font);
        const lineHeight = row.lineHeight || NODE_LINE_HEIGHT;
        const rowHeight = Math.max(
            row.minHeight || lineHeight,
            wrappedLines.length * lineHeight,
        );
        const textStartY = cursorY + Math.max(0, (rowHeight - wrappedLines.length * lineHeight) / 2);
        rowRects.push({
            ...row,
            y: cursorY,
            height: rowHeight,
        });

        wrappedLines.forEach((text, index) => {
            drawLines.push({
                ...row,
                text,
                y: textStartY + index * lineHeight,
                height: lineHeight,
            });
        });

        if (row.portId) {
            portAnchors.set(row.portId, cursorY + rowHeight / 2);
        }

        cursorY += rowHeight + (row.gapAfter ?? 2);
    }

    return {
        width: NODE_WIDTH,
        height: Math.max(92, cursorY + NODE_PADDING - 2),
        rows,
        rowRects,
        drawLines,
        portAnchors,
    };
}

function getNodeRows(node) {
    const data = node.data || {};
    switch (node.type) {
        case "dialogue": {
            const speaker = data.speakerId || "旁白";
            const rawLines = Array.isArray(data.lines) ? data.lines : [{ textKey: data.textKey || "" }];
            const preview = rawLines
                .map((line) => (typeof line === "string" ? line : line?.textKey || ""))
                .filter((line) => String(line).trim())
                .slice(0, 3);
            return [
                { kind: "label", text: `角色: ${speaker}`, minHeight: 16, lineHeight: 16 },
                ...(preview.length
                    ? preview.map((text) => ({ text, minHeight: 18, lineHeight: 18 }))
                    : [{ text: "（未设置）", minHeight: 18, lineHeight: 18 }]),
            ];
        }
        case "choice": {
            const choices = Array.isArray(data.choices) ? data.choices : [];
            const rows = choices.length
                ? choices.map((choice, index) => ({
                      text: `${index + 1}. ${choice?.textKey || `选项 ${index + 1}`}`,
                      minHeight: 22,
                      lineHeight: 18,
                      portId: `choice-${index}`,
                  }))
                : [{ text: "暂无选项", minHeight: 22, lineHeight: 18 }];
            if (data.choiceTimer?.enabled) {
                rows.push({
                    text: `超时: ${data.choiceTimer.timeoutChoiceTextKey || "超时"} (${data.choiceTimer.durationSeconds || 8}s)`,
                    minHeight: 22,
                    lineHeight: 18,
                    portId: "choice-timeout",
                });
            }
            if (data.choiceMode === "ExhaustiveUntilComplete") {
                rows.push({
                    text: "全部选项完成",
                    minHeight: 22,
                    lineHeight: 18,
                    portId: "choice-complete",
                });
            }
            return rows;
        }
        case "jump":
            return [
                { kind: "label", text: "目标节点", minHeight: 16, lineHeight: 16 },
                { text: data.targetNodeId || "未设置", strong: true, minHeight: 20, lineHeight: 18 },
            ];
        case "setvariable":
            return [
                { kind: "label", text: "变量名", minHeight: 16, lineHeight: 16 },
                { text: data.variableName || "未设置", minHeight: 18, lineHeight: 18 },
                { kind: "label", text: "操作 / 值", minHeight: 16, lineHeight: 16 },
                { text: `${data.operation || "Set"}: ${data.value ?? "未设置"}`, minHeight: 18, lineHeight: 18 },
            ];
        case "emitevent":
            return [
                { kind: "label", text: "事件", minHeight: 16, lineHeight: 16 },
                { text: data.eventType || "未设置", strong: true, minHeight: 20, lineHeight: 18 },
                { text: `参数: ${Object.keys(data.params || {}).length}`, minHeight: 18, lineHeight: 18 },
            ];
        case "condition": {
            const branches = Array.isArray(data.condition?.branches) ? data.condition.branches : [];
            return [
                ...(branches.length
                    ? branches.map((branch, index) => ({
                          text: branch?.label || `条件 ${index + 1}`,
                          minHeight: 22,
                          lineHeight: 18,
                          portId: `condition-${index}`,
                      }))
                    : [{ text: "暂无条件分支", minHeight: 22, lineHeight: 18 }]),
                { text: "未匹配", minHeight: 22, lineHeight: 18, portId: "condition-fallback" },
            ];
        }
        case "end":
            return [{ text: "剧情结束", strong: true, minHeight: 22, lineHeight: 18 }];
        case "railstory":
            return [
                { kind: "label", text: "storyId", minHeight: 16, lineHeight: 16 },
                { text: data.storyId || "未绑定 storyId", strong: true, minHeight: 20, lineHeight: 18 },
                ...(data.summary ? [{ text: data.summary, minHeight: 18, lineHeight: 18 }] : []),
            ];
        case "railbranch": {
            const branches = Array.isArray(data.branches) ? data.branches : [];
            return [
                ...(data.summary ? [{ text: data.summary, minHeight: 18, lineHeight: 18 }] : []),
                ...(branches.length
                    ? branches.map((branch, index) => ({
                          text: branch?.label || `分支 ${index + 1}`,
                          minHeight: 22,
                          lineHeight: 18,
                          portId: `branch-${index}`,
                      }))
                    : [{ text: "暂无分支", minHeight: 22, lineHeight: 18 }]),
                { text: "未匹配", minHeight: 22, lineHeight: 18, portId: "branch-fallback" },
            ];
        }
        case "railnote":
            return [{ text: data.summary || "用于总纲说明，可自动通过", minHeight: 18, lineHeight: 18 }];
        case "railend":
            return [{ text: data.summary || "当前路线到此结束", minHeight: 18, lineHeight: 18 }];
        default:
            return [{ text: node.id || "节点", strong: true, minHeight: 20, lineHeight: 18 }];
    }
}

function getNodePorts(node, layout = getNodeLayout(node)) {
    const x = node.position?.x || 0;
    const y = node.position?.y || 0;
    const middleY = y + layout.height / 2;
    const ports = [{ kind: "target", id: undefined, x, y: middleY }];

    if (node.type === "choice") {
        const choices = Array.isArray(node.data?.choices) ? node.data.choices : [];
        const sourceHandles = choices.map((_, index) => `choice-${index}`);
        if (node.data?.choiceTimer?.enabled) sourceHandles.push("choice-timeout");
        if (node.data?.choiceMode === "ExhaustiveUntilComplete") {
            sourceHandles.push("choice-complete");
        }
        for (const handle of sourceHandles) {
            ports.push({
                kind: "source",
                id: handle,
                x: x + NODE_WIDTH,
                y: y + (layout.portAnchors.get(handle) ?? middleY),
            });
        }
    } else if (node.type === "condition") {
        const branches = node.data?.condition?.branches || [];
        const sourceHandles = branches.map((_, index) => `condition-${index}`);
        sourceHandles.push("condition-fallback");
        for (const handle of sourceHandles) {
            ports.push({
                kind: "source",
                id: handle,
                x: x + NODE_WIDTH,
                y: y + (layout.portAnchors.get(handle) ?? middleY),
            });
        }
    } else if (node.type === "railbranch") {
        const branches = node.data?.branches || [];
        const sourceHandles = branches.map((_, index) => `branch-${index}`);
        sourceHandles.push("branch-fallback");
        for (const handle of sourceHandles) {
            ports.push({
                kind: "source",
                id: handle,
                x: x + NODE_WIDTH,
                y: y + (layout.portAnchors.get(handle) ?? middleY),
            });
        }
    } else if (node.type !== "end" && node.type !== "railend") {
        ports.push({ kind: "source", id: undefined, x: x + NODE_WIDTH, y: middleY });
    }

    return ports;
}

function getPortPoint(node, kind, handleId) {
    const ports = getNodePorts(node).filter((port) => port.kind === kind);
    return ports.find((port) => port.id === handleId) || ports[0] || {
        x: (node.position?.x || 0) + (kind === "source" ? NODE_WIDTH : 0),
        y: (node.position?.y || 0) + getNodeLayout(node).height / 2,
    };
}

function handlePointerDown(event) {
    containerRef.value?.focus();
    closeContextMenu();
    const graphPoint = screenToGraph(event.clientX, event.clientY);
    lastPointer = { x: event.clientX, y: event.clientY, graphX: graphPoint.x, graphY: graphPoint.y };

    if (event.button !== 0 && event.button !== 1) return;
    if (event.button === 1) event.preventDefault();
    canvasRef.value?.setPointerCapture(event.pointerId);

    if (event.button === 1) {
        clearSelection();
        interaction = {
            type: "pan",
            startX: event.clientX,
            startY: event.clientY,
            viewportX: viewport.x,
            viewportY: viewport.y,
            moved: false,
        };
        return;
    }

    const handleHit = hitTestHandle(graphPoint.x, graphPoint.y);
    if (handleHit?.port.kind === "source") {
        clearSelection();
        interaction = {
            type: "connect",
            nodeId: handleHit.node.id,
            handleId: handleHit.port.id,
            start: { x: handleHit.port.x, y: handleHit.port.y },
            current: graphPoint,
            moved: false,
        };
        scheduleDraw();
        return;
    }

    const nodeHit = hitTestNode(graphPoint.x, graphPoint.y);
    if (nodeHit) {
        if (!isNodeSelected(nodeHit.id)) {
            selectNode(nodeHit);
        }
        const draggedNodeIds = isNodeSelected(nodeHit.id)
            ? [...selectedNodeIds.value]
            : [nodeHit.id];
        interaction = {
            type: "drag-node",
            nodeId: nodeHit.id,
            nodeIds: draggedNodeIds,
            startPositions: getNodePositionMap(draggedNodeIds),
            offsetX: graphPoint.x - (nodeHit.position?.x || 0),
            offsetY: graphPoint.y - (nodeHit.position?.y || 0),
            startX: event.clientX,
            startY: event.clientY,
            startGraph: graphPoint,
            moved: false,
            dispatchedStart: false,
        };
        return;
    }

    const edgeHit = hitTestEdge(graphPoint.x, graphPoint.y);
    if (edgeHit) {
        selectEdge(edgeHit);
        interaction = { type: "select-edge", startX: event.clientX, startY: event.clientY, moved: false };
        return;
    }

    clearSelection();
    interaction = {
        type: "select-box",
        startX: event.clientX,
        startY: event.clientY,
        startGraph: graphPoint,
        currentGraph: graphPoint,
        moved: false,
    };
}

function handlePointerMove(event) {
    const graphPoint = screenToGraph(event.clientX, event.clientY);
    lastPointer = { x: event.clientX, y: event.clientY, graphX: graphPoint.x, graphY: graphPoint.y };

    if (!interaction) {
        canvasRef.value.style.cursor = getCursorForPoint(graphPoint);
        return;
    }

    if (interaction.type === "pan") {
        const dx = event.clientX - interaction.startX;
        const dy = event.clientY - interaction.startY;
        interaction.moved ||= Math.abs(dx) >= DRAG_THRESHOLD || Math.abs(dy) >= DRAG_THRESHOLD;
        viewport.x = interaction.viewportX + dx;
        viewport.y = interaction.viewportY + dy;
        scheduleDraw();
        return;
    }

    if (interaction.type === "drag-node") {
        const dx = event.clientX - interaction.startX;
        const dy = event.clientY - interaction.startY;
        interaction.moved ||= Math.abs(dx) >= DRAG_THRESHOLD || Math.abs(dy) >= DRAG_THRESHOLD;
        if (interaction.moved && !interaction.dispatchedStart) {
            interaction.dispatchedStart = true;
            window.dispatchEvent(new CustomEvent("node-drag-start"));
        }
        const graphDx = graphPoint.x - interaction.startGraph.x;
        const graphDy = graphPoint.y - interaction.startGraph.y;
        for (const nodeId of interaction.nodeIds || [interaction.nodeId]) {
            const node = getNodeById(nodeId);
            const start = interaction.startPositions?.get(nodeId);
            if (!node || !start) continue;
            node.position = {
                x: start.x + graphDx,
                y: start.y + graphDy,
            };
        }
        scheduleGraphDispatch();
        scheduleDraw();
        return;
    }

    if (interaction.type === "select-box") {
        const dx = event.clientX - interaction.startX;
        const dy = event.clientY - interaction.startY;
        interaction.moved ||= Math.abs(dx) >= DRAG_THRESHOLD || Math.abs(dy) >= DRAG_THRESHOLD;
        interaction.currentGraph = graphPoint;
        scheduleDraw();
        return;
    }

    if (interaction.type === "connect") {
        interaction.current = graphPoint;
        interaction.moved = true;
        scheduleDraw();
    }
}

function handlePointerUp(event) {
    if (!interaction) return;
    const graphPoint = screenToGraph(event.clientX, event.clientY);

    if (interaction.type === "drag-node" && interaction.dispatchedStart) {
        flushGraphDispatch();
        window.dispatchEvent(new CustomEvent("node-drag-stop"));
    }

    if (interaction.type === "connect") {
        const target = hitTestHandle(graphPoint.x, graphPoint.y, "target");
        if (target && target.node.id !== interaction.nodeId) {
            addEdge(interaction.nodeId, target.node.id, interaction.handleId, target.port.id);
        }
    }

    if (interaction.type === "select-box" && interaction.moved) {
        selectNodes(getNodesInSelection(interaction.startGraph, interaction.currentGraph || graphPoint));
    }

    interaction = null;
    canvasRef.value?.releasePointerCapture?.(event.pointerId);
    scheduleDraw();
}

function handlePointerCancel(event) {
    canvasRef.value?.releasePointerCapture?.(event.pointerId);
    if (interaction?.type === "drag-node" && interaction.dispatchedStart) {
        flushGraphDispatch();
        window.dispatchEvent(new CustomEvent("node-drag-stop"));
    }
    interaction = null;
    scheduleDraw();
}

function handlePointerLeave() {
    if (!interaction && canvasRef.value) {
        canvasRef.value.style.cursor = "default";
    }
}

function handleWheel(event) {
    closeContextMenu();
    const rect = canvasRef.value.getBoundingClientRect();
    const localX = event.clientX - rect.left;
    const localY = event.clientY - rect.top;

    if (isLikelyMouseWheel(event) || event.ctrlKey) {
        const before = screenToGraph(event.clientX, event.clientY);
        const zoomFactor = clamp(
            Math.exp(-event.deltaY * WHEEL_ZOOM_SENSITIVITY),
            MIN_WHEEL_ZOOM_FACTOR,
            MAX_WHEEL_ZOOM_FACTOR,
        );
        viewport.zoom = clamp(viewport.zoom * zoomFactor, MIN_ZOOM, MAX_ZOOM);
        viewport.x = localX - before.x * viewport.zoom;
        viewport.y = localY - before.y * viewport.zoom;
    } else {
        viewport.x -= event.deltaX;
        viewport.y -= event.deltaY;
    }
    scheduleDraw();
}

function isLikelyMouseWheel(event) {
    if (event.ctrlKey) return false;
    if (event.deltaMode === 1) return true;
    const absX = Math.abs(event.deltaX || 0);
    const absY = Math.abs(event.deltaY || 0);
    const wheelDeltaY = Math.abs(event.wheelDeltaY || event.wheelDelta || 0);
    return absX < 1 && absY >= 40 && (wheelDeltaY >= 120 || Number.isInteger(event.deltaY));
}

function handleKeyDown(event) {
    if (!containerRef.value?.contains(document.activeElement)) return;
    if (isEditableTarget(document.activeElement)) return;

    if (event.key === "Escape") {
        closeContextMenu();
        clearSelection();
        event.preventDefault();
        return;
    }

    if (event.key !== "Delete" && event.key !== "Backspace") return;
    if (selectedNodeIds.value.size === 0 && !selectedNodeId.value && !selectedEdgeId.value) return;

    if (selectedNodeIds.value.size > 0 || selectedNodeId.value) {
        const nodeIds = selectedNodeIds.value.size > 0
            ? selectedNodeIds.value
            : new Set([selectedNodeId.value]);
        internalNodes.value = internalNodes.value.filter((node) => !nodeIds.has(node.id));
        internalEdges.value = internalEdges.value.filter(
            (edge) => !nodeIds.has(edge.source) && !nodeIds.has(edge.target),
        );
    }

    if (selectedEdgeId.value) {
        internalEdges.value = internalEdges.value.filter((edge) => edge.id !== selectedEdgeId.value);
    }

    clearSelection();
    flushGraphDispatch();
    scheduleDraw();
    event.preventDefault();
}

function handleGlobalPointerDown(event) {
    if (!contextMenu.show) return;
    if (contextMenuRef.value?.contains(event.target)) return;
    if (containerRef.value?.contains(event.target) && event.button === 2) return;
    closeContextMenu();
}

function openContextMenu(event) {
    const point = screenToGraph(event.clientX, event.clientY);
    contextMenu.show = true;
    contextMenu.x = event.clientX;
    contextMenu.y = event.clientY;
    contextMenu.graphX = point.x;
    contextMenu.graphY = point.y;
}

function closeContextMenu() {
    contextMenu.show = false;
}

function createNode(type) {
    const newNode = {
        id: `node-${Date.now()}`,
        type,
        position: { x: contextMenu.graphX, y: contextMenu.graphY },
        data: getDefaultNodeData(type),
    };
    internalNodes.value = [...internalNodes.value, newNode];
    closeContextMenu();
    selectNode(newNode);
    flushGraphDispatch();
    scheduleDraw();
}

function getDefaultNodeData(type) {
    switch (type) {
        case "dialogue": {
            const firstSpeaker = Array.isArray(props.presetSpeakers) ? props.presetSpeakers[0] : null;
            const defaultSpeakerId = typeof firstSpeaker === "string" ? firstSpeaker : firstSpeaker?.id || "";
            return { speakerId: defaultSpeakerId, lines: [{ textKey: "" }] };
        }
        case "choice":
            return {
                choices: [],
                choiceMode: "SinglePass",
                choiceTimer: { enabled: false, durationSeconds: 8, timeoutChoiceTextKey: "超时" },
            };
        case "jump":
            return { targetNodeId: "" };
        case "setvariable":
            return { variableName: "", operation: "Set", value: "" };
        case "emitevent":
            return { eventType: "", params: {} };
        case "condition":
            return { condition: { branches: [{ label: "条件 1", logic: "All", terms: [] }] } };
        case "railstory":
            return { title: "剧情脚本", summary: "", storyId: "" };
        case "railbranch":
            return { title: "路线判断", summary: "", branches: [{ label: "分支 1", logic: "All", terms: [] }] };
        case "railnote":
            return { title: "章节标记", summary: "" };
        case "railend":
            return { title: "总纲结束", summary: "" };
        default:
            return {};
    }
}

function addEdge(source, target, sourceHandle, targetHandle) {
    const newEdge = {
        id: `e${Date.now()}`,
        source,
        sourceHandle: sourceHandle || undefined,
        target,
        targetHandle: targetHandle || undefined,
        type: props.edgeRenderMode === "bezier" ? "default" : "straight",
        animated: false,
        style: "stroke: rgba(168, 85, 247, 0.6); stroke-width: 2px;",
        data: { priority: 0 },
    };
    internalEdges.value = [...internalEdges.value, newEdge];
    selectEdge(newEdge);
    flushGraphDispatch();
}

function selectNode(node) {
    selectedNodeId.value = node?.id || null;
    selectedNodeIds.value = node?.id ? new Set([node.id]) : new Set();
    selectedEdgeId.value = null;
    window.dispatchEvent(new CustomEvent("node-click", { detail: node || null }));
    window.dispatchEvent(new CustomEvent("edge-click", { detail: null }));
    scheduleDraw();
}

function selectNodes(nodes) {
    const selectedNodes = Array.isArray(nodes) ? nodes.filter(Boolean) : [];
    const ids = new Set(selectedNodes.map((node) => node.id).filter(Boolean));
    selectedNodeIds.value = ids;
    selectedNodeId.value = selectedNodes[0]?.id || null;
    selectedEdgeId.value = null;
    window.dispatchEvent(new CustomEvent("node-click", { detail: selectedNodes[0] || null }));
    window.dispatchEvent(new CustomEvent("edge-click", { detail: null }));
    scheduleDraw();
}

function selectEdge(edge) {
    selectedEdgeId.value = edge?.id || null;
    selectedNodeId.value = null;
    selectedNodeIds.value = new Set();
    window.dispatchEvent(new CustomEvent("edge-click", { detail: edge || null }));
    window.dispatchEvent(new CustomEvent("node-click", { detail: null }));
    scheduleDraw();
}

function clearSelection() {
    const hadSelection = selectedNodeIds.value.size > 0 || selectedNodeId.value || selectedEdgeId.value;
    selectedNodeId.value = null;
    selectedNodeIds.value = new Set();
    selectedEdgeId.value = null;
    if (hadSelection) {
        window.dispatchEvent(new CustomEvent("node-click", { detail: null }));
        window.dispatchEvent(new CustomEvent("edge-click", { detail: null }));
    }
    scheduleDraw();
}

function isNodeSelected(nodeId) {
    return selectedNodeIds.value.has(nodeId) || selectedNodeId.value === nodeId;
}

function pruneSelection() {
    if (selectedNodeIds.value.size === 0 && !selectedNodeId.value) return;
    const existingIds = new Set(internalNodes.value.map((node) => node.id));
    const nextIds = new Set([...selectedNodeIds.value].filter((id) => existingIds.has(id)));
    selectedNodeIds.value = nextIds;
    if (!selectedNodeId.value || !existingIds.has(selectedNodeId.value)) {
        selectedNodeId.value = nextIds.values().next().value || null;
    }
}

function getNodePositionMap(nodeIds) {
    const positions = new Map();
    for (const nodeId of nodeIds) {
        const node = getNodeById(nodeId);
        if (!node) continue;
        positions.set(nodeId, {
            x: node.position?.x || 0,
            y: node.position?.y || 0,
        });
    }
    return positions;
}

function scheduleGraphDispatch() {
    if (graphDispatchTimer) return;
    graphDispatchTimer = setTimeout(() => {
        graphDispatchTimer = null;
        dispatchGraphState();
    }, 0);
}

function flushGraphDispatch() {
    if (graphDispatchTimer) {
        clearTimeout(graphDispatchTimer);
        graphDispatchTimer = null;
    }
    dispatchGraphState();
}

function dispatchGraphState() {
    window.dispatchEvent(
        new CustomEvent("graph-state-change", {
            detail: {
                nodes: cloneArray(internalNodes.value),
                edges: cloneArray(internalEdges.value),
            },
        }),
    );
}

function screenToGraph(clientX, clientY) {
    const rect = canvasRef.value?.getBoundingClientRect();
    if (!rect) return { x: clientX, y: clientY };
    return {
        x: (clientX - rect.left - viewport.x) / viewport.zoom,
        y: (clientY - rect.top - viewport.y) / viewport.zoom,
    };
}

function graphToScreen(graphX, graphY) {
    return {
        x: graphX * viewport.zoom + viewport.x,
        y: graphY * viewport.zoom + viewport.y,
    };
}

function hitTestNode(graphX, graphY) {
    for (let index = internalNodes.value.length - 1; index >= 0; index -= 1) {
        const node = internalNodes.value[index];
        const x = node.position?.x || 0;
        const y = node.position?.y || 0;
        const layout = getNodeLayout(node);
        if (graphX >= x && graphX <= x + NODE_WIDTH && graphY >= y && graphY <= y + layout.height) {
            return node;
        }
    }
    return null;
}

function getNodesInSelection(startPoint, endPoint) {
    const rect = normalizeRect(startPoint, endPoint);
    return internalNodes.value.filter((node) => {
        const x = node.position?.x || 0;
        const y = node.position?.y || 0;
        const layout = getNodeLayout(node);
        return rectanglesIntersect(rect, {
            x,
            y,
            width: NODE_WIDTH,
            height: layout.height,
        });
    });
}

function normalizeRect(startPoint, endPoint) {
    const startX = Number(startPoint?.x || 0);
    const startY = Number(startPoint?.y || 0);
    const endX = Number(endPoint?.x || startX);
    const endY = Number(endPoint?.y || startY);
    const x = Math.min(startX, endX);
    const y = Math.min(startY, endY);
    return {
        x,
        y,
        width: Math.abs(endX - startX),
        height: Math.abs(endY - startY),
    };
}

function rectanglesIntersect(a, b) {
    return (
        a.x <= b.x + b.width &&
        a.x + a.width >= b.x &&
        a.y <= b.y + b.height &&
        a.y + a.height >= b.y
    );
}

function hitTestHandle(graphX, graphY, kind = null) {
    for (let index = internalNodes.value.length - 1; index >= 0; index -= 1) {
        const node = internalNodes.value[index];
        for (const port of getNodePorts(node)) {
            if (kind && port.kind !== kind) continue;
            if (distance(graphX, graphY, port.x, port.y) <= HANDLE_RADIUS + 5) {
                return { node, port };
            }
        }
    }
    return null;
}

function hitTestEdge(graphX, graphY) {
    for (let index = internalEdges.value.length - 1; index >= 0; index -= 1) {
        const edge = internalEdges.value[index];
        const sourceNode = getNodeById(edge.source);
        const targetNode = getNodeById(edge.target);
        if (!sourceNode || !targetNode) continue;
        const source = getPortPoint(sourceNode, "source", edge.sourceHandle);
        const target = getPortPoint(targetNode, "target", edge.targetHandle);
        if (pointToSegmentDistance(graphX, graphY, source.x, source.y, target.x, target.y) <= EDGE_HIT_RADIUS / viewport.zoom) {
            return edge;
        }
    }
    return null;
}

function getCursorForPoint(point) {
    const handle = hitTestHandle(point.x, point.y);
    if (handle?.port.kind === "source") return "crosshair";
    if (handle) return "default";
    if (hitTestNode(point.x, point.y)) return "grab";
    if (hitTestEdge(point.x, point.y)) return "pointer";
    return "default";
}

function getNodeById(id) {
    return internalNodes.value.find((node) => node.id === id);
}

function fitView() {
    const canvas = canvasRef.value;
    if (!canvas || internalNodes.value.length === 0) {
        hasFitInitialView = true;
        scheduleDraw();
        return;
    }

    const dpr = window.devicePixelRatio || 1;
    const canvasWidth = canvas.width / dpr;
    const canvasHeight = canvas.height / dpr;
    const bounds = getGraphBounds();
    const padding = 72;
    const zoom = clamp(
        Math.min(
            (canvasWidth - padding * 2) / Math.max(bounds.width, 1),
            (canvasHeight - padding * 2) / Math.max(bounds.height, 1),
        ),
        MIN_ZOOM,
        1.15,
    );

    viewport.zoom = Number.isFinite(zoom) ? zoom : 1;
    viewport.x = canvasWidth / 2 - (bounds.x + bounds.width / 2) * viewport.zoom;
    viewport.y = canvasHeight / 2 - (bounds.y + bounds.height / 2) * viewport.zoom;
    hasFitInitialView = true;
    scheduleDraw();
}

function getGraphBounds() {
    const boxes = internalNodes.value.map((node) => {
        const layout = getNodeLayout(node);
        return {
            x: node.position?.x || 0,
            y: node.position?.y || 0,
            width: NODE_WIDTH,
            height: layout.height,
        };
    });

    const minX = Math.min(...boxes.map((box) => box.x));
    const minY = Math.min(...boxes.map((box) => box.y));
    const maxX = Math.max(...boxes.map((box) => box.x + box.width));
    const maxY = Math.max(...boxes.map((box) => box.y + box.height));
    return { x: minX, y: minY, width: maxX - minX, height: maxY - minY };
}

function zoomBy(factor) {
    const canvas = canvasRef.value;
    if (!canvas) return;
    const dpr = window.devicePixelRatio || 1;
    const centerX = canvas.width / dpr / 2;
    const centerY = canvas.height / dpr / 2;
    const before = {
        x: (centerX - viewport.x) / viewport.zoom,
        y: (centerY - viewport.y) / viewport.zoom,
    };
    viewport.zoom = clamp(viewport.zoom * factor, MIN_ZOOM, MAX_ZOOM);
    viewport.x = centerX - before.x * viewport.zoom;
    viewport.y = centerY - before.y * viewport.zoom;
    scheduleDraw();
}

function roundedRect(x, y, width, height, radius) {
    const r = Math.min(radius, width / 2, height / 2);
    ctx.beginPath();
    ctx.moveTo(x + r, y);
    ctx.arcTo(x + width, y, x + width, y + height, r);
    ctx.arcTo(x + width, y + height, x, y + height, r);
    ctx.arcTo(x, y + height, x, y, r);
    ctx.arcTo(x, y, x + width, y, r);
    ctx.closePath();
}

function getRowFont(row) {
    if (row.kind === "label") {
        return "700 10px Outfit, Noto Sans SC, sans-serif";
    }
    return row.strong
        ? "700 12px Outfit, Noto Sans SC, sans-serif"
        : "600 12px Outfit, Noto Sans SC, sans-serif";
}

function getCanvasTheme() {
    if (themeIsDark) {
        return {
            nodeFill: "rgba(15, 23, 42, 0.78)",
            nodeBorder: "rgba(148, 163, 184, 0.24)",
            nodeShadow: "rgba(0, 0, 0, 0.28)",
            selectedShadowAlpha: 0.2,
            selectedHaloAlpha: 0.34,
            headerTintAlpha: 0.26,
            headerTintMidAlpha: 0.15,
            rowSeparator: "rgba(148, 163, 184, 0.12)",
            titleText: "#e5e7eb",
            strongText: "#f8fafc",
            bodyText: "#dbe4f0",
            mutedText: "#9aa7b8",
            handleFill: "rgba(15, 23, 42, 0.94)",
            targetHandle: "#8e9bb0",
        };
    }

    return {
        nodeFill: "rgba(255, 255, 255, 0.72)",
        nodeBorder: "rgba(255, 255, 255, 0.64)",
        nodeShadow: "rgba(82, 65, 90, 0.1)",
        selectedShadowAlpha: 0.18,
        selectedHaloAlpha: 0.3,
        headerTintAlpha: 0.24,
        headerTintMidAlpha: 0.13,
        rowSeparator: "rgba(82, 65, 90, 0.1)",
        titleText: "#2e3132",
        strongText: "#191c1d",
        bodyText: "#303437",
        mutedText: "#7a7075",
        handleFill: "rgba(255, 255, 255, 0.88)",
        targetHandle: "#8b8491",
    };
}

function wrapTextToWidth(value, maxWidth, font) {
    const text = String(value ?? "").trim();
    if (!text) return [""];

    const words = splitTextForWrap(text);
    const lines = [];
    let current = "";

    for (const word of words) {
        if (word === "\n") {
            if (current) lines.push(current);
            current = "";
            continue;
        }

        const next = current ? `${current}${word}` : word;
        if (!current || measureTextWidth(next, font) <= maxWidth) {
            current = next;
            continue;
        }

        if (current) lines.push(current);

        if (measureTextWidth(word, font) <= maxWidth) {
            current = word;
        } else {
            const broken = breakLongToken(word, maxWidth, font);
            lines.push(...broken.slice(0, -1));
            current = broken[broken.length - 1] || "";
        }
    }

    if (current) lines.push(current);
    return lines.length ? lines : [""];
}

function splitTextForWrap(text) {
    const tokens = [];
    let buffer = "";

    for (const char of text) {
        if (char === "\n") {
            if (buffer) tokens.push(buffer);
            tokens.push("\n");
            buffer = "";
            continue;
        }

        if (/\s/.test(char)) {
            buffer += char;
            tokens.push(buffer);
            buffer = "";
            continue;
        }

        if (isCjkChar(char)) {
            if (buffer) tokens.push(buffer);
            tokens.push(char);
            buffer = "";
            continue;
        }

        buffer += char;
    }

    if (buffer) tokens.push(buffer);
    return tokens;
}

function breakLongToken(token, maxWidth, font) {
    const lines = [];
    let current = "";

    for (const char of String(token)) {
        const next = current + char;
        if (!current || measureTextWidth(next, font) <= maxWidth) {
            current = next;
        } else {
            lines.push(current);
            current = char;
        }
    }

    if (current) lines.push(current);
    return lines;
}

function measureTextWidth(text, font) {
    if (ctx) {
        ctx.save();
        ctx.font = font;
        const width = ctx.measureText(String(text)).width;
        ctx.restore();
        return width;
    }

    return String(text).length * 7;
}

function isCjkChar(char) {
    return /[\u3400-\u9fff\uac00-\ud7af\u3040-\u30ff]/.test(char);
}

function clamp(value, min, max) {
    return Math.min(max, Math.max(min, value));
}

function distance(x1, y1, x2, y2) {
    return Math.hypot(x1 - x2, y1 - y2);
}

function pointToSegmentDistance(px, py, x1, y1, x2, y2) {
    const dx = x2 - x1;
    const dy = y2 - y1;
    if (dx === 0 && dy === 0) return distance(px, py, x1, y1);
    const t = clamp(((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy), 0, 1);
    return distance(px, py, x1 + t * dx, y1 + t * dy);
}

function hexToRgba(hex, alpha) {
    const normalized = hex.replace("#", "");
    const r = parseInt(normalized.slice(0, 2), 16);
    const g = parseInt(normalized.slice(2, 4), 16);
    const b = parseInt(normalized.slice(4, 6), 16);
    return `rgba(${r}, ${g}, ${b}, ${alpha})`;
}

function getNodeMeta(type) {
    return NODE_META[type] || { label: type || "节点", icon: "help", color: "#64748b" };
}

function isEditableTarget(target) {
    return (
        target?.tagName === "INPUT" ||
        target?.tagName === "TEXTAREA" ||
        target?.isContentEditable
    );
}
</script>

<style scoped>
.canvas-graph-editor {
    position: relative;
    width: 100%;
    height: 100%;
    overflow: hidden;
    outline: none;
    touch-action: none;
}

.canvas-graph-surface {
    display: block;
    width: 100%;
    height: 100%;
    cursor: default;
}

.canvas-controls {
    position: absolute;
    right: 16px;
    bottom: 18px;
    display: flex;
    flex-direction: column;
    gap: 6px;
    padding: 6px;
    border-radius: 12px;
}

.canvas-control-btn {
    width: 30px;
    height: 30px;
    border: 0.5px solid rgba(255, 255, 255, 0.36);
    border-radius: 9px;
    background: rgba(255, 255, 255, 0.38);
    color: #2e3132;
    font-size: 15px;
    font-weight: 700;
    line-height: 1;
    cursor: pointer;
    box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.35);
}

.canvas-control-btn:hover {
    background: rgba(255, 255, 255, 0.56);
}

.context-menu {
    position: fixed;
    z-index: 1000;
    min-width: 178px;
    padding: 8px;
    border-radius: 14px;
}

.context-menu-header {
    padding: 7px 9px 8px;
    color: #7a7075;
    font-size: 11px;
    font-weight: 700;
    letter-spacing: 0;
}

.context-menu-item {
    width: 100%;
    min-height: 34px;
    display: flex;
    align-items: center;
    gap: 9px;
    border: 0.5px solid transparent;
    border-radius: 10px;
    padding: 8px 10px;
    background: transparent;
    color: #2e3132;
    font-size: 13px;
    font-weight: 600;
    text-align: left;
    cursor: pointer;
}

.context-menu-item:hover {
    background: rgba(255, 255, 255, 0.34);
    border-color: rgba(255, 255, 255, 0.38);
}

.context-menu-item .icon-glyph {
    width: 16px;
    height: 16px;
    color: #8b7aa8;
}

:global(body.dark-theme) .canvas-control-btn,
:global(body[data-theme="dark"]) .canvas-control-btn {
    background: rgba(15, 23, 42, 0.78);
    color: #e5e7eb;
    border-color: rgba(148, 163, 184, 0.24);
}

:global(body.dark-theme) .canvas-control-btn:hover,
:global(body[data-theme="dark"]) .canvas-control-btn:hover {
    background: rgba(30, 41, 59, 0.92);
}

:global(body.dark-theme) .context-menu-item,
:global(body[data-theme="dark"]) .context-menu-item {
    color: #e5e7eb;
}

:global(body.dark-theme) .context-menu-item:hover,
:global(body[data-theme="dark"]) .context-menu-item:hover {
    background: rgba(148, 163, 184, 0.16);
    border-color: rgba(148, 163, 184, 0.22);
}

:global(body.dark-theme) .context-menu-header,
:global(body[data-theme="dark"]) .context-menu-header {
    color: #9aa7b8;
}

:global(body.dark-theme) .context-menu-item .icon-glyph,
:global(body[data-theme="dark"]) .context-menu-item .icon-glyph {
    color: #b8a8d5;
}
</style>
