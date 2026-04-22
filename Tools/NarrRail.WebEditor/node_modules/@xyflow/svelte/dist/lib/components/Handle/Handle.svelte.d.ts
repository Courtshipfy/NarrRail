import { SvelteComponentTyped } from "svelte";
import type { HandleProps } from '../../types';
declare const __propDef: {
    props: HandleProps;
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {
        default: {};
    };
};
type HandleProps_ = typeof __propDef.props;
export { HandleProps_ as HandleProps };
export type HandleEvents = typeof __propDef.events;
export type HandleSlots = typeof __propDef.slots;
/** The Handle component is the part of a node that can be used to connect nodes. */
export default class Handle extends SvelteComponentTyped<HandleProps_, HandleEvents, HandleSlots> {
}
