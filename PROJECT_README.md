# BeatFox Full-Stack Project

A modern full-stack TypeScript application with React frontend and Node.js backend.

## Project Structure

```
beatfox/
├── client/          # Frontend application
│   ├── src/
│   │   ├── App.tsx
│   │   ├── main.tsx
│   │   └── index.css
│   ├── package.json
│   └── vite.config.ts
├── server/          # Backend API
│   ├── src/
│   │   └── index.ts
│   ├── package.json
│   └── tsconfig.json
└── package.json     # Root workspace configuration
```

## Technology Stack

### Client
- **Framework**: Vite + React 18
- **Language**: TypeScript
- **Styling**: Tailwind CSS
- **Dev Server**: Vite (Hot Module Replacement)

### Server
- **Runtime**: Node.js
- **Language**: TypeScript
- **Framework**: Express
- **Dev Mode**: tsx watch (auto-restart on changes)

## Getting Started

### Install Dependencies

```bash
# Install all dependencies (client + server)
npm run install:all

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

## Available Scripts

### Root Level
- `npm run dev` - Run both client and server
- `npm run dev:client` - Run client only
- `npm run dev:server` - Run server only
- `npm run build` - Build both projects
- `npm run build:client` - Build client only
- `npm run build:server` - Build server only
- `npm run start:server` - Start production server

### Client (cd client/)
- `npm run dev` - Start dev server (http://localhost:5173)
- `npm run build` - Build for production
- `npm run preview` - Preview production build

### Server (cd server/)
- `npm run dev` - Start dev server with watch mode
- `npm run build` - Compile TypeScript to JavaScript
- `npm run start` - Run compiled server

## Environment Variables

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

## Next Steps

- [ ] Add environment variables configuration
- [ ] Set up database connection
- [ ] Add authentication
- [ ] Add more API routes
- [ ] Add tests (Jest/Vitest)
- [ ] Add Docker configuration
- [ ] Set up CI/CD pipeline