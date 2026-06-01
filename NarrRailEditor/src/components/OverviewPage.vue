<template>
    <div
        class="overview-cinematic"
        :class="{ dark: isDarkMode }"
        ref="scrollRoot"
    >
        <nav class="progress-nav" aria-label="Section Progress">
            <button
                v-for="(section, index) in sections"
                :key="section.id"
                class="dot"
                :class="{ active: activeIndex === index }"
                :title="section.kicker"
                @click="scrollToSection(index)"
            />
        </nav>

        <section
            v-for="(section, index) in sections"
            :key="section.id"
            class="scene"
            :class="[
                section.tone,
                section.id === 'intro' ? 'intro-section' : '',
                {
                    visible: visibleIds.has(section.id),
                    reverse: index % 2 === 1,
                },
            ]"
            :data-section-id="section.id"
            :ref="(el) => setSceneRef(el, index)"
        >
            <div
                class="scene-shell"
                :class="{ 'intro-shell': section.id === 'intro' }"
            >
                <template v-if="section.id === 'intro'">
                    <div class="intro-hero">
                        <div class="intro-glow intro-glow-a"></div>
                        <div class="intro-glow intro-glow-b"></div>

                        <p class="intro-kicker">{{ section.kicker }}</p>
                        <h1 class="intro-title flow-text">
                            {{ section.title }}
                        </h1>
                        <p class="intro-subline">{{ section.subline }}</p>

                        <div class="intro-actions">
                            <button
                                class="intro-btn primary"
                                @click="emit('back-library')"
                            >
                                进入脚本库
                            </button>
                            <button
                                class="intro-btn ghost"
                                @click="openGithubRepo"
                            >
                                GitHub 仓库链接
                            </button>
                        </div>
                    </div>
                </template>

                <div v-else class="copy-wrap">
                    <p class="kicker">{{ section.kicker }}</p>
                    <h1 v-if="index === 0" class="headline flow-text">
                        {{ section.title }}
                    </h1>
                    <h2 v-else class="headline flow-text">
                        {{ section.title }}
                    </h2>
                    <p class="subline">{{ section.subline }}</p>

                    <ul v-if="section.highlights?.length" class="detail-list">
                        <li
                            v-for="item in section.highlights"
                            :key="`${section.id}-${item.title}`"
                            class="detail-item"
                        >
                            <p class="detail-title">{{ item.title }}</p>
                            <p class="detail-desc">{{ item.desc }}</p>
                        </li>
                    </ul>
                </div>
            </div>
        </section>
    </div>
</template>

<script setup>
import { nextTick, onBeforeUnmount, onMounted, ref } from "vue";

const props = defineProps({
    isDarkMode: {
        type: Boolean,
        default: false,
    },
});

const emit = defineEmits(["back-library", "start-editor", "toggle-theme"]);

const GITHUB_REPO_URL = "https://github.com/Courtshipfy/NarrRail";

function openGithubRepo() {
    window.open(GITHUB_REPO_URL, "_blank", "noopener,noreferrer");
}

const scrollRoot = ref(null);
const sceneRefs = ref([]);
const visibleIds = ref(new Set());
const activeIndex = ref(0);

let observer = null;
let wheelHandler = null;
let wheelLocked = false;
let animationFrameId = null;
let animationToken = 0;
const SCROLL_DURATION_MS = 780;

