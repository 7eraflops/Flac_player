#include "Flac.hpp"
#include <iostream>
#include <pulse/error.h>
#include <pulse/simple.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    std::string filename = argv[1];

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

    Flac player(flac_stream);
    try
    {
        player.initialize();
        int sample_rate = player.get_stream_info().sample_rate;
        int channels = player.get_stream_info().channels;
        int bit_depth = player.get_stream_info().bits_per_sample;

        pa_sample_format_t format = PA_SAMPLE_S32NE;

        pa_sample_spec ss;
        ss.channels = channels;
        ss.rate = sample_rate;
        ss.format = format;

        pa_buffer_attr buffer_attr;
        buffer_attr.maxlength = (uint32_t)(-1);
        buffer_attr.fragsize = pa_usec_to_bytes(500000, &ss);
        buffer_attr.minreq = pa_usec_to_bytes(100000, &ss);
        buffer_attr.prebuf = pa_usec_to_bytes(0, &ss);
        buffer_attr.tlength = pa_usec_to_bytes(1000000, &ss);

        pa_simple *s = NULL;
        int error = 0;
        s = pa_simple_new(NULL, "FLAC Player", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, &buffer_attr, &error);
        if (!s)
        {
            fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(error));
            return 1;
        }

        while (!player.get_reader().eos())
        {
            player.decode_frame();
            std::vector<buffer_sample_type> buffer = player.get_audio_buffer();
            size_t buffer_size = buffer.size() * sizeof(buffer_sample_type);

            if (pa_simple_write(s, buffer.data(), buffer_size, &error) < 0)
            {
                fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(error));
                pa_simple_free(s);
                return 1;
            }
        }

        pa_simple_drain(s, &error);
        pa_simple_free(s);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}