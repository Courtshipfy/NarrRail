import { SvelteComponentTyped } from "svelte";
declare const __propDef: {
    props: {
        class?: string | undefined;
    } & import("svelte/elements").HTMLButtonAttributes & {
        class?: string | undefined;
        bgColor?: string | undefined;
        bgColorHover?: string | undefined;
        color?: string | undefined;
        colorHover?: string | undefined;
        borderColor?: string | undefined;
    };
    events: {
        click: MouseEvent;
    } & {
        [evt: string]: CustomEvent<any>;
    };
    slots: {
        default: {
            class: string;
        };
    };
};
type ControlButtonProps_ = typeof __propDef.props;
export { ControlButtonProps_ as ControlButtonProps };
export type ControlButtonEvents = typeof __propDef.events;
export type ControlButtonSlots = typeof __propDef.slots;
export default class ControlButton extends SvelteComponentTyped<ControlButtonProps_, ControlButtonEvents, ControlButtonSlots> {
}
