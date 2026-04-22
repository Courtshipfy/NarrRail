import { SvelteComponentTyped } from "svelte";
import type { StepEdgeProps } from '../../types';
declare const __propDef: {
    props: StepEdgeProps;
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {};
};
type StepEdgeProps_ = typeof __propDef.props;
export { StepEdgeProps_ as StepEdgeProps };
export type StepEdgeEvents = typeof __propDef.events;
export type StepEdgeSlots = typeof __propDef.slots;
export default class StepEdge extends SvelteComponentTyped<StepEdgeProps_, StepEdgeEvents, StepEdgeSlots> {
}
