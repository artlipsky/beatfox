import { useState } from 'react';
import { useTheme } from './ThemeProvider';

type ThemeOption = {
  label: string;
  value: 'light' | 'dark' | 'system';
  icon: string;
};

const OPTIONS: ThemeOption[] = [
  { label: 'Light', value: 'light', icon: '☀️' },
  { label: 'System', value: 'system', icon: '💻' },
  { label: 'Dark', value: 'dark', icon: '🌙' }
];

const ThemeToggle = () => {
  const { theme, systemTheme, setTheme } = useTheme();
  const [isFocused, setIsFocused] = useState(false);

  return (
    <div
      role="group"
      aria-label="Theme toggle"
      className={`flex items-center gap-1 rounded-full border border-slate-200 bg-white/80 p-1 text-sm font-medium shadow-sm transition dark:border-slate-700 dark:bg-slate-900/60 ${
        isFocused ? 'ring-2 ring-offset-2 ring-slate-300 dark:ring-slate-500 dark:ring-offset-slate-900' : ''
      }`}
    >
      {OPTIONS.map((option) => {
        const isActive = theme === option.value;
        const title =
          option.value === 'system'
            ? `Match system preference (currently ${systemTheme})`
            : `Switch to ${option.label.toLowerCase()} theme`;

        return (
          <button
            key={option.value}
            type="button"
            onClick={() => setTheme(option.value)}
            onFocus={() => setIsFocused(true)}
            onBlur={() => setIsFocused(false)}
            className={`flex items-center gap-1 rounded-full px-3 py-1.5 transition focus:outline-none ${
              isActive
                ? 'bg-slate-900 text-white shadow-sm dark:bg-slate-200 dark:text-slate-900'
                : 'text-slate-600 hover:text-slate-900 dark:text-slate-300 dark:hover:text-white'
            }`}
            aria-pressed={isActive}
            title={title}
          >
            <span aria-hidden>{option.icon}</span>
            <span className="hidden sm:inline">{option.label}</span>
            {option.value === 'system' && theme === 'system' && (
              <span className="hidden text-[10px] uppercase tracking-wide text-slate-400 dark:text-slate-500 lg:inline">
                {systemTheme}
              </span>
            )}
          </button>
        );
      })}
    </div>
  );
};

export default ThemeToggle;
