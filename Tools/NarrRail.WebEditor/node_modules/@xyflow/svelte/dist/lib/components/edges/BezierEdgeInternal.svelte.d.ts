import { SvelteComponentTyped } from "svelte";
import type { EdgeProps } from '../../types';
declare const __propDef: {
    props: EdgeProps;
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {};
};
export type BezierEdgeInternalProps = typeof __propDef.props;
export type BezierEdgeInternalEvents = typeof __propDef.events;
export type BezierEdgeInternalSlots = typeof __propDef.slots;
export default class BezierEdgeInternal extends SvelteComponentTyped<BezierEdgeInternalProps, BezierEdgeInternalEvents, BezierEdgeInternalSlots> {
}
export {};
