<script>import cc from 'classcat';
export let x;
export let y;
export let width = 0;
export let height = 0;
export let borderRadius = 5;
export let color = undefined;
export let shapeRendering;
export let strokeColor = undefined;
export let strokeWidth = 2;
export let selected = false;
let className = '';
export { className as class };
</script>

<rect
  class={cc(['svelte-flow__minimap-node', className])}
  class:selected
  {x}
  {y}
  rx={borderRadius}
  ry={borderRadius}
  {width}
  {height}
  style={`${color ? `fill: ${color};` : ''}${strokeColor ? `stroke: ${strokeColor};` : ''}${
    strokeWidth ? `stroke-width: ${strokeWidth};` : ''
  }`}
  shape-rendering={shapeRendering}
/>
