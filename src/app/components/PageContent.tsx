import { Card, CardBody } from '@heroui/react';

export default function PageContent() {
  return (
    <Card className="p-6" radius="sm" shadow="sm" fullWidth>
      <CardBody>
        <h1 className="font-semibold text-2xl">Page Title</h1>
      </CardBody>
    </Card>
  );
}
