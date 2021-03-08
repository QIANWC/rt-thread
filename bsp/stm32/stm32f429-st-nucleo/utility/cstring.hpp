#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <memory>
#include <vector>
using std::vector;

namespace ASIO
{
        
#define  LITESTRING_STRLEN_WARNNING 1
constexpr size_t base_size = 32;
constexpr size_t npos = SIZE_MAX;

/*malloc always pad an extra byte for '\0'
 *TODO:何时返回引用？
 *TOO:目前传入litestring参数时调用的还是strxxx函数，未利用strsize已知特性进行优化
 *TODO:优化重复构造和内存移动
 *TODO:内存管理还未测试
 **/
class cstring
{
public:
    cstring()
    {
        _constructor();
    }
    cstring(size_t len)
    {
        cap = std::max(base_size, len);
        d = (char*)operator new(cap + 1);
        *d = '\0';
        sz = 0;
    }
    cstring(const char* psrc)
    {
        if (!psrc)
        {
            _constructor();
            return;
        }
        sz = _strnlen(psrc, SIZE_MAX);
        size_t extsz = sz + sz / 2;
        
        cap = std::max(base_size, extsz);
        d = (char*)operator new(cap + 1);
        memcpy(d, psrc, sz + 1);
    }
    cstring(const char* psrc, size_t pos, size_t len)
    {
        if (!psrc)
        {
            _constructor();
            return;
        }
        
        size_t _sz = _strnlen(psrc, SIZE_MAX);
        if (pos >= _sz)
        {
            //void string
            _constructor();
            return;
        }
        
        sz = std::min(_sz - pos, len);
        cap = std::max(base_size, sz + sz / 2);
        d = (char*)operator new(cap + 1);
        memcpy(d, psrc + pos, sz);
        d[sz] = '\0';
    }
    cstring(size_t len, const char c)
    {
        sz = len;
        cap = std::max(base_size, sz + sz / 2);
        d = (char*)operator new(cap + 1);
        memset(d, c, sz);
        d[sz] = '\0';
    }
    cstring(const cstring& rhs)
    {
        cap = rhs.cap;
        sz = rhs.sz;
        d = new char[sz+1];
        d[sz] = '\0';
        memcpy(d, rhs.d, sz);
    }

    ~cstring()
    {
        sz = 0;
        cap = 0;
        if (!d)
            operator delete(d);
    }

    //simple functions
    const size_t capacity()
    {
        return cap;
    }
    const size_t length()
    {
        return sz;
    }
    const size_t size() const
    {
        return sz;
    }
    bool empty() const
    {
        return sz == 0;
    }
    const char at(size_t n) const
    {
        return d[n];
    }
    char& at(size_t n)
    {
        return d[n];
    }
    const char operator[](size_t n) const
    {
        return d[n];
    }
    char& operator[](size_t n)
    {
        return d[n];
    }
    char* data() const
    {
        return d;
    }
    char* c_str() const
    {
        return d;
    }
    void clear()
    {
        sz = 0;
    }

    //cstring& assign();
    
    //赋值函数
    cstring& operator=(const char* psrc)
    {
        if (!psrc)
        {
            d = (char*)operator new(base_size + 1);
            *d = '\0';
            sz = 0;
            cap = base_size;
            return *this;
        }

        sz = _strnlen(psrc, SIZE_MAX);
        size_t extsz = sz + sz / 2;
        extsz = std::max(base_size, extsz);

        if (d == nullptr)
        {
            cap = extsz;
            d = (char*)operator new(cap + 1);
            memcpy(d, psrc, sz + 1);
        }
        else if (cap < extsz)
        {
            cap = extsz;
            operator delete(d);
            d = (char*)operator new(cap + 1);
            memcpy(d, psrc, sz + 1);
        }
        else
        {
            //cap is enough,no malloc
            memcpy(d, psrc, sz + 1);
        }
        return *this;
    }
    cstring& operator=(const cstring& s)
    {
        if (this == &s)
            return *this;
        sz = s.sz;
        cap = s.cap;
        if (!d)
        {
            d = (char*)operator new(s.cap+1);
        }
        memcpy(d, s.d, cap + 1);
        return *this;
    }
    int copyTo(char* dst, size_t pos = 0, size_t len = npos)
    {
        if (pos >= sz)
        {
            *dst = '\0';
            return 0;
        }
        else
        {
            size_t copysz = std::min(len, sz - pos);
            memcpy(dst, d, copysz);
            dst[copysz] = '\0';
            return copysz;
        }
    }
    
    cstring operator+(const char*psrc)
    {
        size_t rhsz = _strnlen(psrc, SIZE_MAX);
        size_t sumsz = sz + rhsz;
        size_t extsz = std::max(base_size, sumsz + sumsz / 2);
        cstring s(extsz);
        return s._append(d, sz)._append(psrc, rhsz);
    }
    cstring operator+(const cstring& str)
    {
        size_t sumsz = sz + str.sz;
        size_t extsz = std::max(base_size, sumsz + sumsz / 2);
        cstring s(extsz);
        return s._append(d, sz)._append(str.d, str.sz);
    }
    
