#pragma once

#include "common.h"
#include <functional>
#include <mutex>
#include <vector>
#include <xaudio2.h>

template <typename T>
concept play_buffer = requires(T& x) {
    {
        x.get_audio_data()
    } -> std::convertible_to<const std::uint8_t*>;
    {
        x.get_audio_bytes()
    } -> std::convertible_to<std::uint32_t>;
};

template <play_buffer T>
class xaudio_player_core : private IXAudio2VoiceCallback {
private:
    mutable std::mutex _mutex;
    com_ptr<IXAudio2> _xaudio;
    IXAudio2MasteringVoice* _mastering_voice;
    IXAudio2SourceVoice* _source_voice;
    std::vector<T> _buffers;
    bool _eos;
    std::function<bool(xaudio_player_core<T>&, std::uint32_t)> _on_voice_processing_pass_start;
    bool _playing;

public:
    xaudio_player_core() noexcept
        : _mastering_voice(nullptr)
        , _source_voice(nullptr)
        , _eos(false)
        , _playing(false)
    {
    }

    ~xaudio_player_core() noexcept
    {
        finalize();
    }

    HRESULT initialize(const WAVEFORMATEX& wfx) noexcept
    {
        finalize();
        CHECK_HR(XAudio2Create(&_xaudio, 0, XAUDIO2_DEFAULT_PROCESSOR))
        CHECK_HR(_xaudio->CreateMasteringVoice(&_mastering_voice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, nullptr, nullptr))
        CHECK_HR(_xaudio->CreateSourceVoice(&_source_voice, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this, nullptr, nullptr))
        CHECK_HR(set_volume(1.0f))

        return S_OK;
    }

    void finalize() noexcept
    {
        _mastering_voice = nullptr;
        _source_voice = nullptr;
        _xaudio = nullptr;
        _buffers.clear();
    }

    [[nodiscard]] bool is_initialized() const noexcept
    {
        return _xaudio != nullptr && _mastering_voice != nullptr && _source_voice != nullptr;
    }

    HRESULT start(std::function<bool(xaudio_player_core<T>&, std::uint32_t)> on_voice_processing_pass_start) noexcept
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!is_initialized()) {
            return E_FAIL;
        }
        _on_voice_processing_pass_start = on_voice_processing_pass_start;
        CHECK_HR(_source_voice->Start())
        _playing = true;
        _eos = false;
        return S_OK;
    }

    HRESULT stop() noexcept
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!is_initialized()) {
            return S_FALSE;
        }
        CHECK_HR(_source_voice->Stop())
        _playing = false;
        return S_OK;
    }

    bool is_playing() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _playing;
    }

    [[nodiscard]] bool is_buffered() const noexcept
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _buffers.size() > 0;
    }

    [[nodiscard]] XAUDIO2_VOICE_STATE get_voice_state() const noexcept
    {
        if (is_initialized()) {
            XAUDIO2_VOICE_STATE ret;
            _source_voice->GetState(&ret);
            return ret;
        }
        return {};
    }

    [[nodiscard]] float get_volume() const noexcept
    {
        if (is_initialized()) {
            float volume;
            _source_voice->GetVolume(&volume);
            return volume;
        }
        return 0.0f;
    }

    HRESULT set_volume(float volume) noexcept
    {
        if (is_initialized()) {
            return _source_voice->SetVolume(volume);
        }
        return S_FALSE;
    }

    HRESULT submit_audio_data(T&& audio_data) noexcept
    try {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!is_initialized()) {
            return S_FALSE;
        }

        _buffers.push_back(std::move(audio_data));
        const auto it = _buffers.rbegin();

        XAUDIO2_BUFFER buffer = {};
        buffer.pAudioData = it->get_audio_data();
        buffer.AudioBytes = it->get_audio_bytes();
        buffer.pContext = const_cast<std::uint8_t*>(buffer.pAudioData);

        HRESULT hr = _source_voice->SubmitSourceBuffer(&buffer);
        if (FAILED(hr)) {
            _buffers.pop_back();
            return hr;
        }
        return S_OK;
    } catch (const std::bad_alloc&) {
        return E_OUTOFMEMORY;
    }

private:
    void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 BytesRequired) override
    {
        if (_on_voice_processing_pass_start != nullptr) {
            if (_on_voice_processing_pass_start(*this, BytesRequired)) {
                std::lock_guard<std::mutex> lock(_mutex);
                _eos = true;
            }
        }
    }

    void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override
    {
    }

    void STDMETHODCALLTYPE OnStreamEnd() override
    {
    }

    void STDMETHODCALLTYPE OnBufferStart(void* pBufferContext) override
    {
    }

    void STDMETHODCALLTYPE OnBufferEnd(void* pBufferContext) override
    {
        bool s = false;
        {
            std::lock_guard<std::mutex> lock(_mutex);

            for (auto itr = _buffers.begin(); itr != _buffers.end(); itr++) {
                if (pBufferContext == itr->get_audio_data()) {
                    _buffers.erase(itr);
                    break;
                }
            }
            s = _eos && _buffers.size() < 1;
        }
        if (s) {
            stop();
        }
    }

    void STDMETHODCALLTYPE OnLoopEnd(void* pBufferContext) override
    {
    }

    void STDMETHODCALLTYPE OnVoiceError(void* pBufferContext, HRESULT Error) override
    {
    }
};
