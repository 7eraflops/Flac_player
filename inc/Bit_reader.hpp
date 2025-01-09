#pragma once

#include <cstdint>
#include <stdexcept>

namespace mc
{
    /**
     * @brief A class for reading bits from an input stream.
     *
     * This class provides methods to read bits from an input stream, allowing
     * for both unsigned and signed bit reading. It maintains an internal buffer
     * to handle bit-level operations.
     *
     * @tparam Input_stream The type of the input stream (e.g., std::ifstream).
     */
    template <typename Input_stream>
    class Bit_reader
    {
    private:
        Input_stream *m_stream{};
        uint64_t m_bit_buffer{};
        uint8_t m_bits_in_buffer{};

    public:
        /**
         * @brief Constructs a Bit_reader with the given input stream.
         *
         * @param stream The input stream to read from.
         */
        explicit Bit_reader(Input_stream &stream)
            : m_stream(&stream), m_bit_buffer(0), m_bits_in_buffer(0) {}

        /**
         * @brief Checks if the end of the stream has been reached.
         *
         * @return True if the end of the stream has been reached, false otherwise.
         */
        bool eos() const
        {
            if (m_stream->peek() == EOF)
            {
                return true;
            }
            return false;
        }

        /**
         * @brief Reads a byte from the input stream.
         *
         * @return The byte read from the stream.
         * @throws std::runtime_error If the end of the stream is reached or reading fails.
         */
        uint8_t get_byte()
        {
            if (m_stream == nullptr || m_stream->eof())
            {
                throw std::runtime_error("End of stream reached.");
            }

            char byte;
            if (!m_stream->get(byte))
            {
                throw std::runtime_error("Failed to read byte from stream.");
            }
            return static_cast<uint8_t>(byte);
        }

        /**
         * @brief Reads an unsigned integer from the stream with the specified number of bits.
         *
         * @param num_bits The number of bits to read (must be between 0 and 64).
         * @return The unsigned integer read from the stream.
         * @throws std::invalid_argument If num_bits is not between 0 and 64.
         */
        uint64_t read_bits_unsigned(uint8_t num_bits)
        {
            if (num_bits > 64)
            {
                throw std::invalid_argument("Number of bits to read must be between 1 and 64.");
            }

            if (num_bits == 0)
            {
                return 0;
            }

            while (m_bits_in_buffer < num_bits)
            {
                uint8_t byte = get_byte();
                m_bit_buffer = (m_bit_buffer << 8) | byte;
                m_bits_in_buffer += 8;
            }

            uint64_t result = (m_bit_buffer >> (m_bits_in_buffer - num_bits)) & ((1ULL << num_bits) - 1);
            m_bits_in_buffer -= num_bits;

            return result;
        }

        /**
         * @brief Reads a signed integer from the stream with the specified number of bits.
         *
         * @param num_bits The number of bits to read (must be between 0 and 64).
         * @return The signed integer read from the stream.
         */
        int64_t read_bits_signed(uint8_t num_bits)
        {
            uint64_t result = read_bits_unsigned(num_bits);

            if (result & (1ULL << (num_bits - 1)))
            {
                return static_cast<int64_t>(result) - (1ULL << num_bits);
            }
            return static_cast<int64_t>(result);
        }

        /**
         * @brief Aligns the bit reader to the next byte boundary.
         *
         * This method discards any remaining bits in the buffer that do not
         * align to a full byte.
         */
        void align_to_byte()
        {
            m_bits_in_buffer -= m_bits_in_buffer % 8;
        }
    };
} // namespace mc
