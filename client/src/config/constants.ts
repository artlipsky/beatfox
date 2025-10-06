import { BEATFOX_CONFIG } from '../../../config';

export const API_ENDPOINTS = {
  HEALTH: '/api/health',
  DATA: '/api/data',
} as const;

export const getApiBaseUrl = (): string => {
  // Allow environment variable override
  if (import.meta.env.VITE_API_URL) {
    return import.meta.env.VITE_API_URL;
  }

  return BEATFOX_CONFIG.serverUrl;
};