    //append
    cstring& operator+=(const char c)
    {
        return _append(&c, 1);
    }
    cstring& operator+=(const char* psrc)
    {
        if (!psrc)
            return *this;
        return _append(psrc, _strnlen(psrc, SIZE_MAX));
    }
    cstring& operator+=(const cstring& s)
    {
        return _append(s.d, s.sz);
    }
    cstring& append(const char* psrc)
    {
        if (!psrc)
            return *this;
        return _append(psrc, _strnlen(psrc, SIZE_MAX));
    }
    cstring& append(const char* psrc, size_t len = npos)
    {
        if (!psrc)
            return *this;
        return _append(psrc, std::min(len, _strnlen(psrc, SIZE_MAX)));
    }
        
    
    
    //目前还不完全符合strcmp要求，需要返回比较结果的大小
    int compare(const char* psrc, bool case_sensitive = true) const
    {
        if (!d || !psrc)
            return 1;
        if (sz != _strnlen(psrc, SIZE_MAX))
            return 1;
        if (case_sensitive)
            return strncmp(d, psrc, sz);
        else
        {
            for (size_t n = 0; n < sz; ++n)
            {
                //if (toupper(d[n]) != toupper(psrc[n]))
                //TODO:更严谨的方法。目前全部当成字母处理，效率高但并不严谨
                if ((d[n] ^ psrc[n]) & 0xDF)
                    return 1;
            }
            return 0;
        }
    }
    bool operator==(const char* psrc) const
    {
        return compare(psrc) == 0;
    }
    bool operator!=(const char* psrc) const
    {
        return compare(psrc) != 0;
    }
        
    //待测试验证
    size_t resize(size_t n)
    {
        if (n <= cap)
        {
            if (n > sz)
            {
                memset(d + sz, 0, n - sz);
            }
        }
        else
        {
            _reallocate(n);
            memset(d + sz, 0, n - sz);
        }
        sz = n;
        d[sz] = '\0';
        return sz;
    }
    void shrink_to_fit()
    {
        resize(sz);
    }
    
    bool startswith(const char*psrc, bool case_sensitive = true) const
    {
        size_t _sz = _strnlen(psrc, SIZE_MAX);
        if (d)
        {
            return strncmp(d, psrc, _sz) == 0;
        }
        else
        {
            return false;
        }
    }
    bool endswith(const char* psrc, bool case_sensitive = true) const
    {
        size_t _sz = _strnlen(psrc, SIZE_MAX);
        if (d)
        {
            return strncmp(d + sz - _sz, psrc, _sz) == 0;
        }
        else
        {
            return false;
        }
    }
    cstring substr(size_t pos, size_t len = npos) const
    {
        return cstring(c_str(), pos, len);
    }
    
    //单一分隔符
    //如果分隔符未出现，返回vector唯一元素为原字符串
    //如果分隔符出现在末尾，vector最后一个元素为空字符串
    vector<cstring> split(const char c) const
    {
        vector<cstring> result;
        size_t lastpos = 0;
        for (size_t n = 0; n < sz; ++n)
        {
            if (d[n] == c)
            {
                result.push_back(substr(lastpos, n - lastpos));
                lastpos = n + 1;
            }
        }
        result.push_back(substr(lastpos));
        return result;
    }
    //TODO:补充测试用例
    vector<cstring> split(const char* fmt) const
    {
        vector<cstring> result;
        size_t lastpos = 0, fpos = npos;
        size_t fmtsz = _strnlen(fmt, SIZE_MAX);
        while (1)
        {
            fpos = find(fmt[0], lastpos);
            if (fpos == npos || fpos + fmtsz > sz)
                break;
            if (memcmp(d + fpos, fmt, fmtsz) == 0)
            {
                result.push_back(substr(lastpos, fpos - lastpos));
                lastpos = fpos + fmtsz;
            }
        }
        if (lastpos < sz)
        {
            result.push_back(substr(lastpos));
        }
        return result;
    }

