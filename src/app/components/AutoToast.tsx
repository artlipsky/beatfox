import { useEffect } from 'react';
import { addToast } from '@heroui/toast';

export default function AutoToast() {
  useEffect(() => {
    const timer = setTimeout(() => {
      addToast({
        title: 'Experimental Mode',
        description: 'All features are in beta and may change',
        color: 'primary',
        variant: 'solid',
        timeout: 10000,
      });
    }, 1000);

    return () => clearTimeout(timer);
  }, []);

  return null;
}
