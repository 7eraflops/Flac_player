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
