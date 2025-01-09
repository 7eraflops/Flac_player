#include "Wav.hpp"

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

    /*
    The `fmt ` subchunk can be:

    A 16-byte `fmt ` chunk is used for PCM (WAVE_FORMAT_PCM) audio,
    which includes basic info like the number of channels, sample rate,
    and bits per sample. This is the standard for uncompressed audio.

    If the audio format is non-PCM (e.g., ADPCM, IEEE float), the chunk
    size increases to 18 bytes, adding a `cbSize` field for extra format-specific data.

    For complex formats like multi-channel audio or specific speaker
    mappings, the chunk can grow to 40 bytes or more, especially with
    WAVE_FORMAT_EXTENSIBLE, which includes extra fields like a speaker mask
    and GUID. This implementation uses the 16-byte PCM for simplicity, as
    the encoded file will still be playable.

    This would also affect the 'data size' in the 'RIFF' chunk:
    +36 bytes for PCM (current implementation),
    +38 bytes for non-PCM
    +60 bytes for WAVE_FORMAT_EXTENSIBLE
    */
    write_string("fmt ");
    write_int32(16); // see fmt chunk comment
    write_int16(1);  // ~||~
    write_int16(stream_info.channels);
    write_int32(stream_info.sample_rate);
    auto frame_size = (stream_info.bits_per_sample) / 8 * stream_info.channels;
    auto byte_rate = stream_info.sample_rate * frame_size;
    write_int32(byte_rate);
    write_int16(frame_size);
    write_int16(stream_info.bits_per_sample);

    write_string("data");
    write_int32(data_size);
}

void Wav::write_samples(const std::vector<buffer_sample_type> &buffer)
{
    for (const auto &sample : buffer)
    {
        encode_sample(sample);
    }
}

void Wav::encode_sample(buffer_sample_type sample)
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
        write_int24(sample);
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