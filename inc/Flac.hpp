#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "Bit_reader.hpp"
#include "Utf8_decoder.hpp"

class Flac
{
private:
    const std::string m_filename;
    std::ifstream m_flac_stream;
    Bit_reader m_reader;
    struct Stream_info
    {
        uint16_t min_block_size{};
        uint16_t max_block_size{};
        uint32_t min_frame_size{};
        uint32_t max_frame_size{};
        uint32_t sample_rate{};
        uint8_t channels{};
        uint8_t bits_per_sample{};
        uint64_t total_samples{};
        std::array<uint8_t, 16> md5_signature{};
    };
    Stream_info m_stream_info{};

public:
    explicit Flac(const std::string &m_filename) : m_filename(m_filename), m_flac_stream(), m_reader() {};
    ~Flac();

    // Getter functions
    uint16_t get_min_block_size() const { return m_stream_info.min_block_size; }
    uint16_t get_max_block_size() const { return m_stream_info.max_block_size; }
    uint32_t get_min_frame_size() const { return m_stream_info.min_frame_size; }
    uint32_t get_max_frame_size() const { return m_stream_info.max_frame_size; }
    uint32_t get_sample_rate() const { return m_stream_info.sample_rate; }
    uint8_t get_channels() const { return m_stream_info.channels; }
    uint8_t get_bits_per_sample() const { return m_stream_info.bits_per_sample; }
    uint64_t get_total_samples() const { return m_stream_info.total_samples; }
    const std::array<uint8_t, 16> &get_md5_signature() const { return m_stream_info.md5_signature; }

    void open_file();
    void check_flac_marker();
    void read_metadata();
    void read_metadata_block_STREAMINFO();
    void read_frame_header();
};

Flac::~Flac()
{
    if (m_flac_stream.is_open())
    {
        m_flac_stream.close();
    }
}

void Flac::open_file()
{
    m_flac_stream.open(m_filename, std::ios::binary);
    if (!m_flac_stream.is_open())
    {
        throw std::runtime_error("Could not open file: " + m_filename);
    }
    m_reader.setStream(m_flac_stream);
}

void Flac::check_flac_marker()
{
    // The "fLaC" marker is expected in the first 4 bytes of the file.
    constexpr std::string_view flac_marker = "fLaC";
    std::string marker(4, '\0');
    m_flac_stream.read(&marker[0], 4);
    if (m_flac_stream.gcount() != 4 || marker != flac_marker)
    {
        throw std::runtime_error("File is not a valid FLAC file");
    }
}

void Flac::read_metadata()
{
    bool is_last_block = false;
    enum class block_type : u_int8_t
    {
        STREAMINFO = 0,
        PADDING = 1,
        APPLICATION = 2,
        SEEKTABLE = 3,
        VORBIS_COMMENT = 4,
        CUESHEET = 5,
        PICTURE = 6
    };

    while (!is_last_block)
    {
        is_last_block = m_reader.read_bits(1);
        block_type current_block_type = static_cast<block_type>(m_reader.read_bits(7));
        uint32_t block_length = m_reader.read_bits(24);

        switch (current_block_type)
        {
        case block_type::STREAMINFO:
            if (block_length != 34)
            {
                throw std::runtime_error("STREAMINFO block has unexpected length");
            }
            read_metadata_block_STREAMINFO();
            break;
        case block_type::PADDING:
            // TODO: implement function for PADDING block
            m_flac_stream.seekg(block_length, std::ios::cur);
            break;
        case block_type::APPLICATION:
            // TODO: implement function for APPLICATION block
            m_flac_stream.seekg(block_length, std::ios::cur);
            break;
        case block_type::SEEKTABLE:
            // TODO: implement function for SEEKTABLE block
            m_flac_stream.seekg(block_length, std::ios::cur);
            break;
        case block_type::VORBIS_COMMENT:
            // TODO: implement function for VORBIS_COMMENT block
            m_flac_stream.seekg(block_length, std::ios::cur);
            break;
        case block_type::CUESHEET:
            // TODO: implement function for CUESHEET block
            m_flac_stream.seekg(block_length, std::ios::cur);
            break;
        case block_type::PICTURE:
            // TODO: implement function for PICTURE block
            m_flac_stream.seekg(block_length, std::ios::cur);
            break;
        default:
            throw std::runtime_error("Unknown block type");
            break;
        }
    }
}

void Flac::read_metadata_block_STREAMINFO()
{
    m_stream_info.min_block_size = m_reader.read_bits(16);
    m_stream_info.max_block_size = m_reader.read_bits(16);
    m_stream_info.min_frame_size = m_reader.read_bits(24);
    m_stream_info.max_frame_size = m_reader.read_bits(24);
    m_stream_info.sample_rate = m_reader.read_bits(20);
    m_stream_info.channels = m_reader.read_bits(3) + 1;
    m_stream_info.bits_per_sample = m_reader.read_bits(5) + 1;
    m_stream_info.total_samples = m_reader.read_bits(36);

    for (size_t i = 0; i < 16; i++)
    {
        m_stream_info.md5_signature[i] = m_reader.read_bits(8);
    }
}

void Flac::read_frame_header()
{
    if (m_reader.read_bits(14) != 0b11111111111110)
    {
        throw std::runtime_error("Invalid sync code in frame header");
    }
    if (m_reader.read_bits(1))
    {
        throw std::runtime_error("1st reserved bit in frame isn't 0");
    }

    uint8_t blocking_strategy = m_reader.read_bits(1);
    uint8_t block_size_code = m_reader.read_bits(4);
    if (block_size_code == 0b0000)
    {
        throw std::runtime_error("block size code has reserved value (0000)");
    }
    uint8_t sample_rate_code = m_reader.read_bits(4);
    if (sample_rate_code == 0b1111)
    {
        throw std::runtime_error("sample rate code has reserved value (1111)");
    }
    uint8_t channel_assignment_code = m_reader.read_bits(4);
    if (channel_assignment_code > 0b1010)
    {
        throw std::runtime_error("channel assignment code has reserved value (1011-1111)");
    }
    uint8_t sample_size_code = m_reader.read_bits(3);
    if (sample_size_code == 0b011)
    {
        throw std::runtime_error("sample size code has reserved value (011)");
    }
    uint32_t frame_number{};
    uint64_t sample_number{};

    if (m_reader.read_bits(1))
    {
        throw std::runtime_error("2nd reserved bit in frame isn't 0");
    }

    switch (blocking_strategy)
    {
    case 0:
        frame_number = static_cast<uint32_t>(decode_utf8(m_flac_stream));
        break;
    case 1:
        sample_number = decode_utf8(m_flac_stream);
        break;

    default:
        break;
    }

    // Print parsed values for debugging
    std::cout << "Blocking Strategy: " << blocking_strategy << '\n';
    std::cout << "Block Size: " << block_size_code << '\n';
    std::cout << "Sample Rate: " << sample_rate_code << '\n';
    // std::cout << "Channel Assignment: " << static_cast<int>(channel_assignment) << '\n';
    std::cout << "Sample Size: " << sample_size_code << '\n';
    if (blocking_strategy == 0)
    {
        std::cout << "Frame number: " << static_cast<int>(frame_number) << '\n';
    }
    else
    {
        std::cout << "Sample number: " << static_cast<int>(sample_number) << '\n';
    }
}