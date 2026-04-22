import { readable } from 'svelte/store';
function getMediaQuery() {
    if (typeof window === 'undefined' || !window.matchMedia) {
        return null;
    }
    return window.matchMedia('(prefers-color-scheme: dark)');
}
/**
 * Hook for receiving the current color mode class 'dark' or 'light'.
 *
 * @internal
 * @param colorMode - The color mode to use ('dark', 'light' or 'system')
 */
export function useColorModeClass(colorMode = 'light') {
    const colorModeClass = readable('light', (set) => {
        if (colorMode !== 'system') {
            set(colorMode);
            return;
        }
        const mediaQuery = getMediaQuery();
        const updateColorModeClass = () => set(mediaQuery?.matches ? 'dark' : 'light');
        set(mediaQuery?.matches ? 'dark' : 'light');
        mediaQuery?.addEventListener('change', updateColorModeClass);
        return () => {
            mediaQuery?.removeEventListener('change', updateColorModeClass);
        };
    });
    return colorModeClass;
}
