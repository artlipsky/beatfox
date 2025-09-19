import {
  Card,
  CardBody,
  Listbox,
  ListboxItem,
  Divider,
  Button,
  Skeleton,
} from '@heroui/react';
import { GiftIcon } from '@heroicons/react/16/solid';
import { primaryMenuItems, menuItems } from '../constants/menuItems';
import { useLoaded } from '../hooks/useLoaded';

export default function SideBar() {
  const isLoaded = useLoaded(1000);

  return (
    <aside className="hidden sm:flex w-full max-w-56 h-auto">
      <Card
        className="top-22 sticky w-full h-min"
        radius="sm"
        shadow="sm"
        fullWidth
      >
        <CardBody className="gap-2 p-2">
          <Listbox aria-label="Primary navigation">
            {primaryMenuItems.map(item => (
              <ListboxItem
                key={item.key}
                startContent={<item.icon className="opacity-50 size-4" />}
                href="#"
                variant="flat"
              >
                {item.name}
              </ListboxItem>
            ))}
          </Listbox>
          <Divider />
          <Listbox aria-label="Secondary navigation">
            {menuItems.map(item => (
              <ListboxItem
                key={item.key}
                startContent={<item.icon className="opacity-50 size-4" />}
                href="#"
                variant="flat"
              >
                {item.name}
              </ListboxItem>
            ))}
          </Listbox>
          <Skeleton isLoaded={isLoaded} className="rounded-lg">
            <Button
              radius="sm"
              variant="bordered"
              color="warning"
              fullWidth
              startContent={<GiftIcon className="size-4" />}
            >
              Refer and Earn
            </Button>
          </Skeleton>
        </CardBody>
      </Card>
    </aside>
  );
}
