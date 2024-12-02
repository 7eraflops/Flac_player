#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

class Flac
{
private:
    std::ifstream flac_stream{};
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
    Stream_info stream_info{};

public:
    explicit Flac(const std::string &filename);
    ~Flac();

    // Getter functions
    uint16_t get_min_block_size() const { return stream_info.min_block_size; }
    uint16_t get_max_block_size() const { return stream_info.max_block_size; }
    uint32_t get_min_frame_size() const { return stream_info.min_frame_size; }
    uint32_t get_max_frame_size() const { return stream_info.max_frame_size; }
    uint32_t get_sample_rate() const { return stream_info.sample_rate; }
    uint8_t get_channels() const { return stream_info.channels; }
    uint8_t get_bits_per_sample() const { return stream_info.bits_per_sample; }
    uint64_t get_total_samples() const { return stream_info.total_samples; }
    const std::array<uint8_t, 16> &get_md5_signature() const { return stream_info.md5_signature; }

    void check_flac_marker();
    void read_metadata_block_header();
    void read_metadata_block_STREAMINFO();
};

Flac::Flac(const std::string &filename)
{
    flac_stream.open(filename, std::ios::binary);
    if (!flac_stream.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }
}

Flac::~Flac()
{
    if (flac_stream.is_open())
    {
        flac_stream.close();
    }
}

void Flac::check_flac_marker()
{
    // The "fLaC" marker is expected in the first 4 bytes of the file.
    constexpr std::array<char, 4> flac_marker = {'f', 'L', 'a', 'C'};
    std::array<char, 4> marker{};
    flac_stream.read(marker.data(), marker.size());
    if (!flac_stream || !std::equal(marker.begin(), marker.end(), flac_marker.begin()))
    {
        throw std::runtime_error("File is not a valid FLAC file");
    }
}

void Flac::read_metadata_block_header()
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
        uint8_t block_header_info = flac_stream.get();
        is_last_block = (block_header_info & 0x80) ? true : false; // first header bit == 1 -> last metadata block
        block_type current_block_type = static_cast<block_type>(block_header_info & 0x7F);
        uint32_t block_length = 0;

        for (int i = 0; i < 3; ++i)
        {
            block_length = (block_length << 8) | flac_stream.get();
        }

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
            flac_stream.seekg(block_length, std::ios::cur);
            break;
        case block_type::APPLICATION:
            // TODO: implement function for APPLICATION block
            flac_stream.seekg(block_length, std::ios::cur);
            break;
        case block_type::SEEKTABLE:
            // TODO: implement function for SEEKTABLE block
            flac_stream.seekg(block_length, std::ios::cur);
            break;
        case block_type::VORBIS_COMMENT:
            // TODO: implement function for VORBIS_COMMENT block
            flac_stream.seekg(block_length, std::ios::cur);
            break;
        case block_type::CUESHEET:
            // TODO: implement function for CUESHEET block
            flac_stream.seekg(block_length, std::ios::cur);
            break;
        case block_type::PICTURE:
            // TODO: implement function for PICTURE block
            flac_stream.seekg(block_length, std::ios::cur);
            break;
        default:
            std::cerr << "Unknown block type!" << std::endl;
            break;
        }
    }
}

void Flac::read_metadata_block_STREAMINFO()
{
    constexpr uint32_t block_length = 34;
    std::array<uint8_t, block_length> block{};
    flac_stream.read(reinterpret_cast<char *>(block.data()), block_length);

    // Minimum and maximum block size (16 bits each)
    stream_info.min_block_size = (static_cast<uint8_t>(block[0]) << 8) | static_cast<uint8_t>(block[1]);
    stream_info.max_block_size = (static_cast<uint8_t>(block[2]) << 8) | static_cast<uint8_t>(block[3]);

    // Minimum and maximum frame size (24 bits each, might be zero if unknown)
    stream_info.min_frame_size = (static_cast<uint8_t>(block[4]) << 16) |
                                 (static_cast<uint8_t>(block[5]) << 8) |
                                 static_cast<uint8_t>(block[6]);
    stream_info.max_frame_size = (static_cast<uint8_t>(block[7]) << 16) |
                                 (static_cast<uint8_t>(block[8]) << 8) |
                                 static_cast<uint8_t>(block[9]);

    // Sample rate (20 bits)
    stream_info.sample_rate = (static_cast<uint8_t>(block[10]) << 12) |
                              (static_cast<uint8_t>(block[11]) << 4) |
                              ((static_cast<uint8_t>(block[12]) & 0xF0) >> 4);
    // Channels (3 bits)
    stream_info.channels = ((static_cast<uint8_t>(block[12]) & 0x0E) >> 1) + 1;

    // Bits per sample (5 bits)
    stream_info.bits_per_sample = ((static_cast<uint8_t>(block[12]) & 0x01) << 4) |
                                  ((static_cast<uint8_t>(block[13]) & 0xF0) >> 4) + 1;

    // Total samples (36 bits)
    stream_info.total_samples = ((static_cast<uint64_t>(block[13] & 0x0F) << 32) |
                                 (static_cast<uint8_t>(block[14]) << 24) |
                                 (static_cast<uint8_t>(block[15]) << 16) |
                                 (static_cast<uint8_t>(block[16]) << 8) |
                                 static_cast<uint8_t>(block[17]));

    // MD5 Signature (16 bytes)
    std::copy(block.begin() + 18, block.begin() + 34, stream_info.md5_signature.begin());
}