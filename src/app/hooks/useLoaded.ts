import { useState, useEffect } from 'react';

export function useLoaded(delay: number = 1000) {
  const [isLoaded, setIsLoaded] = useState(false);

  useEffect(() => {
    if (typeof window === 'undefined') return;
    
    const timer = setTimeout(() => setIsLoaded(true), delay);
    return () => clearTimeout(timer);
  }, [delay]);

  return isLoaded;
}