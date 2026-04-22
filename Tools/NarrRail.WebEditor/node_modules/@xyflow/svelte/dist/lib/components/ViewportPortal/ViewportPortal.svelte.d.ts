import { SvelteComponentTyped } from "svelte";
declare const __propDef: {
    props: Record<string, never>;
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {
        default: {};
    };
};
export type ViewportPortalProps = typeof __propDef.props;
export type ViewportPortalEvents = typeof __propDef.events;
export type ViewportPortalSlots = typeof __propDef.slots;
export default class ViewportPortal extends SvelteComponentTyped<ViewportPortalProps, ViewportPortalEvents, ViewportPortalSlots> {
}
export {};