    //按fmt中的n个字符顺序分隔出最多n+1个字符串（最后一个可以是空字符串）
    //注意和split的区别
    //"A,B=C"@",="->"A","B","C"
    //"A,B=C;"@",=;"->"A","B","C",""
    //"A,B=C"@",=;"->"A","B","C"
    //
    vector<cstring> split_cascade(const char* fmt) const
    {
        vector<cstring> result;
        size_t lastpos = 0;
        
        const char* pc = fmt, *cend = pc + _strnlen(fmt, SIZE_MAX);
        for (; pc < cend; ++pc)
        {
            size_t fpos = find(*pc, lastpos);
            if (fpos != npos)
            {
                result.push_back(substr(lastpos, fpos - lastpos));
                lastpos = fpos + 1;
            }
            else
            {
                result.push_back(substr(lastpos));
                return result;
            }
        }
        if (pc == cend)
        {
            result.push_back(substr(lastpos));
        }
        return result;
    }
    //splitRef,返回引用或者指针？
    
    
    //bool contains(const char c);
    //bool contains(const char* psrc);
    //find series
    size_t find(const char c, size_t pos = 0) const
    {
        if (!d)
            return npos;
        char* p = d + pos, *pend = d + sz;
        while (p < pend && *p != c)
            ++p;
        return (p == pend) ? (npos) : (p - d);
    }
    size_t find(const char* psrc, size_t pos = 0) const
    {
        //施工中
        if(!psrc || !d)
            return npos;
        
        const char *pd = d + pos, *pend = d + sz;
        size_t src_cnt = _strnlen(psrc, SIZE_MAX);
        size_t lastpos = pos;
        while (lastpos < sz)
        {
            size_t fpos = find(psrc[0], lastpos);
            if (fpos == npos) 
                return npos;//not found
            if (strncmp(d + fpos, psrc, src_cnt) == 0)
                return fpos;
            lastpos = fpos + 1;
            continue;
        }
        return npos;
    }
    
    //insert,erase,replace
    cstring& insert(size_t pos,const char* psrc)
    {
        size_t _sz = _strnlen(psrc, SIZE_MAX);
        if (pos >= sz)
        {
            _append(psrc, _sz);
        }
        else
        {
            //内存搬运还可以分段优化一下
            size_t sumsz = sz + _sz;
            if (sumsz > cap)
            {
                _reallocate(sumsz);
            }
            size_t backmovesz = sz - pos;
            memmove(d + pos + _sz, d + pos, backmovesz);
            memmove(d + pos, psrc, _sz);
            sz = sumsz;
            d[sz] = '\0';
        }
        return *this;
    }
    cstring& erase(size_t pos, size_t len=SIZE_MAX)
    {
        size_t erase_sz = std::min(sz - pos, len);
        if (pos > sz)
            return *this;
        if (sz - pos < len)
        {
            sz = pos;
            d[sz] = '\0';
        }
        else
        {
            size_t leftsz = sz - pos - len;
            memmove(d + pos, d + pos + len, leftsz);
            sz -= len;
            d[sz] = '\0';
        }
        return *this;
    }
    cstring& number(const int v)
    {
        static_assert(base_size >= 10, "string base size not enough for to_string");
        sprintf(d, "%d", v);
        sz = _strnlen(d,SIZE_MAX);
        return *this;
    }

    cstring& number(const float v)
    {
        static_assert(base_size >= 10, "string base size not enough for to_string");
        sprintf(d, "%f", v);
        sz = _strnlen(d, SIZE_MAX);
        return *this;
    }

protected:
    char* d = nullptr;
    size_t sz = 0;
    size_t cap = 0;
private :
    void _constructor()
    {
        d = (char*)operator new(base_size + 1);
        *d = '\0';
        sz = 0;
        cap = base_size;
    }

    size_t _strnlen(const char* psrc, size_t max) const
    {
        size_t n = 0;
        while (psrc[n] != '\0'&&n < max)
            ++n;
#if LITESTRING_STRLEN_WARNNING
        if (n > 10000)
        {
            printf("string warning:string len too long,you may forget padding a null terminator\n");
        }
#endif
        return n;
    }
    
    //搬运到符合空间要求的新内存区域
    void _reallocate(size_t n)
    {
        char* old = d;
        d = (char*)operator new(n + 1);
        if (old&&d)
        {
            memmove(d, old, sz + 1);
        }
        if (!d)
            operator delete(d);
        cap = n;
    }
    //内部函数,假设psrc,len都是合理的
    cstring& _append(const char* psrc, size_t len)
    {
        size_t sumsz = sz + len;
        if (sumsz > cap)
        {
            _reallocate(sumsz);
        }
        memcpy(d + sz, psrc, sumsz);
        sz = sumsz;
        d[sz] = '\0';
        return *this;
    }
public:
    //npos安排在最后避免调试时遮挡重要成员显示
//    static constexpr size_t npos = SIZE_MAX;
    cstring friend operator+(const char* psrc, const cstring& s)
    {
        cstring result(psrc);
        return result += s;
    }
    cstring friend operator+(const cstring& s,const char* psrc)
	{
		cstring result(s);
		return result += psrc;
	}
    template<typename T> static cstring to_string(const T& v);
};

    //no template specialization in class scope
    template<> cstring cstring::to_string<float>(const float& v)
    {
        return cstring().number(v);
    }
    template<> cstring cstring::to_string<int>(const int& v)
    {
        return cstring().number(v);
    }
    using argvec = vector<cstring>;
    argvec& operator<<(argvec& v, const cstring& s)
    {
        v.push_back(s);
        return v;
    }
}

