import { Card, CardBody, Chip, Button, Skeleton, Alert } from '@heroui/react';
import { useLoaded } from '../hooks/useLoaded';

export default function PageContent() {
  const isLoaded = useLoaded(1000);
  return (
    <Card className="p-6" radius="sm" shadow="sm" fullWidth>
      <CardBody className="flex flex-col gap-4">
        <div className="flex justify-between gap-2 w-full">
          <Skeleton isLoaded={isLoaded} className="rounded">
            <h1 className="items-center font-semibold text-2xl">
              Fence Installation #1198367{' '}
              <Chip
                color="warning"
                size="sm"
                variant="solid"
                className="inline-flex bottom-1 relative"
              >
                Preparing Project
              </Chip>
            </h1>
          </Skeleton>
          <Skeleton isLoaded={isLoaded} className="rounded">
            <Button
              color="default"
              size="sm"
              variant="bordered"
              className="shrink-0"
            >
              Change Order
            </Button>
          </Skeleton>
        </div>
        <Skeleton isLoaded={isLoaded} className="rounded">
          <Alert
            color="primary"
            variant="faded"
            title="Expected installation by September 16th"
            description="All required materials for your project have been ordered"
            endContent={
              <Button
                className="shrink-0f"
                size="sm"
                color="primary"
                variant="solid"
              >
                Track Materials
              </Button>
            }
          />
        </Skeleton>
      </CardBody>
    </Card>
  );
}
