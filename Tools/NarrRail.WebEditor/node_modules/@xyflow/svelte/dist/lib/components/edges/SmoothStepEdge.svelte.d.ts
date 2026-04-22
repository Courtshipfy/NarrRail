import { SvelteComponentTyped } from "svelte";
import type { SmoothStepEdgeProps } from '../../types';
declare const __propDef: {
    props: SmoothStepEdgeProps;
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {};
};
type SmoothStepEdgeProps_ = typeof __propDef.props;
export { SmoothStepEdgeProps_ as SmoothStepEdgeProps };
export type SmoothStepEdgeEvents = typeof __propDef.events;
export type SmoothStepEdgeSlots = typeof __propDef.slots;
export default class SmoothStepEdge extends SvelteComponentTyped<SmoothStepEdgeProps_, SmoothStepEdgeEvents, SmoothStepEdgeSlots> {
}
