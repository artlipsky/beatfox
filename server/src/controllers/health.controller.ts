import { Request, Response } from 'express';

export const getHealth = (_req: Request, res: Response): void => {
  res.json({
    status: 'ok',
    message: 'Server is running',
  });
};

export const getData = (_req: Request, res: Response): void => {
  res.json({
    message: 'Hello from BeatFox API',
    timestamp: new Date().toISOString(),
  });
};