const sections = [
    {
        id: "intro",
        tone: "tone-hero",
        kicker: "NarrRail Overview",
        title: "NarrRail\n一条面向叙事游戏的完整生产链路",
        subline:
            "把剧情设计、结构校验、脚本交付与引擎运行连接在一起，让团队围绕统一语义持续迭代。",
        highlights: [
            {
                title: "一体化协作",
                desc: "编剧、程序与技术美术在同一套资产模型上工作，减少沟通与转译成本。",
            },
            {
                title: "从创作到运行",
                desc: "Web 编辑器产出的内容可直接进入 YAML 与 UE 运行时流程，路径清晰可追踪。",
            },
            {
                title: "面向生产",
                desc: "支持持续迭代与版本管理，不是一次性 Demo 工具，而是可落地的工作流。",
            },
        ],
    },
    {
        id: "hero",
        tone: "tone-hero",
        kicker: "NarrRail",
        title: "让剧情创作成为可执行的产品流程",
        subline:
            "从 Web 编辑、到 YAML 协议、再到 Unreal 运行时，一条链路打通创作与实现。",
        highlights: [
            {
                title: "统一资产语义",
                desc: "剧情节点、条件与动作都使用同一套数据模型，减少跨工具翻译损耗。",
            },
            {
                title: "端到端可追踪",
                desc: "每次剧情修改都能在 YAML 与 UE 运行结果上对齐验证，定位问题更直接。",
            },
            {
                title: "面向迭代",
                desc: "流程设计支持反复改稿，不必靠手动重建资产来维持版本一致性。",
            },
        ],
    },
    {
        id: "editor",
        tone: "tone-editor",
        kicker: "NarrRailEditor",
        title: "创作速度，不以牺牲结构正确性为代价",
        subline:
            "节点编排、条件分支、实时校验同步进行，减少返工，让剧本迭代更像设计流程。",
        highlights: [
            {
                title: "图形化叙事编排",
                desc: "支持对话、选项、跳转与条件路径，在画布上直接表达叙事结构。",
            },
            {
                title: "创作效率工具",
                desc: "拖拽连线、右键建点与自动排布配合使用，快速完成大纲到细化的推进。",
            },
            {
                title: "质量保障机制",
                desc: "实时校验与手动校验并存，尽早暴露空节点、断链和条件配置异常。",
            },
        ],
    },
    {
        id: "runtime",
        tone: "tone-runtime",
        kicker: "UE Plugin",
        title: "运行时能力，直接落在引擎里",
        subline:
            "会话推进、分支选择、条件执行、动作触发，都在 Blueprint 可接入的运行接口中完成。",
        highlights: [
            {
                title: "会话驱动 API",
                desc: "通过 Start / Next / Choose 等接口控制剧情推进，便于封装到游戏逻辑。",
            },
            {
                title: "分支执行策略",
                desc: "支持单路径与穷举式推进模式，满足剧情播放与测试验证两类场景。",
            },
            {
                title: "引擎内调试",
                desc: "在 PIE 环境直接观察变量变化、条件命中和事件触发，缩短排错路径。",
            },
        ],
    },
    {
        id: "pipeline",
        tone: "tone-pipeline",
        kicker: "Pipeline",
        title: "四步一闭环，版本稳定推进",
        subline: "创作与校验 -> 导出 YAML -> UE 导入执行 -> PIE 调试回流。",
        highlights: [
            {
                title: "步骤清晰",
                desc: "每个阶段都有明确输入与输出，减少“改了但不知道是否生效”的不确定性。",
            },
            {
                title: "版本可控",
                desc: "YAML 作为中间契约，便于版本管理、差异审查与回退恢复。",
            },
            {
                title: "闭环反馈",
                desc: "运行问题可以直接追溯到脚本层，并快速回到编辑器进行定点修订。",
            },
        ],
    },
    {
        id: "collaboration",
        tone: "tone-collab",
        kicker: "Collaboration",
        title: "不是一个编辑器，而是一套协作系统",
        subline:
            "编剧、客户端、技术美术、制作都在同一资产语义上工作，交付节奏更可控。",
        highlights: [
            {
                title: "编剧可直接验证结构",
                desc: "不依赖程序中间转译，创作阶段就能确认分支完整性与叙事节奏。",
            },
            {
                title: "技术侧接入稳定",
                desc: "客户端与技术美术围绕统一数据格式开发，降低接口歧义和集成成本。",
            },
            {
                title: "制作可量化推进",
                desc: "通过流程节点和校验结果做里程碑验收，减少主观口径差异。",
            },
        ],
    },
    {
        id: "cta",
        tone: "tone-cta",
        kicker: "Ready",
        title: "现在，直接开始编辑",
        subline: "在脚本库创建或打开资产，继续迭代你的剧情系统。",
        highlights: [
            {
                title: "从资产库开始",
                desc: "直接基于现有脚本继续迭代，或新建脚本快速进入下一轮创作。",
            },
            {
                title: "保留上下文",
                desc: "主题设置、编辑状态和关键配置能够延续，减少重复准备动作。",
            },
            {
                title: "持续交付",
                desc: "每次修改都可再次走通导出与运行流程，让剧情资产稳定走向可发布状态。",
            },
        ],
    },
];

function setSceneRef(el, index) {
    if (!el) {
        return;
    }
    sceneRefs.value[index] = el;
}

