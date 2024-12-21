#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Flac.hpp"

class Wav
{
public:
    Wav(std::ofstream &ostream, const Stream_info &streaminfo);
    ~Wav();

    void encode_header(const Stream_info &streaminfo);
    void write_samples(const std::vector<buffer_sample_type> &buffer);

private:
    std::ofstream &output_stream;
    const Stream_info &stream_info;

    void encode_sample(int32_t sample);
    void write_string(const char *value);
    void write_int32(int32_t value);
    void write_int24(int32_t value);
    void write_int16(int16_t value);
    void write_int8(int8_t value);
};

Wav::Wav(std::ofstream &ostream, const Stream_info &stream_info) : output_stream{ostream}, stream_info{stream_info}
{
    encode_header(stream_info);
}

Wav::~Wav()
{
    output_stream.flush();
    output_stream.close();
}

void Wav::encode_header(const Stream_info &stream_info)
{
    write_string("RIFF");
    auto data_size = stream_info.channels * stream_info.total_samples * (stream_info.bits_per_sample) / 8;
    write_int32(data_size + 36);
    write_string("WAVE");

    write_string("fmt ");
    write_int32(16); // Size of fmt chunk
    write_int16(1);  // PCM format
    write_int16(stream_info.channels);
    write_int32(stream_info.sample_rate);
    auto frame_size = (stream_info.bits_per_sample) / 8 * stream_info.channels;
    auto byte_rate = stream_info.sample_rate * frame_size;
    write_int32(byte_rate);
    write_int16(frame_size);
    write_int16(stream_info.bits_per_sample);

    write_string("data");
    write_int32(data_size); // Placeholder for data size
}

void Wav::write_samples(const std::vector<buffer_sample_type> &buffer)
{
    for (const auto &sample : buffer)
    {
        encode_sample(sample);
    }
}

void Wav::encode_sample(int32_t sample)
{
    switch (stream_info.bits_per_sample)
    {
    case 8:
        write_int8(sample);
        break;
    case 12:
        sample <<= 4;
        write_int16(sample);
        break;
    case 16:
        write_int16(sample);
        break;
    case 24:
        sample <<= 8;
        write_int32(sample);
        break;
    case 32:
        write_int32(sample);
        break;
    default:
        throw std::runtime_error("Unsupported bit depth");
    }
}

void Wav::write_string(const char *value)
{
    output_stream.write(value, 4);
}

void Wav::write_int32(int32_t value)
{
    output_stream.put((value >> 0) & 0xFF);
    output_stream.put((value >> 8) & 0xFF);
    output_stream.put((value >> 16) & 0xFF);
    output_stream.put((value >> 24) & 0xFF);
}

void Wav::write_int24(int32_t value)
{
    output_stream.put((value >> 0) & 0xFF);
    output_stream.put((value >> 8) & 0xFF);
    output_stream.put((value >> 16) & 0xFF);
}

void Wav::write_int16(int16_t value)
{
    output_stream.put((value >> 0) & 0xFF);
    output_stream.put((value >> 8) & 0xFF);
}

void Wav::write_int8(int8_t value)
{
    value += 128;
    output_stream.put((value >> 0) & 0xFF);
}

// #define DEBUG

int main(int argc, char *argv[])
{
#ifdef DEBUG
    std::string filename = "../audio/input/fixed.flac";
#else
    std::string filename = argv[1];
#endif

    std::ifstream flac_stream;
    try
    {
        flac_stream.open(filename, std::ios::binary);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    std::ofstream wav_stream("../audio/output/output.wav", std::ios::binary);

    Flac player(flac_stream);
    try
    {
        player.initialize();
        Wav encoder(wav_stream, player.get_stream_info());
        while (!player.get_reader().eos())
        {
            player.decode_frame();
            const std::vector<buffer_sample_type> buffer = player.get_audio_buffer();
            encoder.write_samples(buffer);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}