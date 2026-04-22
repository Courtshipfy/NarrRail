<script>import { onMount } from 'svelte';
let _onMount = undefined;
export { _onMount as onMount };
let _onDestroy = undefined;
export { _onDestroy as onDestroy };
onMount(() => {
    _onMount?.();
    return _onDestroy;
});
</script>
