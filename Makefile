.PHONY: run build preview install clean help lint format typecheck setup

# Default target
help:
	@echo "Available commands:"
	@echo "  make install    - Install dependencies"
	@echo "  make run        - Start development server"
	@echo "  make build      - Build for production"
	@echo "  make preview    - Preview production build"
	@echo "  make lint       - Run ESLint"
	@echo "  make lint-fix   - Run ESLint with auto-fix"
	@echo "  make format     - Format code with Prettier"
	@echo "  make typecheck  - Run TypeScript type checking"
	@echo "  make clean      - Clean node_modules and build files"
	@echo "  make setup      - Install dependencies and start dev server"
	@echo "  make help       - Show this help message"

# Install dependencies
install:
	npm install

# Start development server
run:
	npm run dev

# Build for production
build:
	npm run build

# Preview production build
preview:
	npm run preview

# Clean build files and dependencies
clean:
	rm -rf node_modules
	rm -rf dist
	rm -rf .astro

# Run ESLint
lint:
	npm run lint

# Run ESLint with auto-fix
lint-fix:
	npm run lint:fix

# Format code with Prettier
format:
	npm run format

# Run TypeScript type checking
typecheck:
	npm run typecheck

# Quick setup (install + dev)
setup: install run