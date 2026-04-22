import { SvelteComponentTyped } from "svelte";
import type { NodeRendererProps } from './types';
declare const __propDef: {
    props: NodeRendererProps;
    events: {
        nodeclick: CustomEvent<{
            node: import("../..").Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
        nodemouseenter: CustomEvent<{
            node: import("../..").Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
        nodemousemove: CustomEvent<{
            node: import("../..").Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
        nodemouseleave: CustomEvent<{
            node: import("../..").Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
        nodedrag: CustomEvent<{
            targetNode: import("../..").Node<Record<string, unknown>, string> | null;
            nodes: import("../..").Node<Record<string, unknown>, string>[];
            event: MouseEvent | TouchEvent;
        }>;
        nodedragstart: CustomEvent<{
            targetNode: import("../..").Node<Record<string, unknown>, string> | null;
            nodes: import("../..").Node<Record<string, unknown>, string>[];
            event: MouseEvent | TouchEvent;
        }>;
        nodedragstop: CustomEvent<{
            targetNode: import("../..").Node<Record<string, unknown>, string> | null;
            nodes: import("../..").Node<Record<string, unknown>, string>[];
            event: MouseEvent | TouchEvent;
        }>;
        nodecontextmenu: CustomEvent<{
            node: import("../..").Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
    } & {
        [evt: string]: CustomEvent<any>;
    };
    slots: {};
};
type NodeRendererProps_ = typeof __propDef.props;
export { NodeRendererProps_ as NodeRendererProps };
export type NodeRendererEvents = typeof __propDef.events;
export type NodeRendererSlots = typeof __propDef.slots;
export default class NodeRenderer extends SvelteComponentTyped<NodeRendererProps_, NodeRendererEvents, NodeRendererSlots> {
}
