#pragma once
#include <Framework/Debug/Logger.hpp>

#include <intrin.h>

#define Debug_BreakPoint() __debugbreak()

#define Debug_Assert(condition)						do { if (!(condition)) { ::W::Logger::AssertFailure(__FILE__, __LINE__, #condition, nullptr             ); Debug_BreakPoint(); } } while(false)
#define Debug_AssertMsg(condition, message, ...)	do { if (!(condition)) { ::W::Logger::AssertFailure(__FILE__, __LINE__, #condition, message, __VA_ARGS__); Debug_BreakPoint(); } } while(false)
