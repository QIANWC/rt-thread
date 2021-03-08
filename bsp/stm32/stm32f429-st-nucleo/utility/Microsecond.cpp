
#include "Microsecond.hpp"

//Keil编译问题。。。

    extern "C" void usTimer_isr(void)
    {
        if (__HAL_TIM_GET_FLAG(usTimer, TIM_IT_UPDATE))
        {
            __HAL_TIM_CLEAR_IT(usTimer, TIM_IT_UPDATE);
            Microsecond::us_ticker_overflow_isr();
        }
    }
