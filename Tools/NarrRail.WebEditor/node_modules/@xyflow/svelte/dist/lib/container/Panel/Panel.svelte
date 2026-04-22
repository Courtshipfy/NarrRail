<script>import cc from 'classcat';
import { useStore } from '../../store';
export let position = 'top-right';
export let style = undefined;
let className = undefined;
export { className as class };
const { selectionRectMode } = useStore();
$: positionClasses = `${position}`.split('-');
</script>

<div
  class={cc(['svelte-flow__panel', className, ...positionClasses])}
  {style}
  style:pointer-events={$selectionRectMode ? 'none' : ''}
  {...$$restProps}
>
  <slot />
</div>
