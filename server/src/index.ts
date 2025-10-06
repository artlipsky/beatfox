import { createApp } from './app.js';
import { config } from './config/index.js';

const startServer = (): void => {
  const app = createApp();

  const server = app.listen(config.server.port, config.server.host, () => {
    console.log('🚀 Server started successfully');
    console.log(`   Environment: ${config.nodeEnv}`);
    console.log(`   Domain: http://${config.domain}:${config.server.port}`);
  });

  const shutdown = (signal: string) => {
    console.log(`\n${signal} received, shutting down gracefully...`);
    server.close(() => {
      console.log('Server closed');
      process.exit(0);
    });

    // Force shutdown after 10 seconds
    setTimeout(() => {
      console.error('Forced shutdown after timeout');
      process.exit(1);
    }, 10000);
  };

  process.on('SIGTERM', () => shutdown('SIGTERM'));
  process.on('SIGINT', () => shutdown('SIGINT'));
};

startServer();