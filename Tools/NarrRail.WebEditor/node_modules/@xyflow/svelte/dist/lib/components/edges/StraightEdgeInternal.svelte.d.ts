import { SvelteComponentTyped } from "svelte";
import type { EdgeProps } from '../../types';
declare const __propDef: {
    props: EdgeProps;
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {};
};
export type StraightEdgeInternalProps = typeof __propDef.props;
export type StraightEdgeInternalEvents = typeof __propDef.events;
export type StraightEdgeInternalSlots = typeof __propDef.slots;
export default class StraightEdgeInternal extends SvelteComponentTyped<StraightEdgeInternalProps, StraightEdgeInternalEvents, StraightEdgeInternalSlots> {
}
export {};
