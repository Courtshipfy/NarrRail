import { MATERIAL_SYMBOL_PATHS } from "./icon-paths.js";

const SYMBOL_SELECTOR = ".material-symbols-outlined";
const SVG_NS = "http://www.w3.org/2000/svg";

function getSymbolName(element) {
    const textName = element.textContent?.trim();
    if (textName && MATERIAL_SYMBOL_PATHS[textName]) return textName;
    return element.dataset.materialSymbolName || "";
}

function createSvg(paths) {
    const svg = document.createElementNS(SVG_NS, "svg");
    svg.setAttribute("class", "material-symbol-fallback-svg");
    svg.setAttribute("viewBox", "0 0 24 24");
    svg.setAttribute("aria-hidden", "true");
    svg.setAttribute("focusable", "false");
    svg.setAttribute("fill", "none");

    paths.forEach((pathData) => {
        const path = document.createElementNS(SVG_NS, "path");
        path.setAttribute("d", pathData);
        svg.appendChild(path);
    });

    return svg;
}

function renderSymbol(element) {
    const name = getSymbolName(element);
    const paths = MATERIAL_SYMBOL_PATHS[name];
    if (!paths) return;

    if (
        element.dataset.materialSymbolRendered === name &&
        element.firstElementChild?.classList.contains(
            "material-symbol-fallback-svg",
        )
    ) {
        return;
    }

    element.dataset.materialSymbolName = name;
    element.dataset.materialSymbolRendered = name;
    element.replaceChildren(createSvg(paths));
}

function renderSymbols(root) {
    if (root instanceof Element && root.matches(SYMBOL_SELECTOR)) {
        renderSymbol(root);
    }

    root.querySelectorAll?.(SYMBOL_SELECTOR).forEach(renderSymbol);
}

export function installMaterialSymbolFallback(root = document.body) {
    if (!root || typeof MutationObserver === "undefined") return undefined;

    renderSymbols(root);

    const observer = new MutationObserver((mutations) => {
        mutations.forEach((mutation) => {
            if (mutation.type === "characterData") {
                const parent = mutation.target.parentElement;
                if (parent?.matches(SYMBOL_SELECTOR)) renderSymbol(parent);
                return;
            }

            mutation.addedNodes.forEach((node) => {
                if (node instanceof Element) renderSymbols(node);
            });

            if (mutation.target instanceof Element) {
                renderSymbols(mutation.target);
            }
        });
    });

    observer.observe(root, {
        childList: true,
        characterData: true,
        subtree: true,
    });

    return () => observer.disconnect();
}
