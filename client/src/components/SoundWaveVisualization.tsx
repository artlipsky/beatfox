import { useRef, useEffect, useState } from 'react';
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import {
  WaveSource,
  AcousticConstants,
  generateWaveFieldGrid,
  calculateWaveField,
  displacementToColor,
  type Point3D,
  type ColorMode,
} from '../domain/acoustics/WavePhysics';

const SOURCE_PROFILES = [
  {
    label: 'Left Mix Monitor',
    badge: 'Control room',
    indicatorClass: 'bg-emerald-500 shadow-emerald-500/50',
    color: 0x22c55e,
    frequencyHint: 'Sweep 80–150 Hz to nail crossover into the sub and keep the phantom center stable.',
    amplitudeHint: 'Trim gain until the left channel matches the reference SPL at the mix chair.',
    phaseHint: 'Use small offsets to collapse or widen the stereo image at the listening position.',
    defaults: {
      position: [-3, 0, -2] as const,
      frequency: 110,
      amplitude: 1.2,
      phase: 0,
    },
  },
  {
    label: 'Right Mix Monitor',
    badge: 'Control room',
    indicatorClass: 'bg-sky-500 shadow-sky-500/50',
    color: 0x0ea5e9,
    frequencyHint: 'Mirror the left monitor and chase modal build-up between 60 Hz and 200 Hz.',
    amplitudeHint: 'Match gain with the left channel to protect vocal clarity and imaging.',
    phaseHint: 'Time-align against the left monitor to lock the phantom center.',
    defaults: {
      position: [3, 0, -2] as const,
      frequency: 110,
      amplitude: 1.2,
      phase: Math.PI / 32,
    },
  },
  {
    label: 'Isolation Test Emitter',
    badge: 'Build-out',
    indicatorClass: 'bg-orange-500 shadow-orange-500/50',
    color: 0xf97316,
    frequencyHint: 'Run 63 Hz reference sweeps to evaluate wall assemblies and flanking paths.',
    amplitudeHint: 'Push the source to expose weak studs, door seals, or glazing details.',
    phaseHint: 'Offset phase to model staggered studs, resilient channels, or floated floors.',
    defaults: {
      position: [0, 0, 3] as const,
      frequency: 63,
      amplitude: 0.9,
      phase: Math.PI / 4,
    },
  },
] as const;

const FREQUENCY_RANGE = { min: 40, max: 400, step: 5 } as const;
const AMPLITUDE_RANGE = { min: 0.2, max: 3, step: 0.1 } as const;

/**
 * Scene3D - Pure Three.js visualization
 */
interface Scene3DProps {
  sources: WaveSource[];
  time: number;
  isPlaying: boolean;
  visualizationMode: ColorMode;
  onTimeUpdate: (t: number) => void;
}

