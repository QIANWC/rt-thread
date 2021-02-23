#pragma once

#include <mbed.h>

#include <NonCopyable.h>
using mbed::NonCopyable;
using mbed::Ticker;

#define ASSERT(x) MBED_ASSERT(x)

inline void delayms(uint32_t ms)
{
    Thread::wait(ms);
}
