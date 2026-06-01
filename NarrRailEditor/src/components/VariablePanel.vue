<template>
    <div ref="panelWrapperRef" class="variable-panel-wrapper">
        <div class="panel-hotzone" @mouseenter="handleMouseEnter"></div>

        <div class="panel-trigger" :class="{ hidden: isExpanded }">
            <IconGlyph name="chevron_right" />
        </div>

        <div
            class="variable-panel glass-morphism-strong"
            :class="{ expanded: isExpanded }"
            @mouseleave="handleMouseLeave"
            @focusin="handlePanelFocusIn"
            @focusout="handlePanelFocusOut"
            @compositionstart="handleCompositionStart"
            @compositionend="handleCompositionEnd"
        >
            <div class="variable-panel-header">
                <h2 class="panel-title">变量管理</h2>
                <p class="panel-subtitle">Variables</p>
            </div>

            <div class="variable-panel-content">
                <div class="section-title">预设角色</div>

                <div v-if="presetSpeakers.length > 0" class="speaker-list">
                    <div
                        v-for="(speaker, index) in presetSpeakers"
                        :key="index"
                        class="speaker-item glass-input"
                    >
                        <div class="speaker-header">
                            <span class="speaker-name">{{
                                formatSpeakerLabel(speaker)
                            }}</span>
                            <button
                                class="delete-btn"
                                @click="removeSpeaker(index)"
                            >
                                <IconGlyph name="delete" />
                            </button>
                        </div>
                    </div>
                </div>
                <div v-else class="speaker-panel-empty">
                    <IconGlyph name="record_voice_over" class="empty-icon" />
                    <p>暂无预设角色</p>
                </div>

                <div class="add-speaker-section">
                    <button
                        class="add-speaker-btn"
                        @click="showAddSpeakerForm = !showAddSpeakerForm"
                    >
                        <IconGlyph name="person_add" />
                        <span>添加预设角色</span>
                    </button>

                    <div v-if="showAddSpeakerForm" class="add-form glass-input">
                        <div class="form-group">
                            <label class="form-label">角色 ID</label>
                            <input
                                type="text"
                                class="form-input"
                                v-model="newSpeaker.id"
                                placeholder="speakerId"
                            />
                        </div>

                        <div class="form-group">
                            <label class="form-label">显示名（可选）</label>
                            <input
                                type="text"
                                class="form-input"
                                v-model="newSpeaker.displayName"
                                placeholder="显示名，可留空"
                            />
                        </div>

                        <div class="form-actions">
                            <button class="confirm-btn" @click="addSpeaker">
                                <IconGlyph name="check" />
                                <span>确认</span>
                            </button>
                            <button
                                class="cancel-btn"
                                @click="cancelAddSpeaker"
                            >
                                <IconGlyph name="close" />
                                <span>取消</span>
                            </button>
                        </div>
                    </div>
                </div>

                <div class="section-divider"></div>
                <div class="section-title">变量管理</div>

                <div v-if="variables.length > 0" class="variable-list">
                    <div
                        v-for="(variable, index) in variables"
                        :key="index"
                        class="variable-item glass-input"
                    >
                        <div class="variable-header">
                            <span class="variable-name">{{
                                variable.name
                            }}</span>
                            <button
                                class="delete-btn"
                                @click="removeVariable(index)"
                            >
                                <IconGlyph name="delete" />
                            </button>
                        </div>
                        <div class="variable-details">
                            <span class="variable-type">{{
                                variable.type
                            }}</span>
                            <span class="variable-value">{{
                                formatValue(variable)
                            }}</span>
                        </div>
                    </div>
                </div>
                <div v-else class="variable-panel-empty">
                    <IconGlyph name="data_object" class="empty-icon" />
                    <p>暂无变量</p>
                </div>

                <div class="add-variable-section">
                    <button
                        class="add-variable-btn"
                        @click="showAddForm = !showAddForm"
                    >
                        <IconGlyph name="add" />
                        <span>添加变量</span>
                    </button>

                    <div v-if="showAddForm" class="add-form glass-input">
                        <div class="form-group">
                            <label class="form-label">变量名</label>
                            <input
                                type="text"
                                class="form-input"
                                v-model="newVariable.name"
                                placeholder="variableName"
                            />
                        </div>

                        <div class="form-group">
                            <label class="form-label">类型</label>
                            <select
                                class="form-input"
                                v-model="newVariable.type"
                            >
                                <option value="Bool">Bool</option>
                                <option value="Int">Int</option>
                                <option value="Float">Float</option>
                                <option value="String">String</option>
                            </select>
                        </div>

                        <div class="form-group">
                            <label class="form-label">初始值</label>
                            <input
                                v-if="newVariable.type === 'Bool'"
                                type="checkbox"
                                class="form-checkbox"
                                v-model="newVariable.boolValue"
                            />
                            <input
                                v-else-if="newVariable.type === 'Int'"
                                type="number"
                                class="form-input"
                                v-model.number="newVariable.intValue"
                            />
                            <input
                                v-else-if="newVariable.type === 'Float'"
                                type="number"
                                step="0.01"
                                class="form-input"
                                v-model.number="newVariable.floatValue"
                            />
                            <input
                                v-else
                                type="text"
                                class="form-input"
                                v-model="newVariable.stringValue"
                            />
                        </div>

                        <div class="form-actions">
                            <button class="confirm-btn" @click="addVariable">
                                <IconGlyph name="check" />
                                <span>确认</span>
                            </button>
                            <button class="cancel-btn" @click="cancelAdd">
                                <IconGlyph name="close" />
                                <span>取消</span>
                            </button>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>

