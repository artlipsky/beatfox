import {
  NavbarMenu,
  NavbarMenuItem,
  Link,
} from '@heroui/react';
import { menuItems, logoutItem } from './menuItems';

export default function MobileMenu() {
  return (
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
          <logoutItem.icon className="size-5" />
          {logoutItem.name}
        </Link>
      </NavbarMenuItem>
    </NavbarMenu>
  );
}