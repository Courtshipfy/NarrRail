<script>import { shortcut } from '@svelte-put/shortcut';
import { isInputDOMNode, isMacOs } from '@xyflow/system';
import { useStore } from '../../store';
export let selectionKey = 'Shift';
export let multiSelectionKey = isMacOs() ? 'Meta' : 'Control';
export let deleteKey = 'Backspace';
export let panActivationKey = ' ';
export let zoomActivationKey = isMacOs() ? 'Meta' : 'Control';
const { selectionKeyPressed, multiselectionKeyPressed, deleteKeyPressed, panActivationKeyPressed, zoomActivationKeyPressed, selectionRect } = useStore();
function isKeyObject(key) {
    return key !== null && typeof key === 'object';
}
function getModifier(key) {
    return isKeyObject(key) ? key.modifier || [] : [];
}
function getKeyString(key) {
    if (key === null || key === undefined) {
        // this is a workaround to check if a key is set
        // if not we won't call the callback
        return '';
    }
    return isKeyObject(key) ? key.key : key;
}
function getShortcutTrigger(key, callback) {
    const keys = Array.isArray(key) ? key : [key];
    return keys.map((_key) => {
        const keyString = getKeyString(_key);
        return {
            key: keyString,
            modifier: getModifier(_key),
            enabled: keyString !== null,
            callback
        };
    });
}
function resetKeysAndSelection() {
    selectionRect.set(null);
    selectionKeyPressed.set(false);
    multiselectionKeyPressed.set(false);
    deleteKeyPressed.set(false);
    panActivationKeyPressed.set(false);
    zoomActivationKeyPressed.set(false);
}
</script>

<svelte:window
  on:blur={resetKeysAndSelection}
  on:contextmenu={resetKeysAndSelection}
  use:shortcut={{
    trigger: getShortcutTrigger(selectionKey, () => selectionKeyPressed.set(true)),
    type: 'keydown'
  }}
  use:shortcut={{
    trigger: getShortcutTrigger(selectionKey, () => selectionKeyPressed.set(false)),
    type: 'keyup'
  }}
  use:shortcut={{
    trigger: getShortcutTrigger(multiSelectionKey, () => multiselectionKeyPressed.set(true)),
    type: 'keydown'
  }}
  use:shortcut={{
    trigger: getShortcutTrigger(multiSelectionKey, () => multiselectionKeyPressed.set(false)),
    type: 'keyup'
  }}
  use:shortcut={{
    trigger: getShortcutTrigger(deleteKey, (detail) => {
      const isModifierKey =
        detail.originalEvent.ctrlKey ||
        detail.originalEvent.metaKey ||
        detail.originalEvent.shiftKey;
      if (!isModifierKey && !isInputDOMNode(detail.originalEvent)) {
        deleteKeyPressed.set(true);
      }
    }),
    type: 'keydown'
  }}
  use:shortcut={{
    trigger: getShortcutTrigger(deleteKey, () => deleteKeyPressed.set(false)),
    type: 'keyup'
  }}
  use:shortcut={{
    trigger: getShortcutTrigger(panActivationKey, () => panActivationKeyPressed.set(true)),
    type: 'keydown'
  }}
  use:shortcut={{
    trigger: getShortcutTrigger(panActivationKey, () => panActivationKeyPressed.set(false)),
    type: 'keyup'
  }}
  use:shortcut={{
    trigger: getShortcutTrigger(zoomActivationKey, () => zoomActivationKeyPressed.set(true)),
    type: 'keydown'
  }}
  use:shortcut={{
    trigger: getShortcutTrigger(zoomActivationKey, () => zoomActivationKeyPressed.set(false)),
    type: 'keyup'
  }}
/>
