import { SvelteComponentTyped } from "svelte";
import type { BezierEdgeProps } from '../../types';
declare const __propDef: {
    props: BezierEdgeProps;
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {};
};
type BezierEdgeProps_ = typeof __propDef.props;
export { BezierEdgeProps_ as BezierEdgeProps };
export type BezierEdgeEvents = typeof __propDef.events;
export type BezierEdgeSlots = typeof __propDef.slots;
export default class BezierEdge extends SvelteComponentTyped<BezierEdgeProps_, BezierEdgeEvents, BezierEdgeSlots> {
}
