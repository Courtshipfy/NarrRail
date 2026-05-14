<template>
    <div class="overview-page" :class="{ 'is-dark': isDarkMode }">
        <section class="hero">
            <p class="kicker">NarrRail Platform</p>
            <h1>
                把剧情创作的表达力，
                <span>带进 Unreal 的运行时。</span>
            </h1>
            <p class="hero-subtitle">
                WebEditor 负责创作与校验，UE 插件负责执行与集成。 一份 YAML
                脚本，贯穿从叙事设计到引擎落地的完整链路。
            </p>
            <div class="hero-actions">
                <button class="btn primary" @click="emit('start-editor')">
                    开始创作
                </button>
                <button class="btn secondary" @click="emit('back-library')">
                    返回主页
                </button>
            </div>
            <div class="hero-media glass-morphism">
                <img
                    src="/overview/screen.png"
                    alt="NarrRail WebEditor 操作界面示意"
                />
            </div>
        </section>

        <section class="split-section">
            <div class="copy-block">
                <p class="eyebrow">Web Editor</p>
                <h2>可视化编辑，像排故事一样排流程。</h2>
                <p>
                    节点化编辑、分支拖拽、自动排布、实时校验与 YAML 导入导出，
                    让剧情从“文档描述”变成“结构可验证”的资产。
                </p>
                <ul>
                    <li>对话 / 多行对话 / 选择分支 / 跳转 / 变量 / 事件</li>
                    <li>Choice 支持 Enter 快速新增与拖拽排序</li>
                    <li>阅读模式用于文案审校和节奏检查</li>
                </ul>
            </div>
            <div class="visual-card glass-morphism">
                <img src="/overview/screen.png" alt="Web 编辑器实际操作截图" />
            </div>
        </section>

        <section class="split-section reverse">
            <div class="copy-block">
                <p class="eyebrow">UE Runtime Plugin</p>
                <h2>
                    运行时执行核心，
                    <span>稳定地把剧情跑起来。</span>
                </h2>
                <p>
                    C++ Runtime 负责会话推进、分支选择、条件求值和动作执行；
                    Blueprint 作为业务接入层，便于项目侧调用与事件订阅。
                </p>
                <ul>
                    <li>Start / Next / Choose / Pause / Resume / Stop</li>
                    <li>SinglePass 与 ExhaustiveUntilComplete 双模式</li>
                    <li>PIE 调试与资产校验闭环</li>
                </ul>
            </div>
            <div class="visual-card glass-morphism">
                <img src="/overview/screen.png" alt="UE 插件工作流示意截图" />
            </div>
        </section>

        <section class="flow-section glass-morphism">
            <p class="eyebrow">One Workflow</p>
            <h2>从编辑到运行，只走一条主通路。</h2>
            <div class="flow-steps">
                <div class="step">
                    <span>01</span>
                    <h3>Web 创作与校验</h3>
                    <p>在图编辑器中搭建剧情节点和分支逻辑。</p>
                </div>
                <div class="step">
                    <span>02</span>
                    <h3>导出 YAML</h3>
                    <p>以统一 schema 作为数据交换契约。</p>
                </div>
                <div class="step">
                    <span>03</span>
                    <h3>UE 导入并执行</h3>
                    <p>在 Runtime 中运行并验证交互结果。</p>
                </div>
                <div class="step">
                    <span>04</span>
                    <h3>回流迭代</h3>
                    <p>发现问题后返回 WebEditor 快速修正。</p>
                </div>
            </div>
        </section>
    </div>
</template>

<script setup>
import { computed } from "vue";

const props = defineProps({
    isDarkMode: {
        type: Boolean,
        default: false,
    },
});

const isDarkMode = computed(() => !!props.isDarkMode);
const emit = defineEmits(["back-library", "start-editor"]);
</script>

<style scoped>
.overview-page {
    --max-width: min(1260px, 94vw);
    min-height: 100vh;
    margin: 0 auto;
    padding: 40px 0 72px;
    display: flex;
    flex-direction: column;
    gap: 44px;
}

.hero,
.split-section,
.flow-section {
    width: var(--max-width);
    margin: 0 auto;
}

.kicker,
.eyebrow {
    margin: 0;
    font-size: 12px;
    letter-spacing: 0.14em;
    text-transform: uppercase;
    color: #6b7280;
    font-weight: 700;
}

.hero h1 {
    margin: 10px 0 16px;
    font-size: clamp(42px, 5.3vw, 76px);
    line-height: 1.05;
    letter-spacing: -0.03em;
    font-weight: 800;
    color: #0f172a;
}

.hero h1 span,
.split-section h2 span {
    display: block;
    color: #334155;
}

.hero-subtitle {
    margin: 0;
    max-width: 820px;
    font-size: clamp(16px, 1.6vw, 22px);
    line-height: 1.6;
    color: #475569;
}

