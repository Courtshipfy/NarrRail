import { SvelteComponentTyped } from "svelte";
import type { StraightEdgeProps } from '../../types';
declare const __propDef: {
    props: StraightEdgeProps;
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {};
};
type StraightEdgeProps_ = typeof __propDef.props;
export { StraightEdgeProps_ as StraightEdgeProps };
export type StraightEdgeEvents = typeof __propDef.events;
export type StraightEdgeSlots = typeof __propDef.slots;
export default class StraightEdge extends SvelteComponentTyped<StraightEdgeProps_, StraightEdgeEvents, StraightEdgeSlots> {
}
