import { BEATFOX_CONFIG } from '../../../config';

type NodeEnv = 'development' | 'production' | 'test';

interface EnvironmentVariables {
  PORT: string;
  NODE_ENV: NodeEnv;
  ALLOWED_ORIGINS: string;
}

const DEFAULT_PORT = String(BEATFOX_CONFIG.ports.server);
const DEFAULT_NODE_ENV: NodeEnv = 'development';
const DEFAULT_ALLOWED_ORIGINS = BEATFOX_CONFIG.clientUrl;

const parsePort = (port: string | undefined): number => {
  const parsed = parseInt(port || DEFAULT_PORT, 10);

  if (isNaN(parsed) || parsed < 0 || parsed > 65535) {
    throw new Error(`Invalid PORT: ${port}. Must be a number between 0 and 65535.`);
  }

  return parsed;
};

const parseNodeEnv = (env: string | undefined): NodeEnv => {
  const validEnvs: NodeEnv[] = ['development', 'production', 'test'];
  const nodeEnv = (env || DEFAULT_NODE_ENV) as NodeEnv;

  if (!validEnvs.includes(nodeEnv)) {
    console.warn(`Invalid NODE_ENV: ${env}. Defaulting to ${DEFAULT_NODE_ENV}`);
    return DEFAULT_NODE_ENV;
  }

  return nodeEnv;
};

const parseAllowedOrigins = (origins: string | undefined): string[] => {
  const originsString = origins || DEFAULT_ALLOWED_ORIGINS;
  return originsString.split(',').map(origin => origin.trim()).filter(Boolean);
};

const loadEnvironment = (): EnvironmentVariables => {
  return {
    PORT: process.env.PORT || DEFAULT_PORT,
    NODE_ENV: parseNodeEnv(process.env.NODE_ENV),
    ALLOWED_ORIGINS: process.env.ALLOWED_ORIGINS || DEFAULT_ALLOWED_ORIGINS,
  };
};

const env = loadEnvironment();

export const config = {
  domain: BEATFOX_CONFIG.domain,
  server: {
    port: parsePort(env.PORT),
    host: '0.0.0.0' as const,
  },
  cors: {
    allowedOrigins: parseAllowedOrigins(env.ALLOWED_ORIGINS),
    credentials: true as const,
  },
  nodeEnv: env.NODE_ENV,
  isDevelopment: env.NODE_ENV === 'development',
  isProduction: env.NODE_ENV === 'production',
  isTest: env.NODE_ENV === 'test',
} as const;
