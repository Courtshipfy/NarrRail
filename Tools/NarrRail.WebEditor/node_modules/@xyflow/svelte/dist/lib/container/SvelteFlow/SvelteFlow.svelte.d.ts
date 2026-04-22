import { SvelteComponentTyped } from "svelte";
import type { SvelteFlowProps } from './types';
declare const __propDef: {
    props: SvelteFlowProps;
    events: {
        dragover: DragEvent;
        drop: DragEvent;
        paneclick: CustomEvent<{
            event: MouseEvent | TouchEvent;
        }>;
        panecontextmenu: CustomEvent<{
            event: MouseEvent;
        }>;
        edgeclick: CustomEvent<{
            edge: import("../..").Edge;
            event: MouseEvent | TouchEvent;
        }>;
        edgecontextmenu: CustomEvent<{
            edge: import("../..").Edge;
            event: MouseEvent;
        }>;
        edgemouseenter: CustomEvent<{
            edge: import("../..").Edge;
            event: MouseEvent;
        }>;
        edgemouseleave: CustomEvent<{
            edge: import("../..").Edge;
            event: MouseEvent;
        }>;
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
        nodedragstart: CustomEvent<{
            targetNode: import("../..").Node<Record<string, unknown>, string> | null;
            nodes: import("../..").Node<Record<string, unknown>, string>[];
            event: MouseEvent | TouchEvent;
        }>;
        nodedrag: CustomEvent<{
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
        selectionclick: CustomEvent<{
            nodes: import("../..").Node[];
            event: MouseEvent | TouchEvent;
        }>;
        selectioncontextmenu: CustomEvent<{
            nodes: import("../..").Node[];
            event: MouseEvent | TouchEvent;
        }>;
    } & {
        [evt: string]: CustomEvent<any>;
    };
    slots: {
        connectionLine: {
            slot: string;
        };
        default: {};
    };
};
type SvelteFlowProps_ = typeof __propDef.props;
export { SvelteFlowProps_ as SvelteFlowProps };
export type SvelteFlowEvents = typeof __propDef.events;
export type SvelteFlowSlots = typeof __propDef.slots;
export default class SvelteFlow extends SvelteComponentTyped<SvelteFlowProps_, SvelteFlowEvents, SvelteFlowSlots> {
}
