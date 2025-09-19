import {
  NavbarMenu,
  NavbarMenuItem,
  Link,
  Divider,
} from '@heroui/react';
import { primaryMenuItems, menuItems, logoutItem } from '../../constants/menuItems';

export default function MobileMenu() {
  return (
    <NavbarMenu className="gap-4">
      {primaryMenuItems.map((item, index) => (
        <NavbarMenuItem key={`${item.name}-${index}`}>
          <Link
            underline="hover"
            className="flex items-center gap-2 w-full"
            color="foreground"
            href="#"
            size="lg"
          >
            <item.icon className="opacity-50 size-5" />
            {item.name}
          </Link>
        </NavbarMenuItem>
      ))}
      <Divider className="my-2" />
      {menuItems.map((item, index) => (
        <NavbarMenuItem key={`${item.name}-${index}`}>
          <Link
            underline="hover"
            className="flex items-center gap-2 w-full"
            color="foreground"
            href="#"
            size="lg"
          >
            <item.icon className="opacity-50 size-5" />
            {item.name}
          </Link>
        </NavbarMenuItem>
      ))}
      <Divider className="my-2" />
      <NavbarMenuItem>
        <Link
          underline="hover"
          className="flex items-center gap-2 w-full"
          color="danger"
          href="#"
          size="lg"
        >
          <logoutItem.icon className="size-5" />
          {logoutItem.name}
        </Link>
      </NavbarMenuItem>
    </NavbarMenu>
  );
}