function scrollToSection(index) {
    const root = scrollRoot.value;
    const target = sceneRefs.value[index];
    if (!root || !target) {
        return;
    }

    activeIndex.value = index;
    wheelLocked = true;

    const startTop = root.scrollTop;
    const endTop = target.offsetTop;
    const distance = endTop - startTop;

    if (Math.abs(distance) < 1) {
        wheelLocked = false;
        return;
    }

    if (animationFrameId) {
        window.cancelAnimationFrame(animationFrameId);
        animationFrameId = null;
    }

    const myToken = ++animationToken;
    const startTime = performance.now();

    const easeOutCubic = (t) => 1 - Math.pow(1 - t, 3);

    const tick = (now) => {
        if (!scrollRoot.value || myToken !== animationToken) {
            return;
        }

        const elapsed = now - startTime;
        const t = Math.min(1, elapsed / SCROLL_DURATION_MS);
        const eased = easeOutCubic(t);
        scrollRoot.value.scrollTop = startTop + distance * eased;

        if (t < 1) {
            animationFrameId = window.requestAnimationFrame(tick);
            return;
        }

        scrollRoot.value.scrollTop = endTop;
        animationFrameId = null;
        wheelLocked = false;
        updateActiveIndex();
    };

    animationFrameId = window.requestAnimationFrame(tick);
}

function updateActiveIndex() {
    if (!scrollRoot.value) {
        return;
    }

    const h = Math.max(scrollRoot.value.clientHeight, 1);
    const next = Math.round(scrollRoot.value.scrollTop / h);
    activeIndex.value = Math.max(0, Math.min(sections.length - 1, next));
}

onMounted(async () => {
    await nextTick();

    if (!scrollRoot.value) {
        return;
    }

    scrollRoot.value.scrollTop = 0;
    activeIndex.value = 0;

    observer = new IntersectionObserver(
        (entries) => {
            const next = new Set(visibleIds.value);

            entries.forEach((entry) => {
                const id = entry.target.getAttribute("data-section-id");
                if (!id) {
                    return;
                }

                if (entry.isIntersecting) {
                    next.add(id);
                }
            });

            visibleIds.value = next;
            updateActiveIndex();
        },
        {
            root: scrollRoot.value,
            threshold: 0.42,
        },
    );

    sceneRefs.value.forEach((el) => {
        if (el) {
            observer.observe(el);
        }
    });

    wheelHandler = (event) => {
        if (!scrollRoot.value) {
            return;
        }

        event.preventDefault();

        if (wheelLocked || Math.abs(event.deltaY) < 8) {
            return;
        }

        const direction = event.deltaY > 0 ? 1 : -1;
        const nextIndex = Math.max(
            0,
            Math.min(sections.length - 1, activeIndex.value + direction),
        );

        if (nextIndex === activeIndex.value) {
            return;
        }

        wheelLocked = true;
        scrollToSection(nextIndex);
    };

    scrollRoot.value.addEventListener("wheel", wheelHandler, {
        passive: false,
    });
    scrollRoot.value.addEventListener("scroll", updateActiveIndex, {
        passive: true,
    });

    updateActiveIndex();
});

onBeforeUnmount(() => {
    if (observer) {
        observer.disconnect();
        observer = null;
    }

    if (scrollRoot.value) {
        if (wheelHandler) {
            scrollRoot.value.removeEventListener("wheel", wheelHandler);
        }
        scrollRoot.value.removeEventListener("scroll", updateActiveIndex);
    }

    if (animationFrameId) {
        window.cancelAnimationFrame(animationFrameId);
        animationFrameId = null;
    }
});
</script>

<style scoped>
.overview-cinematic {
    --bg: #f2f4f7;
    --bg-2: #ffffff;
    --text: #0f172a;
    --muted: #5b6475;
    --line: rgba(15, 23, 42, 0.16);
    --card: rgba(255, 255, 255, 0.66);
    --chip: rgba(255, 255, 255, 0.9);
    --primary: #0f6fff;

    position: relative;
    width: 100%;
    height: 100vh;
    overflow-y: auto;
    overflow-x: hidden;
    scroll-snap-type: none;
    scroll-behavior: auto;
    overscroll-behavior: contain;
    color: var(--text);
    background:
        radial-gradient(
            circle at 14% 10%,
            rgba(82, 123, 255, 0.18),
            transparent 45%
        ),
        radial-gradient(
            circle at 84% 86%,
            rgba(10, 194, 167, 0.15),
            transparent 42%
        ),
        linear-gradient(160deg, var(--bg), var(--bg-2));
}

