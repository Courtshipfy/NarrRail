<script>import cc from 'classcat';
export let className = undefined;
export let bgColor = undefined;
export let bgColorHover = undefined;
export let color = undefined;
export let colorHover = undefined;
export let borderColor = undefined;
export { className as class };
</script>

<button
  type="button"
  on:click
  class={cc(['svelte-flow__controls-button', className])}
  style:--xy-controls-button-background-color-props={bgColor}
  style:--xy-controls-button-background-color-hover-props={bgColorHover}
  style:--xy-controls-button-color-props={color}
  style:--xy-controls-button-color-hover-props={colorHover}
  style:--xy-controls-button-border-color-props={borderColor}
  {...$$restProps}
>
  <slot class="button-svg" />
</button>