.hero-actions {
    margin-top: 22px;
    display: flex;
    gap: 10px;
}

.hero-media {
    margin-top: 28px;
    border-radius: 26px;
    overflow: hidden;
    border: 1px solid rgba(148, 163, 184, 0.25);
    box-shadow: 0 24px 60px rgba(15, 23, 42, 0.13);
    position: relative;
}

.hero-media img,
.visual-card img {
    width: 100%;
    height: auto;
    display: block;
}

.split-section {
    display: grid;
    grid-template-columns: minmax(0, 1fr) minmax(0, 1.15fr);
    gap: 20px;
    align-items: center;
}

.split-section.reverse {
    grid-template-columns: minmax(0, 1.15fr) minmax(0, 1fr);
}

.split-section.reverse .copy-block {
    order: 2;
}

.split-section.reverse .visual-card {
    order: 1;
}

.copy-block {
    padding: 12px 8px;
}

.copy-block h2 {
    margin: 8px 0 14px;
    font-size: clamp(28px, 3vw, 46px);
    line-height: 1.15;
    letter-spacing: -0.02em;
    color: #0f172a;
}

.copy-block p {
    margin: 0;
    color: #475569;
    line-height: 1.65;
    font-size: 16px;
}

.copy-block ul {
    margin: 14px 0 0;
    padding-left: 20px;
    color: #1e293b;
    line-height: 1.8;
}

.visual-card {
    border-radius: 20px;
    border: 1px solid rgba(148, 163, 184, 0.24);
    overflow: hidden;
    box-shadow: 0 18px 40px rgba(15, 23, 42, 0.1);
    position: relative;
}

.flow-section {
    border-radius: 20px;
    border: 1px solid rgba(148, 163, 184, 0.24);
    padding: 26px;
}

.flow-section h2 {
    margin: 8px 0 18px;
    font-size: clamp(28px, 3.2vw, 44px);
    line-height: 1.15;
    letter-spacing: -0.02em;
    color: #0f172a;
}

.flow-steps {
    display: grid;
    grid-template-columns: repeat(4, minmax(0, 1fr));
    gap: 14px;
}

.step {
    background: rgba(255, 255, 255, 0.4);
    border: 1px solid rgba(148, 163, 184, 0.22);
    border-radius: 14px;
    padding: 14px;
}

.step span {
    font-size: 13px;
    font-weight: 700;
    color: #6366f1;
}

.step h3 {
    margin: 8px 0 6px;
    font-size: 16px;
    color: #0f172a;
}

.step p {
    margin: 0;
    color: #475569;
    line-height: 1.5;
    font-size: 14px;
}

.overview-page.is-dark .kicker,
.overview-page.is-dark .eyebrow {
    color: #94a3b8;
}

.overview-page.is-dark .hero h1,
.overview-page.is-dark .copy-block h2,
.overview-page.is-dark .flow-section h2 {
    color: #f1f5f9;
}

.overview-page.is-dark .hero h1 span,
.overview-page.is-dark .split-section h2 span {
    color: #cbd5e1;
}

.overview-page.is-dark .hero-subtitle,
.overview-page.is-dark .copy-block p,
.overview-page.is-dark .step p {
    color: #94a3b8;
}

.overview-page.is-dark .copy-block ul {
    color: #e2e8f0;
}

.overview-page.is-dark .hero-media,
.overview-page.is-dark .visual-card,
.overview-page.is-dark .flow-section {
    border-color: rgba(100, 116, 139, 0.35);
    box-shadow: 0 24px 50px rgba(2, 6, 23, 0.35);
}

.overview-page.is-dark .hero-media img,
.overview-page.is-dark .visual-card img {
    filter: brightness(0.72) saturate(0.86) contrast(1.04);
}

.overview-page.is-dark .hero-media::after,
.overview-page.is-dark .visual-card::after {
    content: "";
    position: absolute;
    inset: 0;
    pointer-events: none;
    background: linear-gradient(
        180deg,
        rgba(15, 23, 42, 0.22) 0%,
        rgba(2, 6, 23, 0.38) 100%
    );
}

.overview-page.is-dark .step {
    background: rgba(15, 23, 42, 0.5);
    border-color: rgba(100, 116, 139, 0.36);
}

.overview-page.is-dark .step h3 {
    color: #e2e8f0;
}

@media (max-width: 1120px) {
    .split-section,
    .split-section.reverse {
        grid-template-columns: 1fr;
    }

    .split-section.reverse .copy-block,
    .split-section.reverse .visual-card {
        order: initial;
    }

    .flow-steps {
        grid-template-columns: repeat(2, minmax(0, 1fr));
    }
}

@media (max-width: 720px) {
    .overview-page {
        padding-top: 26px;
    }

    .hero-actions {
        flex-wrap: wrap;
    }

    .flow-steps {
        grid-template-columns: 1fr;
    }
}
</style>
