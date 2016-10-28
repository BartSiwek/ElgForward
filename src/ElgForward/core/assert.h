#pragma once

#include <cstdint>
#include <cstdlib>

#include <windows.h>

#include <dxfw/dxfw.h>

// Enable individual assert levels
// #define ASSERT_ENABLE_FAST
// #define ASSERT_ENABLE_SLOW

// The following take precedence over ASSERT_ENABLE_* macros
// #define ASSERT_DISABLE_CRITICAL
// #define ASSERT_DISABLE_FAST
// #define ASSERT_DISABLE_SLOW
 
// Build-wide defines. Enable an assert level and below
#if defined(ASSERT_BUILD_ENABLE_FAST)
#define ASSERT_DISABLE_FAST
#endif  // defined(ASSERT_BUILD_ENABLE_FAST)

#if defined(ASSERT_BUILD_ENABLE_SLOW)
#define ASSERT_DISABLE_FAST
#define ASSERT_ENABLE_SLOW
#endif  // defined(ASSERT_BUILD_ENABLE_FAST)

// Assert handler
#define HANDLE_ASSERT(file, line, message, ...) DXFW_TRACE(file, line, true, message, __VA_ARGS__); \
                                                IsDebuggerPresent() ? __debugbreak() : std::abort()

// Assert macros
#if !defined(ASSERT_DISABLE_CRITICAL)
#define ASSERT_CRITICAL(condition, message, ...) (condition) ? (void)true : HANDLE_ASSERT(__FILE__, __LINE__, message, __VA_ARGS__)
#else  // !defined(ASSERT_DISABLE_CRITICAL)
#define ASSERT_CRITICAL(condition, message, ...)
#endif  // !defined(ASSERT_DISABLE_CRITICAL)

#if defined(ASSERT_ENABLE_FAST) && !defined(ASSERT_DISABLE_FAST)
#define ASSERT_CRITICAL(condition, message, ...) (condition) ? (void)true : HANDLE_ASSERT(__FILE__, __LINE__, message, __VA_ARGS__)
#else  // defined(ASSERT_ENABLE_FAST) && !defined(ASSERT_DISABLE_FAST)
#define ASSERT_FAST(condition, message, ...)
#endif  // defined(ASSERT_ENABLE_FAST) && !defined(ASSERT_DISABLE_FAST)

#if defined(ASSERT_ENABLE_SLOW) && !defined(ASSERT_DISABLE_SLOW)
#define ASSERT_CRITICAL(condition, message, ...) (condition) ? (void)true : HANDLE_ASSERT(__FILE__, __LINE__, message, __VA_ARGS__)
#else  // defined(ASSERT_ENABLE_SLOW) && !defined(ASSERT_DISABLE_SLOW)
#define ASSERT_SLOW(condition, message, ...)
#endif  // defined(ASSERT_ENABLE_SLOW) && !defined(ASSERT_DISABLE_SLOW)