function Scene3D({ sources, time, isPlaying, visualizationMode, onTimeUpdate }: Scene3DProps) {
  const containerRef = useRef<HTMLDivElement>(null);
  const sceneRef = useRef<THREE.Scene | null>(null);
  const cameraRef = useRef<THREE.PerspectiveCamera | null>(null);
  const rendererRef = useRef<THREE.WebGLRenderer | null>(null);
  const controlsRef = useRef<OrbitControls | null>(null);
  const pointsRef = useRef<THREE.Points | null>(null);
  const gridPointsRef = useRef<Point3D[]>([]);
  const animationRef = useRef<number | null>(null);
  const timeRef = useRef(0);
  const sourceMarkersRef = useRef<THREE.Mesh[]>([]);
  const isPlayingRef = useRef(isPlaying);
  const onTimeUpdateRef = useRef(onTimeUpdate);

  useEffect(() => {
    isPlayingRef.current = isPlaying;
  }, [isPlaying]);

  useEffect(() => {
    onTimeUpdateRef.current = onTimeUpdate;
  }, [onTimeUpdate]);

  // Initialize Three.js scene
  useEffect(() => {
    if (!containerRef.current) return;

    const container = containerRef.current;
    const width = container.clientWidth;
    const height = container.clientHeight;

    // Scene
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x0f172a); // slate-900
    sceneRef.current = scene;

    // Camera
    const camera = new THREE.PerspectiveCamera(60, width / height, 0.1, 1000);
    camera.position.set(0, 15, 15);
    camera.lookAt(0, 0, 0);
    cameraRef.current = camera;

    // Renderer
    const renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(width, height);
    renderer.setPixelRatio(window.devicePixelRatio);
    container.appendChild(renderer.domElement);
    rendererRef.current = renderer;

    // Controls
    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;
    controls.dampingFactor = 0.05;
    controls.minDistance = 5;
    controls.maxDistance = 50;
    controlsRef.current = controls;

    // Lighting
    const ambientLight = new THREE.AmbientLight(0xffffff, 0.4);
    scene.add(ambientLight);

    const pointLight1 = new THREE.PointLight(0xffffff, 1);
    pointLight1.position.set(10, 10, 10);
    scene.add(pointLight1);

    const pointLight2 = new THREE.PointLight(0xffffff, 0.5);
    pointLight2.position.set(-10, -10, -10);
    scene.add(pointLight2);

    // Grid helper
    const gridHelper = new THREE.GridHelper(20, 20, 0x6366f1, 0x8b5cf6);
    gridHelper.position.y = 0;
    scene.add(gridHelper);

    // Generate wave field grid points (memoized)
    const gridPoints = generateWaveFieldGrid(20, 80);
    gridPointsRef.current = gridPoints;

    // Create points geometry for wave field
    const geometry = new THREE.BufferGeometry();
    const positions = new Float32Array(gridPoints.length * 3);
    const colors = new Float32Array(gridPoints.length * 3);

    gridPoints.forEach((point, i) => {
      positions[i * 3] = point.x;
      positions[i * 3 + 1] = point.y;
      positions[i * 3 + 2] = point.z;
      colors[i * 3] = 1;
      colors[i * 3 + 1] = 1;
      colors[i * 3 + 2] = 1;
    });

    geometry.setAttribute('position', new THREE.BufferAttribute(positions, 3));
    geometry.setAttribute('color', new THREE.BufferAttribute(colors, 3));

    const material = new THREE.PointsMaterial({
      size: 0.15,
      vertexColors: true,
      sizeAttenuation: true,
    });

    const points = new THREE.Points(geometry, material);
    scene.add(points);
    pointsRef.current = points;

    // Handle window resize
    const handleResize = () => {
      if (!container || !camera || !renderer) return;
      const width = container.clientWidth;
      const height = container.clientHeight;
      camera.aspect = width / height;
      camera.updateProjectionMatrix();
      renderer.setSize(width, height);
    };

    window.addEventListener('resize', handleResize);

    // Animation loop
    const animate = () => {
      animationRef.current = requestAnimationFrame(animate);

      if (isPlayingRef.current) {
        timeRef.current += 0.016;
        onTimeUpdateRef.current(timeRef.current);
      }

      if (controls) {
        controls.update();
      }

      if (renderer && scene && camera) {
        renderer.render(scene, camera);
      }
    };

    animate();

    // Cleanup
    return () => {
      window.removeEventListener('resize', handleResize);
      if (animationRef.current !== null) {
        cancelAnimationFrame(animationRef.current);
      }
      if (controls) {
        controls.dispose();
      }
      if (renderer) {
        renderer.dispose();
        container.removeChild(renderer.domElement);
      }
      if (geometry) {
        geometry.dispose();
      }
      if (material) {
        material.dispose();
      }
    };
  }, []);

  // Update wave field based on sources and time
  useEffect(() => {
    if (!pointsRef.current || !gridPointsRef.current.length) return;

    const points = pointsRef.current;
    const gridPoints = gridPointsRef.current;
    const posArray = points.geometry.attributes.position.array as Float32Array;
    const colArray = points.geometry.attributes.color.array as Float32Array;

    // Calculate wave field using domain logic
    const waveField = calculateWaveField(gridPoints, sources, time);

    // Update positions and colors
    gridPoints.forEach((_, idx) => {
      const displacement = waveField.displacements[idx];

      // Update Y position based on wave displacement
      posArray[idx * 3 + 1] = displacement * 2;

      // Normalize displacement for color mapping
      const normalizedDisp = waveField.maxDisplacement > 0
        ? displacement / waveField.maxDisplacement
        : 0;

      // Get color from domain function
      const [r, g, b] = displacementToColor(normalizedDisp, visualizationMode);

      colArray[idx * 3] = r;
      colArray[idx * 3 + 1] = g;
      colArray[idx * 3 + 2] = b;
    });

    points.geometry.attributes.position.needsUpdate = true;
    points.geometry.attributes.color.needsUpdate = true;
  }, [sources, time, visualizationMode]);

  // Update source markers
  useEffect(() => {
    if (!sceneRef.current) return;

    const scene = sceneRef.current;

    // Remove old markers
    sourceMarkersRef.current.forEach(marker => {
      scene.remove(marker);
      marker.geometry.dispose();
      (marker.material as THREE.Material).dispose();
    });
    sourceMarkersRef.current = [];

    // Add new markers
    sources.forEach((source, idx) => {
      const geometry = new THREE.SphereGeometry(0.3, 16, 16);
      const color = SOURCE_PROFILES[idx]?.color ?? 0x3b82f6;
      const material = new THREE.MeshStandardMaterial({
        color,
        emissive: color,
        emissiveIntensity: 0.5,
      });
      const marker = new THREE.Mesh(geometry, material);
      marker.position.set(source.position[0], source.position[1], source.position[2]);
      scene.add(marker);
      sourceMarkersRef.current.push(marker);
    });
  }, [sources]);

  // Animate source markers
  useEffect(() => {
    let animId: number;
    const animateMarkers = () => {
      const scale = 1 + Math.sin(Date.now() * 0.004) * 0.2;
      sourceMarkersRef.current.forEach(marker => {
        marker.scale.setScalar(scale);
      });
      animId = requestAnimationFrame(animateMarkers);
    };
    animateMarkers();
    return () => cancelAnimationFrame(animId);
  }, []);

  // Sync isPlaying state
  useEffect(() => {
    if (!isPlaying) {
      timeRef.current = time;
    }
  }, [isPlaying, time]);

  return <div ref={containerRef} className="h-full w-full" />;
}

