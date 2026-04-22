import { SvelteComponentTyped } from "svelte";
import type { EdgeProps } from '../../types';
declare const __propDef: {
    props: EdgeProps;
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {};
};
export type SmoothStepEdgeInternalProps = typeof __propDef.props;
export type SmoothStepEdgeInternalEvents = typeof __propDef.events;
export type SmoothStepEdgeInternalSlots = typeof __propDef.slots;
export default class SmoothStepEdgeInternal extends SvelteComponentTyped<SmoothStepEdgeInternalProps, SmoothStepEdgeInternalEvents, SmoothStepEdgeInternalSlots> {
}
export {};