.overview-cinematic.dark {
    --bg: #070b16;
    --bg-2: #0c1325;
    --text: #eef3fb;
    --muted: #bbc7db;
    --line: rgba(148, 163, 184, 0.34);
    --card: rgba(11, 16, 30, 0.64);
    --chip: rgba(16, 23, 40, 0.84);
    --primary: #69a6ff;

    background:
        radial-gradient(
            circle at 16% 14%,
            rgba(56, 104, 244, 0.32),
            transparent 48%
        ),
        radial-gradient(
            circle at 82% 82%,
            rgba(0, 190, 167, 0.24),
            transparent 46%
        ),
        linear-gradient(160deg, var(--bg), var(--bg-2));
}

.intro-actions {
    margin-top: 18px;
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 12px;
}

.intro-btn {
    border: 1px solid var(--line);
    border-radius: 999px;
    padding: 11px 18px;
    font-size: 13px;
    font-weight: 700;
    letter-spacing: 0.02em;
    cursor: pointer;
    transition:
        transform 180ms ease,
        background 180ms ease,
        border-color 180ms ease,
        color 180ms ease;
}

.intro-btn:hover {
    transform: translateY(-1px);
}

.intro-btn.ghost {
    background: color-mix(in srgb, var(--card) 80%, transparent);
    color: var(--text);
}

.intro-btn.primary {
    background: var(--primary);
    color: #fff;
    border-color: transparent;
}

.progress-nav {
    position: fixed;
    right: 24px;
    top: 50%;
    transform: translateY(-50%);
    z-index: 35;
    display: grid;
    gap: 12px;
}

.dot {
    width: 10px;
    height: 10px;
    border-radius: 999px;
    border: 1px solid var(--line);
    background: transparent;
    cursor: pointer;
    transition:
        transform 180ms ease,
        background 180ms ease,
        border-color 180ms ease;
}

.dot:hover {
    transform: scale(1.15);
}

.dot.active {
    background: var(--primary);
    border-color: var(--primary);
    transform: scale(1.2);
}

.scene {
    height: 100vh;
    width: 100%;
    padding: 110px 54px 54px;
    scroll-snap-align: start;
    scroll-snap-stop: normal;
    display: flex;
    align-items: center;
    opacity: 0;
    transform: translateY(36px);
    transition:
        opacity 560ms ease,
        transform 700ms cubic-bezier(0.2, 0.82, 0.2, 1);
}

.scene.visible {
    opacity: 1;
    transform: translateY(0);
}

.scene-shell {
    width: min(980px, 100%);
    margin: 0 auto;
    display: block;
    align-items: stretch;
}

.intro-shell {
    width: 100%;
}

.intro-hero {
    position: relative;
    overflow: visible;
    padding: 0;
    min-height: min(76vh, 720px);
    display: flex;
    flex-direction: column;
    justify-content: center;
    gap: 20px;
}

.intro-kicker {
    font-size: 12px;
    text-transform: uppercase;
    letter-spacing: 0.24em;
    color: var(--muted);
    opacity: 0.9;
}

.intro-title {
    font-size: clamp(1.9rem, 5.4vw, 5rem);
    line-height: 0.98;
    letter-spacing: -0.02em;
    word-spacing: 0.01em;
    max-width: none;
    width: 100%;
    white-space: pre-line;
}

.intro-subline {
    font-size: clamp(1.05rem, 1.7vw, 1.38rem);
    line-height: 1.75;
    color: color-mix(in srgb, var(--text) 76%, var(--muted));
    max-width: 72ch;
}

.intro-glow {
    position: absolute;
    border-radius: 999px;
    pointer-events: none;
    filter: blur(8px);
    opacity: 0.42;
}

.intro-glow-a {
    width: min(48vw, 520px);
    height: min(48vw, 520px);
    right: -16%;
    top: -22%;
    background: radial-gradient(
        circle,
        rgba(59, 130, 246, 0.42) 0%,
        rgba(59, 130, 246, 0) 66%
    );
    animation: intro-float-a 7.8s ease-in-out infinite;
}

