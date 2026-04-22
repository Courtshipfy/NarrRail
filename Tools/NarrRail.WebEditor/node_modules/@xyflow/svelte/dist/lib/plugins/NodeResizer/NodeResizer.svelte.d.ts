import { SvelteComponentTyped } from "svelte";
import type { NodeResizerProps } from './types';
declare const __propDef: {
    props: NodeResizerProps;
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {};
};
type NodeResizerProps_ = typeof __propDef.props;
export { NodeResizerProps_ as NodeResizerProps };
export type NodeResizerEvents = typeof __propDef.events;
export type NodeResizerSlots = typeof __propDef.slots;
export default class NodeResizer extends SvelteComponentTyped<NodeResizerProps_, NodeResizerEvents, NodeResizerSlots> {
}
