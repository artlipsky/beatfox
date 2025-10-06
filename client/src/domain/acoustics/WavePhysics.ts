/**
 * Domain Model: Acoustic Wave Physics
 *
 * Pure functions and classes for acoustic wave simulation
 * Based on the acoustic wave equation: ∂²p/∂t² = c²∇²p
 */

/**
 * Physical constants for sound propagation in air at 20°C
 */
export interface AcousticConstantsShape {
  readonly SPEED_OF_SOUND: number;
  readonly AIR_DENSITY: number;
  readonly REFERENCE_PRESSURE: number;
}

export const AcousticConstants: AcousticConstantsShape = {
  SPEED_OF_SOUND: 343, // m/s
  AIR_DENSITY: 1.204, // kg/m³
  REFERENCE_PRESSURE: 20e-6, // Pa (threshold of hearing)
};

/**
 * Wave source in 3D space
 */
export class WaveSource {
  public readonly position: readonly [number, number, number];
  public readonly frequency: number; // Hz
  public readonly amplitude: number;
  public readonly phase: number; // radians

  constructor(
    position: readonly [number, number, number],
    frequency: number,
    amplitude: number,
    phase: number
  ) {
    this.position = position;
    this.frequency = frequency;
    this.amplitude = amplitude;
    this.phase = phase;
  }

  /**
   * Calculate wavelength based on frequency
   */
  get wavelength(): number {
    return AcousticConstants.SPEED_OF_SOUND / this.frequency;
  }

  /**
   * Calculate wave number (k = 2π/λ)
   */
  get waveNumber(): number {
    return (2 * Math.PI) / this.wavelength;
  }

  /**
   * Calculate angular frequency (ω = 2πf)
   */
  get angularFrequency(): number {
    return 2 * Math.PI * this.frequency;
  }

  /**
   * Update a property and return new instance (immutable)
   */
  update(key: 'frequency' | 'amplitude' | 'phase', value: number): WaveSource {
    if (key === 'frequency') {
      return new WaveSource(this.position, value, this.amplitude, this.phase);
    }

    if (key === 'amplitude') {
      return new WaveSource(this.position, this.frequency, value, this.phase);
    }

    if (key === 'phase') {
      return new WaveSource(this.position, this.frequency, this.amplitude, value);
    }

    return this;
  }
}

/**
 * Point in 3D space
 */
export interface Point3D {
  readonly x: number;
  readonly y: number;
  readonly z: number;
}

/**
 * Wave displacement calculation result
 */
export interface WaveDisplacement {
  readonly displacement: number;
  readonly pressure: number;
}

/**
 * Calculate distance between two points in 3D space
 */
export function calculateDistance(
  p1: Point3D,
  p2: readonly [number, number, number]
): number {
  const dx = p1.x - p2[0];
  const dy = p1.y - p2[1];
  const dz = p1.z - p2[2];
  return Math.sqrt(dx * dx + dy * dy + dz * dz);
}

/**
 * Calculate wave displacement at a point from a single source
 *
 * Uses the solution to the wave equation for a point source:
 * u(r,t) = (A/r) * sin(kr - ωt + φ)
 *
 * Where:
 * - A = amplitude
 * - r = distance from source
 * - k = wave number (2π/λ)
 * - ω = angular frequency (2πf)
 * - φ = phase offset
 */
export function calculateWaveDisplacementFromSource(
  point: Point3D,
  source: WaveSource,
  time: number,
  minDistance: number = 0.1 // Avoid singularity at source
): number {
  const distance = calculateDistance(point, source.position);

  // Avoid singularity at the source position
  if (distance < minDistance) {
    return 0;
  }

  // Spherical spreading: amplitude decreases as 1/r
  const amplitude = source.amplitude / (distance + 1);

  // Phase calculation: spatial component (kr) - temporal component (ωt) + phase offset
  const spatialPhase = source.waveNumber * distance;
  const temporalPhase = source.angularFrequency * time;
  const totalPhase = spatialPhase - temporalPhase + source.phase;

  // Wave equation solution
  return amplitude * Math.sin(totalPhase);
}

/**
 * Calculate total wave displacement from multiple sources using superposition principle
 *
 * The principle of superposition states that when two or more waves overlap,
 * the resultant displacement is the sum of the individual displacements.
 */
export function calculateTotalWaveDisplacement(
  point: Point3D,
  sources: readonly WaveSource[],
  time: number
): WaveDisplacement {
  let totalDisplacement = 0;

  for (const source of sources) {
    totalDisplacement += calculateWaveDisplacementFromSource(point, source, time);
  }

  // Acoustic pressure is proportional to particle displacement
  const pressure = totalDisplacement * AcousticConstants.AIR_DENSITY * AcousticConstants.SPEED_OF_SOUND;

  return {
    displacement: totalDisplacement,
    pressure,
  };
}

/**
 * Generate a 2D grid of points in 3D space (XZ plane)
 */
export function generateWaveFieldGrid(
  gridSize: number,
  resolution: number,
  yPosition: number = 0
): Point3D[] {
  const points: Point3D[] = [];
  const step = gridSize / (resolution - 1);
  const halfGrid = gridSize / 2;

  for (let i = 0; i < resolution; i++) {
    for (let j = 0; j < resolution; j++) {
      points.push({
        x: i * step - halfGrid,
        y: yPosition,
        z: j * step - halfGrid,
      });
    }
  }

  return points;
}

/**
 * Calculate wave field for an entire grid
 */
export interface WaveFieldResult {
  readonly displacements: Float32Array;
  readonly maxDisplacement: number;
  readonly minDisplacement: number;
}

export function calculateWaveField(
  points: readonly Point3D[],
  sources: readonly WaveSource[],
  time: number
): WaveFieldResult {
  const displacements = new Float32Array(points.length);
  let maxDisplacement = 0;
  let minDisplacement = 0;

  for (let i = 0; i < points.length; i++) {
    const result = calculateTotalWaveDisplacement(points[i], sources, time);
    displacements[i] = result.displacement;

    if (result.displacement > maxDisplacement) {
      maxDisplacement = result.displacement;
    }
    if (result.displacement < minDisplacement) {
      minDisplacement = result.displacement;
    }
  }

  return {
    displacements,
    maxDisplacement,
    minDisplacement,
  };
}

/**
 * Color mapping strategies for visualization
 */
export type ColorMode = 'displacement' | 'pressure';

/**
 * Convert normalized displacement to RGB color
 */
export function displacementToColor(
  normalizedDisplacement: number,
  mode: ColorMode
): readonly [number, number, number] {
  if (mode === 'displacement') {
    // Displacement mode: blue (negative) → white (zero) → red (positive)
    if (normalizedDisplacement > 0) {
      return [1, 1 - normalizedDisplacement, 1 - normalizedDisplacement];
    } else {
      return [1 + normalizedDisplacement, 1 + normalizedDisplacement, 1];
    }
  } else {
    // Pressure mode: intensity based on absolute value
    const pressure = Math.abs(normalizedDisplacement);
    return [0.2 + pressure * 0.8, 0.4 + pressure * 0.4, 0.8];
  }
}
