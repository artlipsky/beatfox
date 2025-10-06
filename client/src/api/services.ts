import { apiClient } from './client';
import { API_ENDPOINTS } from '../config/constants';
import type { HealthCheckResponse, DataResponse } from './types';

export const healthService = {
  check: (): Promise<HealthCheckResponse> => {
    return apiClient.get<HealthCheckResponse>(API_ENDPOINTS.HEALTH);
  },
};

export const dataService = {
  fetch: (): Promise<DataResponse> => {
    return apiClient.get<DataResponse>(API_ENDPOINTS.DATA);
  },
};
