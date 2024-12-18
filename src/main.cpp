#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Flac.hpp"

class WavFileWriter
{
public:
    enum class BitDepth
    {
        Bit16 = 16,
        Bit24 = 24,
        Bit32 = 32
    };

    WavFileWriter(const std::string &filename,
                  uint32_t sampleRate,
                  uint16_t numChannels,
                  BitDepth bitDepth)
        : m_filename(filename),
          m_sampleRate(sampleRate),
          m_numChannels(numChannels),
          m_bitDepth(bitDepth),
          m_bytesPerSample(static_cast<uint16_t>(bitDepth) / 8),
          m_totalSamplesWritten(0)
    {
        m_outputFile.open(filename, std::ios::binary);
        if (!m_outputFile)
        {
            throw std::runtime_error("Could not open file for writing: " + filename);
        }

        // Write placeholder header (will be updated when file is closed)
        writeHeaderPlaceholder();
    }

    void writeSamples(const int16_t *buffer, size_t numSamples)
    {
        // Assumes buffer is already interleaved
        switch (m_bitDepth)
        {
        case BitDepth::Bit16:
            writeInterleavedSamples<int16_t>(reinterpret_cast<const int16_t *>(buffer), numSamples);
            break;
        case BitDepth::Bit24:
            writeInterleavedSamples<int32_t>(reinterpret_cast<const int32_t *>(buffer), numSamples);
            break;
        case BitDepth::Bit32:
            writeInterleavedSamples<int32_t>(reinterpret_cast<const int32_t *>(buffer), numSamples);
            break;
        default:
            throw std::runtime_error("Unsupported bit depth");
        }
    }

    ~WavFileWriter()
    {
        if (m_outputFile.is_open())
        {
            finalizeWavFile();
        }
    }

private:
    template <typename SampleType>
    void writeInterleavedSamples(const SampleType *buffer, size_t numSamples)
    {
        m_outputFile.write(reinterpret_cast<const char *>(buffer),
                           numSamples * m_bytesPerSample);
        m_totalSamplesWritten += numSamples;
    }

    void writeHeaderPlaceholder()
    {
        // WAV header placeholders
        char header[44] = {0};

        // RIFF chunk descriptor
        memcpy(header, "RIFF", 4);     // ChunkID
        memcpy(header + 8, "WAVE", 4); // Format

        // Format subchunk
        memcpy(header + 12, "fmt ", 4); // Subchunk1ID
        header[16] = 16;                // Subchunk1Size for PCM
        header[20] = 1;                 // AudioFormat (PCM)

        // Write number of channels
        header[22] = m_numChannels & 0xFF;
        header[23] = (m_numChannels >> 8) & 0xFF;

        // Write sample rate
        for (int i = 0; i < 4; ++i)
        {
            header[24 + i] = (m_sampleRate >> (i * 8)) & 0xFF;
        }

        // Byte rate
        uint32_t byteRate = m_sampleRate * m_numChannels * m_bytesPerSample;
        for (int i = 0; i < 4; ++i)
        {
            header[28 + i] = (byteRate >> (i * 8)) & 0xFF;
        }

        // Block align
        uint16_t blockAlign = m_numChannels * m_bytesPerSample;
        header[32] = blockAlign & 0xFF;
        header[33] = (blockAlign >> 8) & 0xFF;

        // Bits per sample
        header[34] = static_cast<uint8_t>(m_bitDepth);

        // Data subchunk
        memcpy(header + 36, "data", 4);

        m_outputFile.write(header, 44);
    }

    void finalizeWavFile()
    {
        // Seek and update file size in header
        m_outputFile.seekp(4);
        uint32_t fileSize = 36 + m_totalSamplesWritten * m_bytesPerSample;
        writeLE32(fileSize);

        // Update data chunk size
        m_outputFile.seekp(40);
        uint32_t dataSize = m_totalSamplesWritten * m_bytesPerSample;
        writeLE32(dataSize);

        m_outputFile.close();
    }

    void writeLE32(uint32_t value)
    {
        char bytes[4];
        for (int i = 0; i < 4; ++i)
        {
            bytes[i] = (value >> (i * 8)) & 0xFF;
        }
        m_outputFile.write(bytes, 4);
    }

    std::ofstream m_outputFile;
    std::string m_filename;
    uint32_t m_sampleRate;
    uint16_t m_numChannels;
    BitDepth m_bitDepth;
    uint16_t m_bytesPerSample;
    size_t m_totalSamplesWritten;
};

std::vector<int16_t> convert_to_int16(const std::vector<int32_t> &int32_vec)
{
    std::vector<int16_t> int16_vec;
    int16_vec.reserve(int32_vec.size()); // Reserve space for efficiency

    for (const auto &value : int32_vec)
    {
        int16_vec.push_back(static_cast<int16_t>(value));
    }

    return int16_vec;
}

int main()
{
    std::string filename = "../audio/input/block_size_4096.flac";
    WavFileWriter wavWriter("../audio/output/output.wav", 44100, 1, WavFileWriter::BitDepth::Bit16);
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
        uint32_t frame_index{};
        while (!player.get_reader().eos()) // Replace with actual condition
        {
            auto start = std::chrono::high_resolution_clock::now();
            player.decode_frame();
            const std::vector<buffer_sample_type> &buffer = player.get_audio_buffer();
            size_t buffer_size = buffer.size();
            std::vector<int16_t> newbuf = convert_to_int16(buffer);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Function execution time: " << duration.count() << " milliseconds.\t" << "Frame: " << frame_index++ << std::endl;

            wavWriter.writeSamples(newbuf.data(), buffer_size);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}