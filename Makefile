.PHONY: dev build preview install clean help

# Default target
help:
	@echo "Available commands:"
	@echo "  make install  - Install dependencies"
	@echo "  make dev      - Start development server"
	@echo "  make build    - Build for production"
	@echo "  make preview  - Preview production build"
	@echo "  make clean    - Clean node_modules and build files"
	@echo "  make help     - Show this help message"

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

# Quick setup (install + dev)
setup: install dev