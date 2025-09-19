import React from 'react';
import { HeroUIProvider } from '@heroui/react';

interface Props {
  children: React.ReactNode;
}

export default function Provider({ children }: Props) {
  return <HeroUIProvider>{children}</HeroUIProvider>;
}