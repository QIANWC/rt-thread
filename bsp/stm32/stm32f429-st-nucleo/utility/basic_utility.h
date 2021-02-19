#pragma once

#define NAME(x) #x


//#define EASY_INCLUDE

#if EASY_INCLUDE
//为避免放纵用户使用各种依赖项的同时忘记添加必要的引用，本文件并不会主动引用常见库文件
//快速开发时可以启用EASY_INCLUDE，但正式发布前应该禁用并进行自测
#include <cstdio>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <numeric>
#endif

#include <cassert>
#include <array>
using std::array;//推荐使用array代替C style array
#include <string.h>//memory.h实际引用string.h，历史遗留问题
#include <NonCopyable.h>
using mbed::NonCopyable;
#include "Microsecond.hpp"
using namespace Microsecond;
#include "argument_vector.hpp"

//test utility
#define test_assert(expr) ++test_cnt;if(expr){++pass_cnt;}else{if(fail_stop)goto stop;}

#ifdef __MBED__
inline void msdelay(uint32_t ms)
{
    Thread::wait(ms);
}
#elif defined __RTTHREAD__
//注意使用环境
inline void usdelay(uint32_t us)
{
    rt_hw_us_delay(us);
}
inline void msdelay(uint32_t ms)
{
    Thread::sleep(ms);
}
#define print rt_kprintf

//printf,rt_kprintf
#endif // 
