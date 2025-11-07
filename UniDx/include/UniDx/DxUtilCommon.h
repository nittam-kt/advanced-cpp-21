#pragma once

#include <sstream>
#include <iostream>
#include <filesystem>

#ifdef _DEBUG

#define DEBUG_OUTPUT(v) DxUtilDebugOutput(v, __FILE__, __LINE__)

template<typename T>
inline void DxUtilDebugOutput(const T& value, const char* file, int line)
{
    std::wostringstream oss;
    oss << std::filesystem::path(file).filename().string().c_str() << line << L": " << value << L"\n";
    OutputDebugStringW(oss.str().c_str());
}

#else
#define DEBUG_OUTPUT(v)
#endif
