import {
  Card,
  CardBody,
  Listbox,
  ListboxItem,
  Divider,
  Button,
  Skeleton,
  Chip,
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
                startContent={
                  <Skeleton isLoaded={isLoaded} className="rounded-full">
                    <item.icon className="opacity-50 size-4" />
                  </Skeleton>
                }
                href="#"
                variant="flat"
              >
                <Skeleton isLoaded={isLoaded} className="rounded">
                  {item.name}
                </Skeleton>
              </ListboxItem>
            ))}
          </Listbox>
          <Skeleton isLoaded={isLoaded} className="rounded">
            <p className="px-2 py-1 font-regular text-gray-500 dark:text-gray-400 text-xs uppercase">
              Accessing as Arthur Lipsky
            </p>
          </Skeleton>
          <Divider />
          <Listbox aria-label="Secondary navigation">
            {menuItems.map(item => (
              <ListboxItem
                key={item.key}
                startContent={
                  <Skeleton isLoaded={isLoaded} className="rounded-full">
                    <item.icon className="opacity-50 size-4" />
                  </Skeleton>
                }
                href="#"
                variant="flat"
                endContent={
                  item.key === 'tasks' ? (
                    <Skeleton isLoaded={isLoaded} className="rounded-full">
                      <Chip color="danger" size="sm" variant="solid">
                        3
                      </Chip>
                    </Skeleton>
                  ) : null
                }
              >
                <Skeleton isLoaded={isLoaded} className="rounded">
                  {item.name}
                </Skeleton>
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
