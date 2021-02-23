#pragma once

#include <rtthread.h>
#define ASIO_ASSERT(x) RT_ASSERT(x)

namespace ASIO
{
    typedef void(*callback)(void*);
    class Ticker
    {
    public:
        Ticker()
        {
        }
            
        ~Ticker()
        {
            rt_timer_detach(&timer);
        }
            
        void attach(callback cb, float t, const std::string name = "")
        {
            int tick = t;
            rt_timer_init(&timer, name.c_str(), cb, nullptr, tick, RT_TIMER_FLAG_PERIODIC);
            rt_timer_start(&timer);
        }
    private:
        struct rt_timer timer;
    };
    
    template<typename T>
        class NonCopyable {
        protected:
            /**
             * Disallow construction of NonCopyable objects from outside of its hierarchy.
             */
            NonCopyable() = default;
            /**
             * Disallow destruction of NonCopyable objects from outside of its hierarchy.
             */
            ~NonCopyable() = default;

        public:
            /**
             * Define copy constructor as deleted. Any attempt to copy construct
             * a NonCopyable will fail at compile time.
             */
            NonCopyable(const NonCopyable &) = delete;

            /**
             * Define copy assignment operator as deleted. Any attempt to copy assign
             * a NonCopyable will fail at compile time.
             */
            NonCopyable &operator=(const NonCopyable &) = delete;
        };

}

//注意使用环境
inline void delayus(uint32_t us)
{
    rt_hw_us_delay(us);
}
inline void delayms(uint32_t ms)
{
    Thread::sleep(ms);
}
