#include "Flac.hpp"
#include <alsa/asoundlib.h>
#include <iostream>
#include <stdio.h>

std::vector<int32_t> convert_to_32bit(const std::vector<int64_t> &buffer)
{
    std::vector<int32_t> converted_buffer;
    converted_buffer.reserve(buffer.size());

    for (const auto &sample : buffer)
    {
        converted_buffer.push_back(static_cast<int32_t>(sample));
    }

    return converted_buffer;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <flac_file>\n";
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

    Flac player(flac_stream);
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    int error;

    try
    {
        player.initialize();
        int sample_rate = player.get_stream_info().sample_rate;
        int channels = player.get_stream_info().channels;
        int bit_depth = player.get_stream_info().bits_per_sample;

        const auto &comments = player.get_vorbis_comment().user_comments;
        auto it = comments.find("TITLE");
        if (it != comments.end())
        {
            std::cout << "Track Title: " << it->second << "\n";
        }
        else
        {
            std::cout << "Track Title not found.\n";
        }

        // Open PCM device for playback
        if ((error = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0)
        {
            std::cerr << "Cannot open audio device: " << snd_strerror(error) << "\n";
            return 1;
        }

        // Allocate hardware parameters object
        snd_pcm_hw_params_alloca(&params);

        // Fill params with default values
        if ((error = snd_pcm_hw_params_any(handle, params)) < 0)
        {
            std::cerr << "Cannot configure audio device: " << snd_strerror(error) << "\n";
            snd_pcm_close(handle);
            return 1;
        }

        // Set parameters
        if ((error = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
        {
            std::cerr << "Cannot set access type: " << snd_strerror(error) << "\n";
            snd_pcm_close(handle);
            return 1;
        }

        // Set sample format
        if ((error = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE)) < 0)
        {
            std::cerr << "Cannot set sample format: " << snd_strerror(error) << "\n";
            snd_pcm_close(handle);
            return 1;
        }

        // Set channel count
        if ((error = snd_pcm_hw_params_set_channels(handle, params, channels)) < 0)
        {
            std::cerr << "Cannot set channel count: " << snd_strerror(error) << "\n";
            snd_pcm_close(handle);
            return 1;
        }

        // Set sample rate
        unsigned int actual_rate = sample_rate;
        if ((error = snd_pcm_hw_params_set_rate_near(handle, params, &actual_rate, 0)) < 0)
        {
            std::cerr << "Cannot set sample rate: " << snd_strerror(error) << "\n";
            snd_pcm_close(handle);
            return 1;
        }

        // Set buffer size (similar to PulseAudio's tlength)
        snd_pcm_uframes_t buffer_size = sample_rate; // 1 second buffer
        if ((error = snd_pcm_hw_params_set_buffer_size_near(handle, params, &buffer_size)) < 0)
        {
            std::cerr << "Cannot set buffer size: " << snd_strerror(error) << "\n";
            snd_pcm_close(handle);
            return 1;
        }

        // Apply hardware parameters
        if ((error = snd_pcm_hw_params(handle, params)) < 0)
        {
            std::cerr << "Cannot set parameters: " << snd_strerror(error) << "\n";
            snd_pcm_close(handle);
            return 1;
        }

        // Main playback loop
        while (!player.get_reader().eos())
        {
            player.decode_frame();
            std::vector<int32_t> buffer = convert_to_32bit(player.get_audio_buffer());

            snd_pcm_sframes_t frames = snd_pcm_writei(handle, buffer.data(), buffer.size() / channels);

            if (frames < 0)
            {
                frames = snd_pcm_recover(handle, frames, 0);
                if (frames < 0)
                {
                    std::cerr << "Write failed: " << snd_strerror(frames) << "\n";
                    break;
                }
            }
        }

        // Drain the buffer and close
        snd_pcm_drain(handle);
        snd_pcm_close(handle);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}