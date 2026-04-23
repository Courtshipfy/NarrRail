---
name: Rainbow Flow
colors:
  surface: '#f8f9fa'
  surface-dim: '#d9dadb'
  surface-bright: '#f8f9fa'
  surface-container-lowest: '#ffffff'
  surface-container-low: '#f3f4f5'
  surface-container: '#edeeef'
  surface-container-high: '#e7e8e9'
  surface-container-highest: '#e1e3e4'
  on-surface: '#191c1d'
  on-surface-variant: '#514347'
  inverse-surface: '#2e3132'
  inverse-on-surface: '#f0f1f2'
  outline: '#837377'
  outline-variant: '#d5c2c6'
  surface-tint: '#864d61'
  primary: '#864d61'
  on-primary: '#ffffff'
  primary-container: '#ffb7ce'
  on-primary-container: '#7b4458'
  inverse-primary: '#fab3ca'
  secondary: '#3c627c'
  on-secondary: '#ffffff'
  secondary-container: '#b8dffd'
  on-secondary-container: '#3d637d'
  tertiary: '#60612c'
  on-tertiary: '#ffffff'
  tertiary-container: '#cecf8e'
  on-tertiary-container: '#575824'
  error: '#ba1a1a'
  on-error: '#ffffff'
  error-container: '#ffdad6'
  on-error-container: '#93000a'
  primary-fixed: '#ffd9e3'
  primary-fixed-dim: '#fab3ca'
  on-primary-fixed: '#360b1e'
  on-primary-fixed-variant: '#6a364a'
  secondary-fixed: '#c8e6ff'
  secondary-fixed-dim: '#a5cbe9'
  on-secondary-fixed: '#001e2e'
  on-secondary-fixed-variant: '#234b63'
  tertiary-fixed: '#e6e6a3'
  tertiary-fixed-dim: '#caca8a'
  on-tertiary-fixed: '#1c1d00'
  on-tertiary-fixed-variant: '#484916'
  background: '#f8f9fa'
  on-background: '#191c1d'
  surface-variant: '#e1e3e4'
typography:
  headline-xl:
    fontFamily: Plus Jakarta Sans
    fontSize: 48px
    fontWeight: '800'
    lineHeight: '1.2'
    letterSpacing: -0.02em
  headline-lg:
    fontFamily: Plus Jakarta Sans
    fontSize: 32px
    fontWeight: '700'
    lineHeight: '1.3'
  headline-md:
    fontFamily: Plus Jakarta Sans
    fontSize: 24px
    fontWeight: '700'
    lineHeight: '1.4'
  body-lg:
    fontFamily: Be Vietnam Pro
    fontSize: 18px
    fontWeight: '400'
    lineHeight: '1.6'
  body-md:
    fontFamily: Be Vietnam Pro
    fontSize: 16px
    fontWeight: '400'
    lineHeight: '1.6'
  label-md:
    fontFamily: Plus Jakarta Sans
    fontSize: 14px
    fontWeight: '600'
    lineHeight: '1.2'
    letterSpacing: 0.01em
rounded:
  sm: 0.5rem
  DEFAULT: 1rem
  md: 1.5rem
  lg: 2rem
  xl: 3rem
  full: 9999px
spacing:
  unit: 8px
  container-padding: 32px
  gutter: 24px
  element-gap: 16px
  section-margin: 64px
---

## Brand & Style

This design system is built on the principles of joy, fluidity, and radical approachability. It moves away from the rigid structures of traditional enterprise software, embracing a **Jelly-Glassmorphism** style that feels tactile and responsive. The emotional goal is to reduce user anxiety through "squishy" visual metaphors and a vibrant, candy-coated aesthetic.

The design language leans heavily into a mix of soft glassmorphism and tactile skeuomorphism. Elements should feel like polished gemstones or soft gelatin—semi-translucent, refractive, and physically satisfying to interact with. It is designed for creative tools, social platforms, and lightweight educational apps where the user experience should feel like play rather than work.

## Colors

The palette utilizes a "Pastel Rainbow" spectrum. Instead of solid flat colors, this design system prioritizes soft, multi-stop gradients to create a sense of depth and movement. 

- **Primary & Secondary:** Soft pinks and sky blues form the core interactive base.
- **Backgrounds:** Use extremely desaturated versions of the primary colors or pure off-white (#F8F9FA) to ensure the vibrant components pop.
- **Functional Colors:** Success, warning, and error states should still follow the pastel theme (e.g., a soft mint green for success rather than a harsh forest green).
- **Gradients:** Use the defined "Jelly Gradients" for primary actions and decorative containers to maintain the fluid narrative.

## Typography

The typography selection prioritizes softness and legibility. **Plus Jakarta Sans** is used for headings to provide a geometric yet friendly rhythm, while **Be Vietnam Pro** handles body text with its warm, contemporary feel.

Avoid all-caps styling as it creates a sense of "shouting" which contradicts the lightweight vibe. Headlines should use tight letter-spacing to feel cohesive, while body text should remain airy. Color-wise, avoid pure black (#000000); instead, use a deep charcoal or a tinted dark purple to keep the contrast soft.

## Layout & Spacing

The layout philosophy follows a **Fluid & Breathing** model. Content should never feel cramped; large margins and generous gutters are essential to maintain the "lightweight" feel.

- **Grid:** Use a 12-column fluid grid for desktop with massive 32px side margins.
- **Rhythm:** An 8px base unit drives all spacing. For this design system, "more is more"—when in doubt, increase the white space to let the jelly-like elements float comfortably.
- **Alignment:** Use centered layouts for marketing or landing pages to enhance the approachable, balanced vibe.

## Elevation & Depth

Depth is not achieved through traditional black shadows, but through **Color-Tinted Ambient Glows** and **Backdrop Blurs**.

1.  **Jelly-Glass Layering:** Background surfaces use `backdrop-filter: blur(20px)` combined with a highly transparent white or pastel fill (10-30% opacity).
2.  **Shadows:** Shadows are soft, wide, and tinted with the color of the element itself (e.g., a pink button casts a soft pink shadow). Use a `blur-radius` of 30px-50px for high-elevation items.
3.  **Inner Glows:** To achieve the "jelly" look, add a subtle 1px white inner-border (top and left) to simulate a light source hitting the edge of a translucent object.

## Shapes

The shape language is **Extra Round**. There are no sharp corners in this design system. 

All interactive elements (buttons, inputs, chips) should use a full-pill radius. Larger containers like cards use a minimum of 24px (1.5rem) to 48px (3rem) corner radius. This "squircle" and pill-heavy approach reinforces the bouncy, non-threatening nature of the UI.

## Components

- **Buttons:** Large, pill-shaped, and filled with a "sunrise" or "ocean" gradient. Upon hover, they should scale up slightly (1.05x) to provide a "bouncy" feedback loop.
- **Cards:** Use the Jelly-Glassmorphism style with a 32px corner radius. Apply a soft, multi-colored ambient shadow that matches the card's accent color.
- **Inputs:** Soft-grey or light pastel backgrounds with a 1px white stroke. When focused, the stroke transforms into a vibrant gradient "halo."
- **Chips/Badges:** Small, fully rounded pods with high-saturation pastel fills and dark-tinted text for readability.
- **Checkboxes & Radios:** Over-sized and bouncy. The "checked" state should trigger a small spring animation.
- **Lists:** Items should be separated by whitespace rather than lines, appearing as floating "pill" containers.
- **Floating Action Button (FAB):** A perfect circle using the most vibrant rainbow gradient, acting as the primary creative trigger.