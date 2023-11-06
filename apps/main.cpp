#include <common.h>
#include <xaudio_player_mf.h>

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

class mf_initializer {
private:
    HRESULT _hr;

public:
    mf_initializer()
        : _hr(::MFStartup(MF_VERSION, 0))
    {
    }
    ~mf_initializer()
    {
        if (SUCCEEDED(_hr)) {
            ::MFShutdown();
        }
    }
};

int wmain(int argc, const wchar_t** argv)
{
    if (argc < 2) {
        return 0;
    }

    co_initializer coinit(COINIT_MULTITHREADED);
    mf_initializer mfinit;

    xaudio_player_mf player;

    CHECK_HR(player.initialize(argv[1]))
    CHECK_HR(player.start());

    ::Sleep(10000);

    player.stop();

    return 0;
}