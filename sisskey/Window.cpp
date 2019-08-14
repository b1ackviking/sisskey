#include "Window.h"

#ifdef _WIN64
#include "WinAPI.inl"
#elif defined(__linux__)
#include"XCB.inl"
#endif
