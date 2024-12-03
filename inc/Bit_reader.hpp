#pragma once

#include <cstdint>
#include <fstream>
#include <stdexcept>

class Bit_reader
{
private:
    std::istream *m_stream{};
    uint64_t m_bit_buffer{};
    uint8_t m_bits_in_buffer{};

public:
    explicit Bit_reader()
        : m_stream(nullptr) {}

    void setStream(std::istream &stream)
    {
        m_stream = &stream;
    }

    uint64_t read_bits(uint8_t num_bits)
    {
        if (m_stream == nullptr)
        {
            throw std::runtime_error("Stream not set in Bit_reader.");
        }

        if (num_bits > 64 || num_bits == 0)
        {
            throw std::invalid_argument("Number of bits to read must be between 1 and 64.");
        }

        while (m_bits_in_buffer < num_bits)
        {
            uint8_t byte = 0;
            if (!m_stream->read(reinterpret_cast<char *>(&byte), 1))
            {
                throw std::runtime_error("End of stream reached while reading bits.");
            }
            m_bit_buffer = (m_bit_buffer << 8) | byte;
            m_bits_in_buffer += 8;
        }

        uint64_t result = (m_bit_buffer >> (m_bits_in_buffer - num_bits)) & ((1ULL << num_bits) - 1);
        m_bits_in_buffer -= num_bits;

        return result;
    }
};
