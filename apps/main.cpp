#include <common.h>
#include <xaudio_player_core.h>

struct audio_buffer {
    short buf[48000];
    xaudio_buffer buffer_data;

    audio_buffer() {
        buffer_data.audio_data = reinterpret_cast<std::uint8_t*>(buf);
        buffer_data.audio_bytes = sizeof(buf);
    }

    ~audio_buffer() {}

    const xaudio_buffer& get_xaudio_buffer() const {
        return buffer_data;
    }
};

int main()
{
    co_initializer coinit(COINIT_MULTITHREADED);

    xaudio_player_core<audio_buffer> player;

    WAVEFORMATEX wfex = {};
    wfex.wFormatTag = WAVE_FORMAT_PCM;
    wfex.nChannels = 1;
    wfex.wBitsPerSample = 16;
    wfex.nBlockAlign = wfex.nChannels * wfex.wBitsPerSample / 8;
    wfex.nSamplesPerSec = 48000;
    wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
    wfex.cbSize = 0;

   return 0;
}