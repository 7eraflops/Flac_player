#pragma once

#include <array>
#include <vector>

#include "Bit_reader.hpp"
#include "Decoders.hpp"

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

struct Frame_info
{
    uint8_t blocking_strategy{};
    uint16_t block_size{};
    uint32_t sample_rate{};
    uint8_t channel_assignment{};
    uint8_t sample_size{};
    uint64_t frame_or_sample_number{};
};
class Flac
{
private:
    const std::string m_filename;
    std::ifstream m_flac_stream;
    Bit_reader m_reader;
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
    void read_frame();
    void read_frame_header(Frame_info &frame_info);
    void read_subframe_header(Frame_info &frame_info);
    void decode_subframe_lpc(uint8_t order, Frame_info &frame_info);

    uint16_t decode_block_size(uint8_t block_size_code);
    uint32_t decode_sample_rate(uint8_t sample_rate_code);
    uint8_t decode_sample_size(uint8_t sample_size_code);
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
    m_reader.set_stream(m_flac_stream);
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
    enum class block_type : uint8_t
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

void Flac::read_frame()
{
    Frame_info frame_info{};
    read_frame_header(frame_info);
    read_subframe_header(frame_info);
}

void Flac::read_frame_header(Frame_info &frame_info)
{
    if (m_reader.read_bits(14) != 0b11111111111110)
    {
        throw std::runtime_error("Invalid sync code in frame header");
    }
    if (m_reader.read_bits(1))
    {
        throw std::runtime_error("1st reserved bit in frame isn't 0");
    }

    frame_info.blocking_strategy = m_reader.read_bits(1);
    uint8_t block_size_code = m_reader.read_bits(4);
    uint8_t sample_rate_code = m_reader.read_bits(4);
    frame_info.channel_assignment = m_reader.read_bits(4);
    uint8_t sample_size_code = m_reader.read_bits(3);

    if (m_reader.read_bits(1))
    {
        throw std::runtime_error("2nd reserved bit in frame isn't 0");
    }

    frame_info.frame_or_sample_number = decode_utf8(m_flac_stream);
    frame_info.block_size = decode_block_size(block_size_code);
    frame_info.sample_rate = decode_sample_rate(sample_rate_code);
    frame_info.sample_size = decode_sample_size(sample_size_code);
    uint8_t crc_8 = m_reader.read_bits(8);
    // TODO: Implement CRC checking
}

void Flac::read_subframe_header(Frame_info &frame_info)
{
    if (m_reader.read_bits(1) != 0)
    {
        throw std::runtime_error("The first bit of the subframe is non-zero");
    }

    uint8_t subframe_type_code = m_reader.read_bits(6);
    if ((subframe_type_code >= 2 && subframe_type_code <= 7) ||
        (subframe_type_code >= 16 && subframe_type_code <= 31))
    {
        throw std::runtime_error("subframe type has reserved value");
    }

    uint8_t wasted_bits_per_sample{};
    if (m_reader.read_bits(1))
    {
        wasted_bits_per_sample = decode_unary(m_reader);
    }

    uint8_t order{};

    if (subframe_type_code == 0b000000)
    {
        // TODO: Implement handling for CONSTANT
    }
    else if (subframe_type_code == 0b000001)
    {
        // TODO: Implement handling for VERBATIM
    }
    else if ((subframe_type_code & 0b111000) == 0b001000)
    {
        order = subframe_type_code & 0b000111;
        if (order > 4)
        {
            throw std::runtime_error("SUBFRAME_FIXED has invalid order");
        }
        // TODO: Implement handling for FIXED
    }
    else if ((subframe_type_code & 0b100000) == 0b100000)
    {
        order = (subframe_type_code & 0b011111) + 1;
        decode_subframe_lpc(order, frame_info);
    }
    else
    {
        throw std::runtime_error("Unknown subframe type");
    }
}

void Flac::decode_subframe_lpc(uint8_t order, Frame_info &frame_info)
{
    std::vector<int32_t> warm_up_samples(order);
    for (uint8_t i = 0; i < order; i++)
    {
        warm_up_samples[i] = m_reader.read_bits_signed(frame_info.sample_size);
    }

    uint8_t qlp_bit_precision = m_reader.read_bits(4);
    if (qlp_bit_precision == 0b1111)
    {
        throw std::runtime_error("Invalid QLP precission");
    }
    qlp_bit_precision++;

    int8_t qlp_shift = m_reader.read_bits_signed(5);

    std::vector<int16_t> predictor_coefficients(order);
    for (uint8_t i = 0; i < order; i++)
    {
        predictor_coefficients[i] = m_reader.read_bits_signed(qlp_bit_precision) >> qlp_shift;
    }
}

uint16_t Flac::decode_block_size(uint8_t block_size_code)
{
    uint16_t block_size{};
    switch (block_size_code)
    {
    case 0b0000:
        throw std::runtime_error("block size code has reserved value (0000)");
        break;
    case 0b0001:
        block_size = 192;
        break;
    case 0b0010:
        block_size = 576;
        break;
    case 0b0011:
        block_size = 1152;
        break;
    case 0b0100:
        block_size = 2304;
        break;
    case 0b0101:
        block_size = 4608;
        break;
    case 0b0110:
        block_size = m_reader.read_bits(8) + 1;
        break;
    case 0b0111:
        block_size = m_reader.read_bits(16) + 1;
        break;
    case 0b1000:
        block_size = 256;
        break;
    case 0b1001:
        block_size = 512;
        break;
    case 0b1010:
        block_size = 1024;
        break;
    case 0b1011:
        block_size = 2048;
        break;
    case 0b1100:
        block_size = 4096;
        break;
    case 0b1101:
        block_size = 8192;
        break;
    case 0b1110:
        block_size = 16384;
        break;
    case 0b1111:
        block_size = 32768;
        break;

    default:
        break;
    }
    return block_size;
}

uint32_t Flac::decode_sample_rate(uint8_t sample_rate_code)
{
    uint32_t sample_rate{};
    switch (sample_rate_code)
    {
    case 0b0000:
        sample_rate = m_stream_info.sample_rate;
        break;
    case 0b0001:
        sample_rate = 88200;
        break;
    case 0b0010:
        sample_rate = 176400;
        break;
    case 0b0011:
        sample_rate = 192000;
        break;
    case 0b0100:
        sample_rate = 8000;
        break;
    case 0b0101:
        sample_rate = 16000;
        break;
    case 0b0110:
        sample_rate = 22050;
        break;
    case 0b0111:
        sample_rate = 24000;
        break;
    case 0b1000:
        sample_rate = 32000;
        break;
    case 0b1001:
        sample_rate = 44100;
        break;
    case 0b1010:
        sample_rate = 48000;
        break;
    case 0b1011:
        sample_rate = 96000;
        break;
    case 0b1100:
        sample_rate = m_reader.read_bits(8) * 1000;
        break;
    case 0b1101:
        sample_rate = m_reader.read_bits(16);
        break;
    case 0b1110:
        sample_rate = m_reader.read_bits(16) * 10;
        break;
    case 0b1111:
        throw std::runtime_error("Invalid sample rate code");
        break;

    default:
        break;
    }
    return sample_rate;
}

uint8_t Flac::decode_sample_size(uint8_t sample_size_code)
{
    uint8_t sample_size{};
    switch (sample_size_code)
    {
    case 0b000:
        sample_size = m_stream_info.bits_per_sample;
        break;
    case 0b001:
        sample_size = 8;
        break;
    case 0b010:
        sample_size = 12;
        break;
    case 0b011:
        throw std::runtime_error("sample size code has reserved value");
        break;
    case 0b100:
        sample_size = 16;
        break;
    case 0b101:
        sample_size = 20;
        break;
    case 0b110:
        sample_size = 24;
        break;
    case 0b111:
        sample_size = 32;
        break;

    default:
        break;
    }
    return sample_size;
}