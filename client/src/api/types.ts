export interface ApiResponse<T> {
  data: T;
  status: number;
}

export interface HealthCheckResponse {
  status: string;
  message: string;
}

export interface DataResponse {
  message: string;
  timestamp: string;
}

export class ApiError extends Error {
  public readonly status: number;
  public readonly data?: unknown;

  constructor(
    status: number,
    message: string,
    data?: unknown
  ) {
    super(message);
    this.status = status;
    this.data = data;
    this.name = 'ApiError';
    Object.setPrototypeOf(this, ApiError.prototype);
  }
}
