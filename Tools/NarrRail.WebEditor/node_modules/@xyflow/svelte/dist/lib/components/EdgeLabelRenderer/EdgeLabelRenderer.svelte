<script>import portal from '../../actions/portal';
import { useStore } from '../../store';
const { domNode } = useStore();
</script>

<div use:portal={{ target: '.svelte-flow__edgelabel-renderer', domNode: $domNode }}>
  <slot />
</div>
