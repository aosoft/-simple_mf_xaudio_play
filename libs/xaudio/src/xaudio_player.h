#pragma once

#include "common.h"
#include <xaudio2.h>

class xaudio_player : public IXAudio2VoiceCallback {
private:
    com_ptr<IXAudio2> _xaudio;
    IXAudio2MasteringVoice* _mastering_voice;
    IXAudio2SourceVoice* _source_voice;

public:
    xaudio_player();
    ~xaudio_player();
};