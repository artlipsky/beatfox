import { BEATFOX_CONFIG } from '../../../config';

export const API_ENDPOINTS = {
  HEALTH: '/api/health',
  DATA: '/api/data',
} as const;

export const getApiBaseUrl = (): string => {
  return BEATFOX_CONFIG.serverUrl;
};
