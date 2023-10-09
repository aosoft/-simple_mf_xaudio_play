#include <common.h>
#include <xaudio_player_core.h>

struct audio_buffer {
    std::vector<short> buf;

    audio_buffer(std::int64_t offset)
    {
        buf.resize(48000);

        for (size_t i = 0; i < buf.size(); i++) {
            buf[i] = static_cast<short>(std::sin((2.0 * 3.14159 * 440) * (i + offset) / 48000) * 32767);
        }
    }

    audio_buffer& operator=(audio_buffer&& src)
    {
        buf = std::move(src.buf);
        return *this;
    }

    audio_buffer(audio_buffer&& src)
    {
        buf = std::move(src.buf);
    }

    ~audio_buffer() { }

    const std::uint8_t* get_audio_data() const
    {
        return reinterpret_cast<const std::uint8_t*>(&buf[0]);
    }

    std::uint32_t get_audio_bytes() const
    {
        return buf.size() * sizeof(short);
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

    CHECK_HR(player.initialize(wfex))

    auto buf = audio_buffer(0);
    std::int64_t offset = buf.get_audio_bytes() / 2;
    CHECK_HR(player.submit_audio_data(std::move(buf)));
    CHECK_HR(player.start([&offset](auto& self, auto x) {
        if (offset < 3 * 48000) {
            auto buf = audio_buffer(offset);
            offset += buf.get_audio_bytes() / 2;
            self.submit_audio_data(std::move(buf));
        }
    }))

    while (player.is_buffered()) {
        ::Sleep(100);
    }

    player.stop();

    return 0;
}