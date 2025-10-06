import { createContext, useContext, useEffect, useMemo, useState } from 'react';

type Theme = 'light' | 'dark' | 'system';

type ThemeContextValue = {
  theme: Theme;
  resolvedTheme: 'light' | 'dark';
  setTheme: (theme: Theme) => void;
};

const STORAGE_KEY = 'beatfox-ui-theme';

const ThemeContext = createContext<ThemeContextValue | undefined>(undefined);

const prefersDark = () => window.matchMedia('(prefers-color-scheme: dark)').matches;

const getInitialTheme = (): Theme => {
  if (typeof window === 'undefined') return 'system';
  const stored = window.localStorage.getItem(STORAGE_KEY) as Theme | null;
  if (stored === 'light' || stored === 'dark' || stored === 'system') {
    return stored;
  }
  return 'system';
};

const getInitialResolvedTheme = (): 'light' | 'dark' => {
  if (typeof window === 'undefined') return 'light';
  return prefersDark() ? 'dark' : 'light';
};

export const ThemeProvider = ({ children }: { children: React.ReactNode }) => {
  const [theme, setThemeState] = useState<Theme>(() => getInitialTheme());
  const [resolvedTheme, setResolvedTheme] = useState<'light' | 'dark'>(() => getInitialResolvedTheme());

  useEffect(() => {
    if (typeof window === 'undefined') return;

    const media = window.matchMedia('(prefers-color-scheme: dark)');

    const applyTheme = (value: Theme) => {
      const shouldUseDark = value === 'dark' || (value === 'system' && media.matches);
      document.documentElement.classList.toggle('dark', shouldUseDark);
      document.documentElement.style.colorScheme = shouldUseDark ? 'dark' : 'light';
      setResolvedTheme(shouldUseDark ? 'dark' : 'light');
    };

    applyTheme(theme);

    const handleChange = () => {
      if (theme === 'system') {
        applyTheme('system');
      }
    };

    media.addEventListener('change', handleChange);
    return () => media.removeEventListener('change', handleChange);
  }, [theme]);

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
      setTheme
    }),
    [theme, resolvedTheme]
  );

  return <ThemeContext.Provider value={value}>{children}</ThemeContext.Provider>;
};

export const useTheme = () => {
  const context = useContext(ThemeContext);
  if (!context) {
    throw new Error('useTheme must be used within a ThemeProvider');
  }
  return context;
};
