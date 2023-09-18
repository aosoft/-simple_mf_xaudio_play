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
    CHECK_HR(_source_voice->SetVolume(1.0f))

    return S_OK;
}

void xaudio_player::finalize()
{
    _mastering_voice = nullptr;
    _source_voice = nullptr;
    _xaudio = nullptr;
}

void STDMETHODCALLTYPE xaudio_player::OnVoiceProcessingPassStart(UINT32 BytesRequired) { }
void STDMETHODCALLTYPE xaudio_player::OnVoiceProcessingPassEnd() { }
void STDMETHODCALLTYPE xaudio_player::OnStreamEnd() { }
void STDMETHODCALLTYPE xaudio_player::OnBufferStart(void* pBufferContext) { }
void STDMETHODCALLTYPE xaudio_player::OnBufferEnd(void* pBufferContext) { }
void STDMETHODCALLTYPE xaudio_player::OnLoopEnd(void* pBufferContext) { }
void STDMETHODCALLTYPE xaudio_player::OnVoiceError(void* pBufferContext, HRESULT Error) { }
