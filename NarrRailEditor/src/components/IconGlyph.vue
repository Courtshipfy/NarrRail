<template>
    <svg
        class="icon-glyph"
        viewBox="0 0 24 24"
        aria-hidden="true"
        focusable="false"
    >
        <path
            v-for="path in iconPaths"
            :key="path"
            :d="path"
        />
    </svg>
</template>

<script setup>
import { computed } from "vue";
import { MATERIAL_SYMBOL_PATHS } from "../utils/icon-paths.js";

const props = defineProps({
    name: {
        type: String,
        required: true,
    },
});

const iconPaths = computed(
    () => MATERIAL_SYMBOL_PATHS[props.name] ?? MATERIAL_SYMBOL_PATHS.help,
);
</script>

<style scoped>
.icon-glyph {
    width: 1em;
    height: 1em;
    display: block;
    fill: none;
    stroke: currentColor;
    stroke-width: 2;
    stroke-linecap: round;
    stroke-linejoin: round;
    vector-effect: non-scaling-stroke;
}
</style>
