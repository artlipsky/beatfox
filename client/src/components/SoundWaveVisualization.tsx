import { useRef, useEffect, useState } from 'react';
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { mergeGeometries } from 'three/examples/jsm/utils/BufferGeometryUtils.js';
import { acceleratedRaycast, computeBoundsTree, disposeBoundsTree } from 'three-mesh-bvh';
import {
  WaveSource,
  AcousticConstants,
  generateWaveFieldGrid,
  calculateWaveDisplacementFromSource,
  displacementToColor,
  type Point3D,
  type ColorMode,
} from '../domain/acoustics/WavePhysics';

(THREE.BufferGeometry.prototype as unknown as { computeBoundsTree: typeof computeBoundsTree }).computeBoundsTree = computeBoundsTree;
(THREE.BufferGeometry.prototype as unknown as { disposeBoundsTree: typeof disposeBoundsTree }).disposeBoundsTree = disposeBoundsTree;
(THREE.Mesh.prototype as unknown as { raycast: typeof acceleratedRaycast }).raycast = acceleratedRaycast;

const reflectionRaycaster = new THREE.Raycaster();
const tempVecOrigin = new THREE.Vector3();
const tempVecTarget = new THREE.Vector3();
const tempVecDirection = new THREE.Vector3();

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
const LAYER_COUNT = 2;
const LAYER_SPACING = 0.6;
const VERTICAL_SCALE = 1.8;
const WALL_HEIGHT = 2.6;
const WALL_THICKNESS = 0.4;
const HOUSE_WIDTH = 14;
const HOUSE_DEPTH = 10;
const DOOR_GAP = 2;

type WallSegment = {
  readonly position: readonly [number, number, number];
  readonly size: readonly [number, number, number];
  readonly material: 'exterior' | 'interior';
};

const WALL_SEGMENTS: readonly WallSegment[] = [
  {
    position: [0, WALL_HEIGHT / 2, -HOUSE_DEPTH / 2],
    size: [HOUSE_WIDTH, WALL_HEIGHT, WALL_THICKNESS],
    material: 'exterior',
  },
  {
    position: [0, WALL_HEIGHT / 2, HOUSE_DEPTH / 2],
    size: [HOUSE_WIDTH, WALL_HEIGHT, WALL_THICKNESS],
    material: 'exterior',
  },
  {
    position: [-HOUSE_WIDTH / 2, WALL_HEIGHT / 2, 0],
    size: [WALL_THICKNESS, WALL_HEIGHT, HOUSE_DEPTH],
    material: 'exterior',
  },
  {
    position: [HOUSE_WIDTH / 2, WALL_HEIGHT / 2, 0],
    size: [WALL_THICKNESS, WALL_HEIGHT, HOUSE_DEPTH],
    material: 'exterior',
  },
  {
    position: [-(DOOR_GAP / 2 + (HOUSE_WIDTH - DOOR_GAP) / 4), WALL_HEIGHT / 2, -2],
    size: [(HOUSE_WIDTH - DOOR_GAP) / 2, WALL_HEIGHT, WALL_THICKNESS],
    material: 'interior',
  },
  {
    position: [DOOR_GAP / 2 + (HOUSE_WIDTH - DOOR_GAP) / 4, WALL_HEIGHT / 2, -2],
    size: [(HOUSE_WIDTH - DOOR_GAP) / 2, WALL_HEIGHT, WALL_THICKNESS],
    material: 'interior',
  },
  {
    position: [-2.5, WALL_HEIGHT / 2, 0],
    size: [WALL_THICKNESS, WALL_HEIGHT, 6],
    material: 'interior',
  },
  {
    position: [2.5, WALL_HEIGHT / 2, 1.5],
    size: [WALL_THICKNESS, WALL_HEIGHT, 7],
    material: 'interior',
  },
  {
    position: [0, WALL_HEIGHT / 2, 3],
    size: [6, WALL_HEIGHT, WALL_THICKNESS],
    material: 'interior',
  },
] as const;

