import { Router } from 'express';
import { getHealth, getData } from '../controllers/health.controller.js';

const router = Router();

router.get('/health', getHealth);
router.get('/data', getData);

export default router;
