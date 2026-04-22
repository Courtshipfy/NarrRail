import { SvelteComponentTyped } from "svelte";
import type { Node } from '../../types';
declare const __propDef: {
    props: Record<string, never>;
    events: {
        nodeclick: CustomEvent<{
            node: Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
        nodecontextmenu: CustomEvent<{
            node: Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
        nodedrag: CustomEvent<{
            targetNode: Node<Record<string, unknown>, string> | null;
            nodes: Node<Record<string, unknown>, string>[];
            event: MouseEvent | TouchEvent;
        }>;
        nodedragstart: CustomEvent<{
            targetNode: Node<Record<string, unknown>, string> | null;
            nodes: Node<Record<string, unknown>, string>[];
            event: MouseEvent | TouchEvent;
        }>;
        nodedragstop: CustomEvent<{
            targetNode: Node<Record<string, unknown>, string> | null;
            nodes: Node<Record<string, unknown>, string>[];
            event: MouseEvent | TouchEvent;
        }>;
        nodemouseenter: CustomEvent<{
            node: Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
        nodemouseleave: CustomEvent<{
            node: Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
        nodemousemove: CustomEvent<{
            node: Node<Record<string, unknown>, string>;
            event: MouseEvent | TouchEvent;
        }>;
        selectioncontextmenu: CustomEvent<{
            nodes: Node[];
            event: MouseEvent | TouchEvent;
        }>;
        selectionclick: CustomEvent<{
            nodes: Node[];
            event: MouseEvent | TouchEvent;
        }>;
    } & {
        [evt: string]: CustomEvent<any>;
    };
    slots: {};
};
export type NodeSelectionProps = typeof __propDef.props;
export type NodeSelectionEvents = typeof __propDef.events;
export type NodeSelectionSlots = typeof __propDef.slots;
export default class NodeSelection extends SvelteComponentTyped<NodeSelectionProps, NodeSelectionEvents, NodeSelectionSlots> {
}
export {};