const WALL_COLOR: Record<WallSegment['material'], number> = {
  exterior: 0x1f2937,
  interior: 0x334155,
};

const FLOOR_SIZE = { width: HOUSE_WIDTH + 1.5, depth: HOUSE_DEPTH + 1.5 } as const;
const FLOOR_COLOR = 0x0b1220;
const FLOOR_OPACITY = 0.92;

type WallMaterialKey = WallSegment['material'];
type WallReflectivity = Record<WallMaterialKey, number>;

const DEFAULT_WALL_REFLECTIVITY: WallReflectivity = {
  exterior: 0.65,
  interior: 0.45,
};

type WallMeta = WallSegment & {
  readonly normalAxis: 'x' | 'z';
  readonly plane: number;
};

const WALL_META: readonly WallMeta[] = WALL_SEGMENTS.map((segment) => {
  const normalAxis: 'x' | 'z' = segment.size[0] < segment.size[2] ? 'x' : 'z';
  const plane = normalAxis === 'x' ? segment.position[0] : segment.position[2];
  return {
    ...segment,
    normalAxis,
    plane,
  };
});

const EPSILON = 1e-6;

interface ReflectionDescriptor {
  readonly wall: WallMeta;
  readonly baseSource: WaveSource;
  readonly imageSource: WaveSource;
}

function createReflectionDescriptors(
  sources: readonly WaveSource[],
  wallReflectivity: WallReflectivity,
): ReflectionDescriptor[] {
  const descriptors: ReflectionDescriptor[] = [];

  sources.forEach((source) => {
    WALL_META.forEach((wall) => {
      const reflectivity = wallReflectivity[wall.material];
      if (reflectivity <= 0.01) return;

      const axisIndex = wall.normalAxis === 'x' ? 0 : 2;
      const sourceCoord = source.position[axisIndex];
      const plane = wall.plane;
      if (Math.abs(sourceCoord - plane) < WALL_THICKNESS / 2) return;

      const mirroredPosition = mirrorPositionAcrossWall(source.position, wall);
      const amplitude = source.amplitude * reflectivity;
      if (amplitude <= 0.001) return;

      const phaseShift = wall.material === 'exterior' ? Math.PI : Math.PI / 2;
      const imageSource = new WaveSource(
        [mirroredPosition[0], mirroredPosition[1], mirroredPosition[2]],
        source.frequency,
        amplitude,
        (source.phase + phaseShift) % (Math.PI * 2),
      );

      descriptors.push({ wall, baseSource: source, imageSource });
    });
  });

  return descriptors;
}

function mirrorPositionAcrossWall(position: readonly [number, number, number], wall: WallMeta): readonly [number, number, number] {
  if (wall.normalAxis === 'x') {
    const mirroredX = wall.plane * 2 - position[0];
    return [mirroredX, position[1], position[2]] as const;
  }
  const mirroredZ = wall.plane * 2 - position[2];
  return [position[0], position[1], mirroredZ] as const;
}

function isSameSide(a: number, b: number, plane: number): boolean {
  const da = a - plane;
  const db = b - plane;
  if (Math.abs(da) < EPSILON || Math.abs(db) < EPSILON) return true;
  return da * db > 0;
}

