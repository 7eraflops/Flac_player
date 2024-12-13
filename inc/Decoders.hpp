#pragma once

#include <fstream>

#include "Bit_reader.hpp"

// Function to decode a UTF-8 encoded number from a file stream (up to 5 bytes)
uint64_t decode_utf8(std::ifstream &file_stream)
{
    if (!file_stream.is_open())
    {
        throw std::runtime_error("File stream is not open");
    }

    unsigned char first_byte;
    file_stream.read(reinterpret_cast<char *>(&first_byte), 1);

    if (file_stream.eof())
    {
        throw std::runtime_error("Unexpected EOF while decoding UTF-8");
    }

    uint64_t code_point = 0;
    size_t additional_bytes = 0;

    if ((first_byte & 0x80) == 0)
    {
        code_point = first_byte;
    }
    else if ((first_byte & 0xE0) == 0xC0)
    {
        code_point = first_byte & 0x1F;
        additional_bytes = 1;
    }
    else if ((first_byte & 0xF0) == 0xE0)
    {
        code_point = first_byte & 0x0F;
        additional_bytes = 2;
    }
    else if ((first_byte & 0xF8) == 0xF0)
    {
        code_point = first_byte & 0x07;
        additional_bytes = 3;
    }
    else if ((first_byte & 0xFC) == 0xF8)
    {
        code_point = first_byte & 0x03;
        additional_bytes = 4;
    }
    else
    {
        throw std::runtime_error("Invalid UTF-8 encoding");
    }

    for (size_t i = 0; i < additional_bytes; i++)
    {
        unsigned char next_byte;
        file_stream.read(reinterpret_cast<char *>(&next_byte), 1);

        if (file_stream.eof())
        {
            throw std::runtime_error("Unexpected EOF in UTF-8 sequence");
        }

        if ((next_byte & 0xC0) != 0x80)
        {
            throw std::runtime_error("Invalid continuation byte in UTF-8");
        }

        code_point = (code_point << 6) | (next_byte & 0x3F);
    }

    return code_point;
}

// Function to decode numbers encoded in unary code
uint8_t decode_unary(Bit_reader reader)
{
    uint8_t wasted_bits = 0;

    while (true)
    {
        if (reader.read_bits(1) == 0)
        {
            wasted_bits++;
        }
        else
        {
            return ++wasted_bits;
        }
    }
}
