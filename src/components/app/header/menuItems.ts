import {
  UserIcon,
  ChatBubbleLeftRightIcon,
  ClipboardDocumentListIcon,
  HomeModernIcon,
  CreditCardIcon,
  Cog6ToothIcon,
  ArrowRightCircleIcon,
} from '@heroicons/react/16/solid';

export interface MenuItem {
  name: string;
  icon: React.ComponentType<{ className?: string }>;
  shortcut: string;
  key: string;
}

export const menuItems: MenuItem[] = [
  { name: 'Profile', icon: UserIcon, shortcut: '⌘P', key: 'profile' },
  { name: 'Messages', icon: ChatBubbleLeftRightIcon, shortcut: '⌘M', key: 'messages' },
  { name: 'Tasks', icon: ClipboardDocumentListIcon, shortcut: '⌘T', key: 'tasks' },
  { name: 'Properties', icon: HomeModernIcon, shortcut: '⌘H', key: 'properties' },
  { name: 'Payments', icon: CreditCardIcon, shortcut: '⌘Y', key: 'payments' },
  { name: 'Settings', icon: Cog6ToothIcon, shortcut: '⌘,', key: 'settings' },
];

export const logoutItem: MenuItem = {
  name: 'Log Out',
  icon: ArrowRightCircleIcon,
  shortcut: '⌘Q',
  key: 'logout',
};