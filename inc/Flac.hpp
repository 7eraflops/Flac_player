#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>

class Flac
{
private:
    std::ifstream flac_stream;

public:
    explicit Flac(const std::string &filename);
    ~Flac();
    void check_flac_marker();
    void read_metadata_block_header();
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
    std::array<char, 4> marker;
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
            std::cout << "Processing STREAMINFO block." << std::endl;
            break;
        case block_type::PADDING:
            // TODO: implement function for PADDING block
            break;
        case block_type::APPLICATION:
            // TODO: implement function for APPLICATION block
            break;
        case block_type::SEEKTABLE:
            // TODO: implement function for SEEKTABLE block
            break;
        case block_type::VORBIS_COMMENT:
            // TODO: implement function for VORBIS_COMMENT block
            break;
        case block_type::CUESHEET:
            // TODO: implement function for CUESHEET block
            break;
        case block_type::PICTURE:
            // TODO: implement function for PICTURE block
            break;
        default:
            std::cerr << "Unknown block type!" << std::endl;
            break;
        }
    }
}
