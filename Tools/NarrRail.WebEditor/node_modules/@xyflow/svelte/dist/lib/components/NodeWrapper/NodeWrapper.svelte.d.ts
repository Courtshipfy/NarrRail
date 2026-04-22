import { SvelteComponentTyped } from "svelte";
declare const __propDef: {
    props: Pick<import("../../types").Node, "id" | "data" | "width" | "height" | "sourcePosition" | "targetPosition" | "dragHandle" | "parentId" | "type" | "dragging" | "selectable" | "deletable" | "selected" | "draggable" | "hidden" | "connectable" | "initialWidth" | "initialHeight" | "class" | "style"> & {
        measuredWidth?: number | undefined;
        measuredHeight?: number | undefined;
        type: string;
        positionX: number;
        positionY: number;
        'on:nodeclick'?: ((event: MouseEvent) => void) | undefined;
        resizeObserver?: ResizeObserver | null | undefined;
        isParent?: boolean | undefined;
        zIndex: number;
        node: import("../../types").InternalNode;
        initialized: boolean;
        nodeClickDistance?: number | undefined;
    };
    events: {
        nodeclick: CustomEvent<{
            node: import("../../types").Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
        nodecontextmenu: CustomEvent<{
            node: import("../../types").Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
        nodedrag: CustomEvent<{
            targetNode: import("../../types").Node<Record<string, unknown>, string> | null;
            nodes: import("../../types").Node<Record<string, unknown>, string>[];
            event: MouseEvent | TouchEvent;
        }>;
        nodedragstart: CustomEvent<{
            targetNode: import("../../types").Node<Record<string, unknown>, string> | null;
            nodes: import("../../types").Node<Record<string, unknown>, string>[];
            event: MouseEvent | TouchEvent;
        }>;
        nodedragstop: CustomEvent<{
            targetNode: import("../../types").Node<Record<string, unknown>, string> | null;
            nodes: import("../../types").Node<Record<string, unknown>, string>[];
            event: MouseEvent | TouchEvent;
        }>;
        nodemouseenter: CustomEvent<{
            node: import("../../types").Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
        nodemouseleave: CustomEvent<{
            node: import("../../types").Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
        nodemousemove: CustomEvent<{
            node: import("../../types").Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
    } & {
        [evt: string]: CustomEvent<any>;
    };
    slots: {};
};
type NodeWrapperProps_ = typeof __propDef.props;
export { NodeWrapperProps_ as NodeWrapperProps };
export type NodeWrapperEvents = typeof __propDef.events;
export type NodeWrapperSlots = typeof __propDef.slots;
export default class NodeWrapper extends SvelteComponentTyped<NodeWrapperProps_, NodeWrapperEvents, NodeWrapperSlots> {
}
