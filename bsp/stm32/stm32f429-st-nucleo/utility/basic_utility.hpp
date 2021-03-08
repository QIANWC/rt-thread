#pragma once

#ifndef __BASIC_UTILITY_H__
#define __BASIC_UTILITY_H__

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
using Microsecond::us;
using Microsecond::us_tick;
//using namespace Microsecond;
//#include "NamedVariant.hpp"

#define TESTFAIL_CONTINUE 0
#define TESTFAIL_STOP 1
#define TESTFAIL_BREAKPOINT 2

#ifdef ARCH_ARM_CORTEX_M
#define TRIG_BREAKPOINT __BKPT()
#else
#define TRIG_BREAKPOINT
#endif

#include <rtdbg.h>
//common actions
bool testfail_action(bool expr, int failbehavior, int& test_cnt, int& pass_cnt)
{
    ++test_cnt;
    if (expr)
    {
        ++pass_cnt;
        return false;
    }

    const char* op = "";
    switch (failbehavior)
    {
    case TESTFAIL_CONTINUE:op = "continuing..."; break;
    case TESTFAIL_STOP:op = "abort"; break;
    case TESTFAIL_BREAKPOINT:op = "trigger breakpoint"; TRIG_BREAKPOINT; break;
    default:op = "unknown failbehavior"; break;
    }
    LOG_W("one test case failed,%s", op);
    return true;
}

#if 1
#define test_assert(expr) \
    if (testfail_action((expr),failbehavior,test_cnt,pass_cnt)){\
        if(failbehavior == TESTFAIL_STOP) goto failexit;\
    }
#else
//test utility
#define test_assert(expr) \
    ++test_cnt;\
    if(expr){\
        ++pass_cnt;}\
    else{\
        printf("one test case failed,");\
        switch(failbehavior){\
        case TESTFAIL_CONTINUE:printf("continuing...\n");break;\
        case TESTFAIL_STOP:printf("abort\n");goto failexit;break;\
        case TESTFAIL_BREAKPOINT:printf("trigger breakpoint\n");TRIG_BREAKPOINT;break;\
        default : printf("unknown failbehavior:%d", failbehavior); break;\
        }\
    }
#endif


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
//using ASIO::NonCopyable;
#endif //

#endif
