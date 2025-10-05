import { useState } from 'react'

function App() {
  const [count, setCount] = useState(0)

  return (
    <div className="min-h-screen bg-gray-50 dark:bg-gray-900 flex items-center justify-center">
      <div className="text-center space-y-8 p-8">
        <h1 className="text-4xl sm:text-5xl font-bold text-gray-900 dark:text-white">
          BeatFox Client
        </h1>
        <p className="text-lg text-gray-600 dark:text-gray-400">
          Vite + React + TypeScript + Tailwind
        </p>
        <div className="space-y-4">
          <button
            onClick={() => setCount((count) => count + 1)}
            className="px-6 py-3 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-lg transition-colors"
          >
            Count is {count}
          </button>
          <p className="text-sm text-gray-500 dark:text-gray-500">
            Edit <code className="bg-gray-200 dark:bg-gray-800 px-2 py-1 rounded">src/App.tsx</code> to get started
          </p>
        </div>
      </div>
    </div>
  )
}

export default App