import { Routes, Route } from 'react-router-dom'
import LandingPage from './components/LandingPage'
import DemoPage from './components/DemoPage'

function App() {
  return (
    <Routes>
      <Route path="/" element={<LandingPage />} />
      <Route path="/demo" element={<DemoPage />} />
    </Routes>
  )
}

export default App
