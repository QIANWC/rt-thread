#pragma once

#include <vector>
#include <cmath>

//TODO:提高计算效率
#pragma GCC push_options
#pragma GCC optimize("O2")

template<typename TA, typename TB>
    struct PointBase
    {
        TA a;
        TB b;
        PointBase(TA _a, TB _b)
        {
            a = _a;
            b = _b;
        }
    };
using Point2f = PointBase<float, float>;

//template<typename T>
//    float mean(std::vector<T>& v)
//    {
//        return 0.0f;
//    }
//template<typename T>
//    float variance(std::vector<T>& v)
//    {
//        return 0.0f;
//    }
template<typename T>
    T pop_front(std::vector<T>& v)
    {
        T d = v.front();
        v.erase(v.begin(), 1);
        return d;
    }
template<typename T>
    T subvec(std::iterator<T ,T*> ite, size_t n)
    {
        return std::vector<T>(ite, ite + n);
    }

template<typename TA,typename TB>
class DataSeries2D
{
public:
    using Point = PointBase<TA, TB>;
    
    DataSeries2D(size_t n)
    {
        maxsize = n;
        va.resize(maxsize);
        vb.resize(maxsize);
    }
    ~DataSeries2D()
    {
    }

    int push(TA a, TB b)
    {
        pop();
        va.push_back(a);
        vb.push_back(b);
        return 1;
    }
    int push(const TA* a, const TB* b, size_t n)
    {
        size_t insert_size = std::min(maxsize - vb.size(), n);
        va.insert(va.end(), a, a + insert_size);
        vb.insert(va.end(), b, b + insert_size);
        return insert_size;
    }
        
    Point pop()
    {
        if (va.size() <= 0)
            return Point(0, 0);
        TA a = va.front();
        TB b = vb.front();
        va.erase(va.begin(), va.begin() + 1);
        vb.erase(vb.begin(), vb.begin() + 1);
        return Point(a, b);
    }
    PointBase<std::vector<TA>, std::vector<TB>> pop(size_t n)
    {
        size_t pop_size = std::min(n, va.size());
        std::vector<TA> subva(va.begin(), pop_size);
        std::vector<TB> subvb(vb.begin(), pop_size);
        PointBase<std::vector<TA>, std::vector<TB>> res(subva, subvb);
        va.erase(va.begin(), va.begin() + pop_size);
        vb.erase(vb.begin(), vb.begin() + pop_size);
        return res;
    }
    
    Point mean()
    {
        TA sumA = 0, sumB = 0;
        for (size_t   k = 0; k < vb.size(); ++k)
        {
            sumA += va[k];
            sumB += vb[k];
        }
        return Point((float)sumA / vb.size(), (float)sumB / vb.size());
    }
    
    //s^2
    Point2f variance()
    {
        auto m = mean();
        float am = m.a, bm = m.b;
        float sumA = 0;//如果用整型容易溢出
        float sumB = 0;
        for (size_t k = 0; k < va.size(); ++k)
        {
            float da = va[k] - am, db = vb[k] - bm;
            sumA += da*da;
            sumB += db*db;
        }
        
        size_t n = vb.size();
        if (n >= 2)
            return Point2f(sumA / (n - 1), sumB / (n - 1));
        else
            return Point2f(sumA, sumB);
    }
    Point2f stdvar()
    {
        auto var = variance();
        return Point2f(std::sqrt(var.a), std::sqrt(var.b));
    }
        
    //b=k*a+c,return [k,c]
    Point2f regression()
    {
        auto m = mean();
        float am = m.a, bm = m.b;
        float num = 0, den = 0;
        const size_t n = vb.size();
#if 1
        const TA* pa = va.data(), *paend = pa + n;
        const TB* pb = vb.data();
        for (; pa < paend; ++pa, ++pb)
        {
            float da = *pa - am, db = *pb - bm;
            num += da*db;
            den += da*da;
        }
#else
//        for (size_t k = 0; k < n; ++k)
//        {
//            float da = va[k] - am, db = vb[k] - bm;
//            num += da*db;
//            den += da*da;
//        }
#endif // 0
        float k_hat = num / den, c_hat = bm - am*k_hat;
        return Point2f(k_hat, c_hat);
    }
        
    //TODO:provide a vector resize function
    
    TB regression_predict(TA t);
    
private:
    std::vector<TA> va;//应该改用matrix::vector
    std::vector<TB> vb;
    size_t maxsize = 0;
};

//using TimestampedData<typename T>= DataSeries2D<int64_t, T>;
//template<typename T>
//    DataSeries2D<int64_t, T> TimestampedData<T>;
using TimestampedDataF32 = DataSeries2D<int64_t, float>;
using TimestampedDataI64 = DataSeries2D<int64_t, int64_t>;

#pragma GCC pop_options
