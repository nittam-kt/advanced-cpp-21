#pragma once

#include <string>

#ifdef _DEBUG

namespace UniDx
{

// デバッグ用ネームスペース
namespace Debug
{

    template<typename T>
    inline void Log(const T& value)
    {
        OutputDebugStringW(std::to_wstring(value).c_str());
        OutputDebugStringW(L"\n");
    }
    inline void log_(const wchar_t* value)
    {
        OutputDebugStringW(value);
        OutputDebugStringW(L"\n");
    }
    inline void Log(const std::wstring& value) { log_(value.c_str()); }
    inline void Log(const std::wstring_view& value) { log_(std::wstring(value).c_str()); }
    inline void Log(const wchar_t* value) { log_(value); }
    inline void log_(const char* value)
    {
        OutputDebugStringA(value);
        OutputDebugStringA("\n");
    }
    inline void Log(const std::string& value) { log_(value.c_str()); }
    inline void Log(const std::string_view& value) { log_(std::string(value).c_str()); }
    inline void Log(const char* value) { log_(value); }
}

}

#else

namespace UniDx
{

namespace Debug
{
    template<typename T>
    inline void Log(const T& value) { }
}

}

#endif
