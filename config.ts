/**
 * Global BeatFox Configuration
 * Single source of truth for domain and port configuration
 */

export const BEATFOX_CONFIG = {
  domain: 'beatfox.local' as const,
  ports: {
    client: 5173 as const,
    server: 3001 as const,
  },
  get clientUrl(): string {
    return `http://${this.domain}:${this.ports.client}`;
  },
  get serverUrl(): string {
    return `http://${this.domain}:${this.ports.server}`;
  },
} as const;