function isReflectionPathValid(
  point: Point3D,
  imagePosition: readonly [number, number, number],
  wall: WallMeta,
  wallBVHMesh: THREE.Mesh | null,
  wallFaceLookup: Uint16Array | null,
): boolean {
  const axisIndex = wall.normalAxis === 'x' ? 0 : 2;
  const plane = wall.plane;
  const pointCoord = axisIndex === 0 ? point.x : point.z;
  const imageCoord = axisIndex === 0 ? imagePosition[0] : imagePosition[2];
  const denom = pointCoord - imageCoord;

  if (Math.abs(denom) < EPSILON) return false;

  const t = (plane - imageCoord) / denom;
  if (t <= 0 || t >= 1) return false;

  const intersectX = imagePosition[0] + (point.x - imagePosition[0]) * t;
  const intersectY = imagePosition[1] + (point.y - imagePosition[1]) * t;
  const intersectZ = imagePosition[2] + (point.z - imagePosition[2]) * t;

  if (Math.abs(intersectX - wall.position[0]) > wall.size[0] / 2 + EPSILON) return false;
  if (Math.abs(intersectZ - wall.position[2]) > wall.size[2] / 2 + EPSILON) return false;
  if (intersectY < 0 || intersectY > wall.size[1] + EPSILON) return false;

  if (!wallBVHMesh || !wallFaceLookup) {
    return true;
  }

  const origin = tempVecOrigin.set(imagePosition[0], imagePosition[1], imagePosition[2]);
  const target = tempVecTarget.set(point.x, point.y, point.z);
  tempVecDirection.subVectors(target, origin);
  const distance = tempVecDirection.length();
  if (distance < EPSILON) {
    return false;
  }

  tempVecDirection.normalize();
  reflectionRaycaster.set(origin, tempVecDirection);
  reflectionRaycaster.far = distance - EPSILON;
  const hits = reflectionRaycaster.intersectObject(wallBVHMesh, false);
  if (!hits.length) {
    return false;
  }

  const firstHit = hits[0];
  const faceIndex = firstHit.faceIndex ?? 0;
  const wallIndex = wallFaceLookup[faceIndex];
  return WALL_META[wallIndex] === wall;
}

function isPointInsideWall(segment: WallSegment, point: Point3D): boolean {
  const halfWidth = segment.size[0] / 2;
  const halfDepth = segment.size[2] / 2;
  const dx = Math.abs(point.x - segment.position[0]);
  const dz = Math.abs(point.z - segment.position[2]);
  return dx <= halfWidth && dz <= halfDepth;
}

/**
 * Scene3D - Pure Three.js visualization
 */
interface Scene3DProps {
  sources: WaveSource[];
  time: number;
  isPlaying: boolean;
  visualizationMode: ColorMode;
  onTimeUpdate: (t: number) => void;
  wallReflectivity: WallReflectivity;
}

