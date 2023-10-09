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
    return E_NOTIMPL;
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
