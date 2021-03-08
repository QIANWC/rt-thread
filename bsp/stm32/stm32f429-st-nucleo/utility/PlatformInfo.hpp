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
    
    //TODO:处理器架构、版本，芯片ID等信息
    inline uint32_t GetChipInfo(void)
    {
        //TODO:目前只核对了F4的地址
#if defined STM32F0|defined STM32F1|defined STM32F3|defined STM32F4|defined STM32G4|defined STM32H7
        const uint32_t Addr_CortexM_CPUID = 0xE000ED00;
        uint32_t cpuid = *(uint32_t*)Addr_CortexM_CPUID;
        return cpuid;
#else
        return 0;
#endif
    }
}
