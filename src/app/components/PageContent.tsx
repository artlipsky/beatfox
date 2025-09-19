import { Card, CardBody, Chip, Button } from '@heroui/react';
import { PencilIcon } from '@heroicons/react/16/solid';

export default function PageContent() {
  return (
    <Card className="p-6" radius="sm" shadow="sm" fullWidth>
      <CardBody>
        <div className="flex justify-between items-center gap-2 w-full">
          <h1 className="items-center font-semibold text-2xl">
            Fence Installation #1198367{' '}
            <Chip color="warning" size="sm" variant="solid" className="inline-flex bottom-1 relative">
              Preparing Project
            </Chip>
          </h1>
          <Button
            color="default"
            size="sm"
            variant="bordered"
            startContent={<PencilIcon className="size-4" />}
          >
            Change Order
          </Button>
        </div>
      </CardBody>
    </Card>
  );
}
