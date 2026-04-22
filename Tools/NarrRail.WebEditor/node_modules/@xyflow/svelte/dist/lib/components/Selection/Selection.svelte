<script>export let x = 0;
export let y = 0;
export let width = 0;
export let height = 0;
export let isVisible = true;
</script>

{#if isVisible}
  <div
    class="svelte-flow__selection"
    style:width={typeof width === 'string' ? width : `${width}px`}
    style:height={typeof height === 'string' ? height : `${height}px`}
    style:transform={`translate(${x}px, ${y}px)`}
  />
{/if}

<style>
  .svelte-flow__selection {
    position: absolute;
    top: 0;
    left: 0;
  }
</style>
