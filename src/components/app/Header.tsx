import {
  Navbar,
  NavbarBrand,
  NavbarContent,
  NavbarItem,
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

export default function Header() {
  const isLoaded = useLoaded(1000);

  return (
    <Navbar isBordered>
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

      <NavbarContent justify="end">
        <NavbarItem className="hidden lg:flex">
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
              <DropdownItem
                key="profile"
                variant="flat"
                startContent={<UserIcon className="size-4" />}
                shortcut="⌘P"
              >
                Profile
              </DropdownItem>
              <DropdownItem
                key="messages"
                variant="flat"
                startContent={<ChatBubbleLeftRightIcon className="size-4" />}
                shortcut="⌘M"
              >
                Messages
              </DropdownItem>
              <DropdownItem
                key="tasks"
                variant="flat"
                startContent={<ClipboardDocumentListIcon className="size-4" />}
                shortcut="⌘T"
              >
                Tasks
              </DropdownItem>
              <DropdownItem
                key="properties"
                startContent={<HomeModernIcon className="size-4" />}
                shortcut="⌘H"
                variant="flat"
              >
                Properties
              </DropdownItem>
              <DropdownItem
                key="payments"
                startContent={<CreditCardIcon className="size-4" />}
                shortcut="⌘Y"
                variant="flat"
              >
                Payments
              </DropdownItem>
              <DropdownItem
                key="settings"
                startContent={<Cog6ToothIcon className="size-4" />}
                shortcut="⌘,"
                showDivider
                variant="flat"
              >
                Settings
              </DropdownItem>

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
            </DropdownMenu>
          </Dropdown>
        </NavbarItem>
      </NavbarContent>
    </Navbar>
  );
}
