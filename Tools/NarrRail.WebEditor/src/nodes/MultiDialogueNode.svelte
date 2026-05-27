<script>
  import IconGlyph from '../components/IconGlyph.svelte';
  import { Handle, Position } from '@xyflow/svelte';

  export let data;
  export let selected = false;

  $: speakerLabel = data?.speakerId ? data.speakerId : '旁白';
  $: lines = Array.isArray(data?.lines) ? data.lines : [];
  $: previewLines = lines
    .map((line) => {
      if (!line) return '';
      if (typeof line === 'string') return line;
      return line.textKey || '';
    })
    .filter((text) => String(text || '').trim().length > 0);
</script>

<div class="multi-dialogue-node" class:selected>
  <Handle type="target" position={Position.Left} />

  <div class="node-header">
    <span class="node-icon"><IconGlyph name="speaker_notes" /></span>
    <span class="node-type">多行对话</span>
  </div>

  <div class="node-content">
    <div class="node-field speaker-inline-field">
      <span class="field-label">角色:</span>
      <span class="field-value">{speakerLabel}</span>
    </div>

    <div class="node-field">
      <span class="field-label">内容:</span>
      {#if lines.length === 0}
        <span class="field-value text-preview">（未设置）</span>
      {:else}
        <div class="multi-preview">
          {#each previewLines as text}
            <div class="line-item">{text}</div>
          {/each}
        </div>
      {/if}
    </div>
  </div>

  <Handle type="source" position={Position.Right} />
</div>

<style>
  .multi-dialogue-node {
    min-width: 220px;
    background: rgba(255, 255, 255, 0.9);
    backdrop-filter: blur(20px);
    border-radius: 16px;
    border: 2px solid rgba(59, 130, 246, 0.3);
    box-shadow: 0 4px 16px rgba(59, 130, 246, 0.16),
                inset 0 1px 0 rgba(255, 255, 255, 0.8);
    transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    cursor: pointer;
  }

  .multi-dialogue-node:hover {
    transform: translateY(-2px);
    box-shadow: 0 8px 24px rgba(59, 130, 246, 0.25),
                inset 0 1px 0 rgba(255, 255, 255, 0.9);
    border-color: rgba(59, 130, 246, 0.5);
  }

  .multi-dialogue-node.selected {
    border-color: rgba(59, 130, 246, 0.92);
    box-shadow:
      0 12px 30px rgba(15, 23, 42, 0.18),
      0 0 0 2px rgba(255, 255, 255, 0.95),
      0 0 0 6px rgba(59, 130, 246, 0.22),
      inset 0 1px 0 rgba(255, 255, 255, 0.95);
    transform: translateY(-1px);
  }

  .node-header {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 12px 16px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.05);
    background: linear-gradient(180deg, rgba(59, 130, 246, 0.1) 0%, transparent 100%);
  }

  .node-icon {
    font-size: 20px;
    color: #3b82f6;
  }

  .node-type {
    font-family: 'Outfit', sans-serif;
    font-size: 13px;
    font-weight: 700;
    color: #2563eb;
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
  }

  .multi-preview {
    display: flex;
    flex-direction: column;
    gap: 4px;
  }

  .line-item {
    font-size: 12px;
    line-height: 1.4;
    color: #1e293b;
    white-space: normal;
    overflow-wrap: anywhere;
    word-break: break-word;
  }

  .speaker-inline-field {
    flex-direction: row;
    align-items: center;
    gap: 6px;
  }

  :global(body.dark-theme) .multi-dialogue-node,
  :global(body[data-theme="dark"]) .multi-dialogue-node {
    background: rgba(15, 23, 42, 0.86);
    border-color: rgba(96, 165, 250, 0.42);
    box-shadow: 0 6px 20px rgba(2, 6, 23, 0.45),
                inset 0 1px 0 rgba(255, 255, 255, 0.06);
  }

  :global(body.dark-theme) .multi-dialogue-node:hover,
  :global(body[data-theme="dark"]) .multi-dialogue-node:hover {
    border-color: rgba(96, 165, 250, 0.62);
    box-shadow: 0 10px 28px rgba(2, 6, 23, 0.58),
                inset 0 1px 0 rgba(255, 255, 255, 0.08);
  }

  :global(body.dark-theme) .multi-dialogue-node.selected,
  :global(body[data-theme="dark"]) .multi-dialogue-node.selected {
    border-color: rgba(147, 197, 253, 0.95);
    box-shadow:
      0 14px 32px rgba(2, 6, 23, 0.62),
      0 0 0 2px rgba(15, 23, 42, 0.95),
      0 0 0 6px rgba(96, 165, 250, 0.28),
      inset 0 1px 0 rgba(255, 255, 255, 0.12);
  }

  :global(body.dark-theme) .multi-dialogue-node .node-header,
  :global(body[data-theme="dark"]) .multi-dialogue-node .node-header {
    border-bottom-color: rgba(148, 163, 184, 0.24);
    background: linear-gradient(180deg, rgba(59, 130, 246, 0.2) 0%, transparent 100%);
  }

  :global(body.dark-theme) .multi-dialogue-node .node-icon,
  :global(body[data-theme="dark"]) .multi-dialogue-node .node-icon {
    color: #93c5fd;
  }

  :global(body.dark-theme) .multi-dialogue-node .node-type,
  :global(body[data-theme="dark"]) .multi-dialogue-node .node-type {
    color: #bfdbfe;
  }

  :global(body.dark-theme) .multi-dialogue-node .field-label,
  :global(body[data-theme="dark"]) .multi-dialogue-node .field-label {
    color: #94a3b8;
  }

  :global(body.dark-theme) .multi-dialogue-node .field-value,
  :global(body[data-theme="dark"]) .multi-dialogue-node .field-value,
  :global(body.dark-theme) .multi-dialogue-node .line-item,
  :global(body[data-theme="dark"]) .multi-dialogue-node .line-item {
    color: #e2e8f0;
  }



</style>
