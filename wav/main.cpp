#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Flac.hpp"
#include "Wav.hpp"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <flac file>" << '\n';
        return 1;
    }

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