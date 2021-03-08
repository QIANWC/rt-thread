#pragma once

/*暂不使用RTOS自带功能，直接从定时器获取。
 *us时间提供两种API：
 *32位周期性溢出的相对时间：us_tick();
 *64位绝对时间（后期可能接入NTP同步），us();
 **/

#ifndef MICROSECOND_H
#define MICROSECOND_H

#ifdef __MBED__
#include <us_ticker_api.h>
extern TIM_HandleTypeDef TimMasterHandle;
TIM_HandleTypeDef* usTimer = &TimMasterHandle;
#elif defined __RTTHREAD__
#include <stdint.h>
#include <rtthread.h>
#include <board.h>
#include "main.h"
typedef uint32_t timestamp_t;
typedef uint64_t us_timestamp_t;
#elif defined _WIN32
#include <chrono>
typedef uint32_t timestamp_t;
typedef uint64_t us_timestamp_t;
//TODO:
#endif // __MBED__

#if US_TICK_USE_TIM2
    #ifdef __RTTHREAD__
    extern TIM_HandleTypeDef htim2;
    TIM_HandleTypeDef* usTimer = &htim2;
    #define usTimer_Init MX_TIM2_Init
    #endif
    #define usTimer_irq TIM2_IRQn
    #define usTimer_isr TIM2_IRQHandler
    #define usTimer_DBGMCU_FREEZE __HAL_DBGMCU_FREEZE_TIM2
#elif US_TICK_USE_TIM5
    #ifdef __RTTHREAD__
    extern TIM_HandleTypeDef htim5;
    static TIM_HandleTypeDef* usTimer = &htim5;
    #define usTimer_Init MX_TIM5_Init
    #endif
    #define usTimer_irq TIM5_IRQn
    #define usTimer_isr TIM5_IRQHandler
    #define usTimer_DBGMCU_FREEZE __HAL_DBGMCU_FREEZE_TIM5
#elif defined _WIN32
#else
    #error "us timer not set"
#endif
namespace Microsecond
{

    static uint32_t ticker_overflow_cnt = 0;
    static void us_ticker_overflow_isr()
    {
        ++ticker_overflow_cnt;
    }
    
#ifdef __MBED__
    static void timer_irq_handler(void)
    {
        //Timer Update Event for overflow accumulate
        if (__HAL_TIM_GET_FLAG(&TimMasterHandle, TIM_FLAG_UPDATE) == SET) {
            __HAL_TIM_CLEAR_IT(&TimMasterHandle, TIM_IT_UPDATE);
            us_ticker_overflow_isr();
        }
        // Channel 1 for mbed timeout
        else if (__HAL_TIM_GET_FLAG(&TimMasterHandle, TIM_FLAG_CC1) == SET) {
            if (__HAL_TIM_GET_IT_SOURCE(&TimMasterHandle, TIM_IT_CC1) == SET) {
                __HAL_TIM_CLEAR_IT(&TimMasterHandle, TIM_IT_CC1);
                us_ticker_irq_handler();
            }
        }
    }
    void init()
    {
        //mbed会将中断向量表映射到内存并替换部分ISR
        //需要手动替换mbed默认的timer_irq_handler
//        NVIC_DisableIRQ(usTimer_irq);
        NVIC_SetVector(usTimer_irq, (uint32_t)timer_irq_handler);
//        NVIC_EnableIRQ(usTimer_irq);
        __HAL_TIM_CLEAR_IT(usTimer, TIM_IT_UPDATE);
        __HAL_TIM_ENABLE_IT(usTimer, TIM_IT_UPDATE);
    }
    timestamp_t us_tick(void)
    {
        return us_ticker_read();
    }
#elif defined __RTTHREAD__
    extern "C" void usTimer_isr(void)
    {
        if (__HAL_TIM_GET_FLAG(usTimer, TIM_IT_UPDATE))
        {
            __HAL_TIM_CLEAR_IT(usTimer, TIM_IT_UPDATE);
            Microsecond::us_ticker_overflow_isr();
        }
    }
    extern "C" int init(void)
    {
        usTimer_Init();//if hal_main not used
        //usTimer_DBGMCU_FREEZE();//optional
        __HAL_TIM_CLEAR_IT(usTimer, TIM_IT_UPDATE);
        HAL_TIM_Base_Start_IT(usTimer);
        return 0;
    }
    INIT_BOARD_EXPORT(init);
    timestamp_t us_tick(void)
    {
        return usTimer->Instance->CNT;
    }
#elif defined _WIN32
    timestamp_t us_tick(void)
    {
        //TODO:std::chrono测试验证，API统一
        return std::chrono::microseconds().count();
    }
#endif

#ifndef _CHRONO_
    us_timestamp_t us()
    {
        return ((us_timestamp_t)ticker_overflow_cnt << 32) + us_tick();
    }
#else
    us_timestamp_t us()
    {
        return std::chrono::microseconds().count();
    }
#endif

}

#endif