function Scene3D({ sources, time, isPlaying, visualizationMode, onTimeUpdate, wallReflectivity }: Scene3DProps) {
  const containerRef = useRef<HTMLDivElement>(null);
  const sceneRef = useRef<THREE.Scene | null>(null);
  const cameraRef = useRef<THREE.PerspectiveCamera | null>(null);
  const rendererRef = useRef<THREE.WebGLRenderer | null>(null);
  const controlsRef = useRef<OrbitControls | null>(null);
  const pointsRef = useRef<THREE.Points | null>(null);
  const gridPointsRef = useRef<Point3D[]>([]);
  const layerFactorRef = useRef<number[]>([]);
  const obstructionMaskRef = useRef<Uint8Array>(new Uint8Array());
  const structuralGroupRef = useRef<THREE.Group | null>(null);
  const wallBVHMeshRef = useRef<THREE.Mesh | null>(null);
  const wallFaceLookupRef = useRef<Uint16Array | null>(null);
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

    const structuralGroup = new THREE.Group();

    const floorGeometry = new THREE.PlaneGeometry(FLOOR_SIZE.width, FLOOR_SIZE.depth);
    const floorMaterial = new THREE.MeshStandardMaterial({
      color: FLOOR_COLOR,
      side: THREE.DoubleSide,
      transparent: true,
      opacity: FLOOR_OPACITY,
      roughness: 0.85,
      metalness: 0.05,
    });
    const floor = new THREE.Mesh(floorGeometry, floorMaterial);
    floor.rotation.x = -Math.PI / 2;
    floor.position.y = -0.02;
    structuralGroup.add(floor);

    const bvhGeometries: THREE.BufferGeometry[] = [];
    const faceLookup: number[] = [];

    WALL_SEGMENTS.forEach((segment, idx) => {
      const geometryForBVH = new THREE.BoxGeometry(segment.size[0], segment.size[1], segment.size[2]);
      geometryForBVH.translate(segment.position[0], segment.position[1], segment.position[2]);
      const indexAttr = geometryForBVH.index;
      const triangleCount = (indexAttr ? indexAttr.count : geometryForBVH.attributes.position.count) / 3;
      for (let i = 0; i < triangleCount; i += 1) {
        faceLookup.push(idx);
      }
      bvhGeometries.push(geometryForBVH);
    });

    const mergedWallsGeometry = mergeGeometries(bvhGeometries, true);
    bvhGeometries.forEach((geometry) => geometry.dispose());

    if (mergedWallsGeometry) {
      mergedWallsGeometry.computeBoundsTree();
      wallFaceLookupRef.current = new Uint16Array(faceLookup);
      wallBVHMeshRef.current = new THREE.Mesh(
        mergedWallsGeometry,
        new THREE.MeshBasicMaterial({ visible: false })
      );
    } else {
      wallFaceLookupRef.current = null;
      wallBVHMeshRef.current = null;
    }

    WALL_SEGMENTS.forEach((segment) => {
      const geometry = new THREE.BoxGeometry(segment.size[0], segment.size[1], segment.size[2]);
      const reflectivity = DEFAULT_WALL_REFLECTIVITY[segment.material];
      const opacity = Math.min(0.98, 0.55 + reflectivity * 0.35);
      const color = new THREE.Color(WALL_COLOR[segment.material]).multiplyScalar(0.75 + reflectivity * 0.25);
      const material = new THREE.MeshStandardMaterial({
        color,
        transparent: true,
        opacity,
        roughness: 0.85,
        metalness: 0.05 + reflectivity * 0.05,
      });
      const wall = new THREE.Mesh(geometry, material);
      wall.position.set(segment.position[0], segment.position[1], segment.position[2]);
      wall.userData = {
        ...wall.userData,
        wallMaterial: segment.material,
      };
      structuralGroup.add(wall);
    });

    scene.add(structuralGroup);
    structuralGroupRef.current = structuralGroup;

    // Generate wave field grid points (memoized)
    const layerOffsets = Array.from({ length: LAYER_COUNT }, (_, layerIdx) => {
      const centeredIndex = layerIdx - (LAYER_COUNT - 1) / 2;
      return centeredIndex * LAYER_SPACING;
    });

    const gridPoints: Point3D[] = [];
    const layerFactors: number[] = [];
    layerOffsets.forEach((offset, layerIdx) => {
      const layerPoints = generateWaveFieldGrid(20, 80, offset);
      const factor = LAYER_COUNT > 1 ? layerIdx / (LAYER_COUNT - 1) : 0.5;
      gridPoints.push(...layerPoints);
      layerPoints.forEach(() => layerFactors.push(factor));
    });

    gridPointsRef.current = gridPoints;
    layerFactorRef.current = layerFactors;

    const obstructionMask = new Uint8Array(gridPoints.length);
    gridPoints.forEach((point, idx) => {
      const blocked = WALL_SEGMENTS.some((segment) => isPointInsideWall(segment, point));
      obstructionMask[idx] = blocked ? 1 : 0;
    });
    obstructionMaskRef.current = obstructionMask;

    // Create points geometry for wave field
    const geometry = new THREE.BufferGeometry();
    const positions = new Float32Array(gridPoints.length * 3);
    const colors = new Float32Array(gridPoints.length * 3);

    gridPoints.forEach((point, i) => {
      positions[i * 3] = point.x;
      positions[i * 3 + 1] = point.y;
      positions[i * 3 + 2] = point.z;
      const layerFactor = layerFactors[i] ?? 0.5;
      const brightness = 0.6 + layerFactor * 0.35;
      colors[i * 3] = brightness;
      colors[i * 3 + 1] = brightness;
      colors[i * 3 + 2] = brightness;
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
      if (structuralGroupRef.current) {
        structuralGroupRef.current.children.forEach((child: THREE.Object3D) => {
          if ((child as THREE.Mesh).isMesh) {
            const mesh = child as THREE.Mesh;
            mesh.geometry.dispose();
            const meshMaterial = mesh.material;
            if (Array.isArray(meshMaterial)) {
              meshMaterial.forEach((mat) => mat.dispose());
            } else {
              (meshMaterial as THREE.Material).dispose();
            }
          }
        });
        scene.remove(structuralGroupRef.current);
        structuralGroupRef.current = null;
      }
      if (wallBVHMeshRef.current) {
        const { geometry } = wallBVHMeshRef.current;
        (geometry as unknown as { disposeBoundsTree?: () => void }).disposeBoundsTree?.();
        geometry.dispose();
        wallBVHMeshRef.current = null;
        wallFaceLookupRef.current = null;
      }
      if (geometry) {
        geometry.dispose();
      }
      if (material) {
        material.dispose();
      }
    };
  }, []);

  useEffect(() => {
    if (!structuralGroupRef.current) return;

    structuralGroupRef.current.children.forEach((child: THREE.Object3D) => {
      if (!(child as THREE.Mesh).isMesh) return;
      const mesh = child as THREE.Mesh & { userData?: { wallMaterial?: WallMaterialKey } };
      const wallMaterialKey = mesh.userData?.wallMaterial as WallMaterialKey | undefined;
      if (!wallMaterialKey) return;

      const reflectivity = wallReflectivity[wallMaterialKey];
      const newOpacity = Math.min(0.98, 0.55 + reflectivity * 0.35);
      const colorScalar = 0.75 + reflectivity * 0.25;
      const baseColor = new THREE.Color(WALL_COLOR[wallMaterialKey]);
      const updatedColor = baseColor.multiplyScalar(colorScalar);

      const applyMaterialAdjustments = (standardMat: THREE.MeshStandardMaterial) => {
        standardMat.opacity = newOpacity;
        standardMat.color.copy(updatedColor);
        standardMat.metalness = 0.05 + reflectivity * 0.05;
        standardMat.roughness = 0.85;
        standardMat.needsUpdate = true;
      };

      const meshMaterial = mesh.material;
      if (Array.isArray(meshMaterial)) {
        meshMaterial.forEach((mat) => {
          if ((mat as THREE.MeshStandardMaterial).isMeshStandardMaterial) {
            applyMaterialAdjustments(mat as THREE.MeshStandardMaterial);
          }
        });
      } else if ((meshMaterial as THREE.MeshStandardMaterial).isMeshStandardMaterial) {
        applyMaterialAdjustments(meshMaterial as THREE.MeshStandardMaterial);
      }
    });
  }, [wallReflectivity]);

  // Update wave field based on sources and time
  useEffect(() => {
    if (!pointsRef.current || !gridPointsRef.current.length) return;

    const points = pointsRef.current;
    const gridPoints = gridPointsRef.current;
    const layerFactors = layerFactorRef.current;
    const obstructionMask = obstructionMaskRef.current;
    const wallBVHMesh = wallBVHMeshRef.current;
    const wallFaceLookup = wallFaceLookupRef.current;
    const posArray = points.geometry.attributes.position.array as Float32Array;
    const colArray = points.geometry.attributes.color.array as Float32Array;

    const reflectionDescriptors = createReflectionDescriptors(sources, wallReflectivity);
    const displacements = new Float32Array(gridPoints.length);
    let maxAbsDisplacement = 0;

    gridPoints.forEach((point, idx) => {
      if (obstructionMask[idx] === 1) {
        displacements[idx] = 0;
        return;
      }

      let totalDisplacement = 0;

      sources.forEach((source) => {
        totalDisplacement += calculateWaveDisplacementFromSource(point, source, time);
      });

      reflectionDescriptors.forEach(({ wall, baseSource, imageSource }) => {
        const axisIndex = wall.normalAxis === 'x' ? 0 : 2;
        const pointCoord = axisIndex === 0 ? point.x : point.z;
        const sourceCoord = baseSource.position[axisIndex];
        if (!isSameSide(sourceCoord, pointCoord, wall.plane)) return;
        if (!isReflectionPathValid(point, imageSource.position, wall, wallBVHMesh, wallFaceLookup)) return;
        totalDisplacement += calculateWaveDisplacementFromSource(point, imageSource, time);
      });

      displacements[idx] = totalDisplacement;
      const absDisp = Math.abs(totalDisplacement);
      if (absDisp > maxAbsDisplacement) {
        maxAbsDisplacement = absDisp;
      }
    });

    const normalization = maxAbsDisplacement > EPSILON ? maxAbsDisplacement : 1;

    gridPoints.forEach((point, idx) => {
      const isBlocked = obstructionMask[idx] === 1;
      if (isBlocked) {
        const layerFactor = layerFactors[idx] ?? 0.5;
        const wallShade = 0.2 + layerFactor * 0.15;
        posArray[idx * 3 + 1] = point.y;
        colArray[idx * 3] = wallShade;
        colArray[idx * 3 + 1] = wallShade;
        colArray[idx * 3 + 2] = wallShade;
        return;
      }

      const displacement = displacements[idx];
      posArray[idx * 3 + 1] = point.y + displacement * VERTICAL_SCALE;

      const normalizedDisp = displacement / normalization;
      const [r, g, b] = displacementToColor(normalizedDisp, visualizationMode);

      const layerFactor = layerFactors[idx] ?? 0.5;
      const brightness = 0.75 + layerFactor * 0.25;

      colArray[idx * 3] = r * brightness;
      colArray[idx * 3 + 1] = g * brightness;
      colArray[idx * 3 + 2] = b * brightness;
    });

    points.geometry.attributes.position.needsUpdate = true;
    points.geometry.attributes.color.needsUpdate = true;
  }, [sources, time, visualizationMode, wallReflectivity]);

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
  wallReflectivity: WallReflectivity;
  onWallReflectivityChange: (material: WallMaterialKey, value: number) => void;
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
  wallReflectivity,
  onWallReflectivityChange,
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

          <div className="rounded-xl border border-slate-700/50 bg-slate-800/50 p-4 shadow-lg backdrop-blur">
            <div className="mb-3 text-sm font-semibold text-white">Wall reflectivity</div>
            <p className="mb-4 text-xs text-slate-400">
              Control how much energy each surface reflects back into the layout. Higher values mimic hard drywall or glazing, lower values approximate absorptive treatments.
            </p>
            {(['interior', 'exterior'] as WallMaterialKey[]).map((materialKey) => (
              <div key={materialKey} className="mb-4 last:mb-0">
                <div className="mb-1 flex items-baseline justify-between">
                  <label className="text-xs font-medium text-slate-300 capitalize">{materialKey} partitions</label>
                  <span className="font-mono text-xs text-slate-400">{Math.round(wallReflectivity[materialKey] * 100)}%</span>
                </div>
                <input
                  type="range"
                  min={0}
                  max={0.9}
                  step={0.05}
                  value={wallReflectivity[materialKey]}
                  onChange={(event) => onWallReflectivityChange(materialKey, parseFloat(event.target.value))}
                  className="w-full accent-blue-500"
                />
              </div>
            ))}
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
  const [wallReflectivity, setWallReflectivity] = useState<WallReflectivity>(() => ({
    ...DEFAULT_WALL_REFLECTIVITY,
  }));
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

  const handleWallReflectivityChange = (material: WallMaterialKey, value: number) => {
    const clampedValue = Math.max(0, Math.min(0.95, value));
    setWallReflectivity((prev) => ({
      ...prev,
      [material]: clampedValue,
    }));
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
          wallReflectivity={wallReflectivity}
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
          wallReflectivity={wallReflectivity}
          onWallReflectivityChange={handleWallReflectivityChange}
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
