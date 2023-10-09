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

