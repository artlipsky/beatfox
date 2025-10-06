import {
  useCallback,
  useEffect,
  useMemo,
  useState,
  type ReactNode
} from 'react';
import {
  ThemeContext,
  type ResolvedTheme,
  type Theme
} from './ThemeContext';

const STORAGE_KEY = 'beatfox-ui-theme';

const prefersDark = () => window.matchMedia('(prefers-color-scheme: dark)').matches;

const getSystemPreference = (): ResolvedTheme => {
  if (typeof window === 'undefined') return 'light';
  return prefersDark() ? 'dark' : 'light';
};

const getInitialTheme = (): Theme => {
  if (typeof window === 'undefined') return 'system';
  const stored = window.localStorage.getItem(STORAGE_KEY) as Theme | null;
  if (stored === 'light' || stored === 'dark' || stored === 'system') {
    return stored;
  }
  return 'system';
};

export function ThemeProvider({ children }: { children: ReactNode }) {
  const [theme, setThemeState] = useState<Theme>(() => getInitialTheme());
  const [systemTheme, setSystemTheme] = useState<ResolvedTheme>(() => getSystemPreference());

  const resolvedTheme: ResolvedTheme = theme === 'system' ? systemTheme : theme;

  const applyTheme = useCallback((mode: Theme, preference: ResolvedTheme) => {
    if (typeof document === 'undefined') return;
    const shouldUseDark = mode === 'dark' || (mode === 'system' && preference === 'dark');
    document.documentElement.classList.toggle('dark', shouldUseDark);
    document.documentElement.style.colorScheme = shouldUseDark ? 'dark' : 'light';
  }, []);

  useEffect(() => {
    if (typeof window === 'undefined') return;

    const media = window.matchMedia('(prefers-color-scheme: dark)');
    const currentPreference: ResolvedTheme = media.matches ? 'dark' : 'light';
    setSystemTheme(currentPreference);
    applyTheme(theme, currentPreference);

    const handleChange = (event: MediaQueryListEvent) => {
      const nextPreference: ResolvedTheme = event.matches ? 'dark' : 'light';
      setSystemTheme(nextPreference);
      if (theme === 'system') {
        applyTheme('system', nextPreference);
      }
    };

    media.addEventListener('change', handleChange);
    return () => media.removeEventListener('change', handleChange);
  }, [theme, applyTheme]);

  useEffect(() => {
    applyTheme(theme, systemTheme);
  }, [theme, systemTheme, applyTheme]);

  const setTheme = (value: Theme) => {
    setThemeState(value);
    if (typeof window !== 'undefined') {
      window.localStorage.setItem(STORAGE_KEY, value);
    }
  };

  const value = useMemo(
    () => ({
      theme,
      resolvedTheme,
      systemTheme,
      setTheme
    }),
    [theme, resolvedTheme, systemTheme]
  );

  return <ThemeContext.Provider value={value}>{children}</ThemeContext.Provider>;
}
