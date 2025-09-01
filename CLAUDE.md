# BeatFox Development Guidelines

This document outlines the coding standards and best practices for the BeatFox project.

## Technology Stack

- **Framework**: Astro with React integration
- **UI Library**: HeroUI v2.8.3 with Tailwind CSS v4
- **Styling**: Tailwind CSS v4 CSS-first approach using `@tailwindcss/vite`
- **Theme**: Dark mode support with `class="dark"` toggle

## Component Standards

### HeroUI Components
- **Always use `client:visible`** for HeroUI components to ensure proper hydration and animations
- Import components from `@heroui/react`: `import { Button, Image } from '@heroui/react'`
- Use HeroUI props like `color`, `variant`, `fullWidth`, `isBlurred`, `radius` for consistent styling

### Responsive Design Patterns
- Use progressive responsive classes for better UX across devices
- **Padding**: `px-4 sm:px-6 lg:px-8` for horizontal, `py-8 sm:py-12 lg:py-16` for vertical
- **Gaps**: `gap-4 sm:gap-6 lg:gap-8` for progressive spacing
- **Text sizes**: `text-base sm:text-lg lg:text-xl` or `text-2xl sm:text-3xl lg:text-4xl xl:text-5xl`
- **Width constraints**: Use `max-w-md sm:max-w-lg` or `max-w-2xl` to improve readability

### Layout Patterns
- **Flex centering**: Use `flex items-center justify-center` for vertical and horizontal centering
- **Content containers**: Wrap main content in `max-w-2xl` or similar for optimal reading width
- **Button groups**: Use `flex sm:flex-row flex-col justify-center gap-4 w-full sm:w-auto` for responsive button layouts
- **Full-height containers**: Use `min-h-screen` for full viewport height

### Color and Theme
- **Text colors**: Use semantic color classes
  - Primary text: `text-gray-900 dark:text-gray-100`
  - Secondary text: `text-gray-600 dark:text-gray-400`
- **Dark mode**: Ensure all text and components have dark mode variants

## Code Style

### Astro Components
```astro
---
import Layout from '../layouts/Layout.astro';
import { Button, Image } from '@heroui/react';
---

<Layout title="Page Title">
  <main class="flex justify-center items-center mx-auto px-4 sm:px-6 lg:px-8 py-8 sm:py-12 lg:py-16 min-h-screen container">
    <div class="flex flex-col items-center gap-4 sm:gap-6 max-w-2xl text-center">
      <!-- Content -->
    </div>
  </main>
</Layout>
```

### Button Implementation
```astro
<Button fullWidth color="warning" client:visible>
  Get Started
</Button>
<Button fullWidth color="default" variant="flat" client:visible>
  Learn More
</Button>
```

### Image Implementation
```astro
<Image
  width={200}
  isBlurred
  radius="none"
  src="/logo-col.svg"
  alt="Descriptive Alt Text"
  className="mx-auto w-32 sm:w-40 lg:w-48"
  client:visible
/>
```

## CSS Configuration

### Global Styles (src/styles/global.css)
```css
@import 'tailwindcss';
@plugin '../../hero.ts';
@source '../../node_modules/@heroui/theme/dist';
@custom-variant dark (&:is(.dark *));
```

### HeroUI Plugin (hero.ts)
```ts
import { heroui } from "@heroui/react";
export default heroui();
```

## Development Commands

- **Build**: `npm run build`
- **Dev**: `npm run dev`
- **Lint**: `npm run lint`
- **Type check**: `npm run typecheck`

## Assets

- **Static assets**: Place in `public/` directory (e.g., `/logo-col.svg`)
- **Processed assets**: Place in `src/assets/` for optimization

## Deployment

- **GitHub Pages**: Configured with `.github/workflows/deploy.yml`
- **Custom domain**: Site configured for `https://beatfox.ru`
- **No Jekyll**: `.nojekyll` file prevents Jekyll processing

## Important Notes

- HeroUI components REQUIRE `client:visible` for proper functionality and animations
- Use Tailwind CSS v4 CSS-first approach, not traditional config file
- All components should support dark mode
- Maintain responsive design patterns for consistent UX
- Test on multiple screen sizes during development