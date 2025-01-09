#pragma once
#include <fstream>
#include <stdexcept>
#include <vector>

#include "Flac_types.hpp"

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

    void encode_sample(buffer_sample_type sample);
    void write_string(const char *value);
    void write_int32(int32_t value);
    void write_int24(int32_t value);
    void write_int16(int16_t value);
    void write_int8(int8_t value);
};