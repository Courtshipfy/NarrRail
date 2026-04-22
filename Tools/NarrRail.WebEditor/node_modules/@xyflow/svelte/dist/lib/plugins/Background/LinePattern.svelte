<script>import cc from 'classcat';
export let lineWidth = 1;
export let dimensions;
export let variant = undefined;
let className = '';
export { className as class };
</script>

<path
  stroke-width={lineWidth}
  d={`M${dimensions[0] / 2} 0 V${dimensions[1]} M0 ${dimensions[1] / 2} H${dimensions[0]}`}
  class={cc(['svelte-flow__background-pattern', variant, className])}
/>
