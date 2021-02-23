#pragma once

#include "../utility/basic_utility.h"
#include "../utility/SeriesData.hpp"

class QuadEncInfo
{
public:
    QuadEncInfo(int32_t _cnt, int32_t _stamp)
        : count(_cnt)
        , stamp(_stamp)
    {
    }
    int32_t count;
    us_timestamp_t stamp;
};
    
//int,uint还需要仔细考虑
class QuadratureEncoder : public NonCopyable<QuadratureEncoder>
{
public:
    QuadratureEncoder(int32_t cnt_per_rev)
        : cpr(cnt_per_rev), stampedpos(10)
    {
    }
    ~QuadratureEncoder()
    {
    }

    int64_t update(const QuadEncInfo& newcnt)
    {
        int32_t d = newcnt.count - cnt_in_rev;
        bool dpostive = d >= 0, doverhalf = abs(d) >= cpr / 2;
        if (dpostive)
        {
            if (doverhalf)
            {
                //underflow
                --rev;
            }
            else
            {
                //increment
            }
        }
        else
        {
            if (doverhalf)
            {
                //inc,overflow
                ++rev;
            }
            else
            {
                //decrement
            }
        }
        cnt_in_rev = newcnt.count;
        laststamp = stamp;
        lastcnt = cnt;
        stamp = newcnt.stamp; //stamp也要考虑溢出问题
        cnt = rev*cpr + cnt_in_rev;
        stampedpos.push(stamp, cnt);

        return cnt;
    }
            
    //消耗时间随数据点数增长较快，注意执行优先级
    int64_t update_speed()
    {
        auto regcoef = stampedpos.regression();
        cps = regcoef.a * 1000'000.0f;
        rps = cps / cpr;
        return cps;
    }
        
    int64_t getposcnt()
    {
        return cnt;
    }
    float getpos()
    {
        return (float)cnt / cpr;
    }
        
    float getvel_cps()
    {
        return cps;
    }
    float getvel_rps()
    {
        return rps;
    }
    
private:
    TIM_HandleTypeDef* htim;
    int32_t cpr; //count per revolution
    int64_t cnt, lastcnt; //global absolute position cnt
    int64_t stamp, laststamp; //us timestamp
    int32_t rev, cnt_in_rev; //cnt in revolution
    TimestampedDataI64 stampedpos;
    float cps, rps;
};

//QuadratureEncoderPulse
//use pulse edge stamp to get more precise info