<script setup>
import { ref, reactive, onMounted, onUnmounted } from "vue";
import {
    formatSpeakerLabel,
    validateSpeakerId,
    validateVariableName,
} from "../utils/editor-form-utils.js";

const props = defineProps({
    variables: {
        type: Array,
        default: () => [],
    },
    presetSpeakers: {
        type: Array,
        default: () => [],
    },
});

const emit = defineEmits(["update", "update-speakers"]);

const panelWrapperRef = ref(null);
const isExpanded = ref(false);
const showAddForm = ref(false);
const showAddSpeakerForm = ref(false);
const isComposing = ref(false);
const isPanelInputFocused = ref(false);
const newSpeaker = reactive({
    id: "",
    displayName: "",
});
const newVariable = reactive({
    name: "",
    type: "Bool",
    boolValue: false,
    intValue: 0,
    floatValue: 0.0,
    stringValue: "",
});

function handleMouseEnter() {
    isExpanded.value = true;
}

function handlePanelFocusIn() {
    isPanelInputFocused.value = true;
    isExpanded.value = true;
}

function handlePanelFocusOut(event) {
    const nextFocused = event?.relatedTarget;
    const panelEl = event?.currentTarget;
    isPanelInputFocused.value = !!(
        nextFocused &&
        panelEl &&
        typeof panelEl.contains === "function" &&
        panelEl.contains(nextFocused)
    );
}

function handleCompositionStart() {
    isComposing.value = true;
}

function handleCompositionEnd() {
    isComposing.value = false;
}

function handleMouseLeave() {
    if (isComposing.value || isPanelInputFocused.value) return;
    isExpanded.value = false;
}

function handleGlobalPointerDown(event) {
    const panelEl = panelWrapperRef.value;
    const target = event?.target;
    if (!panelEl || !target) return;

    if (typeof panelEl.contains === "function" && panelEl.contains(target)) {
        return;
    }

    if (isComposing.value) return;

    isPanelInputFocused.value = false;
    isExpanded.value = false;
}

