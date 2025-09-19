import { ToastProvider } from '@heroui/react';

interface ToastWrapperProps {
  children: React.ReactNode;
}

export default function ToastWrapper({ children }: ToastWrapperProps) {
  return (
    <>
      <ToastProvider placement="bottom-left" />
      {children}
    </>
  );
}