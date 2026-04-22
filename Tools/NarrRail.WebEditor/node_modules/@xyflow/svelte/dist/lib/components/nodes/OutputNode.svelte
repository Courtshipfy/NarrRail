<script>import { Position } from '@xyflow/system';
import { Handle } from '../Handle';
export let data = { label: 'Node' };
export let targetPosition = undefined;
// this is a workaround for suppressing the warning about unused props
$$restProps;
</script>

{data?.label}
<Handle type="target" position={targetPosition ?? Position.Top} />
