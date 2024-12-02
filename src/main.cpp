#include "Flac.hpp"
#include <iostream>
#include <stdexcept>

int main()
{
    try
    {
        Flac flac_file("../audio/sample3.flac");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}