.intro-glow-b {
    width: min(34vw, 360px);
    height: min(34vw, 360px);
    left: -12%;
    bottom: -26%;
    background: radial-gradient(
        circle,
        rgba(16, 185, 129, 0.34) 0%,
        rgba(16, 185, 129, 0) 70%
    );
    animation: intro-float-b 9.4s ease-in-out infinite;
}

.scene.visible .intro-title {
    animation: intro-title-in 720ms cubic-bezier(0.2, 0.9, 0.2, 1) both;
}

.scene.visible .intro-subline {
    animation: intro-subline-in 860ms cubic-bezier(0.2, 0.86, 0.2, 1) both;
    animation-delay: 90ms;
}

.copy-wrap {
    padding: clamp(8px, 1.8vw, 18px);
    display: flex;
    flex-direction: column;
    justify-content: center;
}

.kicker {
    font-size: 12px;
    text-transform: uppercase;
    letter-spacing: 0.16em;
    color: var(--muted);
    margin-bottom: 16px;
}

.headline {
    font-size: clamp(1.2rem, 2.4vw, 2.4rem);
    line-height: 1.08;
    letter-spacing: -0.01em;
    margin-bottom: 16px;
    max-width: none;
    white-space: nowrap;
}

.flow-text {
    background: linear-gradient(
        90deg,
        #ff4d6d 0%,
        #ff8a3d 18%,
        #ffd166 34%,
        #22c55e 50%,
        #38bdf8 67%,
        #6366f1 82%,
        #ec4899 100%
    );
    background-size: 220% 100%;
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
    color: transparent;
    animation: flow-headline-gradient 5.2s ease-in-out infinite;
}

.subline {
    font-size: clamp(1.04rem, 1.25vw, 1.28rem);
    line-height: 1.62;
    color: var(--muted);
    max-width: 56ch;
}

.detail-list {
    margin-top: 18px;
    list-style: none;
    display: grid;
    gap: 10px;
    max-width: 70ch;
}

.detail-item {
    padding-left: 14px;
    border-left: 2px solid var(--line);
}

.detail-title {
    font-size: 0.98rem;
    font-weight: 700;
    line-height: 1.45;
}

.detail-desc {
    margin-top: 2px;
    color: var(--muted);
    font-size: 0.93rem;
    line-height: 1.55;
}

.tone-hero .headline {
    max-width: none;
}

.tone-cta .headline {
    max-width: none;
}

@media (max-width: 1160px) {
    .scene-shell {
        width: min(860px, 100%);
    }
}

@media (max-width: 760px) {
    .intro-hero {
        padding: 0;
        min-height: 68vh;
    }

    .intro-title {
        font-size: clamp(1.6rem, 8.4vw, 2.6rem);
        line-height: 1.02;
    }
    .intro-actions {
        width: 100%;
        gap: 10px;
    }

    .intro-btn {
        width: 100%;
        text-align: center;
    }

    .progress-nav {
        right: 10px;
    }

    .scene {
        padding: 170px 14px 22px;
    }

    .headline {
        font-size: clamp(1.12rem, 4.6vw, 1.6rem);
        white-space: nowrap;
    }
}

@keyframes flow-headline-gradient {
    0% {
        background-position: 0% 50%;
    }
    50% {
        background-position: 100% 50%;
    }
    100% {
        background-position: 0% 50%;
    }
}

@keyframes intro-float-a {
    0%,
    100% {
        transform: translate3d(0, 0, 0) scale(1);
        opacity: 0.38;
    }
    50% {
        transform: translate3d(-10px, 14px, 0) scale(1.06);
        opacity: 0.52;
    }
}

@keyframes intro-float-b {
    0%,
    100% {
        transform: translate3d(0, 0, 0) scale(1);
        opacity: 0.3;
    }
    50% {
        transform: translate3d(16px, -12px, 0) scale(1.1);
        opacity: 0.46;
    }
}

@keyframes intro-title-in {
    from {
        opacity: 0;
        transform: translateY(20px) scale(0.98);
    }
    to {
        opacity: 1;
        transform: translateY(0) scale(1);
    }
}

@keyframes intro-subline-in {
    from {
        opacity: 0;
        transform: translateY(16px);
    }
    to {
        opacity: 1;
        transform: translateY(0);
    }
}
</style>