/**
 * InfoOverlay - Display physics information
 */
function InfoOverlay() {
  return (
    <div className="pointer-events-none absolute left-4 top-4 rounded-xl border border-slate-700/50 bg-slate-900/90 px-4 py-3 shadow-2xl backdrop-blur-xl">
      <div className="flex items-center gap-2 text-sm font-semibold text-white">
        <span className="text-base">🔊</span>
        Acoustic Wave Simulation
      </div>
      <div className="mt-2 space-y-1 text-xs text-slate-300">
        <div className="font-mono">c = {AcousticConstants.SPEED_OF_SOUND} m/s</div>
        <div className="font-mono text-slate-400">φ = k·r - ω·t</div>
      </div>
    </div>
  );
}

/**
 * ControlPanel - UI controls for wave parameters
 */
interface ControlPanelProps {
  isPlaying: boolean;
  visualizationMode: ColorMode;
  sources: WaveSource[];
  isExpanded: boolean;
  onPlayPause: () => void;
  onVisualizationModeChange: (mode: ColorMode) => void;
  onSourceUpdate: (index: number, source: WaveSource) => void;
  onToggleExpand: () => void;
}

function ControlPanel({
  isPlaying,
  visualizationMode,
  sources,
  isExpanded,
  onPlayPause,
  onVisualizationModeChange,
  onSourceUpdate,
  onToggleExpand,
}: ControlPanelProps) {
  const [expandedSources, setExpandedSources] = useState<Set<number>>(() => new Set([0]));

  const toggleSourceCard = (index: number) => {
    setExpandedSources((prev) => {
      const next = new Set(prev);
      if (next.has(index)) {
        next.delete(index);
      } else {
        next.add(index);
      }
      return next;
    });
  };

  return (
    <div className="border-t border-slate-700/70 bg-slate-900/95 backdrop-blur">
      {/* Header Controls - Always visible and clickable */}
      <div className="relative z-20 border-b border-slate-700/50 bg-slate-900 px-4 py-3">
        <div className="flex flex-wrap items-center justify-between gap-3">
          <div className="flex items-center gap-3">
            <button
              onClick={onPlayPause}
              type="button"
              className="flex items-center gap-2 rounded-lg bg-gradient-to-r from-blue-600 to-blue-500 px-4 py-2 text-sm font-semibold text-white shadow-lg transition hover:from-blue-500 hover:to-blue-400 hover:shadow-xl"
            >
              <span className="text-base">{isPlaying ? '⏸' : '▶'}</span>
              {isPlaying ? 'Pause' : 'Play'}
            </button>

            <button
              onClick={onToggleExpand}
              type="button"
              className="flex items-center gap-2 rounded-lg bg-slate-800 px-3 py-2 text-sm text-slate-300 transition hover:bg-slate-700 hover:text-white"
            >
              <span>{isExpanded ? '▼' : '▲'}</span>
              <span>{isExpanded ? 'Collapse' : 'Expand'}</span>
            </button>
          </div>

          <div className="flex gap-2">
            <button
              onClick={() => onVisualizationModeChange('displacement')}
              type="button"
              className={`rounded-lg px-4 py-2 text-xs font-semibold shadow transition ${
                visualizationMode === 'displacement'
                  ? 'bg-gradient-to-r from-purple-600 to-purple-500 text-white shadow-purple-500/50'
                  : 'bg-slate-800 text-slate-300 hover:bg-slate-700 hover:text-white'
              }`}
            >
              Displacement
            </button>
            <button
              onClick={() => onVisualizationModeChange('pressure')}
              type="button"
              className={`rounded-lg px-4 py-2 text-xs font-semibold shadow transition ${
                visualizationMode === 'pressure'
                  ? 'bg-gradient-to-r from-purple-600 to-purple-500 text-white shadow-purple-500/50'
                  : 'bg-slate-800 text-slate-300 hover:bg-slate-700 hover:text-white'
              }`}
            >
              Pressure
            </button>
          </div>
        </div>
      </div>

      {/* Expandable Controls */}
      {isExpanded && (
        <div className="max-h-[350px] space-y-3 overflow-y-auto p-4">
          <div className="grid gap-3 md:grid-cols-2">
            {sources.map((source, idx) => {
              const profile = SOURCE_PROFILES[idx];
              const indicatorClass = profile?.indicatorClass ?? 'bg-blue-500 shadow-blue-500/50';
              const safeAmplitude = Math.max(source.amplitude, 0.001);
              const gainDb = 20 * Math.log10(safeAmplitude);
              const phaseDegrees = (source.phase * 180) / Math.PI;
              const periodMs = source.frequency > 0 ? 1000 / source.frequency : 0;
              const alignmentMs = periodMs * (source.phase / (2 * Math.PI));
              const isCardExpanded = expandedSources.has(idx);

              return (
                <div
                  key={idx}
                  className="rounded-xl border border-slate-700/50 bg-slate-800/50 p-4 shadow-lg backdrop-blur"
                >
                  <button
                    type="button"
                    onClick={() => toggleSourceCard(idx)}
                    className="mb-2 flex w-full items-center justify-between gap-3 text-left text-sm font-semibold text-white transition hover:text-blue-200"
                    aria-expanded={isCardExpanded}
                  >
                    <span className="flex items-center gap-2">
                      <span>{profile?.label ?? `Source ${idx + 1}`}</span>
                      {profile?.badge && (
                        <span className="rounded-full border border-slate-700/80 bg-slate-900/70 px-2 py-0.5 text-[10px] font-medium uppercase tracking-wide text-slate-300">
                          {profile.badge}
                        </span>
                      )}
                    </span>
                    <span className="flex items-center gap-2 text-xs font-medium text-slate-300">
                      <span className={`h-3 w-3 rounded-full shadow-lg ${indicatorClass}`} />
                      <span className="rounded-full border border-slate-700/60 bg-slate-900/70 px-2 py-0.5 text-[10px] uppercase tracking-wide text-slate-400">
                        {isCardExpanded ? 'Hide controls' : 'Show controls'}
                      </span>
                      <span aria-hidden>{isCardExpanded ? '▾' : '▸'}</span>
                    </span>
                  </button>

                  {isCardExpanded && (
                    <div className="mt-3 space-y-3 text-left">
                      <div>
                        <div className="mb-1 flex items-baseline justify-between">
                          <label className="text-xs font-medium text-slate-300">Crossover / Frequency</label>
                          <span className="font-mono text-xs text-slate-400">
                            {source.frequency.toFixed(0)} Hz
                          </span>
                        </div>
                      <input
                        type="range"
                        min={FREQUENCY_RANGE.min}
                        max={FREQUENCY_RANGE.max}
                        step={FREQUENCY_RANGE.step}
                        value={source.frequency}
                        onChange={(e) => onSourceUpdate(idx, source.update('frequency', parseFloat(e.target.value)))}
                        className="w-full accent-purple-500"
                      />
                      <div className="mt-1 flex items-center justify-between text-[11px] text-slate-400">
                        <span>{profile?.frequencyHint ?? 'Set the fundamental you want to monitor.'}</span>
                        <span className="font-mono text-xs text-slate-500">λ ≈ {source.wavelength.toFixed(2)} m</span>
                      </div>
                    </div>

                      <div>
                        <div className="mb-1 flex items-baseline justify-between">
                          <label className="text-xs font-medium text-slate-300">Gain Trim</label>
                          <span className="font-mono text-xs text-slate-400">
                            {gainDb >= 0 ? '+' : ''}{gainDb.toFixed(1)} dB
                          </span>
                        </div>
                      <input
                        type="range"
                        min={AMPLITUDE_RANGE.min}
                        max={AMPLITUDE_RANGE.max}
                        step={AMPLITUDE_RANGE.step}
                        value={source.amplitude}
                        onChange={(e) => onSourceUpdate(idx, source.update('amplitude', parseFloat(e.target.value)))}
                        className="w-full accent-purple-500"
                      />
                      <div className="mt-1 text-[11px] text-slate-400">
                        {profile?.amplitudeHint ?? 'Calibrate level for even coverage or isolation stress tests.'}
                      </div>
                    </div>

                      <div>
                        <div className="mb-1 flex items-baseline justify-between">
                          <label className="text-xs font-medium text-slate-300">Phase / Delay</label>
                          <span className="font-mono text-xs text-slate-400">
                            {phaseDegrees.toFixed(0)}° · {alignmentMs.toFixed(1)} ms
                          </span>
                        </div>
                      <input
                        type="range"
                        min={0}
                        max={Math.PI * 2}
                        step={0.1}
                        value={source.phase}
                        onChange={(e) => onSourceUpdate(idx, source.update('phase', parseFloat(e.target.value)))}
                        className="w-full accent-purple-500"
                      />
                        <div className="mt-1 text-[11px] text-slate-400">
                          {profile?.phaseHint ?? 'Offset arrival time to explore constructive vs destructive interference.'}
                        </div>
                      </div>
                    </div>
                  )}
                </div>
              );
            })}
          </div>

          <div className="rounded-xl border border-blue-500/20 bg-blue-950/20 p-4">
            <div className="mb-2 flex items-center gap-2 text-sm font-semibold text-blue-300">
              <span>ℹ️</span>
              <span>Physics Reference</span>
            </div>
            <ul className="space-y-1 text-xs text-slate-400">
              <li className="font-mono">• ∂²p/∂t² = c²∇²p (c = {AcousticConstants.SPEED_OF_SOUND} m/s)</li>
              <li>• Spherical spreading: Amplitude ∝ 1/r</li>
              <li>• Superposition of wave sources</li>
              <li>• Wavelength λ = c/f, Wave number k = 2π/λ</li>
            </ul>
          </div>
        </div>
      )}
    </div>
  );
}

