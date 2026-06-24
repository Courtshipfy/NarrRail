<template>
    <div ref="panelWrapperRef" class="property-panel-wrapper">
        <!-- 展开的面板 -->
        <div
            class="property-panel glass-morphism-strong"
            :class="{ expanded: isExpanded }"
            @focusin="handlePanelFocusIn"
            @focusout="handlePanelFocusOut"
            @compositionstart="handleCompositionStart"
            @compositionend="handleCompositionEnd"
        >
            <div class="property-panel-header">
                <h2 class="panel-title">属性面板</h2>
                <p class="panel-subtitle">Inspector</p>
            </div>

            <div class="property-panel-content">
                <div v-if="localNode" class="property-form">
                    <div class="form-group glass-input">
                        <label class="form-label">节点 ID</label>
                        <input
                            type="text"
                            class="form-input"
                            v-model="localNode.id"
                            @compositionstart="handleCompositionStart"
                            @compositionend="handleCompositionEnd"
                            @blur="handleInputChange"
                        />
                    </div>

                    <div class="form-group glass-input">
                        <label class="form-label">节点类型</label>
                        <input
                            type="text"
                            class="form-input"
                            :value="localNode.type"
                            readonly
                        />
                    </div>

                    <template v-if="localNode.type === 'railstory'">
                        <div class="form-group glass-input">
                            <label class="form-label">标题</label>
                            <input
                                type="text"
                                class="form-input"
                                v-model="localNode.data.title"
                                @compositionstart="handleCompositionStart"
                                @compositionend="handleCompositionEnd"
                                @blur="handleInputChange"
                            />
                        </div>

                        <div class="form-group glass-input">
                            <label class="form-label">引用脚本</label>
                            <select
                                class="form-input"
                                v-model="localNode.data.storyId"
                                @change="handleUpdate"
                            >
                                <option value="">（选择 storyId）</option>
                                <option
                                    v-for="story in storyEntries"
                                    :key="`rail-story-${story.storyId}`"
                                    :value="story.storyId"
                                >
                                    {{ story.storyId }}
                                </option>
                            </select>
                            <input
                                type="text"
                                class="form-input"
                                v-model="localNode.data.storyId"
                                placeholder="或手动输入 storyId"
                                @compositionstart="handleCompositionStart"
                                @compositionend="handleCompositionEnd"
                                @blur="handleInputChange"
                            />
                        </div>

                        <div class="form-group glass-input">
                            <label class="form-label">摘要</label>
                            <textarea
                                class="form-textarea"
                                v-model="localNode.data.summary"
                                @compositionstart="handleCompositionStart"
                                @compositionend="handleCompositionEnd"
                                @blur="handleInputChange"
                            ></textarea>
                        </div>
                    </template>

                    <template v-else-if="localNode.type === 'railnote' || localNode.type === 'railend'">
                        <div class="form-group glass-input">
                            <label class="form-label">标题</label>
                            <input
                                type="text"
                                class="form-input"
                                v-model="localNode.data.title"
                                @compositionstart="handleCompositionStart"
                                @compositionend="handleCompositionEnd"
                                @blur="handleInputChange"
                            />
                        </div>
                        <div class="form-group glass-input">
                            <label class="form-label">摘要</label>
                            <textarea
                                class="form-textarea"
                                v-model="localNode.data.summary"
                                @compositionstart="handleCompositionStart"
                                @compositionend="handleCompositionEnd"
                                @blur="handleInputChange"
                            ></textarea>
                        </div>
                    </template>

                    <template v-else-if="localNode.type === 'railbranch'">
                        <div class="form-group glass-input">
                            <label class="form-label">标题</label>
                            <input
                                type="text"
                                class="form-input"
                                v-model="localNode.data.title"
                                @compositionstart="handleCompositionStart"
                                @compositionend="handleCompositionEnd"
                                @blur="handleInputChange"
                            />
                        </div>
                        <div class="form-group glass-input">
                            <label class="form-label">摘要</label>
                            <textarea
                                class="form-textarea"
                                v-model="localNode.data.summary"
                                @compositionstart="handleCompositionStart"
                                @compositionend="handleCompositionEnd"
                                @blur="handleInputChange"
                            ></textarea>
                        </div>

                        <div class="form-group glass-input">
                            <label class="form-label">路线分支</label>
                            <div
                                v-for="(branch, branchIndex) in localNode.data.branches"
                                :key="`rail-branch-${branchIndex}`"
                                class="condition-branch"
                            >
                                <div class="condition-branch-header">
                                    <input
                                        type="text"
                                        class="form-input"
                                        v-model="branch.label"
                                        placeholder="分支标签"
                                        @compositionstart="handleCompositionStart"
                                        @compositionend="handleCompositionEnd"
                                        @blur="handleInputChange"
                                    />
                                    <select
                                        class="form-input condition-logic-select"
                                        v-model="branch.logic"
                                        @change="handleUpdate"
                                    >
                                        <option value="All">All</option>
                                        <option value="Any">Any</option>
                                    </select>
                                    <button
                                        class="remove-choice-btn"
                                        @click="removeRailBranch(branchIndex)"
                                    >
                                        <IconGlyph name="close" />
                                    </button>
                                </div>

                                <div
                                    v-for="(term, termIndex) in branch.terms"
                                    :key="`rail-branch-term-${branchIndex}-${termIndex}`"
                                    class="choice-item"
                                >
                                    <select
                                        class="form-input"
                                        :value="term.variable?.name || ''"
                                        @change="
                                            updateRailTermVariable(
                                                branchIndex,
                                                termIndex,
                                                $event.target.value,
                                            )
                                        "
                                    >
                                        <option value="">（选择变量）</option>
                                        <option
                                            v-for="variable in availableVariables"
                                            :key="`rail-condition-var-${variable.name}`"
                                            :value="variable.name"
                                        >
                                            {{ formatVariableOption(variable) }}
                                        </option>
                                    </select>
                                    <select
                                        class="form-input"
                                        v-model="term.operator"
                                        @change="handleUpdate"
                                    >
                                        <option value="==">==</option>
                                        <option value="!=">!=</option>
                                        <option value=">">&gt;</option>
                                        <option value=">=">&gt;=</option>
                                        <option value="<">&lt;</option>
                                        <option value="<=">&lt;=</option>
                                    </select>
                                    <input
                                        type="text"
                                        class="form-input"
                                        v-model="term.compareValue"
                                        placeholder="比较值"
                                        @compositionstart="handleCompositionStart"
                                        @compositionend="handleCompositionEnd"
                                        @blur="handleInputChange"
                                    />
                                    <button
                                        class="remove-choice-btn"
                                        @click="removeRailTerm(branchIndex, termIndex)"
                                    >
                                        <IconGlyph name="close" />
                                    </button>
                                </div>

                                <button
                                    class="add-choice-btn condition-term-add"
                                    @click="addRailTerm(branchIndex)"
                                >
                                    <IconGlyph name="add" />
                                    <span>添加条件项</span>
                                </button>
                                <p class="choice-hint">
                                    输出句柄：branch-{{ branchIndex }}
                                </p>
                            </div>

                            <button class="add-choice-btn" @click="addRailBranch">
                                <IconGlyph name="add" />
                                <span>添加路线分支</span>
                            </button>
                            <p class="choice-hint">
                                未匹配时走 branch-fallback（可选）
                            </p>
                        </div>
                    </template>

                    <template v-else-if="localNode.type === 'dialogue'">
                        <div class="form-group glass-input">
                            <label class="form-label">角色 ID</label>
                            <select
                                class="form-input"
                                v-model="localNode.data.speakerId"
                                @change="handleUpdate"
                            >
                                <option value="">（旁白）</option>
                                <option
                                    v-for="(speaker, index) in presetSpeakers"
                                    :key="`preset-speaker-${index}`"
                                    :value="getSpeakerId(speaker)"
                                >
                                    {{ formatPresetSpeakerLabel(speaker) }}
                                </option>
                            </select>
                            <input
                                type="text"
                                class="form-input"
                                v-model="localNode.data.speakerId"
                                placeholder="可手动输入自定义角色（留空为旁白）"
                                @compositionstart="handleCompositionStart"
                                @compositionend="handleCompositionEnd"
                                @blur="handleInputChange"
                            />
                        </div>

                        <div class="form-group glass-input">
                            <div class="multi-lines-header">
                                <label class="form-label"
                                    >对话台词（Enter 新增下一行）</label
                                >
                                <button
                                    class="open-dialogue-modal-btn"
                                    @click="openMultiDialogueModal"
                                    title="全屏编辑对话"
                                    aria-label="全屏编辑对话"
                                >
                                    <IconGlyph name="open_in_full" />
                                </button>
                            </div>
                            <div
                                class="multi-lines-editor"
                                @dragover.prevent="
                                    onDialogueContainerDragOver($event)
                                "
                                @drop.prevent="onDialogueContainerDrop($event)"
                            >
                                <div
                                    v-for="(line, index) in localNode.data
                                        .lines"
                                    :key="`line-${index}`"
                                    class="line-edit-row"
                                    :class="{
                                        'is-editing':
                                            activeMultiDialogueLineIndex ===
                                            index,
                                        'is-dragging':
                                            draggingDialogueLineIndex === index,
                                        'is-drag-over':
                                            isDialogueLineDragOver(index),
                                        'drop-before':
                                            isDialogueLineDropBefore(index),
                                        'drop-after':
                                            isDialogueLineDropAfter(index),
                                    }"
                                    @dragenter.prevent="
                                        onDialogueLineDragEnter(index, $event)
                                    "
                                    @dragover.prevent="
                                        onDialogueLineDragOver(index, $event)
                                    "
                                    @drop.prevent="
                                        onDialogueLineDrop(index, $event)
                                    "
                                >
                                    <button
                                        class="line-drag-handle"
                                        draggable="true"
                                        title="拖拽排序"
                                        aria-label="拖拽排序"
                                        @dragstart="
                                            onDialogueLineDragStart(
                                                index,
                                                $event,
                                            )
                                        "
                                        @dragend="onDialogueLineDragEnd"
                                    >
                                        <IconGlyph name="drag_indicator" />
                                    </button>
                                    <input
                                        :ref="
                                            (el) =>
                                                setMultiDialogueLineRef(
                                                    el,
                                                    index,
                                                )
                                        "
                                        type="text"
                                        class="form-input"
                                        v-model="line.textKey"
                                        placeholder="输入台词..."
                                        @keydown.enter.prevent="
                                            insertLineAfter(index)
                                        "
                                        @keydown.backspace="
                                            handleLineBackspace(index, $event)
                                        "
                                        @paste="
                                            handleDialogueLinePaste(
                                                index,
                                                $event,
                                            )
                                        "
                                        @focus="setActiveDialogueLine(index)"
                                        @compositionstart="
                                            handleCompositionStart
                                        "
                                        @compositionend="handleCompositionEnd"
                                        @blur="handleDialogueLineBlur(index)"
                                    />

                                    <button
                                        class="remove-choice-btn line-remove-btn"
                                        :class="{
                                            'is-filled':
                                                String(
                                                    line?.textKey || '',
                                                ).trim().length > 0,
                                        }"
                                        @click="removeDialogueLine(index)"
                                        title="删除该行"
                                        aria-label="删除该行"
                                    >
                                        <IconGlyph name="close" />
                                    </button>
                                </div>
                            </div>
                            <p class="choice-hint">
                                按 Enter 新增下一行；留空角色时将作为“旁白”显示
                            </p>
                        </div>
                    </template>

                    <template v-else-if="localNode.type === 'choice'">
                        <div class="form-group glass-input">
                            <label class="form-label">选择模式</label>
                            <select
                                class="form-input"
                                v-model="localNode.data.choiceMode"
                                @change="handleUpdate"
                            >
                                <option value="SinglePass">
                                    普通分支（单次）
                                </option>
                                <option value="ExhaustiveUntilComplete">
                                    穷举分支（选完全部后继续）
                                </option>
                            </select>
                        </div>

                        <p
                            v-if="
                                localNode.data.choiceMode ===
                                'ExhaustiveUntilComplete'
                            "
                            class="choice-hint"
                        >
                            提示：请从“穷举完成后”连接点拖线到完成节点
                        </p>

                        <div class="form-group glass-input">
                            <label class="choice-toggle-row">
                                <input
                                    type="checkbox"
                                    v-model="localNode.data.choiceTimer.enabled"
                                    @change="handleUpdate"
                                />
                                <span>启用选项倒计时</span>
                            </label>

                            <div
                                v-if="localNode.data.choiceTimer.enabled"
                                class="choice-timer-grid"
                            >
                                <label class="form-label">倒计时秒数</label>
                                <input
                                    type="number"
                                    class="form-input"
                                    v-model.number="
                                        localNode.data.choiceTimer
                                            .durationSeconds
                                    "
                                    min="0.1"
                                    step="0.1"
                                    @blur="handleUpdate"
                                />

                                <label class="form-label">超时选择文本</label>
                                <input
                                    type="text"
                                    class="form-input"
                                    v-model="
                                        localNode.data.choiceTimer
                                            .timeoutChoiceTextKey
                                    "
                                    placeholder="例如：沉默"
                                    @compositionstart="handleCompositionStart"
                                    @compositionend="handleCompositionEnd"
                                    @blur="handleInputChange"
                                />

                                <p class="choice-hint">
                                    启用后节点会出现“超时选择”连接点；时间结束后自动沿该连线继续
                                </p>
                            </div>
                        </div>

                        <div class="form-group glass-input">
                            <div class="multi-lines-header">
                                <label class="form-label"
                                    >选项列表（Enter 新增下一项）</label
                                >
                                <button
                                    class="open-dialogue-modal-btn"
                                    @click="openChoiceModal"
                                    title="弹窗编辑选项"
                                    aria-label="弹窗编辑选项"
                                >
                                    <IconGlyph name="open_in_full" />
                                </button>
                            </div>
                            <div
                                v-for="(choice, index) in localNode.data
                                    .choices"
                                :key="index"
                                class="choice-item"
                            >
                                <input
                                    type="text"
                                    class="form-input"
                                    v-model="choice.textKey"
                                    placeholder="选项文本"
                                    @keydown.enter.prevent="
                                        insertChoiceAfter(index)
                                    "
                                    @compositionstart="handleCompositionStart"
                                    @compositionend="handleCompositionEnd"
                                    @blur="handleInputChange"
                                />
                                <button
                                    class="remove-choice-btn"
                                    @click="removeChoice(index)"
                                >
                                    <IconGlyph name="close" />
                                </button>
                            </div>
                            <button class="add-choice-btn" @click="addChoice">
                                <IconGlyph name="add" />
                                <span>添加选项</span>
                            </button>
                            <p class="choice-hint">
                                提示：从节点右侧的连接点拖线到目标节点
                            </p>
                        </div>
                    </template>

                    <template v-else-if="localNode.type === 'jump'">
                        <div class="form-group glass-input">
                            <label class="form-label">目标节点 ID</label>
                            <input
                                type="text"
                                class="form-input"
                                v-model="localNode.data.targetNodeId"
                                @compositionstart="handleCompositionStart"
                                @compositionend="handleCompositionEnd"
                                @blur="handleInputChange"
                            />
                        </div>
                    </template>

                    <template v-else-if="localNode.type === 'condition'">
                        <div class="form-group glass-input">
                            <div class="multi-lines-header">
                                <label class="form-label">条件分支</label>
                                <button
                                    class="open-dialogue-modal-btn"
                                    @click="openConditionModal"
                                    title="弹窗编辑条件"
                                    aria-label="弹窗编辑条件"
                                >
                                    <IconGlyph name="open_in_full" />
                                </button>
                            </div>
                            <div
                                v-for="(branch, branchIndex) in localNode.data
                                    .condition.branches"
                                :key="`cond-branch-${branchIndex}`"
                                class="condition-branch"
                            >
                                <div class="condition-branch-header">
                                    <input
                                        type="text"
                                        class="form-input"
                                        v-model="branch.label"
                                        placeholder="分支名称"
                                        @compositionstart="
                                            handleCompositionStart
                                        "
                                        @compositionend="handleCompositionEnd"
                                        @blur="handleInputChange"
                                    />
                                    <select
                                        class="form-input condition-logic-select"
                                        v-model="branch.logic"
                                        @change="handleUpdate"
                                    >
                                        <option value="All">
                                            All（全部满足）
                                        </option>
                                        <option value="Any">
                                            Any（任一满足）
                                        </option>
                                    </select>
                                    <button
                                        class="remove-choice-btn"
                                        @click="
                                            removeConditionBranch(branchIndex)
                                        "
                                        title="删除条件分支"
                                        aria-label="删除条件分支"
                                    >
                                        <IconGlyph name="close" />
                                    </button>
                                </div>

                                <div
                                    v-for="(term, termIndex) in branch.terms"
                                    :key="`cond-term-${branchIndex}-${termIndex}`"
                                    class="choice-item"
                                >
                                    <select
                                        class="form-input"
                                        :value="
                                            getConditionTermVariableName(term)
                                        "
                                        @change="
                                            handleConditionVariableSelect(
                                                term,
                                                $event.target.value,
                                            )
                                        "
                                    >
                                        <option value="">（选择变量）</option>
                                        <option
                                            v-for="variable in availableVariables"
                                            :key="`condition-variable-${variable.name}`"
                                            :value="variable.name"
                                        >
                                            {{
                                                formatVariableOption(variable)
                                            }}
                                        </option>
                                        <option
                                            v-if="
                                                getConditionTermVariableName(
                                                    term,
                                                ) &&
                                                !findVariableByName(
                                                    getConditionTermVariableName(
                                                        term,
                                                    ),
                                                )
                                            "
                                            :value="
                                                getConditionTermVariableName(
                                                    term,
                                                )
                                            "
                                        >
                                            {{
                                                `${getConditionTermVariableName(term)}（未在变量管理中）`
                                            }}
                                        </option>
                                    </select>
                                    <select
                                        class="form-input"
                                        v-model="term.variable.type"
                                        :disabled="
                                            !!findVariableByName(
                                                getConditionTermVariableName(
                                                    term,
                                                ),
                                            )
                                        "
                                        @change="handleUpdate"
                                    >
                                        <option value="Bool">Bool</option>
                                        <option value="Int">Int</option>
                                        <option value="Float">Float</option>
                                        <option value="String">String</option>
                                    </select>
                                    <select
                                        class="form-input"
                                        v-model="term.variable.scope"
                                        :disabled="
                                            !!findVariableByName(
                                                getConditionTermVariableName(
                                                    term,
                                                ),
                                            )
                                        "
                                        @change="handleUpdate"
                                    >
                                        <option value="Session">Session</option>
                                        <option value="Global">Global</option>
                                    </select>
                                    <select
                                        class="form-input"
                                        v-model="term.operator"
                                        @change="handleUpdate"
                                    >
                                        <option value="==">==</option>
                                        <option value="!=">!=</option>
                                        <option value=">">&gt;</option>
                                        <option value=">=">&gt;=</option>
                                        <option value="<">&lt;</option>
                                        <option value="<=">&lt;=</option>
                                    </select>
                                    <input
                                        type="text"
                                        class="form-input"
                                        v-model="term.compareValue"
                                        placeholder="比较值"
                                        @compositionstart="
                                            handleCompositionStart
                                        "
                                        @compositionend="handleCompositionEnd"
                                        @blur="handleInputChange"
                                    />
                                    <button
                                        class="remove-choice-btn"
                                        @click="
                                            removeConditionTerm(
                                                branchIndex,
                                                termIndex,
                                            )
                                        "
                                    >
                                        <IconGlyph name="close" />
                                    </button>
                                </div>

                                <button
                                    class="add-choice-btn condition-term-add"
                                    @click="addConditionTerm(branchIndex)"
                                >
                                    <IconGlyph name="add" />
                                    <span>添加条件项</span>
                                </button>
                                <p class="choice-hint">
                                    输出句柄：condition-{{ branchIndex }}
                                </p>
                            </div>

                            <button
                                class="add-choice-btn"
                                @click="addConditionBranch"
                            >
                                <IconGlyph name="add" />
                                <span>添加条件分支</span>
                            </button>
                            <p class="choice-hint">
                                从上到下匹配第一个满足的条件；未匹配时走 condition-fallback（可选）
                            </p>
                            <p
                                v-if="availableVariables.length === 0"
                                class="choice-hint"
                            >
                                请先在左侧变量管理中添加变量
                            </p>
                        </div>
                    </template>

                    <template v-else-if="localNode.type === 'setvariable'">
                        <div class="form-group glass-input">
                            <label class="form-label">变量名</label>
                            <select
                                class="form-input"
                                v-model="localNode.data.variableName"
                                @change="handleSetVariableSelect"
                            >
                                <option value="">（选择变量）</option>
                                <option
                                    v-for="variable in availableVariables"
                                    :key="`set-variable-${variable.name}`"
                                    :value="variable.name"
                                >
                                    {{ formatVariableOption(variable) }}
                                </option>
                                <option
                                    v-if="
                                        localNode.data.variableName &&
                                        !findVariableByName(
                                            localNode.data.variableName,
                                        )
                                    "
                                    :value="localNode.data.variableName"
                                >
                                    {{
                                        `${localNode.data.variableName}（未在变量管理中）`
                                    }}
                                </option>
                            </select>
                            <p
                                v-if="availableVariables.length === 0"
                                class="choice-hint"
                            >
                                请先在左侧变量管理中添加变量
                            </p>
                        </div>

                        <div class="form-group glass-input">
                            <label class="form-label">操作</label>
                            <select
                                class="form-input"
                                v-model="localNode.data.operation"
                                @change="handleUpdate"
                            >
                                <option value="Set">Set (设置)</option>
                                <option value="Add">Add (增加)</option>
                                <option value="Subtract">
                                    Subtract (减少)
                                </option>
                                <option value="Multiply">
                                    Multiply (乘以)
                                </option>
                                <option value="Divide">Divide (除以)</option>
                            </select>
                        </div>

                        <div class="form-group glass-input">
                            <label class="form-label">值</label>
                            <input
                                type="text"
                                class="form-input"
                                v-model="localNode.data.value"
                                @compositionstart="handleCompositionStart"
                                @compositionend="handleCompositionEnd"
                                @blur="handleInputChange"
                            />
                        </div>
                    </template>

                    <template v-else-if="localNode.type === 'emitevent'">
                        <div class="form-group glass-input">
                            <label class="form-label">事件类型</label>
                            <input
                                type="text"
                                class="form-input"
                                v-model="localNode.data.eventType"
                                @compositionstart="handleCompositionStart"
                                @compositionend="handleCompositionEnd"
                                @blur="handleInputChange"
                                placeholder="inventory.add_item"
                            />
                        </div>

                        <div class="form-group glass-input">
                            <label class="form-label">参数</label>
                            <div class="event-param-list">
                                <div
                                    v-for="(entry, index) in getEventParamEntries()"
                                    :key="`event-param-${index}`"
                                    class="event-param-row"
                                >
                                    <input
                                        type="text"
                                        class="form-input"
                                        :value="entry.key"
                                        placeholder="key"
                                        @input="
                                            updateEventParamKey(
                                                index,
                                                $event.target.value,
                                            )
                                        "
                                        @compositionstart="
                                            handleCompositionStart
                                        "
                                        @compositionend="handleCompositionEnd"
                                        @keydown.stop
                                        @blur="handleInputChange"
                                    />
                                    <input
                                        type="text"
                                        class="form-input"
                                        :value="formatEventParamValue(entry.value)"
                                        placeholder="value"
                                        @input="
                                            updateEventParamValue(
                                                index,
                                                $event.target.value,
                                            )
                                        "
                                        @compositionstart="
                                            handleCompositionStart
                                        "
                                        @compositionend="handleCompositionEnd"
                                        @keydown.stop
                                        @blur="handleInputChange"
                                    />
                                    <button
                                        class="remove-choice-btn event-param-remove"
                                        @click="removeEventParam(index)"
                                        title="删除参数"
                                    >
                                        <IconGlyph name="close" />
                                    </button>
                                </div>
                            </div>
                            <button
                                class="add-choice-btn event-param-add"
                                @click="addEventParam"
                            >
                                <IconGlyph name="add" />
                                <span>添加参数</span>
                            </button>
                            <p class="choice-hint">
                                数字、true、false 会按对应类型导出，其他内容按字符串导出
                            </p>
                        </div>
                    </template>

                    <button
                        class="update-button bouncy-feedback spring-animation"
                        @click="handleUpdate"
                    >
                        <IconGlyph name="check_circle" />
                        <span>更新属性</span>
                    </button>

                    <button
                        class="entry-node-button bouncy-feedback spring-animation"
                        :class="{ 'is-entry': isEntryNode }"
                        @click="handleSetEntryNode"
                        :disabled="isEntryNode"
                    >
                        <IconGlyph
                            :name="
                                isEntryNode
                                    ? 'check_circle'
                                    : 'radio_button_unchecked'
                            "
                        />
                        <span>{{
                            isEntryNode ? "当前入口节点" : "设置为入口节点"
                        }}</span>
                    </button>
                </div>
                <div v-else class="property-panel-empty">
                    <IconGlyph name="touch_app" class="empty-icon" />
                    <p>点击节点以编辑属性</p>
                </div>
            </div>
        </div>
    </div>

    <Teleport to="body">
        <div
            v-if="showMultiDialogueModal && localNode?.type === 'dialogue'"
            class="multi-dialogue-modal-overlay"
            @click.self="closeMultiDialogueModal"
        >
            <div
                :class="[
                    'multi-dialogue-modal',
                    'glass-morphism-strong',
                    { 'is-dark-theme': isDarkMode },
                ]"
                @mousedown.stop
                @click.stop
            >
                <div class="multi-dialogue-modal-header">
                    <div>
                        <h3>对话全屏编辑</h3>
                        <p>拖拽左侧图标调整顺序，Enter 新增行</p>
                    </div>
                    <button
                        class="multi-dialogue-modal-close"
                        @click="closeMultiDialogueModal"
                        title="关闭"
                        aria-label="关闭"
                    >
                        ×
                    </button>
                </div>

                <div class="multi-dialogue-modal-body">
                    <div
                        class="multi-lines-editor fullscreen"
                        @dragover.prevent="onDialogueContainerDragOver($event)"
                        @drop.prevent="onDialogueContainerDrop($event)"
                    >
                        <div
                            v-for="(line, index) in localNode.data.lines"
                            :key="`modal-line-${index}`"
                            class="line-edit-row"
                            :class="{
                                'is-editing':
                                    activeMultiDialogueLineIndex === index,
                                'is-dragging':
                                    draggingDialogueLineIndex === index,
                                'is-drag-over': isDialogueLineDragOver(index),
                                'drop-before': isDialogueLineDropBefore(index),
                                'drop-after': isDialogueLineDropAfter(index),
                            }"
                            @dragenter.prevent="
                                onDialogueLineDragEnter(index, $event)
                            "
                            @dragover.prevent="
                                onDialogueLineDragOver(index, $event)
                            "
                            @drop.prevent="onDialogueLineDrop(index, $event)"
                        >
                            <button
                                class="line-drag-handle"
                                draggable="true"
                                title="拖拽排序"
                                aria-label="拖拽排序"
                                @dragstart="
                                    onDialogueLineDragStart(index, $event)
                                "
                                @dragend="onDialogueLineDragEnd"
                            >
                                <IconGlyph name="drag_indicator" />
                            </button>

                            <textarea
                                :ref="
                                    (el) => setMultiDialogueLineRef(el, index)
                                "
                                class="form-textarea modal-line-textarea"
                                v-model="line.textKey"
                                rows="1"
                                placeholder="输入台词..."
                                @input="handleModalLineInput(index, $event)"
                                @keydown.enter.prevent="insertLineAfter(index)"
                                @keydown.backspace="
                                    handleLineBackspace(index, $event)
                                "
                                @paste="handleDialogueLinePaste(index, $event)"
                                @focus="handleModalLineFocus(index, $event)"
                                @compositionstart="handleCompositionStart"
                                @compositionend="handleCompositionEnd"
                                @blur="handleDialogueLineBlur(index)"
                            ></textarea>

                            <button
                                class="remove-choice-btn line-remove-btn"
                                :class="{
                                    'is-filled':
                                        String(line?.textKey || '').trim()
                                            .length > 0,
                                }"
                                @click="removeDialogueLine(index)"
                                title="删除该行"
                                aria-label="删除该行"
                            >
                                <IconGlyph name="close" />
                            </button>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </Teleport>

    <Teleport to="body">
        <div
            v-if="showChoiceModal && localNode?.type === 'choice'"
            class="multi-dialogue-modal-overlay"
            @click.self="closeChoiceModal"
        >
            <div
                :class="[
                    'multi-dialogue-modal',
                    'dialogue-modal-small',
                    'choice-modal-small',
                    'glass-morphism-strong',
                    { 'is-dark-theme': isDarkMode },
                ]"
                @mousedown.stop
                @click.stop
            >
                <div class="multi-dialogue-modal-header">
                    <div>
                        <h3>选项编辑</h3>
                        <p>拖拽左侧图标调整顺序，Enter 新增选项</p>
                    </div>
                    <button
                        class="multi-dialogue-modal-close"
                        @click="closeChoiceModal"
                        title="关闭"
                        aria-label="关闭"
                    >
                        ×
                    </button>
                </div>

                <div class="multi-dialogue-modal-body">
                    <div
                        class="multi-lines-editor fullscreen"
                        @dragover.prevent="onChoiceContainerDragOver($event)"
                        @drop.prevent="onChoiceContainerDrop($event)"
                    >
                        <div
                            v-for="(choice, index) in localNode.data.choices"
                            :key="`modal-choice-${index}`"
                            class="line-edit-row"
                            :class="{
                                'is-editing': activeChoiceIndex === index,
                                'is-dragging': draggingChoiceIndex === index,
                                'is-drag-over': isChoiceDragOver(index),
                                'drop-before': isChoiceDropBefore(index),
                                'drop-after': isChoiceDropAfter(index),
                            }"
                            @dragenter.prevent="
                                onChoiceDragEnter(index, $event)
                            "
                            @dragover.prevent="onChoiceDragOver(index, $event)"
                            @drop.prevent="onChoiceDrop(index, $event)"
                        >
                            <button
                                class="line-drag-handle"
                                draggable="true"
                                title="拖拽排序"
                                aria-label="拖拽排序"
                                @dragstart="onChoiceDragStart(index, $event)"
                                @dragend="onChoiceDragEnd"
                            >
                                <IconGlyph name="drag_indicator" />
                            </button>

                            <textarea
                                :ref="(el) => setChoiceRef(el, index)"
                                class="form-textarea modal-line-textarea"
                                v-model="choice.textKey"
                                rows="1"
                                placeholder="输入选项文本..."
                                @input="handleChoiceInput(index, $event)"
                                @keydown.enter.prevent="
                                    insertChoiceAfter(index)
                                "
                                @focus="handleChoiceFocus(index, $event)"
                                @compositionstart="handleCompositionStart"
                                @compositionend="handleCompositionEnd"
                                @blur="handleChoiceBlur(index)"
                            ></textarea>

                            <button
                                class="remove-choice-btn line-remove-btn"
                                :class="{
                                    'is-filled':
                                        String(choice?.textKey || '').trim()
                                            .length > 0,
                                }"
                                @click="removeChoice(index)"
                                title="删除该选项"
                                aria-label="删除该选项"
                            >
                                <IconGlyph name="close" />
                            </button>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </Teleport>

    <Teleport to="body">
        <div
            v-if="showConditionModal && localNode?.type === 'condition'"
            class="multi-dialogue-modal-overlay"
            @click.self="closeConditionModal"
        >
            <div
                :class="[
                    'multi-dialogue-modal',
                    'condition-modal-wide',
                    'glass-morphism-strong',
                    { 'is-dark-theme': isDarkMode },
                ]"
                @mousedown.stop
                @click.stop
            >
                <div class="multi-dialogue-modal-header">
                    <div>
                        <h3>条件分支编辑</h3>
                        <p>从上到下匹配第一个满足的条件，未匹配时走 fallback 出口</p>
                    </div>
                    <button
                        class="multi-dialogue-modal-close"
                        @click="closeConditionModal"
                        title="关闭"
                        aria-label="关闭"
                    >
                        X
                    </button>
                </div>

                <div class="multi-dialogue-modal-body condition-modal-body">
                    <div class="condition-modal-branches">
                        <div
                            v-for="(branch, branchIndex) in localNode.data
                                .condition.branches"
                            :key="`modal-cond-branch-${branchIndex}`"
                            class="condition-branch condition-branch-modal"
                        >
                            <div class="condition-branch-header">
                                <input
                                    type="text"
                                    class="form-input"
                                    v-model="branch.label"
                                    placeholder="分支名称"
                                    @compositionstart="handleCompositionStart"
                                    @compositionend="handleCompositionEnd"
                                    @blur="handleInputChange"
                                />
                                <select
                                    class="form-input condition-logic-select"
                                    v-model="branch.logic"
                                    @change="handleUpdate"
                                >
                                    <option value="All">All（全部满足）</option>
                                    <option value="Any">Any（任一满足）</option>
                                </select>
                                <button
                                    class="remove-choice-btn"
                                    @click="removeConditionBranch(branchIndex)"
                                    title="删除条件分支"
                                    aria-label="删除条件分支"
                                >
                                    <IconGlyph name="close" />
                                </button>
                            </div>

                            <div
                                v-for="(term, termIndex) in branch.terms"
                                :key="`modal-cond-term-${branchIndex}-${termIndex}`"
                                class="choice-item condition-term-modal"
                            >
                                <select
                                    class="form-input"
                                    :value="getConditionTermVariableName(term)"
                                    @change="
                                        handleConditionVariableSelect(
                                            term,
                                            $event.target.value,
                                        )
                                    "
                                >
                                    <option value="">（选择变量）</option>
                                    <option
                                        v-for="variable in availableVariables"
                                        :key="`modal-condition-variable-${variable.name}`"
                                        :value="variable.name"
                                    >
                                        {{ formatVariableOption(variable) }}
                                    </option>
                                    <option
                                        v-if="
                                            getConditionTermVariableName(term) &&
                                            !findVariableByName(
                                                getConditionTermVariableName(
                                                    term,
                                                ),
                                            )
                                        "
                                        :value="
                                            getConditionTermVariableName(term)
                                        "
                                    >
                                        {{
                                            `${getConditionTermVariableName(term)}（未在变量管理中）`
                                        }}
                                    </option>
                                </select>
                                <select
                                    class="form-input"
                                    v-model="term.variable.type"
                                    :disabled="
                                        !!findVariableByName(
                                            getConditionTermVariableName(term),
                                        )
                                    "
                                    @change="handleUpdate"
                                >
                                    <option value="Bool">Bool</option>
                                    <option value="Int">Int</option>
                                    <option value="Float">Float</option>
                                    <option value="String">String</option>
                                </select>
                                <select
                                    class="form-input"
                                    v-model="term.variable.scope"
                                    :disabled="
                                        !!findVariableByName(
                                            getConditionTermVariableName(term),
                                        )
                                    "
                                    @change="handleUpdate"
                                >
                                    <option value="Session">Session</option>
                                    <option value="Global">Global</option>
                                </select>
                                <select
                                    class="form-input"
                                    v-model="term.operator"
                                    @change="handleUpdate"
                                >
                                    <option value="==">==</option>
                                    <option value="!=">!=</option>
                                    <option value=">">&gt;</option>
                                    <option value=">=">&gt;=</option>
                                    <option value="<">&lt;</option>
                                    <option value="<=">&lt;=</option>
                                </select>
                                <input
                                    type="text"
                                    class="form-input"
                                    v-model="term.compareValue"
                                    placeholder="比较值"
                                    @compositionstart="handleCompositionStart"
                                    @compositionend="handleCompositionEnd"
                                    @blur="handleInputChange"
                                />
                                <button
                                    class="remove-choice-btn"
                                    @click="
                                        removeConditionTerm(
                                            branchIndex,
                                            termIndex,
                                        )
                                    "
                                    title="删除条件项"
                                    aria-label="删除条件项"
                                >
                                    <IconGlyph name="close" />
                                </button>
                            </div>

                            <button
                                class="add-choice-btn condition-term-add"
                                @click="addConditionTerm(branchIndex)"
                            >
                                <IconGlyph name="add" />
                                <span>添加条件项</span>
                            </button>
                            <p class="choice-hint">
                                输出句柄：condition-{{ branchIndex }}
                            </p>
                        </div>
                    </div>

                    <button class="add-choice-btn" @click="addConditionBranch">
                        <IconGlyph name="add" />
                        <span>添加条件分支</span>
                    </button>
                </div>
            </div>
        </div>
    </Teleport>

    <Teleport to="body">
        <div
            v-if="showDialogueModal && localNode?.type === 'dialogue'"
            class="multi-dialogue-modal-overlay"
            @click.self="closeDialogueModal"
        >
            <div
                :class="[
                    'multi-dialogue-modal',
                    'dialogue-modal-small',
                    'glass-morphism-strong',
                    { 'is-dark-theme': isDarkMode },
                ]"
                @mousedown.stop
                @click.stop
            >
                <div class="multi-dialogue-modal-header">
                    <div>
                        <h3>对话编辑</h3>
                        <p>Esc 关闭，Ctrl+S 完成</p>
                    </div>
                    <button
                        class="multi-dialogue-modal-close"
                        @click="closeDialogueModal"
                        title="关闭"
                        aria-label="关闭"
                    >
                        ×
                    </button>
                </div>

                <div class="multi-dialogue-modal-body">
                    <div class="form-group dialogue-modal-text-wrapper">
                        <textarea
                            class="form-textarea dialogue-modal-textarea"
                            v-model="localNode.data.textKey"
                            @compositionstart="handleCompositionStart"
                            @compositionend="handleCompositionEnd"
                            @blur="handleInputChange"
                        ></textarea>
                    </div>
                </div>
            </div>
        </div>
    </Teleport>
