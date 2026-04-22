import { SvelteComponentTyped } from "svelte";
import type { ResizeControlProps } from './types';
declare const __propDef: {
    props: ResizeControlProps;
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {
        default: {};
    };
};
type ResizeControlProps_ = typeof __propDef.props;
export { ResizeControlProps_ as ResizeControlProps };
export type ResizeControlEvents = typeof __propDef.events;
export type ResizeControlSlots = typeof __propDef.slots;
export default class ResizeControl extends SvelteComponentTyped<ResizeControlProps_, ResizeControlEvents, ResizeControlSlots> {
}
