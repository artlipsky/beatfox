import {
  Link,
  Image,
  Skeleton,
} from '@heroui/react';

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
          src="../src/assets/ergeon-logo.svg"
          alt="Ergeon Logo"
          className="w-40"
        />
      </Link>
    </Skeleton>
  );
}