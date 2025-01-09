#pragma once

#include <cstdint>
#include <unordered_map>

/**
 * @brief Type alias for the sample type used in buffers.
 *
 * This type represents the sample values used in audio buffers.
 */
using buffer_sample_type = int64_t;

/**
 * @brief Structure to hold FLAC stream information.
 *
 * This structure contains metadata about the FLAC stream, including block sizes,
 * frame sizes, sample rate, number of channels, bits per sample, and total samples.
 */
struct Stream_info
{
    uint16_t min_block_size{};   ///< Minimum block size in the stream.
    uint16_t max_block_size{};   ///< Maximum block size in the stream.
    uint32_t min_frame_size{};   ///< Minimum frame size in the stream.
    uint32_t max_frame_size{};   ///< Maximum frame size in the stream.
    uint32_t sample_rate{};      ///< Sample rate of the stream.
    uint8_t channels{};          ///< Number of channels in the stream.
    uint8_t bits_per_sample{};   ///< Bits per sample in the stream.
    uint64_t total_samples{};    ///< Total number of samples in the stream.
};

/**
 * @brief Structure to hold FLAC frame information.
 *
 * This structure contains metadata about a FLAC frame, including blocking strategy,
 * block size, sample rate, channel assignment, bits per sample, frame or sample number,
 * and CRC values.
 */
struct Frame_info
{
    uint8_t blocking_strategy{};     ///< Blocking strategy used in the frame.
    uint16_t block_size{};           ///< Block size of the frame.
    uint32_t sample_rate{};          ///< Sample rate of the frame.
    uint8_t channel_assignment{};    ///< Channel assignment in the frame.
    uint8_t bits_per_sample{};       ///< Bits per sample in the frame.
    uint64_t frame_or_sample_number{}; ///< Frame or sample number.
    uint8_t crc_8{};                 ///< 8-bit CRC value for the frame header.
    uint16_t crc_16{};               ///< 16-bit CRC value for the frame.
};

/**
 * @brief Structure to hold Vorbis comments.
 *
 * This structure contains metadata in the form of Vorbis comments, including
 * a vendor string and user comments.
 */
struct Vorbis_comment
{
    std::string vendor_string; ///< Vendor string in the Vorbis comment.
    std::unordered_map<std::string, std::string> user_comments; ///< User comments in the Vorbis comment.
};

/**
 * @brief Enumeration of FLAC block types.
 *
 * This enumeration defines the various block types used in FLAC metadata blocks.
 */
enum class block_type : uint8_t
{
    STREAMINFO = 0,    ///< Stream information block.
    PADDING = 1,       ///< Padding block.
    APPLICATION = 2,   ///< Application-specific block.
    SEEKTABLE = 3,     ///< Seek table block.
    VORBIS_COMMENT = 4, ///< Vorbis comment block.
    CUESHEET = 5,      ///< Cuesheet block.
    PICTURE = 6        ///< Picture block.
};
