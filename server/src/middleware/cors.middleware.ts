import cors from 'cors';
import { config } from '../config/index.js';

export const corsMiddleware = cors({
  origin: config.cors.allowedOrigins,
  credentials: config.cors.credentials,
});