function addSpeaker() {
    const speakerValidation = validateSpeakerId(props.presetSpeakers, newSpeaker.id);
    if (!speakerValidation.ok) {
        alert(speakerValidation.message);
        return;
    }

    const id = speakerValidation.value;
    const displayName = String(newSpeaker.displayName || "").trim();

    const nextSpeaker = displayName ? { id, displayName } : { id };
    const updated = [...props.presetSpeakers, nextSpeaker];
    emit("update-speakers", updated);
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

function cancelAddSpeaker() {
    resetSpeakerForm();
}

function addVariable() {
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

    switch (newVariable.type) {
        case "Bool":
            variable.boolValue = newVariable.boolValue;
            break;
        case "Int":
            variable.intValue = newVariable.intValue;
            break;
        case "Float":
            variable.floatValue = newVariable.floatValue;
            break;
        case "String":
            variable.stringValue = newVariable.stringValue;
            break;
    }

    const updated = [...props.variables, variable];
    emit("update", updated);

    resetForm();
}

function removeVariable(index) {
    const updated = [...props.variables];
    updated.splice(index, 1);
    emit("update", updated);
}

function resetForm() {
    newVariable.name = "";
    newVariable.type = "Bool";
    newVariable.boolValue = false;
    newVariable.intValue = 0;
    newVariable.floatValue = 0.0;
    newVariable.stringValue = "";
    showAddForm.value = false;
}

function cancelAdd() {
    resetForm();
}

function formatValue(variable) {
    switch (variable.type) {
        case "Bool":
            return variable.boolValue ? "true" : "false";
        case "Int":
            return variable.intValue;
        case "Float":
            return variable.floatValue.toFixed(2);
        case "String":
            return `"${variable.stringValue}"`;
        default:
            return "";
    }
}

onMounted(() => {
    window.addEventListener("pointerdown", handleGlobalPointerDown, true);
});

onUnmounted(() => {
    window.removeEventListener("pointerdown", handleGlobalPointerDown, true);
});
</script>

<style scoped>
.variable-panel-wrapper {
    position: fixed;
    left: 0;
    top: 0;
    bottom: 0;
    z-index: 30;
    pointer-events: none;
}

.panel-hotzone {
    position: absolute;
    left: 0;
    top: 0;
    bottom: 0;
    width: 48px;
    pointer-events: auto;
    cursor: pointer;
}

.panel-trigger {
    position: absolute;
    left: 0;
    top: 50%;
    transform: translateY(-50%);
    width: 48px;
    height: 96px;
    display: flex;
    align-items: center;
    justify-content: center;
    background: rgba(255, 255, 255, 0.2);
    backdrop-filter: blur(16px);
    border-radius: 0 16px 16px 0;
    border: 1px solid rgba(255, 255, 255, 0.6);
    border-left: none;
    cursor: pointer;
    transition: all 0.3s ease;
    box-shadow: 4px 0 16px rgba(0, 0, 0, 0.05);
    pointer-events: none;
}

.panel-trigger.hidden {
    opacity: 0;
}

.panel-hotzone:hover ~ .panel-trigger:not(.hidden) {
    background: rgba(255, 255, 255, 0.3);
    width: 56px;
}

.panel-trigger :deep(.icon-glyph) {
    font-size: 28px;
    color: #af52de;
    animation: pulse-arrow 2s ease-in-out infinite;
}

@keyframes pulse-arrow {
    0%,
    100% {
        transform: translateX(0);
    }
    50% {
        transform: translateX(4px);
    }
}

.variable-panel {
    position: absolute;
    left: 16px;
    top: 88px;
    bottom: 92px;
    width: 320px;
    display: flex;
    flex-direction: column;
    overflow: hidden;
    border-radius: 16px;
    transform: translateX(calc(-100% - 16px));
    transition: transform 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    pointer-events: none;
}

.variable-panel.expanded {
    transform: translateX(0);
    pointer-events: auto;
}

.variable-panel-header {
    padding: 24px;
    border-bottom: 1px solid rgba(255, 255, 255, 0.3);
}

.panel-title {
    font-family: "Outfit", sans-serif;
    font-size: 24px;
    font-weight: 700;
    color: #af52de;
    margin-bottom: 4px;
}

.panel-subtitle {
    font-size: 12px;
    font-weight: 600;
    color: #94a3b8;
    text-transform: uppercase;
    letter-spacing: 0.05em;
}

.variable-panel-content {
    flex: 1;
    padding: 24px;
    overflow-y: auto;
    display: flex;
    flex-direction: column;
    gap: 16px;
}

.variable-panel-empty {
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    flex: 1;
    color: #94a3b8;
    text-align: center;
    gap: 16px;
}

.empty-icon {
    font-size: 64px;
    opacity: 0.3;
}

.section-title {
    font-size: 11px;
    font-weight: 800;
    color: #7e22ce;
    text-transform: uppercase;
    letter-spacing: 0.08em;
    font-family: "Outfit", sans-serif;
}

.section-divider {
    height: 1px;
    background: rgba(148, 163, 184, 0.25);
    margin: 2px 0;
}

.speaker-list,
.variable-list {
    display: flex;
    flex-direction: column;
    gap: 12px;
}

.speaker-item,
.variable-item {
    background: rgba(175, 82, 222, 0.15);
    backdrop-filter: blur(16px);
    padding: 16px;
    border-radius: 12px;
    border: 1px solid rgba(255, 255, 255, 0.6);
}

.speaker-header,
.variable-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 8px;
}

