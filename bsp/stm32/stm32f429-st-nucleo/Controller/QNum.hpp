#pragma once

#include <cstdint>
#include <cmath>

#include "basic_utility.h"

namespace qmath
{
    template<size_t N>
    constexpr int get_qscale(void)
    {
        return 2 * get_qscale<N - 1>();
    }
    template<>
    constexpr int get_qscale<0>(void)
    {
        return 1;
    }
template<typename T,size_t Q>
class QNum
{
public:
    using qtype = T;
//    typedef unsigned T qtype;
    static constexpr float ScaleFromFp = (float)get_qscale<Q>() ;
    static constexpr float ScaleToFp = 1.0f / (float)get_qscale<Q>();
    static constexpr int ScaleFromInt = get_qscale<Q>();
    
    QNum()
    {
    }
    QNum(const QNum& q)
    {
        *this = q;
    }
    QNum(const float& f)
    {
        V = f*ScaleFromFp;
    }
    QNum(const double& d)
    {
        V = d*ScaleFromFp;
    }
    QNum(const int& q)
    {
        V = q*ScaleFromInt;
    }
        
    ~QNum()
    {}
        
    operator int()
    {
        return V >> Q;
    }
    operator float()
    {
        return (float)V*ScaleToFp;
    }
    operator double()
    {
        return (double)V*ScaleToFp;
    }
    
    QNum operator=(const QNum& q)
    {
        return (V = q.V);
    }
    QNum operator=(const float& f)
    {
        V = f*ScaleFromFp;
        return *this;
    }
    QNum operator=(const int&i)
    {
        V = i*ScaleFromInt;
        return *this;
    }
        
    QNum operator+(const QNum& q)
    {
        return _fromraw(V + q.V);
    }
    QNum operator+(const float f)
    {
        return *this + QNum(f);
    }
    QNum operator+(const int i)
    {
        return *this + QNum(i);
    }

    QNum operator-(const QNum& q)
    {
        return _fromraw(V - q.V);
    }
    QNum operator-(const float f)
    {
        return *this - QNum(f);
    }
    QNum operator-(const int& i)
    {
        return *this - QNum(i);
    }
        
    QNum operator*(const QNum& q)
    {
        return _fromraw((V*q.V) >> Q);
    }
    QNum operator*(const float& f)
    {
        return (*this)*QNum(f);
    }
    QNum operator*(const int& i)
    {
        return (*this)*QNum(i);
    }
    
    QNum operator/(const QNum& q)
    {
        return _fromraw((V << Q)/q.V);//shift first
    }
    QNum operator/(const float& f)
    {
        return (*this) / QNum(f);
    }
    QNum operator/(const int& i)
    {
        return (*this) / QNum(i);
    }
    
    QNum operator==(const QNum& q)
    {
        return V == q.V;
    }
    QNum operator==(const float& f)
    {
        return (*this) == QNum(f);
    }
    QNum operator==(const int& i)
    {
        return (*this) == QNum(i);
    }

    bool operator!=(const QNum& q)
    {
        return V != q.V;
    }
    bool operator!=(const float& f)
    {
        return (*this) != QNum(f);
    }
    bool operator!=(const int& i)
    {
        return (*this) != QNum(i);
    }
protected:
    QNum _fromraw(const qtype& v)
    {
        QNum q;
        q.V = v;
        return q;
    }
private:
    qtype V;
};
    
    namespace Test
    {
#include <cstdio>
        bool near(float a, float b)
        {
            return fabs(a - b) <= 0.001f;
        }
            
        int test(int failbehavior = TESTFAIL_CONTINUE)
        {
            using q15 = QNum<int32_t, 15>;//q15_int32 or say q15_precision32
            printf("QNum Test:\n");
            q15 v1 = 1.0f, v2 = 2.0f, v3 = 3.0f, v0_1 = 0.1f, v0_5 = 0.5f;
            printf("test values:%f,%f,%f,%f,%f\n", (float)v1, (float)v2, (float)v3, (float)v0_1, (float)v0_5);
            int pass_cnt = 0, test_cnt = 0;
            
            //目前的测试方法不一定对，精度方面还欠考虑，覆盖面还不够
            test_assert(near(q15(1) + q15(2), 3));
            test_assert(near(q15(1.0f - 2.0f), -1.0f));
            test_assert(near(v1 + v2, 3.0f));
            test_assert(near(v1 - 2, -1));
            test_assert(near(v1 - v2, -1.0f));
            test_assert(near(v1 * v0_5, v0_5));
            test_assert(near(v1 / v2, 0.5f));
            //...
            
failexit :
            if(pass_cnt == test_cnt)
            {
                printf("QNum test passed %d/%d\n", pass_cnt, test_cnt);
                return 0;
            }
            else
            {
                printf("QNum test failed %d/%d\n", pass_cnt, test_cnt);
                return -1;
            }
        }
    }
}
