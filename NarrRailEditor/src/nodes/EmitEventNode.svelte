<script>
  import IconGlyph from '../components/IconGlyph.svelte';
  import { Handle, Position } from '@xyflow/svelte';

  export let data;
  export let selected = false;

  $: params = data.params || {};
  $: paramEntries = params && typeof params === 'object' && !Array.isArray(params)
    ? Object.entries(params)
    : [];
  $: eventLabel = data.eventType || '未设置';

  function formatParamValue(value) {
    if (typeof value === 'string') return value;
    if (value == null) return '';
    try {
      return JSON.stringify(value);
    } catch {
      return String(value);
    }
  }
</script>

<div class="emitevent-node" class:selected>
  <Handle type="target" position={Position.Left} />

  <div class="node-header">
    <span class="node-icon"><IconGlyph name="notifications_active" /></span>
    <span class="node-type">触发事件</span>
  </div>
  <div class="node-content">
    <div class="node-field">
      <span class="field-label">事件:</span>
      <span class="field-value">{eventLabel}</span>
    </div>
    {#if paramEntries.length > 0}
      <div class="node-field">
        <span class="field-label">参数:</span>
        <div class="params-list">
          {#each paramEntries as [key, value]}
            <div class="param-row">
              <span class="param-key">{key}</span>
              <span class="param-value">{formatParamValue(value)}</span>
            </div>
          {/each}
        </div>
      </div>
    {/if}
  </div>

  <Handle type="source" position={Position.Right} />
</div>

<style>
  .emitevent-node {
    min-width: 200px;
    background: rgba(255, 255, 255, 0.9);
    backdrop-filter: blur(20px);
    border-radius: 16px;
    border: 2px solid rgba(255, 149, 0, 0.3);
    box-shadow: 0 4px 16px rgba(255, 149, 0, 0.15),
                inset 0 1px 0 rgba(255, 255, 255, 0.8);
    transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    cursor: pointer;
  }

  .emitevent-node:hover {
    transform: translateY(-2px);
    box-shadow: 0 8px 24px rgba(255, 149, 0, 0.25),
                inset 0 1px 0 rgba(255, 255, 255, 0.9);
    border-color: rgba(255, 149, 0, 0.5);
  }

  .emitevent-node.selected {
    border-color: rgba(255, 149, 0, 0.72);
    transform: translateY(-1px);
    box-shadow: 0 10px 26px rgba(15, 23, 42, 0.18),
                0 0 0 3px rgba(59, 130, 246, 0.24),
                0 0 0 6px rgba(59, 130, 246, 0.12),
                inset 0 1px 0 rgba(255, 255, 255, 0.95);
  }

  .node-header {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 12px 16px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.05);
    background: linear-gradient(180deg, rgba(255, 149, 0, 0.08) 0%, transparent 100%);
  }

  .node-icon {
    font-size: 20px;
    color: #ff9500;
  }

  .node-type {
    font-family: 'Outfit', sans-serif;
    font-size: 13px;
    font-weight: 700;
    color: #ff9500;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  .node-content {
    padding: 12px 16px;
    display: flex;
    flex-direction: column;
    gap: 8px;
  }

  .node-field {
    display: flex;
    flex-direction: column;
    gap: 4px;
  }

  .field-label {
    font-size: 10px;
    font-weight: 700;
    color: #86868b;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  .field-value {
    font-size: 13px;
    font-weight: 600;
    color: #1d1d1f;
    line-height: 1.4;
    white-space: normal;
    overflow-wrap: anywhere;
    word-break: break-word;
  }

  .params-list {
    display: flex;
    flex-direction: column;
    gap: 4px;
  }

  .param-row {
    display: grid;
    grid-template-columns: minmax(0, 0.8fr) minmax(0, 1fr);
    gap: 6px;
    align-items: baseline;
    font-size: 12px;
    line-height: 1.35;
  }

  .param-key {
    color: #ff9500;
    font-weight: 700;
    overflow-wrap: anywhere;
    word-break: break-word;
  }

  .param-value {
    color: #1d1d1f;
    font-weight: 600;
    overflow-wrap: anywhere;
    word-break: break-word;
  }
</style>
