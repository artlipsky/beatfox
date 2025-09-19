import { Switch } from '@heroui/react';
import { useState, useEffect } from 'react';
import { SunIcon, MoonIcon } from '@heroicons/react/24/outline';

export default function DarkModeToggle() {
  const [isDark, setIsDark] = useState(false);

  useEffect(() => {
    // Check if dark mode is already enabled
    const htmlElement = document.documentElement;
    setIsDark(htmlElement.classList.contains('dark'));
  }, []);

  const toggleDarkMode = (isSelected: boolean) => {
    const htmlElement = document.documentElement;
    if (isSelected) {
      htmlElement.classList.add('dark');
    } else {
      htmlElement.classList.remove('dark');
    }
    setIsDark(isSelected);
  };

  return (
    <Switch
      color="primary"
      size="md"
      isSelected={isDark}
      onValueChange={toggleDarkMode}
      startContent={<SunIcon className="size-4" />}
      endContent={<MoonIcon className="size-4" />}
    />
  );
}
