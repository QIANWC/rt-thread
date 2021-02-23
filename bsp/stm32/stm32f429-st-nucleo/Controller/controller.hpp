#pragma once

#include <algorithm>

#include "basic_filter.hpp"

namespace controller
{
    using std::clamp;

    template<typename T>
        class PIDBase :public NonCopyable<PIDBase<T>>
        {
        public:
            PIDBase()
            {
                reset();
            }
        
            ~PIDBase()
            {
            }
    
            enum AntiWindupMethod
            {
                None             = 0,
                Clamp,
                BackCalculattion,
//                Sgn
            };
            void reset()
            {
                p = 0, i = 0, d = 0, u = 0;
                fir.reset();
            }
            void set_gain(const T& _p, const T& _i, const T& _d)
            {
                lock();
                kp = _p;
                ki = _i;
                kd = _d;
                unlock();
            }
            void set_max(const T& _p, const T& _i, const T& _d)
            {
                lock();
                ASIO_ASSERT(_p >= 0);// "pmax invalid"
                ASIO_ASSERT(_i >= 0);// "imax invalid"
                ASIO_ASSERT(_d >= 0);// "dmax invalid"
                pmax = _p;
                imax = _i;
                dmax = _d;
                unlock();
            }
            void set_umax(const T& _u)
            {
                lock();
                umax = _u;
                unlock();
            }
            void set_lpf_coef(std::initializer_list<T> l)
            {
                fir.set_coef(l);
            }
            void set_ts(const T& _sec)
            {
                Ts = _sec;
            }
                
            void set_target(const T& x)
            {
                tar = x;
            }
        
            T compute(const T& state)
            {
                val = state;
                err_d = err;
                err = tar - val;
                switch (antiwindup_method)
                {
                default:
                case None:
                case Clamp: 
                    {
                        //p,i,d,u都使用独立的限幅
                        p = clamp(kp*err, -pmax, pmax);
                        i = clamp(i + ki*Ts*err, -imax, imax);
                        //d-LPF
                        d = fir << clamp(kd*(err - err_d), -dmax, dmax);
                        u = clamp(p + i + d, -umax, umax);
                    }break;
                case BackCalculattion:
                    {
                        //pd无限幅，i使用反向计算，u使用限幅
                        p = ki*err;
                        //d-LPF
                        d = fir << kd*(err - err_d);
                        //back_calculation应该立即作用还是延迟一拍？
                        T i_raw = i + ki*Ts*err;
                        T u_preclamp = p + i_raw + d;
                        T u_postclamp = clamp(u_preclamp, -umax, umax);
                        T back_cal = kb*(u_postclamp - u_preclamp);
                        i = i + ki*Ts*(err + back_cal);
                        u = clamp(p + i + d, -umax, umax);
                        //还可以合并一些计算
                    }break;
                }
                return u;
            }
            T operator<<(const T& state)
            {
                return compute(state);
            }
    
            void operator>>(T& _u)
            {
                _u = u;
            }
    
            //获得一次PID类全部状态采样，状态采集和分析使用
            size_t state_sample(void *dst)
            {
                lock();
                memcpy(dst, this, sizeof(this));
                unlock();
                return sizeof(this);
            }
    
        private:
            T kp = 0, ki = 0, kd = 0;
            T p = 0, i = 0, d = 0, u = 0;
            T pmax = 0, imax = 0, dmax = 0, umax = 0;
            T tar = 0, val = 0, err = 0, err_d = 0;
            T Ts = 0.001f, radix = 1.0f;
            T kb = 0;
            uint8_t antiwindup_method = Clamp;
            uint8_t use_radix = false;
            filter::FIR<T, 2> fir = { 0.5f, 0.5f };
    
            void lock()
            {
                //disable isr
            }
            void unlock()
            {
                //enable isr
            }
        };

    using pidclass = PIDBase<float>;

    namespace Test
    {
        void test()
        {
            pidclass pid;
            pid.set_gain(1, 1, 1);
            printf("Controller test not ready yet\n");
//            for (;;)
//            {
//                //需要虚拟系统才能有效仿真
//            }
        }
    }
}