.speaker-name,
.variable-name {
    font-family: "Outfit", sans-serif;
    font-size: 14px;
    font-weight: 700;
    color: #1e293b;
}

.speaker-panel-empty {
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    color: #94a3b8;
    text-align: center;
    gap: 12px;
    padding: 8px 0 4px;
}

.delete-btn {
    width: 28px;
    height: 28px;
    display: flex;
    align-items: center;
    justify-content: center;
    background: rgba(239, 68, 68, 0.1);
    border: none;
    border-radius: 6px;
    cursor: pointer;
    transition: all 0.2s ease;
}

.delete-btn:hover {
    background: rgba(239, 68, 68, 0.2);
}

.delete-btn :deep(.icon-glyph) {
    font-size: 18px;
    color: #ef4444;
}

.variable-details {
    display: flex;
    justify-content: space-between;
    align-items: center;
}

.variable-type {
    font-size: 11px;
    font-weight: 700;
    color: #af52de;
    text-transform: uppercase;
    letter-spacing: 0.05em;
}

.variable-value {
    font-size: 13px;
    font-weight: 600;
    color: #1e293b;
}

.add-speaker-section,
.add-variable-section {
    display: flex;
    flex-direction: column;
    gap: 12px;
}

.add-speaker-btn,
.add-variable-btn {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 8px;
    width: 100%;
    padding: 12px;
    background: linear-gradient(
        135deg,
        rgba(175, 82, 222, 0.8),
        rgba(168, 85, 247, 0.8)
    );
    color: white;
    border: 1px solid rgba(255, 255, 255, 0.4);
    border-radius: 12px;
    cursor: pointer;
    font-size: 14px;
    font-weight: 700;
    font-family: "Outfit", sans-serif;
    transition: all 0.2s ease;
}

.add-speaker-btn:hover,
.add-variable-btn:hover {
    transform: scale(1.02);
}

.add-speaker-btn :deep(.icon-glyph),
.add-variable-btn :deep(.icon-glyph) {
    font-size: 20px;
}

.add-form {
    background: rgba(175, 82, 222, 0.15);
    backdrop-filter: blur(16px);
    padding: 16px;
    border-radius: 12px;
    border: 1px solid rgba(255, 255, 255, 0.6);
    display: flex;
    flex-direction: column;
    gap: 12px;
}

.form-group {
    display: flex;
    flex-direction: column;
    gap: 6px;
}

.form-label {
    font-size: 10px;
    font-weight: 800;
    color: #af52de;
    text-transform: uppercase;
    letter-spacing: 0.1em;
    font-family: "Outfit", sans-serif;
}

.form-input {
    width: 100%;
    padding: 10px 12px;
    background: rgba(255, 255, 255, 0.4);
    border: none;
    border-radius: 8px;
    font-size: 13px;
    font-weight: 600;
    color: #1e293b;
    font-family: "Noto Sans SC", "Outfit", sans-serif;
    transition: all 0.2s ease;
}

.form-input:focus {
    outline: none;
    background: rgba(255, 255, 255, 0.6);
    box-shadow: 0 0 0 2px rgba(175, 82, 222, 0.3);
}

.form-checkbox {
    width: 20px;
    height: 20px;
    cursor: pointer;
}

.form-actions {
    display: flex;
    gap: 8px;
    margin-top: 8px;
}

.confirm-btn,
.cancel-btn {
    flex: 1;
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 6px;
    padding: 10px;
    border: none;
    border-radius: 8px;
    cursor: pointer;
    font-size: 13px;
    font-weight: 700;
    transition: all 0.2s ease;
}

.confirm-btn {
    background: rgba(34, 197, 94, 0.8);
    color: white;
}

.confirm-btn:hover {
    background: rgba(34, 197, 94, 1);
}

.cancel-btn {
    background: rgba(148, 163, 184, 0.3);
    color: #1e293b;
}

.cancel-btn:hover {
    background: rgba(148, 163, 184, 0.5);
}

.confirm-btn :deep(.icon-glyph),
.cancel-btn :deep(.icon-glyph) {
    font-size: 18px;
}
</style>
