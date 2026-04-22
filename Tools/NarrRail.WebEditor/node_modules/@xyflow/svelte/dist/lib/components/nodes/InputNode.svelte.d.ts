import { SvelteComponentTyped } from "svelte";
declare const __propDef: {
    props: Pick<import("../../types").Node<Record<string, unknown>, string>, "id" | "data" | "width" | "height" | "sourcePosition" | "targetPosition" | "dragHandle" | "parentId"> & Required<Pick<import("../../types").Node<Record<string, unknown>, string>, "type" | "dragging" | "zIndex" | "selectable" | "deletable" | "selected" | "draggable">> & {
        isConnectable: boolean;
        positionAbsoluteX: number;
        positionAbsoluteY: number;
    } & {
        type: any;
    };
    events: {
        [evt: string]: CustomEvent<any>;
    };
    slots: {};
};
export type InputNodeProps = typeof __propDef.props;
export type InputNodeEvents = typeof __propDef.events;
export type InputNodeSlots = typeof __propDef.slots;
export default class InputNode extends SvelteComponentTyped<InputNodeProps, InputNodeEvents, InputNodeSlots> {
}
export {};
