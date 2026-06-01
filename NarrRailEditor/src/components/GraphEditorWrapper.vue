<template>
    <div ref="container" class="svelte-wrapper"></div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, watch } from "vue";
import GraphEditorComponent from "./GraphEditor.svelte";

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
});

const container = ref(null);
let svelteInstance = null;

onMounted(() => {
    if (container.value) {
        try {
            svelteInstance = new GraphEditorComponent({
                target: container.value,
                props: {
                    nodes: props.nodes,
                    edges: props.edges,
                    edgeRenderMode: props.edgeRenderMode,
                    presetSpeakers: props.presetSpeakers,
                },
            });
        } catch (error) {
            console.error("Error creating Svelte instance:", error);
        }
    }
});

onUnmounted(() => {
    if (svelteInstance) {
        svelteInstance.$destroy();
    }
});

// 监听 props 变化并更新 Svelte 组件
watch(
    () => props.nodes,
    (newNodes) => {
        if (svelteInstance) {
            svelteInstance.$set({ nodes: newNodes });
        }
    },
    { deep: true },
);

watch(
    () => props.edges,
    (newEdges) => {
        if (svelteInstance) {
            svelteInstance.$set({ edges: newEdges });
        }
    },
    { deep: true },
);

watch(
    () => props.edgeRenderMode,
    (newEdgeRenderMode) => {
        if (svelteInstance) {
            svelteInstance.$set({ edgeRenderMode: newEdgeRenderMode });
        }
    },
);

watch(
    () => props.presetSpeakers,
    (newPresetSpeakers) => {
        if (svelteInstance) {
            svelteInstance.$set({ presetSpeakers: newPresetSpeakers });
        }
    },
    { deep: true },
);
</script>

<style scoped>
.svelte-wrapper {
    width: 100%;
    height: 100%;
}
</style>
