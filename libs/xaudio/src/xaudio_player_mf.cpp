#include "xaudio_player_mf.h"

xaudio_player_mf::xaudio_player_mf()
    : _stream_index(0)
{
}

HRESULT xaudio_player_mf::initialize(IMFSourceReader* source_reader, std::uint32_t stream_index) noexcept
{
    if (source_reader == nullptr) {
        return E_POINTER;
    }

    return E_NOTIMPL;
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
        com_ptr<IMFSample> sample;
        if (SUCCEEDED(source_reader->ReadSample(stream_index, 0, nullptr, nullptr, nullptr, &sample))) {
            DWORD buffer_count;
            if (SUCCEEDED(sample->GetBufferCount(&buffer_count)) && buffer_count > 0) {
                com_ptr<IMFMediaBuffer> buffer;
                if (SUCCEEDED(sample->GetBufferByIndex(0, &buffer))) {
                    play_buffer_mf_locked locked;
                    if (SUCCEEDED(play_buffer_mf_locked::create(buffer.Get(), locked))) {
                        core.submit_audio_data(std::move(locked));
                    }
                }
            }
        }
    });
}

HRESULT play_buffer_mf_locked::create(IMFMediaBuffer* buffer, play_buffer_mf_locked& ret)
{
    if (buffer == nullptr) {
        return E_POINTER;
    }

    DWORD max_length;
    DWORD current_length;
    CHECK_HR(buffer->Lock(const_cast<std::uint8_t**>(&ret._locked_buffer), &max_length, &current_length));
    ret._locked_bytes = current_length;
    ret._buffer = buffer;
    return S_OK;
}

play_buffer_mf_locked::~play_buffer_mf_locked()
{
    if (_buffer != nullptr && _locked_buffer != nullptr && _locked_bytes > 0) {
        _buffer->Unlock();
    }
}

play_buffer_mf_locked::play_buffer_mf_locked(play_buffer_mf_locked&& other)
    : _buffer(std::move(other._buffer))
    , _locked_buffer(other._locked_buffer)
    , _locked_bytes(other._locked_bytes)
{
    other._locked_buffer = nullptr;
    other._locked_bytes = 0;
}

play_buffer_mf_locked& play_buffer_mf_locked::operator=(play_buffer_mf_locked&& other)
{
    _buffer = std::move(other._buffer);
    _locked_buffer = other._locked_buffer;
    _locked_bytes = other._locked_bytes;
    other._locked_buffer = nullptr;
    other._locked_bytes = 0;
    return *this;
}
