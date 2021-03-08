#pragma once

#include <cstdint>
#include <cmath>


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

}
