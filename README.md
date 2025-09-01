# BeatFox 🦊

A modern web application built with Astro, React, HeroUI, and Tailwind CSS.

## ✨ Tech Stack

- **[Astro](https://astro.build)** - Modern static site generator with island architecture
- **[React](https://reactjs.org)** - UI library for interactive components  
- **[HeroUI](https://heroui.com)** - Beautiful React components built on Tailwind CSS
- **[Tailwind CSS](https://tailwindcss.com)** - Utility-first CSS framework
- **[TypeScript](https://typescriptlang.org)** - Type-safe JavaScript
- **[ESLint](https://eslint.org)** + **[Prettier](https://prettier.io)** - Code linting and formatting

## 🚀 Project Structure

```text
/
├── public/
│   └── favicon.svg
├── src/
│   ├── components/          # Reusable React/Astro components
│   ├── layouts/            # Page layouts
│   │   └── Layout.astro    # Base HTML layout
│   └── pages/              # File-based routing
│       └── index.astro     # Homepage
├── .vscode/
│   └── settings.json       # Auto-format on save configuration
├── tailwind.config.js      # Tailwind & HeroUI configuration
├── eslint.config.js        # ESLint configuration
├── .prettierrc            # Prettier formatting rules
└── astro.config.mjs       # Astro configuration
```

## 🧞 Commands

All commands are run from the root of the project:

| Command                 | Action                                                |
| :---------------------- | :---------------------------------------------------- |
| `npm install`           | Install dependencies                                  |
| `npm run dev`           | Start development server at `localhost:4321`         |
| `npm run build`         | Build production site to `./dist/`                   |
| `npm run preview`       | Preview production build locally                      |
| `npm run lint`          | Lint code with ESLint                                |
| `npm run lint:fix`      | Fix auto-fixable ESLint issues                       |
| `npm run format`        | Format code with Prettier                            |
| `npm run format:check`  | Check code formatting                                |
| `npm run typecheck`     | Run TypeScript type checking                         |

## 🛠️ Development Setup

1. **Clone & Install**
   ```bash
   git clone <repository-url>
   cd beatfox
   npm install
   ```

2. **Start Development**
   ```bash
   npm run dev
   ```

3. **VS Code Setup**
   - Install the Prettier extension (`esbenp.prettier-vscode`)
   - Auto-format on save is already configured in `.vscode/settings.json`

## 🧩 Using HeroUI Components

HeroUI components need the `client:visible` directive for client-side hydration:

```astro
---
import { Button } from '@heroui/react';
---

<Button color="primary" client:visible>
  Click me!
</Button>
```

## 📦 Building for Production

```bash
npm run build
npm run preview  # Test production build locally
```

## 🎨 Styling

- **Tailwind CSS** for utility classes
- **HeroUI** for pre-built components with dark mode support
- **Custom styles** can be added to `src/styles/` (create as needed)

## 📝 Code Quality

- **Auto-format on save** configured for VS Code
- **ESLint** for code linting
- **Prettier** for consistent formatting  
- **TypeScript** for type safety
