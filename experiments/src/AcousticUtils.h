#pragma once

#include <cmath>

/*
 * AcousticUtils - Utility functions for acoustic calculations
 *
 * Provides common acoustic conversions and calculations used
 * throughout the simulation.
 */
namespace AcousticUtils {

/*
 * Convert acoustic pressure to Sound Pressure Level (dB SPL)
 *
 * Sound Pressure Level is a logarithmic measure of sound intensity
 * relative to the threshold of human hearing (20 μPa).
 *
 * @param pressure Acoustic pressure in Pascals (Pa)
 * @return Sound Pressure Level in decibels (dB SPL)
 *
 * Formula: SPL = 20 * log10(p / p_ref)
 * where p_ref = 20 μPa = 20e-6 Pa (threshold of hearing)
 *
 * Examples:
 * - 0.02 Pa → 60 dB (normal conversation)
 * - 5.0 Pa → 94 dB (hand clap)
 * - 20.0 Pa → 100 dB (shout)
 */
inline float pressureToDbSpl(float pressure) {
    constexpr float REFERENCE_PRESSURE = 20e-6f;  // 20 μPa
    return 20.0f * std::log10f(pressure / REFERENCE_PRESSURE);
}

}  // namespace AcousticUtils
