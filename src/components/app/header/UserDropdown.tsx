import {
  Dropdown,
  DropdownTrigger,
  DropdownMenu,
  DropdownItem,
  User,
  Skeleton,
} from '@heroui/react';
import { menuItems, logoutItem } from './menuItems';

interface UserDropdownProps {
  isLoaded: boolean;
}

export default function UserDropdown({ isLoaded }: UserDropdownProps) {
  return (
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
              key={item.key}
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
            key={logoutItem.key}
            startContent={<logoutItem.icon className="size-4" />}
            shortcut={logoutItem.shortcut}
            color="danger"
            variant="flat"
          >
            {logoutItem.name}
          </DropdownItem>
        </>
      </DropdownMenu>
    </Dropdown>
  );
}