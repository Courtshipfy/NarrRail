<script labg="ts" context="module">
  const defaultSize = {
    [BackgroundVariant.Dots]: 1,
    [BackgroundVariant.Lines]: 1,
    [BackgroundVariant.Cross]: 6
  };
</script>

<script>import cc from 'classcat';
import DotPattern from './DotPattern.svelte';
import LinePattern from './LinePattern.svelte';
import { useStore } from '../../store';
import { BackgroundVariant } from './types';
export let id = undefined;
export let variant = BackgroundVariant.Dots;
export let gap = 20;
export let size = 1;
export let lineWidth = 1;
export let bgColor = undefined;
export let patternColor = undefined;
export let patternClass = undefined;
let className = '';
export { className as class };
const { viewport, flowId } = useStore();
const patternSize = size || defaultSize[variant];
const isDots = variant === BackgroundVariant.Dots;
const isCross = variant === BackgroundVariant.Cross;
const gapXY = Array.isArray(gap) ? gap : [gap, gap];
$: patternId = `background-pattern-${$flowId}-${id ? id : ''}`;
$: scaledGap = [gapXY[0] * $viewport.zoom || 1, gapXY[1] * $viewport.zoom || 1];
$: scaledSize = patternSize * $viewport.zoom;
$: patternDimensions = (isCross ? [scaledSize, scaledSize] : scaledGap);
$: patternOffset = isDots
    ? [scaledSize / 2, scaledSize / 2]
    : [patternDimensions[0] / 2, patternDimensions[1] / 2];
</script>

<svg
  class={cc(['svelte-flow__background', className])}
  data-testid="svelte-flow__background"
  style:--xy-background-color-props={bgColor}
  style:--xy-background-pattern-color-props={patternColor}
>
  <pattern
    id={patternId}
    x={$viewport.x % scaledGap[0]}
    y={$viewport.y % scaledGap[1]}
    width={scaledGap[0]}
    height={scaledGap[1]}
    patternUnits="userSpaceOnUse"
    patternTransform={`translate(-${patternOffset[0]},-${patternOffset[1]})`}
  >
    {#if isDots}
      <DotPattern radius={scaledSize / 2} class={patternClass} />
    {:else}
      <LinePattern dimensions={patternDimensions} {variant} {lineWidth} class={patternClass} />
    {/if}
  </pattern>
  <rect x="0" y="0" width="100%" height="100%" fill={`url(#${patternId})`} />
</svg>

<style>
  .svelte-flow__background {
    position: absolute;
    width: 100%;
    height: 100%;
    top: 0;
    left: 0;
  }
</style>
