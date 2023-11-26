#include "xaudio_player_mf.h"

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mfreadwrite.lib")

xaudio_player_mf::xaudio_player_mf() noexcept
    : _stream_index(0)
{
}

HRESULT xaudio_player_mf::initialize(const wchar_t* url, std::uint32_t stream_index)
{
    com_ptr<IMFSourceReader> source_reader;

    CHECK_HR(MFCreateSourceReaderFromURL(url, nullptr, &source_reader));

    return initialize(source_reader.Get(), stream_index);
}

HRESULT xaudio_player_mf::initialize(IMFSourceReader* source_reader, std::uint32_t stream_index) noexcept
{
    if (source_reader == nullptr) {
        return E_POINTER;
    }

    finalize();

    com_ptr<IMFMediaType> media_type;
    WAVEFORMATEXTENSIBLE wfex;
    WAVEFORMATEXTENSIBLE* wfex_temp;

    CHECK_HR(MFCreateMediaType(&media_type))
    CHECK_HR(media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio))
    CHECK_HR(media_type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM))
    CHECK_HR(source_reader->SetCurrentMediaType(stream_index, nullptr, media_type.Get()))
    media_type.Reset();
    CHECK_HR(source_reader->GetCurrentMediaType(stream_index, &media_type))
    CHECK_HR(MFCreateWaveFormatExFromMFMediaType(media_type.Get(), reinterpret_cast<WAVEFORMATEX**>(&wfex_temp), nullptr, MFWaveFormatExConvertFlag_ForceExtensible))
    wfex = *wfex_temp;
    ::CoTaskMemFree(wfex_temp);

    _source_reader = source_reader;
    _stream_index = stream_index;

    CHECK_HR(_core.initialize(reinterpret_cast<WAVEFORMATEX&>(wfex)));

    return S_OK;
}

void xaudio_player_mf::finalize() noexcept
{
    _core.finalize();
    _source_reader = nullptr;
}

HRESULT xaudio_player_mf::start() noexcept
{
    if (_source_reader == nullptr) {
        return E_FAIL;
    }

    return _core.start([source_reader = _source_reader, stream_index = _stream_index](auto& core, auto bytes_required) {
        if (bytes_required < 1) {
            return false;
        }
        com_ptr<IMFSample> sample;
        DWORD stream_flags;
        auto last_bytes = bytes_required;
        while (true) {
            if (SUCCEEDED(source_reader->ReadSample(stream_index, 0, nullptr, &stream_flags, nullptr, &sample))) {
                DWORD buffer_count;
                if (stream_flags & MF_SOURCE_READERF_ENDOFSTREAM) {
                    return true;
                }
                if (sample != nullptr && SUCCEEDED(sample->GetBufferCount(&buffer_count)) && buffer_count > 0) {
                    com_ptr<IMFMediaBuffer> buffer;
                    if (SUCCEEDED(sample->GetBufferByIndex(0, &buffer))) {
                        play_buffer_mf_locked locked;
                        if (SUCCEEDED(play_buffer_mf_locked::create(buffer.Get(), locked))) {
                            auto bytes = locked.get_audio_bytes();
                            core.submit_audio_data(std::move(locked));
                            if (bytes >= last_bytes) {
                                break;
                            }
                            last_bytes -= bytes;
                        }
                    }
                }
            }
        }
        return false;
    });
}

play_buffer_mf_locked::play_buffer_mf_locked()
    : _locked_buffer(nullptr)
    , _locked_bytes(0)
{
}

HRESULT play_buffer_mf_locked::create(IMFMediaBuffer* buffer, play_buffer_mf_locked& ret)
{
    if (buffer == nullptr) {
        return E_POINTER;
    }

    DWORD max_length;
    DWORD current_length;
    play_buffer_mf_locked temp = {};
    CHECK_HR(buffer->Lock(const_cast<std::uint8_t**>(&temp._locked_buffer), &max_length, &current_length));
    temp._locked_bytes = current_length;
    temp._buffer = buffer;
    std::swap(ret, temp);
    return S_OK;
}

play_buffer_mf_locked::play_buffer_mf_locked(play_buffer_mf_locked&& other) noexcept
    : _buffer(other._buffer)
    , _locked_buffer(other._locked_buffer)
    , _locked_bytes(other._locked_bytes)
{
    other.unlock();
}

play_buffer_mf_locked& play_buffer_mf_locked::operator=(play_buffer_mf_locked&& other) noexcept
{
    unlock();
    _buffer = other._buffer;
    _locked_buffer = other._locked_buffer;
    _locked_bytes = other._locked_bytes;
    other.unlock();
    return *this;
}

void play_buffer_mf_locked::unlock() {
    if (_buffer != nullptr && _locked_buffer != nullptr && _locked_bytes > 0) {
        _buffer->Unlock();
    }
    _locked_buffer = nullptr;
    _locked_bytes = 0;
}
