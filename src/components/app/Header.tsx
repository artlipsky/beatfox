import {
  Navbar,
  NavbarBrand,
  NavbarContent,
  NavbarItem,
  NavbarMenuToggle,
  NavbarMenu,
  NavbarMenuItem,
  Link,
  Image,
  Skeleton,
  User,
  Dropdown,
  DropdownTrigger,
  DropdownMenu,
  DropdownItem,
} from '@heroui/react';
import {
  UserIcon,
  ChatBubbleLeftRightIcon,
  ClipboardDocumentListIcon,
  HomeModernIcon,
  CreditCardIcon,
  Cog6ToothIcon,
  ArrowRightCircleIcon,
} from '@heroicons/react/16/solid';
import { useLoaded } from '../../hooks/useLoaded';
import { useState } from 'react';

export default function Header() {
  const isLoaded = useLoaded(1000);
  const [isMenuOpen, setIsMenuOpen] = useState(false);

  const menuItems = [
    { name: 'Profile', icon: UserIcon, shortcut: '⌘P' },
    { name: 'Messages', icon: ChatBubbleLeftRightIcon, shortcut: '⌘M' },
    { name: 'Tasks', icon: ClipboardDocumentListIcon, shortcut: '⌘T' },
    { name: 'Properties', icon: HomeModernIcon, shortcut: '⌘H' },
    { name: 'Payments', icon: CreditCardIcon, shortcut: '⌘Y' },
    { name: 'Settings', icon: Cog6ToothIcon, shortcut: '⌘,' },
  ];

  return (
    <Navbar isBordered isMenuOpen={isMenuOpen} onMenuOpenChange={setIsMenuOpen}>
      <NavbarBrand>
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
      </NavbarBrand>

      <NavbarContent className="hidden sm:flex" justify="end">
        <NavbarItem>
          <Dropdown backdrop="blur" placement="bottom-end">
            <DropdownTrigger>
              <Skeleton isLoaded={isLoaded} className="rounded-lg">
                <User
                  className="cursor-pointer"
                  as="button"
                  isFocusable
                  name="Arthur Lipsky"
                  avatarProps={{
                    src: '../src/assets/artlipsky.png',
                  }}
                  description="UI/UX Designer"
                />
              </Skeleton>
            </DropdownTrigger>
            <DropdownMenu aria-label="User menu actions">
              <>
                {menuItems.map((item, index) => (
                  <DropdownItem
                    key={item.name.toLowerCase()}
                    variant="flat"
                    startContent={<item.icon className="size-4" />}
                    shortcut={item.shortcut}
                    showDivider={index === menuItems.length - 1}
                  >
                    {item.name}
                  </DropdownItem>
                ))}

                <DropdownItem
                  className="text-danger"
                  key="logout"
                  startContent={<ArrowRightCircleIcon className="size-4" />}
                  shortcut="⌘Q"
                  color="danger"
                  variant="flat"
                >
                  Log Out
                </DropdownItem>
              </>
            </DropdownMenu>
          </Dropdown>
        </NavbarItem>
      </NavbarContent>

      <NavbarContent className="sm:hidden" justify="end">
        <NavbarMenuToggle
          className="cursor-pointer"
          aria-label={isMenuOpen ? 'Close menu' : 'Open menu'}
        />
      </NavbarContent>

      <NavbarMenu className="gap-4">
        {menuItems.map((item, index) => (
          <NavbarMenuItem key={`${item.name}-${index}`}>
            <Link
              underline="hover"
              className="flex items-center gap-2 w-full"
              color="foreground"
              href="#"
              size="lg"
            >
              <item.icon className="size-5" />
              {item.name}
            </Link>
          </NavbarMenuItem>
        ))}
        <NavbarMenuItem>
          <Link
            underline="hover"
            className="flex items-center gap-2 w-full"
            color="danger"
            href="#"
            size="lg"
          >
            <ArrowRightCircleIcon className="size-5" />
            Log Out
          </Link>
        </NavbarMenuItem>
      </NavbarMenu>
    </Navbar>
  );
}
