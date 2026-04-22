import { SvelteComponentTyped } from "svelte";
import type { BaseEdgeProps } from '../BaseEdge/types';
declare const __propDef: {
    props: {
        style?: BaseEdgeProps['labelStyle'];
        x?: BaseEdgeProps['labelX'];
        y?: BaseEdgeProps['labelY'];
    };
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {
        default: {};
    };
};
export type EdgeLabelProps = typeof __propDef.props;
export type EdgeLabelEvents = typeof __propDef.events;
export type EdgeLabelSlots = typeof __propDef.slots;
export default class EdgeLabel extends SvelteComponentTyped<EdgeLabelProps, EdgeLabelEvents, EdgeLabelSlots> {
}
export {};
