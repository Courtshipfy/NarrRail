<script>
  import { MATERIAL_SYMBOL_PATHS } from '../utils/icon-paths.js';

  export let name;
  let className = '';
  export { className as class };

  $: iconPaths = MATERIAL_SYMBOL_PATHS[name] ?? MATERIAL_SYMBOL_PATHS.help;
</script>

<svg
  class={`icon-glyph ${className}`.trim()}
  viewBox="0 0 24 24"
  aria-hidden="true"
  focusable="false"
>
  {#each iconPaths as path}
    <path d={path} />
  {/each}
</svg>

<style>
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
