import { useEffect } from 'react';
import { addToast } from '@heroui/react';

export default function AutoToast() {
  useEffect(() => {
    // Show toast when component mounts (page opens)
    addToast({
      title: "Welcome!",
      description: "Project details page loaded successfully",
      color: "success"
    });
  }, []);

  return null; // This component doesn't render anything
}