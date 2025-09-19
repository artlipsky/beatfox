import {
  Navbar,
  NavbarBrand,
  NavbarContent,
  NavbarItem,
  NavbarMenuToggle,
  Skeleton,
} from '@heroui/react';
import { useLoaded } from '../../../hooks/useLoaded';
import { useState } from 'react';
import MobileMenu from './MobileMenu';
import UserDropdown from './UserDropdown';
import Logo from './Logo';

export default function Header() {
  const isLoaded = useLoaded(1000);
  const [isMenuOpen, setIsMenuOpen] = useState(false);

  return (
    <Navbar isBordered isMenuOpen={isMenuOpen} onMenuOpenChange={setIsMenuOpen}>
      <NavbarBrand>
        <Logo isLoaded={isLoaded} />
      </NavbarBrand>

      <NavbarContent className="hidden sm:flex" justify="end">
        <NavbarItem>
          <UserDropdown isLoaded={isLoaded} />
        </NavbarItem>
      </NavbarContent>

      <NavbarContent className="sm:hidden" justify="end">
        <NavbarMenuToggle
          className="text-primary hover:text-primary-600 transition-colors cursor-pointer"
          aria-label={isMenuOpen ? 'Close menu' : 'Open menu'}
        />
      </NavbarContent>

      <MobileMenu />
    </Navbar>
  );
}
