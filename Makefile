.DEFAULT_GOAL := help
.PHONY: help install env-setup dev dev-client dev-server build build-client build-server start-server setup-hosts remove-hosts clean validate

# Colors for output
CYAN := \033[0;36m
GREEN := \033[0;32m
YELLOW := \033[0;33m
RED := \033[0;31m
NC := \033[0m # No Color

# Default target
help:
	@echo "$(CYAN)BeatFox Makefile Commands:$(NC)"
	@echo ""
	@echo "$(GREEN)Setup:$(NC)"
	@echo "  make install          - Install all dependencies"
	@echo "  make env-setup        - Create .env files from examples"
	@echo "  make validate         - Validate project setup"
	@echo ""
	@echo "$(GREEN)Development:$(NC)"
	@echo "  make dev              - Run both client and server (beatfox.local)"
	@echo "  make dev-client       - Run only the client (beatfox.local)"
	@echo "  make dev-server       - Run only the server (beatfox.local)"
	@echo ""
	@echo "$(GREEN)Build:$(NC)"
	@echo "  make build            - Build both client and server"
	@echo "  make build-client     - Build only the client"
	@echo "  make build-server     - Build only the server"
	@echo ""
	@echo "$(GREEN)Production:$(NC)"
	@echo "  make start-server     - Start the production server"
	@echo ""
	@echo "$(GREEN)Local Domain Setup:$(NC)"
	@echo "  make setup-hosts      - Add beatfox.local to /etc/hosts (auto-run with dev)"
	@echo "  make remove-hosts     - Remove beatfox.local from /etc/hosts (requires sudo)"
	@echo ""
	@echo "$(GREEN)Cleanup:$(NC)"
	@echo "  make clean            - Remove node_modules and build artifacts"

# Setup
install:
	@echo "$(CYAN)Installing dependencies...$(NC)"
	@npm install
	@echo "$(GREEN)✓ Dependencies installed successfully$(NC)"

env-setup:
	@echo "$(CYAN)Setting up environment files...$(NC)"
	@if [ ! -f client/.env ]; then \
		cp client/.env.example client/.env 2>/dev/null || echo "# Client environment variables" > client/.env; \
		echo "$(GREEN)✓ Created client/.env$(NC)"; \
	else \
		echo "$(YELLOW)⚠ client/.env already exists, skipping$(NC)"; \
	fi
	@if [ ! -f server/.env ]; then \
		cp server/.env.example server/.env 2>/dev/null || true; \
		echo "$(GREEN)✓ Created server/.env$(NC)"; \
	else \
		echo "$(YELLOW)⚠ server/.env already exists, skipping$(NC)"; \
	fi
	@echo "$(GREEN)✓ Environment setup complete$(NC)"

validate:
	@echo "$(CYAN)Validating project setup...$(NC)"
	@echo -n "Checking Node.js... "
	@which node > /dev/null && echo "$(GREEN)✓$(NC)" || (echo "$(RED)✗ Node.js not found$(NC)" && exit 1)
	@echo -n "Checking npm... "
	@which npm > /dev/null && echo "$(GREEN)✓$(NC)" || (echo "$(RED)✗ npm not found$(NC)" && exit 1)
	@echo -n "Checking dependencies... "
	@test -d node_modules && echo "$(GREEN)✓$(NC)" || echo "$(YELLOW)⚠ Run 'make install'$(NC)"
	@echo -n "Checking client dependencies... "
	@test -d client/node_modules && echo "$(GREEN)✓$(NC)" || echo "$(YELLOW)⚠ Run 'make install'$(NC)"
	@echo -n "Checking server dependencies... "
	@test -d server/node_modules && echo "$(GREEN)✓$(NC)" || echo "$(YELLOW)⚠ Run 'make install'$(NC)"
	@echo "$(GREEN)✓ Validation complete$(NC)"

# Development
dev: setup-hosts
	@echo "$(CYAN)Starting development servers...$(NC)"
	@echo "$(GREEN)Client: http://beatfox.local:5173$(NC)"
	@echo "$(GREEN)Server: http://beatfox.local:3001$(NC)"
	@npm run dev

dev-client: setup-hosts
	@echo "$(CYAN)Starting client development server...$(NC)"
	@echo "$(GREEN)Available at: http://beatfox.local:5173$(NC)"
	@npm run dev:client

dev-server: setup-hosts
	@echo "$(CYAN)Starting server development server...$(NC)"
	@echo "$(GREEN)Available at: http://beatfox.local:3001$(NC)"
	@npm run dev:server

# Build
build:
	@echo "$(CYAN)Building client and server...$(NC)"
	@npm run build
	@echo "$(GREEN)✓ Build complete$(NC)"

build-client:
	@echo "$(CYAN)Building client...$(NC)"
	@npm run build:client
	@echo "$(GREEN)✓ Client build complete$(NC)"

build-server:
	@echo "$(CYAN)Building server...$(NC)"
	@npm run build:server
	@echo "$(GREEN)✓ Server build complete$(NC)"

# Production
start-server:
	@echo "$(CYAN)Starting production server...$(NC)"
	@npm run start:server

# Local domain setup
setup-hosts:
	@echo "$(CYAN)Configuring beatfox.local in /etc/hosts...$(NC)"
	@if grep -q "beatfox.local" /etc/hosts 2>/dev/null; then \
		echo "$(YELLOW)⚠ beatfox.local already exists in /etc/hosts$(NC)"; \
	else \
		echo "127.0.0.1 beatfox.local" | sudo tee -a /etc/hosts > /dev/null; \
		echo "$(GREEN)✓ beatfox.local added to /etc/hosts$(NC)"; \
	fi

remove-hosts:
	@echo "$(CYAN)Removing beatfox.local from /etc/hosts...$(NC)"
	@if grep -q "beatfox.local" /etc/hosts 2>/dev/null; then \
		sudo sed -i '' '/beatfox.local/d' /etc/hosts 2>/dev/null; \
		echo "$(GREEN)✓ beatfox.local removed from /etc/hosts$(NC)"; \
	else \
		echo "$(YELLOW)⚠ beatfox.local not found in /etc/hosts$(NC)"; \
	fi

# Cleanup
clean:
	@echo "$(CYAN)Cleaning build artifacts and dependencies...$(NC)"
	@rm -rf node_modules && echo "$(GREEN)✓ Removed root node_modules$(NC)" || true
	@rm -rf client/node_modules && echo "$(GREEN)✓ Removed client/node_modules$(NC)" || true
	@rm -rf server/node_modules && echo "$(GREEN)✓ Removed server/node_modules$(NC)" || true
	@rm -rf client/dist && echo "$(GREEN)✓ Removed client/dist$(NC)" || true
	@rm -rf server/dist && echo "$(GREEN)✓ Removed server/dist$(NC)" || true
	@echo "$(GREEN)✓ Cleanup complete$(NC)"
