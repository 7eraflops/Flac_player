#pragma once

#include <array>
#include <fstream>
#include <stdexcept>
#include <string>

class Flac
{
private:
    std::ifstream flac_file;

public:
    explicit Flac(const std::string &filename);
    ~Flac();
    void check_flac_marker();
};

Flac::Flac(const std::string &filename)
{
    flac_file.open(filename, std::ios::binary);
    if (!flac_file.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }
}

Flac::~Flac()
{
    if (flac_file.is_open())
    {
        flac_file.close();
    }
}

void Flac::check_flac_marker()
{
    // The "fLaC" marker is expected in the first 4 bytes of the file.
    constexpr std::array<char, 4> flac_marker = {'f', 'L', 'a', 'C'};
    std::array<char, 4> marker;
    flac_file.read(marker.data(), marker.size());
    if (!flac_file || !std::equal(marker.begin(), marker.end(), flac_marker.begin()))
    {
        throw std::runtime_error("File is not a valid FLAC file");
    }
}