</template>

<script setup>
import { ref, watch, computed, onMounted, onUnmounted, nextTick } from "vue";
import { normalizeEditorDialogueNode } from "../utils/dialogue-node.js";

const props = defineProps({
    selectedNode: {
        type: Object,
        default: null,
    },
    entryNodeId: {
        type: String,
        default: "",
    },
    variables: {
        type: Array,
        default: () => [],
    },
    presetSpeakers: {
        type: Array,
        default: () => [],
    },
    storyEntries: {
        type: Array,
        default: () => [],
    },
    assetMode: {
        type: String,
        default: "story",
    },
    isDarkMode: {
        type: Boolean,
        default: false,
    },
    openMultiDialogueRequest: {
        type: Object,
        default: null,
    },
    openDialogueRequest: {
        type: Object,
        default: null,
    },
    openChoiceRequest: {
        type: Object,
        default: null,
    },
    openConditionRequest: {
        type: Object,
        default: null,
    },
});

const emit = defineEmits(["update", "set-entry-node"]);

const isDarkMode = computed(() => !!props.isDarkMode);
const availableVariables = computed(() =>
    (Array.isArray(props.variables) ? props.variables : [])
        .map((variable) => ({
            ...variable,
            name: String(variable?.name || "").trim(),
        }))
        .filter((variable) => variable.name),
);

