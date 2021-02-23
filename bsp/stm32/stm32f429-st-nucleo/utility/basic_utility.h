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

#include "PlatformInfo.hpp"
#include <cassert>
#include <array>
using std::array;//推荐使用array代替C style array
#include <string.h>//memory.h实际引用string.h，历史遗留问题
#include "Microsecond.hpp"
using namespace Microsecond;
#include "argument_vector.hpp"

//test utility
#define test_assert(expr) \
    ++test_cnt;\
    if(expr){\
        ++pass_cnt;}\
    else{\
        if(failstop){\
            printf("test abort@case:%d", test_cnt);\
            goto failexit;}\
        else{\
            /*continue even encounter failure*/\
        }\
    }

enum class ConsoleColor
{
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White
};
 
void SetConsoleForegroundColor(ConsoleColor color)
{
    printf("\x1b[%dm", 30 + (int)color);
}

#ifdef __MBED__
#include "mbed_portout.hpp"
#elif defined __RTTHREAD__
#include "rtt_portout.hpp"
using ASIO::Ticker;
using ASIO::NonCopyable;
#endif // 
