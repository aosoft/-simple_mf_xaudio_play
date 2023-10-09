#pragma once

#include "xaudio_player_core.h"
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>

class play_buffer_mf_locked {
private:
    com_ptr<IMFMediaBuffer> _buffer;
    const std::uint8_t* _locked_buffer;
    std::uint32_t _locked_bytes;

public:
    play_buffer_mf_locked() = default;
    static HRESULT create(IMFMediaBuffer* buffer, play_buffer_mf_locked& ret);

    ~play_buffer_mf_locked();
    play_buffer_mf_locked(play_buffer_mf_locked&& other);
    play_buffer_mf_locked& operator=(play_buffer_mf_locked&& other);

    const std::uint8_t* get_audio_data() const
    {
        return _locked_buffer;
    }

    std::uint32_t get_audio_bytes() const
    {
        return _locked_bytes;
    }
};

class xaudio_player_mf {
private:
    xaudio_player_core<play_buffer_mf_locked> _core;
    com_ptr<IMFSourceReader> _source_reader;
    std::uint32_t _stream_index;

public:
    xaudio_player_mf();

    HRESULT initialize(IMFSourceReader* source_reader, std::uint32_t stream_index) noexcept;
    void finalize() noexcept;

    HRESULT start() noexcept;

    HRESULT stop() noexcept
    {
        return _core.stop();
    }

    [[nodiscard]] bool is_initialized() const noexcept
    {
        return _core.is_initialized();
    }

    [[nodiscard]] bool is_buffered() const noexcept
    {
        return _core.is_buffered();
    }
};