const panelWrapperRef = ref(null);
const isExpanded = ref(false);
const localNode = ref(null);
const isComposing = ref(false);
const isPanelInputFocused = ref(false);
const multiDialogueLineRefs = ref([]);
const activeMultiDialogueLineIndex = ref(-1);
const draggingDialogueLineIndex = ref(-1);
const dragOverDialogueLineIndex = ref(-1);
const dragOverDialogueLinePlacement = ref("after");
const showMultiDialogueModal = ref(false);
const showDialogueModal = ref(false);
const showChoiceModal = ref(false);
const showConditionModal = ref(false);
const choiceRefs = ref([]);
const activeChoiceIndex = ref(-1);
const draggingChoiceIndex = ref(-1);
const dragOverChoiceIndex = ref(-1);
const dragOverChoicePlacement = ref("after");

function getVariableScope(variable) {
    if (variable?.scope === "Global" || variable?.bGlobalScope === true) {
        return "Global";
    }
    return "Session";
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

function isPlainObject(value) {
    return value != null && typeof value === "object" && !Array.isArray(value);
}

function normalizeEventParamsData(data) {
    const target = data && typeof data === "object" ? data : {};
    const sourceParams = isPlainObject(target.params) ? target.params : {};

    return {
        eventType: String(target.eventType || ""),
        params: { ...sourceParams },
    };
}

function getEventParamEntries() {
    if (!localNode.value || localNode.value.type !== "emitevent") return [];
    const data = localNode.value.data || {};
    const params = isPlainObject(data.params) ? data.params : {};
    return Object.entries(params).map(
        ([key, value]) => ({
            key,
            value,
        }),
    );
}

function formatEventParamValue(value) {
    if (typeof value === "string") return value;
    if (value == null) return "";
    try {
        return JSON.stringify(value);
    } catch {
        return String(value);
    }
}

function parseEventParamValue(value) {
    const text = String(value ?? "");
    const trimmed = text.trim();
    if (!trimmed) return "";

    if (
        trimmed === "true" ||
        trimmed === "false" ||
        trimmed === "null" ||
        /^-?(0|[1-9]\d*)(\.\d+)?([eE][+-]?\d+)?$/.test(trimmed) ||
        trimmed.startsWith("{") ||
        trimmed.startsWith("[") ||
        trimmed.startsWith('"')
    ) {
        try {
            return JSON.parse(trimmed);
        } catch {
            return text;
        }
    }

    return text;
}

function makeUniqueEventParamKey(baseKey, entries, currentIndex = -1) {
    const cleanBase = String(baseKey ?? "");
    const existing = new Set(
        entries
            .map(([key], index) => (index === currentIndex ? null : key))
            .filter(Boolean),
    );

    if (!existing.has(cleanBase)) return cleanBase;

    let suffix = 2;
    let candidate = `${cleanBase}${suffix}`;
    while (existing.has(candidate)) {
        suffix += 1;
        candidate = `${cleanBase}${suffix}`;
    }
    return candidate;
}

function setEventParamEntries(entries) {
    if (!localNode.value || localNode.value.type !== "emitevent") return;
    localNode.value.data = normalizeEventParamsData(localNode.value.data);
    localNode.value.data.params = Object.fromEntries(entries);
    handleUpdate();
}

function getVariableType(variable) {
    const type = String(variable?.type || variable?.variableType || "String");
    return ["Bool", "Int", "Float", "String"].includes(type) ? type : "String";
}

function findVariableByName(name) {
    const normalizedName = String(name || "").trim();
    if (!normalizedName) return null;
    return (
        availableVariables.value.find(
            (variable) => variable.name === normalizedName,
        ) || null
    );
}

function formatVariableOption(variable) {
    const name = String(variable?.name || "").trim();
    const type = getVariableType(variable);
    const scope = getVariableScope(variable);
    return `${name} - ${type} - ${scope}`;
}

function createDefaultConditionTerm() {
    const firstVariable = availableVariables.value[0] || null;
    return {
        variable: {
            name: firstVariable?.name || "",
            type: firstVariable ? getVariableType(firstVariable) : "String",
            scope: firstVariable ? getVariableScope(firstVariable) : "Session",
        },
        operator: "==",
        compareValue: "",
    };
}

function normalizeConditionTerm(term) {
    if (!term || typeof term !== "object") return;
    if (!term.variable || typeof term.variable !== "object") {
        term.variable = { name: "", type: "String", scope: "Session" };
    }

    const selectedVariable = findVariableByName(term.variable.name);
    if (selectedVariable) {
        term.variable.name = selectedVariable.name;
        term.variable.type = getVariableType(selectedVariable);
        term.variable.scope = getVariableScope(selectedVariable);
    } else {
        term.variable.name = String(term.variable.name || "").trim();
        term.variable.type = getVariableType(term.variable);
        term.variable.scope = getVariableScope(term.variable);
    }

    if (!term.operator) {
        term.operator = "==";
    }
    if (term.compareValue === undefined || term.compareValue === null) {
        term.compareValue = "";
    }
}

function normalizeConditionBranch(branch, index) {
    const normalized =
        branch && typeof branch === "object" && !Array.isArray(branch)
            ? branch
            : {};
    normalized.label = String(
        normalized.label || `条件 ${Number(index || 0) + 1}`,
    ).trim();
    if (normalized.logic !== "Any") {
        normalized.logic = "All";
    }
    if (!Array.isArray(normalized.terms)) {
        normalized.terms = [];
    }
    normalized.terms.forEach((term) => normalizeConditionTerm(term));
    return normalized;
}

function normalizeConditionData(condition) {
    const normalized =
        condition && typeof condition === "object" && !Array.isArray(condition)
            ? condition
            : {};

    if (!Array.isArray(normalized.branches)) {
        normalized.branches = Array.isArray(normalized.terms)
            ? [
                  {
                      label: "条件 1",
                      logic: normalized.logic || "All",
                      terms: normalized.terms,
                  },
              ]
            : [];
    }

    if (normalized.branches.length === 0) {
        normalized.branches.push({
            label: "条件 1",
            logic: "All",
            terms: [createDefaultConditionTerm()],
        });
    }

    normalized.branches = normalized.branches.map((branch, index) =>
        normalizeConditionBranch(branch, index),
    );

    delete normalized.logic;
    delete normalized.terms;
    return normalized;
}

function normalizeRailBranches(branches) {
    return (Array.isArray(branches) ? branches : []).map((branch, index) => ({
        label: String(branch?.label || `分支 ${index + 1}`).trim(),
        logic: branch?.logic === "Any" ? "Any" : "All",
        terms: (Array.isArray(branch?.terms) ? branch.terms : []).map(
            (term) => ({
                variable: {
                    name: String(term?.variable?.name || "").trim(),
                    type: term?.variable?.type || "String",
                    scope: term?.variable?.scope || "Global",
                },
                operator: term?.operator || "==",
                compareValue: String(term?.compareValue ?? ""),
            }),
        ),
    }));
}

function getConditionTermVariableName(term) {
    return String(term?.variable?.name || "").trim();
}

function handleConditionVariableSelect(term, variableName) {
    if (!term || typeof term !== "object") return;
    if (!term.variable || typeof term.variable !== "object") {
        term.variable = { name: "", type: "String", scope: "Session" };
    }

    const selectedVariable = findVariableByName(variableName);
    term.variable.name = selectedVariable
        ? selectedVariable.name
        : String(variableName || "").trim();
    term.variable.type = selectedVariable
        ? getVariableType(selectedVariable)
        : getVariableType(term.variable);
    term.variable.scope = selectedVariable
        ? getVariableScope(selectedVariable)
        : getVariableScope(term.variable);
    handleUpdate();
}

function handleSetVariableSelect() {
    if (!localNode.value || localNode.value.type !== "setvariable") return;
    localNode.value.data = localNode.value.data || {};
    localNode.value.data.variableName = String(
        localNode.value.data.variableName || "",
    ).trim();
    handleUpdate();
}

function addRailBranch() {
    if (!localNode.value || localNode.value.type !== "railbranch") return;
    const branches = normalizeRailBranches(localNode.value.data?.branches);
    branches.push({
        label: `分支 ${branches.length + 1}`,
        logic: "All",
        terms: [],
    });
    localNode.value.data.branches = branches;
    handleUpdate();
}

function removeRailBranch(index) {
    if (!localNode.value || localNode.value.type !== "railbranch") return;
    const branches = normalizeRailBranches(localNode.value.data?.branches);
    branches.splice(index, 1);
    localNode.value.data.branches = branches;
    handleUpdate();
}

function addRailTerm(branchIndex) {
    if (!localNode.value || localNode.value.type !== "railbranch") return;
    const branches = normalizeRailBranches(localNode.value.data?.branches);
    if (!branches[branchIndex]) return;
    const firstVariable = availableVariables.value[0] || null;
    branches[branchIndex].terms.push({
        variable: {
            name: firstVariable?.name || "",
            type: firstVariable ? getVariableType(firstVariable) : "String",
            scope: firstVariable ? getVariableScope(firstVariable) : "Global",
        },
        operator: "==",
        compareValue: "",
    });
    localNode.value.data.branches = branches;
    handleUpdate();
}

function removeRailTerm(branchIndex, termIndex) {
    if (!localNode.value || localNode.value.type !== "railbranch") return;
    const branches = normalizeRailBranches(localNode.value.data?.branches);
    branches[branchIndex]?.terms?.splice(termIndex, 1);
    localNode.value.data.branches = branches;
    handleUpdate();
}

function updateRailTermVariable(branchIndex, termIndex, variableName) {
    if (!localNode.value || localNode.value.type !== "railbranch") return;
    const branches = normalizeRailBranches(localNode.value.data?.branches);
    const term = branches[branchIndex]?.terms?.[termIndex];
    if (!term) return;
    const selectedVariable = findVariableByName(variableName);
    term.variable = {
        name: selectedVariable
            ? selectedVariable.name
            : String(variableName || "").trim(),
        type: selectedVariable ? getVariableType(selectedVariable) : "String",
        scope: selectedVariable ? getVariableScope(selectedVariable) : "Global",
    };
    localNode.value.data.branches = branches;
    handleUpdate();
}

// 监听 selectedNode 变化，同步到本地副本
watch(
    () => props.selectedNode,
    (newNode) => {
        const isEditingCurrentNodeInModal =
            (showMultiDialogueModal.value ||
                showDialogueModal.value ||
                showChoiceModal.value ||
                showConditionModal.value) &&
            localNode.value &&
            newNode &&
            localNode.value.id === newNode.id;

        if (isEditingCurrentNodeInModal) {
            return;
        }

        if (newNode) {
            multiDialogueLineRefs.value = [];
            choiceRefs.value = [];
            activeMultiDialogueLineIndex.value = -1;
            activeChoiceIndex.value = -1;
            draggingDialogueLineIndex.value = -1;
            draggingChoiceIndex.value = -1;
            dragOverDialogueLineIndex.value = -1;
            dragOverChoiceIndex.value = -1;
            dragOverDialogueLinePlacement.value = "after";
            dragOverChoicePlacement.value = "after";
            showMultiDialogueModal.value = false;
            showDialogueModal.value = false;
            showChoiceModal.value = false;
            showConditionModal.value = false;
            localNode.value = normalizeEditorDialogueNode(
                JSON.parse(JSON.stringify(newNode)),
            );
            if (localNode.value.type === "dialogue") {
                localNode.value.data = localNode.value.data || {};
            }
            if (localNode.value.type === "choice") {
                localNode.value.data = localNode.value.data || {};
                if (!Array.isArray(localNode.value.data.choices)) {
                    localNode.value.data.choices = [];
                }
                localNode.value.data.choiceTimer = normalizeChoiceTimer(
                    localNode.value.data.choiceTimer,
                );
                if (
                    localNode.value.data.choiceMode !==
                    "ExhaustiveUntilComplete"
                ) {
                    localNode.value.data.choiceMode = "SinglePass";
                }
            }
            if (localNode.value.type === "condition") {
                localNode.value.data = localNode.value.data || {};
                localNode.value.data.condition = normalizeConditionData(
                    localNode.value.data.condition,
                );
            }
            if (localNode.value.type === "setvariable") {
                localNode.value.data = localNode.value.data || {};
                localNode.value.data.variableName = String(
                    localNode.value.data.variableName || "",
                ).trim();
            }
            if (localNode.value.type === "emitevent") {
                localNode.value.data = normalizeEventParamsData(
                    localNode.value.data || {},
                );
            }
            if (localNode.value.type === "railstory") {
                localNode.value.data = {
                    title: "剧情脚本",
                    summary: "",
                    storyId: "",
                    ...(localNode.value.data || {}),
                };
            }
            if (
                localNode.value.type === "railnote" ||
                localNode.value.type === "railend"
            ) {
                localNode.value.data = {
                    title:
                        localNode.value.type === "railend"
                            ? "总纲结束"
                            : "章节标记",
                    summary: "",
                    ...(localNode.value.data || {}),
                };
            }
            if (localNode.value.type === "railbranch") {
                localNode.value.data = {
                    title: "路线判断",
                    summary: "",
                    ...(localNode.value.data || {}),
                };
                localNode.value.data.branches = normalizeRailBranches(
                    localNode.value.data.branches,
                );
                if (localNode.value.data.branches.length === 0) {
                    localNode.value.data.branches.push({
                        label: "分支 1",
                        logic: "All",
                        terms: [],
                    });
                }
            }
            isExpanded.value = true; // 选中节点时自动展开
        } else {
            multiDialogueLineRefs.value = [];
            choiceRefs.value = [];
            activeMultiDialogueLineIndex.value = -1;
            activeChoiceIndex.value = -1;
            draggingDialogueLineIndex.value = -1;
            draggingChoiceIndex.value = -1;
            dragOverDialogueLineIndex.value = -1;
            dragOverChoiceIndex.value = -1;
            dragOverDialogueLinePlacement.value = "after";
            dragOverChoicePlacement.value = "after";
            showMultiDialogueModal.value = false;
            showDialogueModal.value = false;
            showChoiceModal.value = false;
            showConditionModal.value = false;
            localNode.value = null;
            isExpanded.value = false; // 取消选中时自动收起
        }
    },
    { immediate: true },
);

watch(
    availableVariables,
    () => {
        if (!localNode.value || localNode.value.type !== "condition") return;
        localNode.value.data.condition = normalizeConditionData(
            localNode.value.data?.condition,
        );
    },
    { deep: true },
);

// 计算当前节点是否为入口节点
const isEntryNode = computed(() => {
    return localNode.value && localNode.value.id === props.entryNodeId;
});

watch(
    () => props.openMultiDialogueRequest,
    async (request) => {
        const requestNodeId = String(request?.nodeId || "").trim();
        if (!requestNodeId) return;

        await nextTick();

        if (
            localNode.value &&
            localNode.value.type === "dialogue" &&
            localNode.value.id === requestNodeId
        ) {
            isExpanded.value = false;
            isPanelInputFocused.value = false;
            openMultiDialogueModal();
        }
    },
);

watch(
    () => props.openDialogueRequest,
    async (request) => {
        const requestNodeId = String(request?.nodeId || "").trim();
        if (!requestNodeId) return;

        await nextTick();

        if (
            localNode.value &&
            localNode.value.type === "dialogue" &&
            localNode.value.id === requestNodeId
        ) {
            isExpanded.value = false;
            isPanelInputFocused.value = false;
            openMultiDialogueModal();
        }
    },
);

watch(
    () => props.openChoiceRequest,
    async (request) => {
        const requestNodeId = String(request?.nodeId || "").trim();
        if (!requestNodeId) return;

        await nextTick();

        if (
            localNode.value &&
            localNode.value.type === "choice" &&
            localNode.value.id === requestNodeId
        ) {
            isExpanded.value = false;
            isPanelInputFocused.value = false;
            openChoiceModal();
        }
    },
);

watch(
    () => props.openConditionRequest,
    async (request) => {
        const requestNodeId = String(request?.nodeId || "").trim();
        if (!requestNodeId) return;

        await nextTick();

        if (
            localNode.value &&
            localNode.value.type === "condition" &&
            localNode.value.id === requestNodeId
        ) {
            isExpanded.value = false;
            isPanelInputFocused.value = false;
            openConditionModal();
        }
    },
);

// 处理中文输入法
function handleCompositionStart() {
    isComposing.value = true;
}

function handleCompositionEnd() {
    isComposing.value = false;
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

function handleGlobalPointerDown(event) {
    if (
        showMultiDialogueModal.value ||
        showDialogueModal.value ||
        showChoiceModal.value
    ) {
        return;
    }

    const panelEl = panelWrapperRef.value;
    const target = event?.target;
    if (!panelEl || !target) return;

    if (typeof panelEl.contains === "function" && panelEl.contains(target)) {
        return;
    }

    // 已选中节点时，在画布内进行点击/拖拽不自动收起属性面板，
    // 避免松手后因重新选中同节点触发“再次弹出动画”。
    const targetEl =
        target && typeof target.closest === "function" ? target : null;
    const isInsideGraphCanvas = !!(
        targetEl &&
        (targetEl.closest(".graph-editor") ||
            targetEl.closest(".svelte-wrapper"))
    );
    if (localNode.value && isInsideGraphCanvas) {
        return;
    }

    if (isComposing.value) return;

    isPanelInputFocused.value = false;
    isExpanded.value = false;
}

function handleInputChange() {
    // 只在非输入法状态下保存
    if (!isComposing.value) {
        handleUpdate();
    }
}

function getSpeakerId(speaker) {
    if (typeof speaker === "string") return speaker;
    return String(speaker?.id || "");
}

function formatPresetSpeakerLabel(speaker) {
    if (typeof speaker === "string") return speaker;
    const id = String(speaker?.id || "").trim();
    const displayName = String(speaker?.displayName || "").trim();
    if (displayName && displayName !== id) return `${displayName} (${id})`;
    return id || "未命名角色";
}

function updateNodeId(event) {
    if (!localNode.value) return;
    localNode.value.id = event.target.value;
}

function updateData(key, value) {
    if (!localNode.value) return;
    localNode.value.data = {
        ...localNode.value.data,
        [key]: value,
    };
}

function addChoice() {
    if (!localNode.value || localNode.value.type !== "choice") return;
    const choices = [...(localNode.value.data.choices || [])];
    choices.push({ textKey: "" });
    localNode.value.data.choices = choices;
    handleUpdate();
}

function setChoiceRef(el, index) {
    if (!el) return;
    choiceRefs.value[index] = el;
    if (String(el?.tagName || "").toUpperCase() === "TEXTAREA") {
        autoResizeTextarea(el);
    }
}

function setActiveChoice(index) {
    activeChoiceIndex.value = index;
}

async function focusChoice(index) {
    await nextTick();
    const input = choiceRefs.value[index];
    if (input && typeof input.focus === "function") {
        input.focus();
        if (typeof input.setSelectionRange === "function") {
            const len = String(input.value || "").length;
            input.setSelectionRange(len, len);
        }
    }
}

function insertChoiceAfter(index) {
    if (!localNode.value || localNode.value.type !== "choice") return;
    const choices = Array.isArray(localNode.value.data.choices)
        ? [...localNode.value.data.choices]
        : [];
    const insertAt = index + 1;
    choices.splice(insertAt, 0, { textKey: "" });
    localNode.value.data.choices = choices;
    handleUpdate();
    focusChoice(insertAt);
}

function handleChoiceInput(index, event) {
    setActiveChoice(index);
    autoResizeTextarea(event?.target);
    handleUpdate();
}

function handleChoiceFocus(index, event) {
    setActiveChoice(index);
    autoResizeTextarea(event?.target);
}

function handleChoiceBlur(index) {
    if (activeChoiceIndex.value === index) {
        activeChoiceIndex.value = -1;
    }
    handleInputChange();
}

function removeChoice(index) {
    if (!localNode.value || localNode.value.type !== "choice") return;
    const choices = [...(localNode.value.data.choices || [])];
    choices.splice(index, 1);
    localNode.value.data.choices = choices;
}

function updateChoice(index, key, value) {
    if (!localNode.value || localNode.value.type !== "choice") return;
    const choices = [...(localNode.value.data.choices || [])];
    choices[index] = { ...choices[index], [key]: value };
    localNode.value.data.choices = choices;
}

function setMultiDialogueLineRef(el, index) {
    if (!el) return;
    multiDialogueLineRefs.value[index] = el;
    if (String(el?.tagName || "").toUpperCase() === "TEXTAREA") {
        autoResizeTextarea(el);
    }
}

function setActiveDialogueLine(index) {
    activeMultiDialogueLineIndex.value = index;
}

function autoResizeTextarea(el) {
    if (!el || String(el?.tagName || "").toUpperCase() !== "TEXTAREA") {
        return;
    }
    el.style.height = "auto";
    el.style.height = `${Math.min(el.scrollHeight, 220)}px`;
}

function handleModalLineInput(index, event) {
    setActiveDialogueLine(index);
    autoResizeTextarea(event?.target);
    handleUpdate();
}

function handleModalLineFocus(index, event) {
    setActiveDialogueLine(index);
    autoResizeTextarea(event?.target);
}

function handleDialogueLineBlur(index) {
    if (activeMultiDialogueLineIndex.value === index) {
        activeMultiDialogueLineIndex.value = -1;
    }
    handleInputChange();
}

async function focusDialogueLine(index) {
    await nextTick();
    const input = multiDialogueLineRefs.value[index];
    if (input && typeof input.focus === "function") {
        input.focus();
        if (typeof input.setSelectionRange === "function") {
            const len = String(input.value || "").length;
            input.setSelectionRange(len, len);
        }
    }
}

function insertLineAfter(index) {
    if (!localNode.value || localNode.value.type !== "dialogue") return;
    const lines = Array.isArray(localNode.value.data.lines)
        ? [...localNode.value.data.lines]
        : [];
    const insertAt = index + 1;
    lines.splice(insertAt, 0, { textKey: "" });
    localNode.value.data.lines = lines;
    handleUpdate();
    focusDialogueLine(insertAt);
}

function handleDialogueLinePaste(index, event) {
    if (!localNode.value || localNode.value.type !== "dialogue") return;
    const raw = event?.clipboardData?.getData("text") || "";
    if (!raw.includes("\n")) return;

    event.preventDefault();

    const pastedLines = raw
        .replace(/\r\n/g, "\n")
        .replace(/\r/g, "\n")
        .split("\n");

    if (pastedLines.length === 0) return;

    const lines = Array.isArray(localNode.value.data.lines)
        ? [...localNode.value.data.lines]
        : [];

    const first = pastedLines[0] || "";
    lines[index] = { ...(lines[index] || {}), textKey: first };

    if (pastedLines.length > 1) {
        const additional = pastedLines
            .slice(1)
            .map((text) => ({ textKey: text }));
        lines.splice(index + 1, 0, ...additional);
    }

    localNode.value.data.lines = lines;
    handleUpdate();
    focusDialogueLine(index + pastedLines.length - 1);
}

function onDialogueLineDragStart(index, event) {
    draggingDialogueLineIndex.value = index;
    dragOverDialogueLineIndex.value = index;
    dragOverDialogueLinePlacement.value = "after";

    if (event?.dataTransfer) {
        event.dataTransfer.effectAllowed = "move";
        event.dataTransfer.setData("text/plain", String(index));

        const currentTarget = event?.currentTarget;
        if (
            currentTarget &&
            typeof currentTarget.closest === "function" &&
            typeof event.dataTransfer.setDragImage === "function"
        ) {
            const rowEl = currentTarget.closest(".line-edit-row");
            if (rowEl && typeof rowEl.cloneNode === "function") {
                const ghost = rowEl.cloneNode(true);
                ghost.style.position = "fixed";
                ghost.style.top = "-9999px";
                ghost.style.left = "-9999px";
                ghost.style.width = `${rowEl.clientWidth}px`;
                ghost.style.opacity = "0.96";
                ghost.style.transform = "scale(1.01)";
                ghost.style.boxShadow =
                    "0 14px 26px rgba(15, 23, 42, 0.22), 0 2px 8px rgba(59, 130, 246, 0.28)";
                ghost.style.background = "rgba(255, 255, 255, 0.92)";
                ghost.style.borderRadius = "10px";
                ghost.style.pointerEvents = "none";
                document.body.appendChild(ghost);
                event.dataTransfer.setDragImage(ghost, 20, 20);
                setTimeout(() => {
                    if (ghost.parentNode) {
                        ghost.parentNode.removeChild(ghost);
                    }
                }, 0);
            }
        }
    }
}

function onDialogueLineDragEnd() {
    draggingDialogueLineIndex.value = -1;
    dragOverDialogueLineIndex.value = -1;
    dragOverDialogueLinePlacement.value = "after";
}

function onDialogueLineDragEnter(index, event) {
    onDialogueLineDragOver(index, event);
}

function updateDragTargetFromPointer(rootEl, clientY) {
    if (!rootEl) return;

    const rowElements = Array.from(rootEl.querySelectorAll(".line-edit-row"));
    if (rowElements.length === 0) {
        dragOverDialogueLineIndex.value = -1;
        dragOverDialogueLinePlacement.value = "after";
        return;
    }

    const safeClientY = Number.isFinite(Number(clientY))
        ? Number(clientY)
        : rowElements[0].getBoundingClientRect().top;

    let nearestIndex = 0;
    let nearestDistance = Number.POSITIVE_INFINITY;
    let placement = "after";

    rowElements.forEach((rowEl, index) => {
        if (!rowEl || typeof rowEl.getBoundingClientRect !== "function") {
            return;
        }
        const rect = rowEl.getBoundingClientRect();
        const centerY = rect.top + rect.height / 2;
        const distance = Math.abs(safeClientY - centerY);
        if (distance < nearestDistance) {
            nearestDistance = distance;
            nearestIndex = index;
            placement = safeClientY <= centerY ? "before" : "after";
        }
    });

    dragOverDialogueLineIndex.value = nearestIndex;
    dragOverDialogueLinePlacement.value = placement;
}

function onDialogueContainerDragOver(event) {
    if (draggingDialogueLineIndex.value < 0) return;

    const rootEl = event?.currentTarget;
    if (!rootEl || typeof rootEl.querySelectorAll !== "function") return;

    if (event?.dataTransfer) {
        event.dataTransfer.dropEffect = "move";
    }

    updateDragTargetFromPointer(rootEl, event?.clientY);
}

function onDialogueContainerDrop(event) {
    if (draggingDialogueLineIndex.value < 0) {
        onDialogueLineDragEnd();
        return;
    }

    const rootEl = event?.currentTarget;
    if (rootEl && typeof rootEl.querySelectorAll === "function") {
        updateDragTargetFromPointer(rootEl, event?.clientY);
    }

    const fallbackTargetIndex =
        dragOverDialogueLineIndex.value >= 0
            ? dragOverDialogueLineIndex.value
            : 0;

    onDialogueLineDrop(fallbackTargetIndex, event);
}

function onDialogueLineDragOver(index, event) {
    if (draggingDialogueLineIndex.value < 0) return;
    if (event?.dataTransfer) {
        event.dataTransfer.dropEffect = "move";
    }

    dragOverDialogueLineIndex.value = index;

    const currentTarget = event?.currentTarget;
    if (
        !currentTarget ||
        typeof currentTarget.getBoundingClientRect !== "function"
    ) {
        dragOverDialogueLinePlacement.value = "after";
        return;
    }

    const rect = currentTarget.getBoundingClientRect();
    const offsetY = Number(event?.clientY || 0) - rect.top;
    dragOverDialogueLinePlacement.value =
        offsetY <= rect.height / 2 ? "before" : "after";
}

function isDialogueLineDragOver(index) {
    return (
        draggingDialogueLineIndex.value >= 0 &&
        dragOverDialogueLineIndex.value === index
    );
}

function isDialogueLineDropBefore(index) {
    return (
        isDialogueLineDragOver(index) &&
        dragOverDialogueLinePlacement.value === "before" &&
        draggingDialogueLineIndex.value !== index
    );
}

function isDialogueLineDropAfter(index) {
    return (
        isDialogueLineDragOver(index) &&
        dragOverDialogueLinePlacement.value === "after" &&
        draggingDialogueLineIndex.value !== index
    );
}

function onDialogueLineDrop(targetIndex, event) {
    if (!localNode.value || localNode.value.type !== "dialogue") return;

    const fromIndex = draggingDialogueLineIndex.value;
    if (fromIndex < 0) {
        onDialogueLineDragEnd();
        return;
    }

    const lines = Array.isArray(localNode.value.data.lines)
        ? [...localNode.value.data.lines]
        : [];

    const visualTargetIndex =
        dragOverDialogueLineIndex.value >= 0
            ? dragOverDialogueLineIndex.value
            : targetIndex;

    if (
        fromIndex >= lines.length ||
        visualTargetIndex < 0 ||
        visualTargetIndex >= lines.length
    ) {
        onDialogueLineDragEnd();
        return;
    }

    let placement = dragOverDialogueLinePlacement.value;

    const currentTarget = event?.currentTarget;
    const isLineRowTarget =
        currentTarget &&
        typeof currentTarget.classList?.contains === "function" &&
        currentTarget.classList.contains("line-edit-row");
    if (
        isLineRowTarget &&
        typeof currentTarget.getBoundingClientRect === "function" &&
        Number.isFinite(Number(event?.clientY))
    ) {
        const rect = currentTarget.getBoundingClientRect();
        const offsetY = Number(event.clientY) - rect.top;
        placement = offsetY <= rect.height / 2 ? "before" : "after";
    }

    const [moved] = lines.splice(fromIndex, 1);

    let insertIndex = visualTargetIndex;
    if (placement === "after") {
        insertIndex += 1;
    }

    if (fromIndex < visualTargetIndex) {
        insertIndex -= 1;
    }

    insertIndex = Math.max(0, Math.min(insertIndex, lines.length));

    lines.splice(insertIndex, 0, moved);
    localNode.value.data.lines = lines;
    handleUpdate();
    focusDialogueLine(insertIndex);
    onDialogueLineDragEnd();
}

function onChoiceDragStart(index, event) {
    draggingChoiceIndex.value = index;
    dragOverChoiceIndex.value = index;
    dragOverChoicePlacement.value = "after";

    if (event?.dataTransfer) {
        event.dataTransfer.effectAllowed = "move";
        event.dataTransfer.setData("text/plain", String(index));
    }
}

function onChoiceDragEnd() {
    draggingChoiceIndex.value = -1;
    dragOverChoiceIndex.value = -1;
    dragOverChoicePlacement.value = "after";
}

function onChoiceDragEnter(index, event) {
    onChoiceDragOver(index, event);
}

function onChoiceContainerDragOver(event) {
    if (draggingChoiceIndex.value < 0) return;

    if (event?.dataTransfer) {
        event.dataTransfer.dropEffect = "move";
    }

    const rootEl = event?.currentTarget;
    if (!rootEl || typeof rootEl.querySelectorAll !== "function") return;

    const rowElements = Array.from(rootEl.querySelectorAll(".line-edit-row"));
    if (rowElements.length === 0) {
        dragOverChoiceIndex.value = -1;
        dragOverChoicePlacement.value = "after";
        return;
    }

    const safeClientY = Number.isFinite(Number(event?.clientY))
        ? Number(event.clientY)
        : rowElements[0].getBoundingClientRect().top;

    let nearestIndex = 0;
    let nearestDistance = Number.POSITIVE_INFINITY;
    let placement = "after";

    rowElements.forEach((rowEl, rowIndex) => {
        const rect = rowEl.getBoundingClientRect();
        const centerY = rect.top + rect.height / 2;
        const distance = Math.abs(safeClientY - centerY);
        if (distance < nearestDistance) {
            nearestDistance = distance;
            nearestIndex = rowIndex;
            placement = safeClientY <= centerY ? "before" : "after";
        }
    });

    dragOverChoiceIndex.value = nearestIndex;
    dragOverChoicePlacement.value = placement;
}

function onChoiceContainerDrop(event) {
    if (draggingChoiceIndex.value < 0) {
        onChoiceDragEnd();
        return;
    }

    const fallbackTargetIndex =
        dragOverChoiceIndex.value >= 0 ? dragOverChoiceIndex.value : 0;
    onChoiceDrop(fallbackTargetIndex, event);
}

function onChoiceDragOver(index, event) {
    if (draggingChoiceIndex.value < 0) return;
    if (event?.dataTransfer) {
        event.dataTransfer.dropEffect = "move";
    }

    dragOverChoiceIndex.value = index;

    const currentTarget = event?.currentTarget;
    if (
        !currentTarget ||
        typeof currentTarget.getBoundingClientRect !== "function"
    ) {
        dragOverChoicePlacement.value = "after";
        return;
    }

    const rect = currentTarget.getBoundingClientRect();
    const offsetY = Number(event?.clientY || 0) - rect.top;
    dragOverChoicePlacement.value =
        offsetY <= rect.height / 2 ? "before" : "after";
}

function isChoiceDragOver(index) {
    return (
        draggingChoiceIndex.value >= 0 && dragOverChoiceIndex.value === index
    );
}

function isChoiceDropBefore(index) {
    return (
        isChoiceDragOver(index) &&
        dragOverChoicePlacement.value === "before" &&
        draggingChoiceIndex.value !== index
    );
}

function isChoiceDropAfter(index) {
    return (
        isChoiceDragOver(index) &&
        dragOverChoicePlacement.value === "after" &&
        draggingChoiceIndex.value !== index
    );
}

function onChoiceDrop(targetIndex, event) {
    if (!localNode.value || localNode.value.type !== "choice") return;

    const fromIndex = draggingChoiceIndex.value;
    if (fromIndex < 0) {
        onChoiceDragEnd();
        return;
    }

    const choices = Array.isArray(localNode.value.data.choices)
        ? [...localNode.value.data.choices]
        : [];

    const visualTargetIndex =
        dragOverChoiceIndex.value >= 0
            ? dragOverChoiceIndex.value
            : targetIndex;

    if (
        fromIndex >= choices.length ||
        visualTargetIndex < 0 ||
        visualTargetIndex >= choices.length
    ) {
        onChoiceDragEnd();
        return;
    }

    let placement = dragOverChoicePlacement.value;

    const currentTarget = event?.currentTarget;
    const isLineRowTarget =
        currentTarget &&
        typeof currentTarget.classList?.contains === "function" &&
        currentTarget.classList.contains("line-edit-row");
    if (
        isLineRowTarget &&
        typeof currentTarget.getBoundingClientRect === "function" &&
        Number.isFinite(Number(event?.clientY))
    ) {
        const rect = currentTarget.getBoundingClientRect();
        const offsetY = Number(event.clientY) - rect.top;
        placement = offsetY <= rect.height / 2 ? "before" : "after";
    }

    const [moved] = choices.splice(fromIndex, 1);

    let insertIndex = visualTargetIndex;
    if (placement === "after") {
        insertIndex += 1;
    }

    if (fromIndex < visualTargetIndex) {
        insertIndex -= 1;
    }

    insertIndex = Math.max(0, Math.min(insertIndex, choices.length));

    choices.splice(insertIndex, 0, moved);
    localNode.value.data.choices = choices;
    handleUpdate();
    focusChoice(insertIndex);
    onChoiceDragEnd();
}

function handleLineBackspace(index, event) {
    if (!localNode.value || localNode.value.type !== "dialogue") return;
    const lines = Array.isArray(localNode.value.data.lines)
        ? [...localNode.value.data.lines]
        : [];
    if (lines.length <= 1) return;

    const current = lines[index];
    const text = typeof current === "string" ? current : current?.textKey || "";
    if (String(text).length > 0) return;

    event.preventDefault();
    lines.splice(index, 1);
    localNode.value.data.lines = lines;
    handleUpdate();
    focusDialogueLine(Math.max(0, index - 1));
}

function removeDialogueLine(index) {
    if (!localNode.value || localNode.value.type !== "dialogue") return;
    const lines = Array.isArray(localNode.value.data.lines)
        ? [...localNode.value.data.lines]
        : [];
    if (lines.length <= 1) {
        lines[0] = { textKey: "" };
    } else {
        lines.splice(index, 1);
    }
    localNode.value.data.lines = lines;
    handleUpdate();
}

function addEventParam() {
    if (!localNode.value || localNode.value.type !== "emitevent") return;
    localNode.value.data = normalizeEventParamsData(localNode.value.data);
    const entries = Object.entries(localNode.value.data.params || {});
    const key = makeUniqueEventParamKey(`param${entries.length + 1}`, entries);
    entries.push([key, ""]);
    setEventParamEntries(entries);
}

function removeEventParam(index) {
    if (!localNode.value || localNode.value.type !== "emitevent") return;
    localNode.value.data = normalizeEventParamsData(localNode.value.data);
    const entries = Object.entries(localNode.value.data.params || {});
    if (index < 0 || index >= entries.length) return;
    entries.splice(index, 1);
    setEventParamEntries(entries);
}

function updateEventParamKey(index, nextKey) {
    if (!localNode.value || localNode.value.type !== "emitevent") return;
    localNode.value.data = normalizeEventParamsData(localNode.value.data);
    const entries = Object.entries(localNode.value.data.params || {});
    if (index < 0 || index >= entries.length) return;

    const value = entries[index][1];
    entries[index] = [
        makeUniqueEventParamKey(nextKey, entries, index),
        value,
    ];
    setEventParamEntries(entries);
}

function updateEventParamValue(index, nextValue) {
    if (!localNode.value || localNode.value.type !== "emitevent") return;
    localNode.value.data = normalizeEventParamsData(localNode.value.data);
    const entries = Object.entries(localNode.value.data.params || {});
    if (index < 0 || index >= entries.length) return;

    entries[index] = [entries[index][0], parseEventParamValue(nextValue)];
    setEventParamEntries(entries);
}

function addConditionBranch() {
    if (!localNode.value || localNode.value.type !== "condition") return;
    localNode.value.data.condition = normalizeConditionData(
        localNode.value.data?.condition,
    );
    const branches = Array.isArray(localNode.value.data.condition.branches)
        ? [...localNode.value.data.condition.branches]
        : [];

    branches.push({
        label: `条件 ${branches.length + 1}`,
        logic: "All",
        terms: [createDefaultConditionTerm()],
    });

    localNode.value.data.condition.branches = branches;
    handleUpdate();
}

function removeConditionBranch(index) {
    if (!localNode.value || localNode.value.type !== "condition") return;
    localNode.value.data.condition = normalizeConditionData(
        localNode.value.data?.condition,
    );
    const branches = Array.isArray(localNode.value.data.condition.branches)
        ? [...localNode.value.data.condition.branches]
        : [];
    branches.splice(index, 1);
    if (branches.length === 0) {
        branches.push({
            label: "条件 1",
            logic: "All",
            terms: [createDefaultConditionTerm()],
        });
    }
    localNode.value.data.condition.branches = branches.map((branch, idx) =>
        normalizeConditionBranch(branch, idx),
    );
    handleUpdate();
}

function addConditionTerm(branchIndex) {
    if (!localNode.value || localNode.value.type !== "condition") return;
    localNode.value.data.condition = normalizeConditionData(
        localNode.value.data?.condition,
    );
    const branch = localNode.value.data.condition.branches?.[branchIndex];
    if (!branch) return;
    branch.terms = Array.isArray(branch.terms) ? [...branch.terms] : [];
    branch.terms.push(createDefaultConditionTerm());
    handleUpdate();
}

function removeConditionTerm(branchIndex, termIndex) {
    if (!localNode.value || localNode.value.type !== "condition") return;
    localNode.value.data.condition = normalizeConditionData(
        localNode.value.data?.condition,
    );
    const branch = localNode.value.data.condition.branches?.[branchIndex];
    if (!branch) return;
    branch.terms = Array.isArray(branch.terms) ? [...branch.terms] : [];
    branch.terms.splice(termIndex, 1);
    handleUpdate();
}

async function openMultiDialogueModal() {
    if (!localNode.value || localNode.value.type !== "dialogue") return;
    showDialogueModal.value = false;
    showChoiceModal.value = false;
    showConditionModal.value = false;
    isExpanded.value = false;
    isPanelInputFocused.value = false;
    showMultiDialogueModal.value = true;

    await nextTick();
    multiDialogueLineRefs.value.forEach((el) => {
        autoResizeTextarea(el);
    });
}

function closeMultiDialogueModal() {
    showMultiDialogueModal.value = false;
    activeMultiDialogueLineIndex.value = -1;
    draggingDialogueLineIndex.value = -1;
    dragOverDialogueLineIndex.value = -1;
    dragOverDialogueLinePlacement.value = "after";
    if (localNode.value) {
        isExpanded.value = true;
    }
}

function openDialogueModal() {
    if (!localNode.value || localNode.value.type !== "dialogue") return;
    openMultiDialogueModal();
}

async function openChoiceModal() {
    if (!localNode.value || localNode.value.type !== "choice") return;
    showMultiDialogueModal.value = false;
    showDialogueModal.value = false;
    showConditionModal.value = false;
    isExpanded.value = false;
    isPanelInputFocused.value = false;
    showChoiceModal.value = true;

    await nextTick();
    choiceRefs.value.forEach((el) => {
        autoResizeTextarea(el);
    });
}

function openConditionModal() {
    if (!localNode.value || localNode.value.type !== "condition") return;
    showMultiDialogueModal.value = false;
    showDialogueModal.value = false;
    showChoiceModal.value = false;
    localNode.value.data = localNode.value.data || {};
    localNode.value.data.condition = normalizeConditionData(
        localNode.value.data.condition,
    );
    isExpanded.value = false;
    isPanelInputFocused.value = false;
    showConditionModal.value = true;
}

function closeDialogueModal() {
    showDialogueModal.value = false;
    if (localNode.value) {
        isExpanded.value = true;
    }
}

function closeChoiceModal() {
    showChoiceModal.value = false;
    activeChoiceIndex.value = -1;
    draggingChoiceIndex.value = -1;
    dragOverChoiceIndex.value = -1;
    dragOverChoicePlacement.value = "after";
    if (localNode.value) {
        isExpanded.value = true;
    }
}

function closeConditionModal() {
    showConditionModal.value = false;
    if (localNode.value) {
        isExpanded.value = true;
    }
}

function handleUpdate() {
    // 提交本地修改到父组件
    if (localNode.value) {
        emit("update", localNode.value);
    }
}

function handleModalShortcutKeydown(event) {
    if (
        !showMultiDialogueModal.value &&
        !showDialogueModal.value &&
        !showChoiceModal.value &&
        !showConditionModal.value
    )
        return;

    const key = String(event?.key || "").toLowerCase();
    const isCtrlOrMeta = !!(event?.ctrlKey || event?.metaKey);

    if (key === "escape") {
        event.preventDefault();
        if (showMultiDialogueModal.value) {
            closeMultiDialogueModal();
        } else if (showChoiceModal.value) {
            closeChoiceModal();
        } else if (showConditionModal.value) {
            closeConditionModal();
        } else {
            closeDialogueModal();
        }
        return;
    }

    if (isCtrlOrMeta && key === "s") {
        event.preventDefault();
        if (showMultiDialogueModal.value) {
            closeMultiDialogueModal();
        } else if (showChoiceModal.value) {
            closeChoiceModal();
        } else if (showConditionModal.value) {
            closeConditionModal();
        } else {
            closeDialogueModal();
        }
    }
}

function handleSetEntryNode() {
    if (localNode.value && !isEntryNode.value) {
        emit("set-entry-node", localNode.value.id);
    }
}

onMounted(() => {
    window.addEventListener("pointerdown", handleGlobalPointerDown, true);
    window.addEventListener("keydown", handleModalShortcutKeydown, true);
});

onUnmounted(() => {
    window.removeEventListener("pointerdown", handleGlobalPointerDown, true);
    window.removeEventListener("keydown", handleModalShortcutKeydown, true);
});
</script>

<style scoped>
.property-panel-wrapper {
    position: fixed;
    right: 0;
    top: 0;
    bottom: 0;
    z-index: 30;
    pointer-events: none;
}

/* 面板本体 */
.property-panel {
    position: absolute;
    right: 16px;
    top: 88px;
    bottom: 92px;
    width: 320px;
    display: flex;
    flex-direction: column;
    overflow: hidden;
    border-radius: 16px;
    transform: translateX(calc(100% + 32px));
    opacity: 0;
    transition: all 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    pointer-events: none;
}

.property-panel.expanded {
    transform: translateX(0);
    opacity: 1;
    pointer-events: auto;
}

.property-panel-header {
    padding: 24px;
    border-bottom: 1px solid rgba(255, 255, 255, 0.3);
}

.panel-title {
    font-family: "Outfit", sans-serif;
    font-size: 24px;
    font-weight: 700;
    color: #3b82f6;
    margin-bottom: 4px;
}

.panel-subtitle {
    font-size: 12px;
    font-weight: 600;
    color: #94a3b8;
    text-transform: uppercase;
    letter-spacing: 0.05em;
}

.property-panel-content {
    flex: 1;
    padding: 24px;
    overflow-y: auto;
}

.property-panel-empty {
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    height: 100%;
    color: #94a3b8;
    text-align: center;
    gap: 16px;
}

.empty-icon {
    font-size: 64px;
    opacity: 0.3;
}

.property-form {
    display: flex;
    flex-direction: column;
    gap: 20px;
}

.form-group {
    display: flex;
    flex-direction: column;
    gap: 8px;
}

.glass-input {
    background: rgba(147, 197, 253, 0.15);
    backdrop-filter: blur(16px);
    padding: 16px;
    border-radius: 12px;
    border: 1px solid rgba(255, 255, 255, 0.6);
}

.form-label {
    font-size: 10px;
    font-weight: 800;
    color: #3b82f6;
    text-transform: uppercase;
    letter-spacing: 0.1em;
    font-family: "Outfit", sans-serif;
}

.form-input,
.form-textarea {
    width: 100%;
    padding: 12px 16px;
    background: rgba(255, 255, 255, 0.4);
    border: none;
    border-radius: 8px;
    font-size: 14px;
    font-weight: 600;
    color: #1e293b;
    font-family: "Noto Sans SC", "Outfit", sans-serif;
    transition: all 0.3s ease;
}

.form-input:focus,
.form-textarea:focus {
    outline: none;
    background: rgba(255, 255, 255, 0.6);
    box-shadow: 0 0 0 2px rgba(59, 130, 246, 0.3);
}

.form-textarea {
    min-height: 100px;
    resize: vertical;
}

.update-button {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 8px;
    width: 100%;
    padding: 16px;
    background: linear-gradient(
        135deg,
        rgba(59, 130, 246, 0.8),
        rgba(168, 85, 247, 0.8)
    );
    color: white;
    border: 1px solid rgba(255, 255, 255, 0.4);
    border-radius: 9999px;
    cursor: pointer;
    font-size: 14px;
    font-weight: 800;
    font-family: "Outfit", sans-serif;
    box-shadow:
        0 12px 32px rgba(59, 130, 246, 0.3),
        inset 0 1px 2px rgba(255, 255, 255, 0.5);
}

.update-button:hover {
    transform: scale(1.02);
}

.update-button :deep(.icon-glyph) {
    font-size: 20px;
}

.entry-node-button {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 8px;
    width: 100%;
    padding: 16px;
    margin-top: 12px;
    background: rgba(255, 255, 255, 0.8);
    color: #64748b;
    border: 2px solid rgba(148, 163, 184, 0.3);
    border-radius: 9999px;
    cursor: pointer;
    font-size: 14px;
    font-weight: 700;
    font-family: "Outfit", sans-serif;
    transition: all 0.3s ease;
}

.entry-node-button:hover:not(:disabled) {
    border-color: rgba(59, 130, 246, 0.5);
    color: #3b82f6;
    transform: scale(1.02);
}

.entry-node-button.is-entry {
    background: linear-gradient(
        135deg,
        rgba(52, 199, 89, 0.15),
        rgba(48, 209, 88, 0.15)
    );
    color: #34c759;
    border-color: rgba(52, 199, 89, 0.5);
    cursor: default;
}

.entry-node-button:disabled {
    cursor: not-allowed;
    opacity: 0.9;
}

.entry-node-button :deep(.icon-glyph) {
    font-size: 20px;
}

.dev-notice {
    color: #94a3b8;
    font-size: 11px;
    text-align: center;
    font-style: italic;
    margin-top: 8px;
}

.choice-item {
    display: flex;
    flex-direction: column;
    gap: 8px;
    padding: 12px;
    background: rgba(255, 255, 255, 0.3);
    border-radius: 8px;
    margin-bottom: 8px;
    position: relative;
}

.condition-branch {
    padding: 12px;
    margin-bottom: 12px;
    background: rgba(255, 255, 255, 0.26);
    border: 1px solid rgba(59, 130, 246, 0.18);
    border-radius: 8px;
}

.condition-branch-header {
    display: grid;
    grid-template-columns: minmax(0, 1fr) 120px auto;
    gap: 8px;
    align-items: center;
    margin-bottom: 10px;
}

.condition-branch-header .remove-choice-btn {
    position: static;
}

.condition-logic-select {
    min-width: 0;
}

.condition-term-add {
    margin-top: 4px;
}

.remove-choice-btn {
    position: absolute;
    top: 8px;
    right: 8px;
    width: 24px;
    height: 24px;
    display: flex;
    align-items: center;
    justify-content: center;
    background: rgba(239, 68, 68, 0.8);
    border: none;
    border-radius: 50%;
    cursor: pointer;
    transition: all 0.2s ease;
}

.remove-choice-btn:hover {
    background: rgba(239, 68, 68, 1);
    transform: scale(1.1);
}

.remove-choice-btn :deep(.icon-glyph) {
    font-size: 16px;
    color: white;
}

.add-choice-btn {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 6px;
    width: 100%;
    padding: 10px;
    background: rgba(59, 130, 246, 0.1);
    border: 1px dashed rgba(59, 130, 246, 0.4);
    border-radius: 8px;
    cursor: pointer;
    font-size: 13px;
    font-weight: 600;
    color: #3b82f6;
    transition: all 0.2s ease;
}

.add-choice-btn:hover {
    background: rgba(59, 130, 246, 0.2);
    border-color: rgba(59, 130, 246, 0.6);
}

.add-choice-btn :deep(.icon-glyph) {
    font-size: 18px;
}

.choice-hint {
    margin-top: 8px;
    font-size: 11px;
    color: #86868b;
    font-style: italic;
    text-align: center;
}

.choice-toggle-row {
    display: flex;
    align-items: center;
    gap: 8px;
    font-size: 13px;
    font-weight: 700;
    color: #1e293b;
}

.choice-toggle-row input {
    width: 16px;
    height: 16px;
}

.choice-timer-grid {
    display: grid;
    grid-template-columns: 1fr;
    gap: 8px;
}

.event-param-list {
    display: flex;
    flex-direction: column;
    gap: 8px;
}

.event-param-row {
    display: grid;
    grid-template-columns: minmax(0, 0.8fr) minmax(0, 1fr) 28px;
    gap: 8px;
    align-items: center;
}

.event-param-row .form-input {
    min-width: 0;
}

.event-param-remove {
    position: static;
    width: 28px;
    height: 28px;
}

.event-param-add {
    margin-top: 8px;
}

.multi-lines-header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 8px;
}

.open-dialogue-modal-btn {
    border: none;
    background: rgba(59, 130, 246, 0.1);
    color: #2563eb;
    border-radius: 8px;
    width: 28px;
    height: 28px;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    cursor: pointer;
}

.open-dialogue-modal-btn :deep(.icon-glyph) {
    font-size: 18px;
}

.multi-lines-editor {
    display: flex;
    flex-direction: column;
    gap: 8px;
}

.line-edit-row {
    display: grid;
    grid-template-columns: 26px 1fr 26px;
    gap: 8px;
    align-items: center;
    border-radius: 10px;
    position: relative;
    transition:
        transform 0.18s cubic-bezier(0.2, 0.8, 0.2, 1),
        box-shadow 0.18s ease,
        background 0.18s ease,
        opacity 0.18s ease;
}

.line-edit-row.is-drag-over {
    background: rgba(59, 130, 246, 0.08);
}

.line-edit-row.is-dragging {
    opacity: 0.6;
    transform: scale(0.985);
}

.line-edit-row.drop-before {
    transform: translateY(8px);
}

.line-edit-row.drop-after {
    transform: translateY(-8px);
}

.line-edit-row.drop-before::before,
.line-edit-row.drop-after::after {
    content: "";
    position: absolute;
    left: 4px;
    right: 4px;
    height: 2px;
    border-radius: 999px;
    background: linear-gradient(
        90deg,
        rgba(59, 130, 246, 0.22),
        rgba(99, 102, 241, 0.8),
        rgba(59, 130, 246, 0.22)
    );
    box-shadow: 0 0 0 3px rgba(99, 102, 241, 0.08);
    pointer-events: none;
}

.line-edit-row.drop-before::before {
    top: -6px;
}

.line-edit-row.drop-after::after {
    bottom: -6px;
}

.line-drag-handle {
    width: 22px;
    height: 22px;
    border: none;
    background: transparent;
    color: rgba(30, 58, 138, 0.38);
    border-radius: 0;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    cursor: grab;
    padding: 0;
}

.line-drag-handle:hover {
    color: rgba(30, 58, 138, 0.62);
}

.line-drag-handle:active {
    cursor: grabbing;
    color: rgba(30, 58, 138, 0.78);
}

.line-edit-row.is-dragging .line-drag-handle {
    cursor: grabbing;
    color: rgba(30, 58, 138, 0.82);
}

.line-drag-handle :deep(.icon-glyph) {
    font-size: 16px;
    line-height: 1;
}

.line-edit-row .remove-choice-btn {
    position: static;
}

.line-remove-btn {
    opacity: 0;
    pointer-events: none;
    transform: translateX(6px) scale(0.92);
    transition: all 0.16s ease;
}

.line-edit-row:not(.is-editing):hover .line-remove-btn.is-filled {
    opacity: 1;
    pointer-events: auto;
    transform: translateX(0) scale(1);
}

.multi-dialogue-modal-overlay {
    --nr-md-overlay-bg: rgba(15, 23, 42, 0.24);
    --nr-md-panel-bg: rgba(255, 255, 255, 1);
    --nr-md-panel-border: rgba(148, 163, 184, 0.28);
    --nr-md-header-border: rgba(148, 163, 184, 0.24);
    --nr-md-title: #1e3a8a;
    --nr-md-subtitle: #64748b;
    --nr-md-close: rgba(30, 58, 138, 0.42);
    --nr-md-close-hover: rgba(30, 58, 138, 0.72);

    position: fixed;
    inset: 0;
    z-index: 120;
    background: var(--nr-md-overlay-bg);
    backdrop-filter: blur(4px);
    display: grid;
    place-items: center;
    padding: 22px;
}

:global(body.light-theme) .multi-dialogue-modal-overlay,
:global(body[data-theme="light"]) .multi-dialogue-modal-overlay {
    --nr-md-overlay-bg: rgba(15, 23, 42, 0.18);
    --nr-md-panel-bg: rgba(255, 255, 255, 1);
    --nr-md-panel-border: rgba(148, 163, 184, 0.26);
    --nr-md-header-border: rgba(148, 163, 184, 0.22);
    --nr-md-title: #1e3a8a;
    --nr-md-subtitle: #64748b;
    --nr-md-close: rgba(30, 58, 138, 0.42);
    --nr-md-close-hover: rgba(30, 58, 138, 0.72);
}

:global(body.dark-theme) .multi-dialogue-modal-overlay,
:global(body[data-theme="dark"]) .multi-dialogue-modal-overlay {
    --nr-md-overlay-bg: rgba(2, 6, 23, 0.52);
    --nr-md-panel-bg: rgba(15, 23, 42, 1);
    --nr-md-panel-border: rgba(96, 165, 250, 0.26);
    --nr-md-header-border: rgba(96, 165, 250, 0.22);
    --nr-md-title: #dbeafe;
    --nr-md-subtitle: #94a3b8;
    --nr-md-close: rgba(191, 219, 254, 0.52);
    --nr-md-close-hover: rgba(219, 234, 254, 0.86);
}

:global(body.dark-theme) .line-edit-row.drop-before::before,
:global(body.dark-theme) .line-edit-row.drop-after::after,
:global(body[data-theme="dark"]) .line-edit-row.drop-before::before,
:global(body[data-theme="dark"]) .line-edit-row.drop-after::after {
    background: linear-gradient(
        90deg,
        rgba(125, 211, 252, 0.22),
        rgba(96, 165, 250, 0.86),
        rgba(125, 211, 252, 0.22)
    );
    box-shadow: 0 0 0 3px rgba(96, 165, 250, 0.16);
}

.multi-dialogue-modal {
    width: min(74vw, 1160px);
    aspect-ratio: 16 / 9;
    max-height: 78vh;
    border-radius: 16px;
    display: flex;
    flex-direction: column;
    overflow: hidden;
    background: var(--nr-md-panel-bg);
    border: 1px solid var(--nr-md-panel-border);
}

.dialogue-modal-small {
    width: min(56vw, 760px);
    aspect-ratio: auto;
    max-height: 72vh;
}

.choice-modal-small {
    width: min(48vw, 640px);
    max-height: 68vh;
}

.condition-modal-wide {
    width: min(1040px, calc(100vw - 48px));
    aspect-ratio: auto;
    max-height: 82vh;
}

.condition-modal-body {
    max-height: min(72vh, 760px);
    overflow: auto;
}

.condition-modal-branches {
    display: flex;
    flex-direction: column;
    gap: 12px;
}

.condition-branch-modal {
    margin-bottom: 0;
}

.condition-term-modal {
    display: grid;
    grid-template-columns:
        minmax(180px, 1.4fr) 96px 100px 82px minmax(140px, 1fr)
        auto;
    align-items: center;
}

.condition-term-modal .remove-choice-btn {
    position: static;
}

.multi-dialogue-modal.glass-morphism-strong {
    background: var(--nr-md-panel-bg) !important;
    backdrop-filter: none;
    -webkit-backdrop-filter: none;
}

.multi-dialogue-modal.is-dark-theme,
.multi-dialogue-modal.is-dark-theme.glass-morphism-strong {
    background: #0f172a !important;
    border-color: rgba(96, 165, 250, 0.26) !important;
}

:global(body.dark-theme) .multi-dialogue-modal,
:global(body[data-theme="dark"]) .multi-dialogue-modal,
:global(body.dark-theme) .multi-dialogue-modal.glass-morphism-strong,
:global(body[data-theme="dark"]) .multi-dialogue-modal.glass-morphism-strong {
    background: #0f172a !important;
    border-color: rgba(96, 165, 250, 0.26) !important;
}

.multi-dialogue-modal.is-dark-theme .multi-dialogue-modal-header {
    border-bottom-color: rgba(96, 165, 250, 0.22) !important;
}

:global(body.dark-theme) .multi-dialogue-modal-header,
:global(body[data-theme="dark"]) .multi-dialogue-modal-header {
    border-bottom-color: rgba(96, 165, 250, 0.22) !important;
}

.multi-dialogue-modal.is-dark-theme .multi-dialogue-modal-header h3 {
    color: #dbeafe !important;
}

:global(body.dark-theme) .multi-dialogue-modal-header h3,
:global(body[data-theme="dark"]) .multi-dialogue-modal-header h3 {
    color: #dbeafe !important;
}

.multi-dialogue-modal.is-dark-theme .multi-dialogue-modal-header p {
    color: #94a3b8 !important;
}

:global(body.dark-theme) .multi-dialogue-modal-header p,
:global(body[data-theme="dark"]) .multi-dialogue-modal-header p {
    color: #94a3b8 !important;
}

.multi-dialogue-modal-header {
    display: flex;
    justify-content: space-between;
    align-items: flex-start;
    gap: 10px;
    padding: 16px 18px;
    border-bottom: 1px solid var(--nr-md-header-border);
}

.multi-dialogue-modal-header h3 {
    margin: 0;
    font-size: 18px;
    color: var(--nr-md-title);
}

.multi-dialogue-modal-header p {
    margin: 4px 0 0;
    font-size: 12px;
    color: var(--nr-md-subtitle);
}

.multi-dialogue-modal-close {
    border: none;
    background: transparent;
    color: var(--nr-md-close);
    width: 24px;
    height: 24px;
    border-radius: 0;
    font-size: 18px;
    line-height: 1;
    cursor: pointer;
    padding: 0;
    transition: color 0.16s ease;
}

.multi-dialogue-modal-close:hover {
    color: var(--nr-md-close-hover);
}

.multi-dialogue-modal-body {
    flex: 1;
    overflow: auto;
    padding: 16px 18px;
}

.dialogue-modal-textarea {
    min-height: 160px;
}

.multi-lines-editor.fullscreen {
    gap: 10px;
}

.modal-line-textarea {
    min-height: 40px;
    height: auto;
    max-height: 220px;
    resize: vertical;
    line-height: 1.45;
    overflow-y: auto;
}
</style>
