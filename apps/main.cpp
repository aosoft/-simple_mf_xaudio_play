#include <common.h>
#include <xaudio_player_core.h>

struct audio_buffer {
    std::vector<short> buf;
    xaudio_buffer buffer_data;

    audio_buffer(std::int64_t offset)
    {
        buf.resize(48000);

        buffer_data.audio_data = reinterpret_cast<std::uint8_t*>(&buf[0]);
        buffer_data.audio_bytes = buf.size() * 2;

        for (size_t i = 0; i < buf.size(); i++) {
            buf[i] = static_cast<short>(std::sin((2.0 * 3.14159 * 440) * (i + offset) / 48000) * 32767);
        }
    }

    audio_buffer& operator=(audio_buffer&& src)
    {
        buffer_data = src.buffer_data;
        src.buffer_data = {};
        buf = std::move(src.buf);
        return *this;
    }

    audio_buffer(audio_buffer&& src)
    {
        buffer_data = src.buffer_data;
        src.buffer_data = {};
        buf = std::move(src.buf);
    }

    ~audio_buffer() { }

    const xaudio_buffer& get_xaudio_buffer() const
    {
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

    CHECK_HR(player.initialize(wfex))

    auto buf = audio_buffer(0);
    CHECK_HR(player.submit_audio_data(std::move(buf)));

    std::int64_t offset = buf.get_xaudio_buffer().audio_bytes / 2;
    CHECK_HR(player.start([&offset](auto& self, auto x) {
        if (offset < 3 * 48000) {
            auto buf = audio_buffer(offset);
            offset += buf.get_xaudio_buffer().audio_bytes / 2;
            CHECK_HR(self.submit_audio_data(std::move(buf)));
        }
    }))

    while (player.is_buffered()) {
        ::Sleep(100);
    }

    player.stop();

    return 0;
}