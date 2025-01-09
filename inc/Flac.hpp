#pragma once

#include <fstream>
#include <unordered_map>
#include <vector>

#include "Bit_reader.hpp"
#include "Flac_constants.hpp"
#include "Flac_types.hpp"
#include "decoders.hpp"
namespace mc
{
    /**
     * @brief A class for decoding FLAC audio files.
     *
     * This class provides methods to decode FLAC audio files, including reading
     * metadata, decoding frames, and accessing decoded audio data.
     */
    class Flac
    {
    private:
        uint8_t m_channel_index{};
        uint64_t m_sample_count{};
        uint64_t m_frame_count{};
        Stream_info m_stream_info{};
        Frame_info m_frame_info{};
        Vorbis_comment m_vorbis_comment;
        std::ifstream &m_flac_stream;
        Bit_reader<std::ifstream> m_reader;
        std::vector<buffer_sample_type> m_audio_buffer;

        // internal functions
        // decoding values from bit codes
        uint16_t decode_block_size(uint8_t block_size_code);
        uint32_t decode_sample_rate(uint8_t sample_rate_code);
        uint8_t decode_sample_size(uint8_t sample_size_code);
        // stream decoding functions that have to be used in a specific order and shouldn't be accessible to user
        void check_flac_marker();
        void read_metadata();
        void read_metadata_block_STREAMINFO();
        void read_metadata_block_PADDING();
        void read_metadata_block_APPLICATION();
        void read_metadata_block_SEEKTABLE();
        void read_metadata_block_VORBIS_COMMENT();
        void read_metadata_block_CUESHEET();
        void read_metadata_block_PICTURE();
        void decode_subframe(uint8_t bits_per_sample);
        void decode_subframe_fixed(uint8_t predictor_order, uint8_t bits_per_sample);
        void decode_subframe_lpc(uint8_t predictor_order, uint8_t bits_per_sample);
        void linear_prediction(uint8_t predictor_order, const int16_t *predictor_coefficients, int8_t qlp_shift);
        void decode_residuals(uint8_t predictor_order);

    public:
        /**
         * @brief Constructs a Flac decoder with the given input stream.
         *
         * @param flac_stream The input stream to read the FLAC file from.
         */
        explicit Flac(std::ifstream &flac_stream) : m_flac_stream(flac_stream), m_reader(m_flac_stream) {};

        /**
         * @brief Destructor for the Flac class.
         */
        ~Flac();

        /**
         * @brief Gets the stream information of the FLAC file.
         *
         * @return A reference to the Stream_info structure containing the stream information.
         */
        const Stream_info &get_stream_info() { return m_stream_info; }

        /**
         * @brief Gets the frame information of the current FLAC frame.
         *
         * @return A reference to the Frame_info structure containing the frame information.
         */
        const Frame_info &get_frame_info() { return m_frame_info; }

        /**
         * @brief Gets the Vorbis comments of the FLAC file.
         *
         * @return A reference to the Vorbis_comment structure containing the Vorbis comments.
         */
        const Vorbis_comment &get_vorbis_comment() { return m_vorbis_comment; }

        /**
         * @brief Gets the bit reader used for reading the FLAC file.
         *
         * @return A reference to the Bit_reader object used for reading the FLAC file.
         */
        const Bit_reader<std::ifstream> &get_reader() const { return m_reader; }

        /**
         * @brief Gets the audio buffer containing the decoded audio samples.
         *
         * @return A reference to the vector containing the decoded audio samples.
         */
        const std::vector<buffer_sample_type> &get_audio_buffer() const { return m_audio_buffer; }

        /**
         * @brief Initializes the FLAC decoder.
         *
         * This function initializes the FLAC decoder by reading the FLAC marker and metadata.
         */
        void initialize();

        /**
         * @brief Decodes a frame from the FLAC file.
         *
         * This function decodes a single frame from the FLAC file and stores the decoded
         * audio samples in the audio buffer.
         */
        void decode_frame();
    };
} // namespace mc
