import { SvelteComponentTyped } from "svelte";
declare const __propDef: {
    props: {
        onMount?: (() => void) | undefined;
        onDestroy?: (() => void) | undefined;
    };
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {};
};
export type CallOnMountProps = typeof __propDef.props;
export type CallOnMountEvents = typeof __propDef.events;
export type CallOnMountSlots = typeof __propDef.slots;
export default class CallOnMount extends SvelteComponentTyped<CallOnMountProps, CallOnMountEvents, CallOnMountSlots> {
}
export {};
