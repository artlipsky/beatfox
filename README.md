# BeatFox Full-Stack Project

A modern full-stack TypeScript application with React frontend and Node.js backend.

## Project Structure

```
beatfox/
├── client/                    # Frontend application
│   ├── src/
│   │   ├── api/              # API client layer
│   │   │   ├── client.ts     # HTTP client (class-based)
│   │   │   ├── services.ts   # Service layer
│   │   │   └── types.ts      # API types & errors
│   │   ├── config/           # Configuration
│   │   │   └── constants.ts  # App constants
│   │   ├── App.tsx
│   │   ├── main.tsx
│   │   └── index.css
│   ├── .env.example
│   ├── package.json
│   └── vite.config.ts
├── server/                    # Backend API
│   ├── src/
│   │   ├── config/           # Configuration management
│   │   │   └── index.ts      # Environment & config
│   │   ├── controllers/      # Request handlers
│   │   │   └── health.controller.ts
│   │   ├── middleware/       # Express middleware
│   │   │   ├── cors.middleware.ts
│   │   │   └── error.middleware.ts
│   │   ├── routes/           # Route definitions
│   │   │   └── api.routes.ts
│   │   ├── app.ts            # App factory
│   │   └── index.ts          # Entry point
│   ├── .env.example
│   ├── package.json
│   └── tsconfig.json
├── Makefile                   # Development commands
└── package.json               # Root workspace configuration
```

## Technology Stack

### Client
- **Framework**: Vite + React 19
- **Language**: TypeScript
- **Styling**: Tailwind CSS v4
- **Architecture**: Service layer pattern with typed API client
- **Dev Server**: Vite (Hot Module Replacement)

### Server
- **Runtime**: Node.js
- **Language**: TypeScript
- **Framework**: Express
- **Architecture**: MVC pattern (Routes → Controllers → Services)
- **Features**: Configuration management, error handling, graceful shutdown
- **Dev Mode**: tsx watch (auto-restart on changes)

## Architecture & Best Practices

This project follows Clean Code principles and SOLID design patterns:

### Server Architecture
- **Configuration Layer**: Centralized environment variable management with validation
- **Middleware Layer**: CORS, error handling, request parsing
- **Route Layer**: API endpoint definitions
- **Controller Layer**: Request handling and response formatting
- **Error Handling**: Custom error classes with proper HTTP status codes
- **Graceful Shutdown**: SIGTERM/SIGINT handling for clean server shutdown

### Client Architecture
- **API Client**: Class-based HTTP client with proper error handling
- **Service Layer**: Domain-specific API services
- **Type Safety**: Full TypeScript coverage with API response types
- **Configuration**: Auto-detection of API URL based on hostname
- **Error Handling**: Custom ApiError class for consistent error management

## Getting Started

### Quick Start with Makefile

The project includes a Makefile for convenient development:

```bash
# View all available commands
make help

# Install dependencies
make install

# Run both client and server
make dev

# Run client or server separately
make dev-client
make dev-server
```

### Using beatfox.local Domain

For local development with a custom domain:

```bash
# Add beatfox.local to /etc/hosts (requires sudo)
make setup-hosts

# Run development servers
make dev-local

# Access the application at:
# - Client: http://beatfox.local:5173
# - Server: http://beatfox.local:3001

# Remove beatfox.local from /etc/hosts when done
make remove-hosts
```

The client automatically detects whether you're using `localhost` or `beatfox.local` and configures the API URL accordingly.

### Install Dependencies

```bash
# Install all dependencies (client + server)
npm run install:all
# Or use make
make install

# Or install individually
cd client && npm install
cd server && npm install
```

### Development

Run both client and server concurrently:

```bash
npm run dev
```

Or run them separately:

```bash
# Terminal 1 - Client (http://localhost:5173)
npm run dev:client

# Terminal 2 - Server (http://localhost:3001)
npm run dev:server
```

### Production Build

```bash
# Build both client and server
npm run build

# Start production server
npm run start:server
```

## API Endpoints

- `GET /api/health` - Health check endpoint
- `GET /api/data` - Sample data endpoint

## Using the API Client

The client provides a typed service layer for API calls:

```typescript
import { healthService, dataService } from './api/services';
import { ApiError } from './api/types';

// Example: Health check
try {
  const health = await healthService.check();
  console.log(health.status); // 'ok'
} catch (error) {
  if (error instanceof ApiError) {
    console.error(`API Error ${error.status}: ${error.message}`);
  }
}

// Example: Fetch data
const data = await dataService.fetch();
console.log(data.message); // 'Hello from BeatFox API'
```

### Custom API Calls

For custom endpoints, use the API client directly:

```typescript
import { apiClient } from './api/client';

// GET request
const result = await apiClient.get<MyType>('/api/custom');

// POST request
const created = await apiClient.post<MyType>('/api/create', { data });

// Other methods: PUT, PATCH, DELETE
```

## Makefile Commands

Run `make help` to see all available commands. Common commands:

### Setup & Installation
```bash
make install          # Install all dependencies
make env-setup        # Create .env files from examples
make validate         # Validate project setup
```

### Development
```bash
make dev              # Run both client and server
make dev-client       # Run client only
make dev-server       # Run server only
make dev-local        # Run with beatfox.local domain
```

### Build & Production
```bash
make build            # Build both client and server
make build-client     # Build client only
make build-server     # Build server only
make start-server     # Start production server
```

### Local Domain
```bash
make setup-hosts      # Add beatfox.local to /etc/hosts
make remove-hosts     # Remove beatfox.local from /etc/hosts
```

### Cleanup
```bash
make clean            # Remove node_modules and build artifacts
```

## Available npm Scripts

### Root Level
- `npm run dev` - Run both client and server
- `npm run dev:client` - Run client only
- `npm run dev:server` - Run server only
- `npm run build` - Build both projects
- `npm run build:client` - Build client only
- `npm run build:server` - Build server only
- `npm run start:server` - Start production server

## Environment Variables

### Client
The client automatically detects the API URL based on the current hostname. To override:

Create `client/.env` file:
```
VITE_API_URL=http://localhost:3001
# Or for beatfox.local
VITE_API_URL=http://beatfox.local:3001
```

See `client/.env.example` for reference.

### Server
Create `server/.env` file:
```
PORT=3001
```

## Development Workflow

1. Start both servers: `npm run dev`
2. Client runs on http://localhost:5173
3. Server runs on http://localhost:3001
4. Client can make API calls to http://localhost:3001/api/*

## Deployment

### GitHub Pages (Client)

The client is automatically deployed to GitHub Pages on every push to `master`:

- **URL**: https://beatfox.ru
- **Workflow**: `.github/workflows/deploy.yml`
- **Build output**: `client/dist/`

The workflow builds the Vite client and deploys it to GitHub Pages with the custom domain configured in `client/public/CNAME`.

## Next Steps

- [ ] Add environment variables configuration
- [ ] Set up database connection
- [ ] Add authentication
- [ ] Add more API routes
- [ ] Add tests (Jest/Vitest)
- [ ] Add Docker configuration
- [ ] Set up CI/CD pipeline
