#pragma once

#include <cstdint>

/**
 * @brief Namespace containing constants used in the FLAC decoder.
 *
 * This namespace defines various constants used in the FLAC decoding process,
 * including prediction coefficients, markers, sync codes, block sizes, sample rates,
 * and bits per sample.
 */
namespace Flac_constants
{
    /**
     * @brief Fixed prediction coefficients for FLAC decoding.
     *
     * These coefficients are used in the fixed linear prediction coding (LPC)
     * method for FLAC audio compression.
     */
    static constexpr int16_t fixed_prediction_coefficients[5][4] = {
        {},
        {1},
        {2, -1},
        {3, -3, 1},
        {4, -6, 4, -1},
    };

    /**
     * @brief The FLAC marker used to identify FLAC files.
     *
     * This marker is the ASCII representation of "fLaC".
     */
    static constexpr uint32_t flac_marker = 0x664c6143;

    /**
     * @brief The frame sync code used in FLAC frames.
     *
     * This code is used to identify the beginning of a FLAC frame.
     */
    static constexpr uint16_t frame_sync_code = 0b11111111111110;

    /**
     * @brief Block sizes used in FLAC frames.
     *
     * These block sizes represent the number of samples in a FLAC frame.
     * Some values are placeholders for variable-length cases.
     */
    static constexpr uint16_t block_sizes[] = {
        0, 192, 576, 1152, 2304, 4608,
        0, 0, // Placeholders for variable-length cases
        256, 512, 1024, 2048, 4096, 8192, 16384, 32768};

    /**
     * @brief Sample rates used in FLAC frames.
     *
     * These sample rates represent the number of samples per second in a FLAC frame.
     * Some values are placeholders for variable-length cases or reserved values.
     */
    static constexpr uint32_t sample_rates[] = {
        0, // Reserved, use STREAMINFO sample rate
        88200, 176400, 192000, 8000, 16000, 22050,
        24000, 32000, 44100, 48000, 96000,
        0, // Placeholder for 8-bit sample rate
        0, // Placeholder for 16-bit sample rate (Hz)
        0, // Placeholder for 16-bit sample rate (10*Hz)
        0  // Invalid code
    };

    /**
     * @brief Bits per sample values used in FLAC frames.
     *
     * These values represent the number of bits used to encode each sample in a FLAC frame.
     * Some values are reserved or invalid.
     */
    static constexpr uint8_t bits_per_sample_table[] = {
        0, // Reserved, use STREAMINFO bits per sample
        8, 12,
        0, // Reserved value
        16, 20, 24, 32};
}
