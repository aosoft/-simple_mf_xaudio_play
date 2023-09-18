#pragma once

#include "common.h"
#include <xaudio2.h>

class xaudio_player : private IXAudio2VoiceCallback {
private:
    com_ptr<IXAudio2> _xaudio;
    IXAudio2MasteringVoice* _mastering_voice;
    IXAudio2SourceVoice* _source_voice;

public:
    xaudio_player();
    ~xaudio_player();

    HRESULT initialize(const WAVEFORMATEX& wfx);
    void finalize();

private:
    void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 BytesRequired) override;
    void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override;
    void STDMETHODCALLTYPE OnStreamEnd() override;
    void STDMETHODCALLTYPE OnBufferStart(void* pBufferContext) override;
    void STDMETHODCALLTYPE OnBufferEnd(void* pBufferContext) override;
    void STDMETHODCALLTYPE OnLoopEnd(void* pBufferContext) override;
    void STDMETHODCALLTYPE OnVoiceError(void* pBufferContext, HRESULT Error) override;
};