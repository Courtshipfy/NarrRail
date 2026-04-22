<script>import { Position } from '@xyflow/system';
import { Handle } from '../Handle';
export let data = { label: 'Node' };
export let targetPosition = undefined;
export let sourcePosition = undefined;
// this is a workaround for suppressing the warning about unused props
$$restProps;
</script>

<Handle type="target" position={targetPosition ?? Position.Top} />
{data?.label}
<Handle type="source" position={sourcePosition ?? Position.Bottom} />
