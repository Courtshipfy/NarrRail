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
export type DefaultNodeProps = typeof __propDef.props;
export type DefaultNodeEvents = typeof __propDef.events;
export type DefaultNodeSlots = typeof __propDef.slots;
export default class DefaultNode extends SvelteComponentTyped<DefaultNodeProps, DefaultNodeEvents, DefaultNodeSlots> {
}
export {};
