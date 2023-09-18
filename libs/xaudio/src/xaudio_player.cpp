#include "xaudio_player.h"

xaudio_player::xaudio_player()
    : _mastering_voice(nullptr)
    , _source_voice(nullptr)
{
}

xaudio_player::~xaudio_player()
{
    finalize();
}

HRESULT xaudio_player::initialize(const WAVEFORMATEX& wfx)
{
    finalize();
    CHECK_HR(XAudio2Create(&_xaudio, 0, XAUDIO2_DEFAULT_PROCESSOR))
    CHECK_HR(_xaudio->CreateMasteringVoice(&_mastering_voice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, nullptr, nullptr))
    CHECK_HR(_xaudio->CreateSourceVoice(&_source_voice, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this, nullptr, nullptr))
    CHECK_HR(set_volume(1.0f))

    return S_OK;
}

void xaudio_player::finalize()
{
    _mastering_voice = nullptr;
    _source_voice = nullptr;
    _xaudio = nullptr;
}

HRESULT xaudio_player::start()
{
    return S_OK;
}

HRESULT xaudio_player::stop()
{
    return S_OK;
}

XAUDIO2_VOICE_STATE xaudio_player::get_voice_state() const
{
    if (is_initialized()) {
        XAUDIO2_VOICE_STATE ret;
        _source_voice->GetState(&ret);
        return ret;
    }
    return {};
}

float xaudio_player::get_volume() const
{
    if (is_initialized()) {
        float volume;
        _source_voice->GetVolume(&volume);
        return volume;
    }
    return 0.0f;
}

HRESULT xaudio_player::set_volume(float volume)
{
    if (is_initialized()) {
        return _source_voice->SetVolume(volume);
    }
    return S_FALSE;
}

void STDMETHODCALLTYPE xaudio_player::OnVoiceProcessingPassStart(UINT32 BytesRequired) { }
void STDMETHODCALLTYPE xaudio_player::OnVoiceProcessingPassEnd() { }
void STDMETHODCALLTYPE xaudio_player::OnStreamEnd() { }
void STDMETHODCALLTYPE xaudio_player::OnBufferStart(void* pBufferContext) { }
void STDMETHODCALLTYPE xaudio_player::OnBufferEnd(void* pBufferContext) { }
void STDMETHODCALLTYPE xaudio_player::OnLoopEnd(void* pBufferContext) { }
void STDMETHODCALLTYPE xaudio_player::OnVoiceError(void* pBufferContext, HRESULT Error) { }
