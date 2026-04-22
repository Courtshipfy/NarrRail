import { PanOnScrollMode, XYPanZoom } from '@xyflow/system';
export default function zoom(domNode, params) {
    const { panZoom, minZoom, maxZoom, initialViewport, viewport, dragging, translateExtent, paneClickDistance } = params;
    const panZoomInstance = XYPanZoom({
        domNode,
        minZoom,
        maxZoom,
        translateExtent,
        viewport: initialViewport,
        paneClickDistance,
        onDraggingChange: dragging.set
    });
    const currentViewport = panZoomInstance.getViewport();
    viewport.set(currentViewport);
    panZoom.set(panZoomInstance);
    panZoomInstance.update(params);
    return {
        update(params) {
            panZoomInstance.update(params);
        }
    };
}