/**
 * SoundWaveVisualization - Main component (UI orchestration only)
 */
export default function SoundWaveVisualization() {
  const [isPlaying, setIsPlaying] = useState(true);
  const [visualizationMode, setVisualizationMode] = useState<ColorMode>('displacement');
  const [isExpanded, setIsExpanded] = useState(true);
  const [sources, setSources] = useState<WaveSource[]>(() =>
    SOURCE_PROFILES.map((profile) =>
      new WaveSource(
        profile.defaults.position,
        profile.defaults.frequency,
        profile.defaults.amplitude,
        profile.defaults.phase,
      ),
    )
  );
  const [time, setTime] = useState(0);

  const handleSourceUpdate = (index: number, newSource: WaveSource) => {
    setSources((prev) => {
      const updated = [...prev];
      updated[index] = newSource;
      return updated;
    });
  };

  return (
    <div className="flex h-full w-full flex-col">
      <div className="relative flex-1 min-h-0">
        <Scene3D
          sources={sources}
          time={time}
          isPlaying={isPlaying}
          visualizationMode={visualizationMode}
          onTimeUpdate={setTime}
        />
        <InfoOverlay />
      </div>

      <div className="relative z-10 flex-shrink-0">
        <ControlPanel
          isPlaying={isPlaying}
          visualizationMode={visualizationMode}
          sources={sources}
          isExpanded={isExpanded}
          onPlayPause={() => setIsPlaying(!isPlaying)}
          onVisualizationModeChange={setVisualizationMode}
          onSourceUpdate={handleSourceUpdate}
          onToggleExpand={() => setIsExpanded(!isExpanded)}
        />
      </div>
    </div>
  );
}

/**
 * SoundWaveDemo - Wrapper component
 */
export function SoundWaveDemo() {
  return (
    <div className="h-[600px] w-full overflow-hidden rounded-xl border border-slate-700/70 bg-slate-900 shadow-2xl md:h-[700px] lg:h-[900px]">
      <SoundWaveVisualization />
    </div>
  );
}
