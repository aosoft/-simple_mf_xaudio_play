#pragma once

#include <crtdbg.h>
#include <memory>
#include <wrl/client.h>

template <class Intf>
using com_ptr = Microsoft::WRL::ComPtr<Intf>;

#define CHECK_HR(hr)         \
    {                        \
        HRESULT hrtmp = hr;  \
        if (FAILED(hrtmp)) { \
            _CrtDbgBreak();  \
            return hrtmp;    \
        }                    \
    }

#define CHECK_POINTER(p) CHECK_HR(p != nullptr ? S_OK : E_POINTER);

struct handle_deleter {
    void operator()(HANDLE p) const
    {
        CloseHandle(p);
    }
};

using unique_handle = std::unique_ptr<std::remove_pointer<HANDLE>::type, handle_deleter>;

class co_initializer {
public:
    co_initializer(DWORD dwCoInit)
    {
        ::CoInitializeEx(nullptr, dwCoInit);
    }

    ~co_initializer()
    {
        ::CoUninitialize();
    }
};
