#include "Flac.hpp"
#include <iostream>
#include <stdexcept>

int main()
{
    Flac flac_file("../audio/sample3.flac");
    try
    {
        flac_file.open_file();
        flac_file.check_flac_marker();
        flac_file.read_metadata();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    std::cout << "Min Block Size: " << flac_file.get_min_block_size() << '\n';
    std::cout << "Max Block Size: " << flac_file.get_max_block_size() << '\n';
    std::cout << "Min Frame Size: " << flac_file.get_min_frame_size() << '\n';
    std::cout << "Max Frame Size: " << flac_file.get_max_frame_size() << '\n';
    std::cout << "Sample Rate: " << flac_file.get_sample_rate() << " Hz\n";
    std::cout << "Channels: " << static_cast<int>(flac_file.get_channels()) << '\n';
    std::cout << "Bits per Sample: " << static_cast<int>(flac_file.get_bits_per_sample()) << '\n';
    std::cout << "Total Samples: " << flac_file.get_total_samples() << '\n';
    std::cout << "MD5 Signature: ";
    for (uint8_t byte : flac_file.get_md5_signature())
    {
        std::cout << std::hex << static_cast<int>(byte);
    }
    std::cout << std::dec << '\n';

    return 0;
}