#pragma once

#include <cstddef>
#include <array>
#include "../utility/basic_utility.h"

namespace filter
{
    
    template<typename T, size_t N>
        std::array<T, N> operator<<(std::array<T, N>& a, const T& new_state)
        {
            size_t n = 0;
            for (; n < N - 1; ++n)
            {
                a[n] = a[n + 1];
            }
            a[n] = new_state;
            return a;
        }
        
    template<typename T>
        std::vector<T> operator<<(std::vector<T>& a, const T& new_state)
        {
            size_t n = 0;
            size_t N = a.size();
            for (; n < N - 1; ++n)
            {
                a[n] = a[n + 1];
            }
            a[n] = new_state;
            return a;
        }
        
    template<typename T, size_t N>
        class FIR :public NonCopyable<FIR<T, N>>
        {
        public:
            using arr = std::array<T, N>;
            FIR()
            {
                reset();
            }
            FIR(const T* pcoef)
            {
                set_coef(pcoef);
            }
            FIR(const std::initializer_list<const T>& l)
            {
                set_coef(l);
            }
            ~FIR()
            {
            }
            
            void reset()
            {
                x.fill(0);
            }
                
            void set_coef(const T* const pcoef)
            {
                assert(pcoef);
                reset();
                for (size_t n = 0; n < N; ++n)
                {
                    coef[n] = pcoef[n];
                }
            }
            void set_coef(const std::initializer_list<const T>& l)
            {
                set_coef(l.begin());
            }
        
            T compute(const T& new_state)
            {
                x << new_state;
                T sum = 0;
                for (size_t n = 0; n < N; ++n)
                {
                    sum += x[n]*coef[n];
                }
                return sum;
            }
            T operator<<(const T& new_state)
            {
                return compute(new_state);
            }
        
        private:
            arr x, coef;
        };
    
    template<typename T, size_t N, size_t D>
        class IIR :public NonCopyable<IIR<T, N, D>>
        {
        public:
            using arr = std::array<T, N>;
            IIR()
            {
                reset();
            }
                
            IIR(const T* num, const T* den)
            {
                set_coef(num, den);
                reset();
            }
            IIR(std::initializer_list<const T> num, std::initializer_list<T> den)
            {
                set_coef(num, den);
            }
            ~IIR()
            {
            }
                
            void reset()
            {
                x.fill(0);
                y.fill(0);
            }
                
            void set_coef(const T* _num, const T* _den)
            {
                assert(_num);
                assert(_den);
                reset();
                for (size_t n = 0; n < N; ++n)
                {
                    num[n] = _num[n];
                }
                for (size_t d = 0; d < D; ++d)
                {
                    den[d] = _den[d];
                }
            }
            void set_coef(const std::initializer_list<const T>& _num, const std::initializer_list<T>& _den)
            {
                set_coef(_num.begin(), _den.begin());
            }
                
            T compute(const T& new_state)
            {
                x << new_state;
                T sumx = 0, sumy = 0;
                //ystate[0] = 0;//ignore first y
                for(size_t n = 1 ; n < N ; ++n)
                {
                    sumx += x[n]*num[n];
                    sumy += y[n]*den[n];
                }
                T r = sumx - sumy;
                y << r;
                return r;
            }
            T operator<<(const T& new_state)
            {
                return compute(new_state);
            }
        private:
            arr x, y, num, den;
        };

    //Biquad也可以通过特化二阶IIR得到
    template<class T>
    using bq = IIR<T, 3, 3>;
    bq<float> bqf();
    template<typename T>
        using Biquad = IIR<T, 3, 3>;
//    template<typename T>
//        class Biquad :public NonCopyable<Biquad<T>>
//        {
//        public:
//            using arr = std::array<T, 3>;
//            Biquad();
//            ~Biquad();
//            void reset()
//            {
//                x.fill(0);
//                y.fill(0);
//            }
//                
//            void set_coef(std::initializer_list<T> num, std::initializer_list<T> den)
//            {
//                
//            }
//            
//            T compute(const T& new_state)
//            {
//                x << new_state;
//                T sumx = 0, sumy = 0, r = 0;
//                for (size_t n = 1; n < 3; ++n)
//                {
//                    sumx += x[n]*num[n];
//                    sumy += y[n]*num[n];
//                }
//                r = sumx - sumy;
//                y << r;
//                return r;
//            }
//            T operator<<(const T& new_state)
//            {
//                return compute(new_state);
//            }
//        private:
//            arr x, y, num, den;
//        };
    
    namespace Test
    {

        int test(bool failstop)
        {
            //developing...
            FIR<float, 3> fir2 = { 1.0f, 1.0f, 1.0f };
            FIR<float, 3> fir3({ 1.0f, 1.0f, 1.0f });
            FIR<float, 3> fir4(std::initializer_list<const float>{ 0.3f, 0.3f, 0.3f });

            
            
            return 0;
        }
        
    }
    
}
