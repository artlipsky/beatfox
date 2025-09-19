import { Link, Image, Skeleton } from '@heroui/react';
import logo from '../../assets/ergeon-logo.svg';

interface LogoProps {
  isLoaded: boolean;
}

export default function Logo({ isLoaded }: LogoProps) {
  return (
    <Skeleton isLoaded={isLoaded} className="rounded-lg">
      <Link
        className="cursor-pointer"
        href="https://ergeon.com"
        target="_blank"
        rel="noopener"
      >
        <Image
          radius="none"
          src={logo.src}
          alt="Ergeon Logo"
          className="w-40"
        />
      </Link>
    </Skeleton>
  );
}
