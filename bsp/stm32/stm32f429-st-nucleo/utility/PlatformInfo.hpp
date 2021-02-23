#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

namespace PlatformInfo
{
    constexpr uint8_t sizeof_char = sizeof(char);
    constexpr uint8_t sizeof_short = sizeof(short);
    constexpr uint8_t sizeof_int = sizeof(int);
    constexpr uint8_t sizeof_long = sizeof(long);
    constexpr uint8_t sizeof_longlong = sizeof(long long);
    constexpr uint8_t sizeof_float = sizeof(float);
    constexpr uint8_t sizeof_double = sizeof(double);
    
    constexpr uint8_t sizeof_ptr = sizeof(int *);
    
    constexpr bool ischar_signed = ((int)(char)(0x80)) < 0;
    constexpr bool isschar_signed = ((int)(signed char)(0x80)) < 0;
    
    constexpr uint16_t ByteOrderMark = 0xFEFF;
    constexpr uint8_t ByteOrderMark_u8_little[] = { 0xFF, 0xFE };
    constexpr bool isendian_little = ((ByteOrderMark & 0x00FF) == ByteOrderMark_u8_little[0]);
    inline constexpr bool IsCTypeStandard(void)
    {
        return 
            (sizeof_char == 1)&&\
            (sizeof_short == 2)&&\
            (sizeof_int == 4)&&\
            (sizeof_long == 4)&&\
            (sizeof_longlong == 8)&&\
            (sizeof_float == 4)&&\
            (sizeof_double == 8)&&\
            (sizeof_ptr == 4)&&\
            (ischar_signed == false)&&\
            (isschar_signed == true)&&\
            (isendian_little == true);
        //ByteOrder Identify method need review
    }
    
    void GetChipInfo(void)
    {
        const uint32_t Addr_CortexM_CPUID = 0xE000ED00;
        uint32_t cpuid = *(uint32_t*)Addr_CortexM_CPUID;
    }
    
    namespace Test
    {
        int test(bool failstop = true)
        {
            printf("Platform Test:\n");
            int retval = 0;
            if (PlatformInfo::IsCTypeStandard())
            {
                printf("platform use standard type\n");
                retval = 0;
            }
            else
            {
                printf("[warning]platform type is not standard\n");
                retval = -1;
            }
            return retval;
        }
    }
}
