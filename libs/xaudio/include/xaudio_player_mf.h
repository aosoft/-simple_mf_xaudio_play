#pragma once

#include "xaudio_player_core.h"
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>

class play_buffer_mf {
private:
    com_ptr<IMFMediaBuffer> _buffer;

public:
    play_buffer_mf() = default;
    play_buffer_mf(IMFMediaBuffer* buffer) : _buffer(buffer) {}
    play_buffer_mf(play_buffer_mf&& other) : _buffer(std::move(other._buffer)) {}
    play_buffer_mf& operator=(play_buffer_mf&& other)
    {
        _buffer = std::move(other._buffer);
        return *this;
    }

    const std::uint8_t* get_audio_data() const
    {
        return nullptr;
    }

    std::uint32_t get_audio_bytes() const
    {
        return 0;
    }
};

class xaudio_player_mf {
private:
    xaudio_player_core<play_buffer_mf> _core;
    com_ptr<IMFSourceReader> _source_reader;
    std::uint32_t _stream_index;

public:
    xaudio_player_mf();

    HRESULT initialize(IMFSourceReader* source_reader, std::uint32_t stream_index) noexcept;
    void finalize() noexcept;

    HRESULT start() noexcept;

    HRESULT stop() noexcept {
        return _core.stop();
    }

    [[nodiscard]] bool is_initialized() const noexcept {
        return _core.is_initialized();
    }

    [[nodiscard]] bool is_buffered() const noexcept {
        return _core.is_buffered();
    }
};