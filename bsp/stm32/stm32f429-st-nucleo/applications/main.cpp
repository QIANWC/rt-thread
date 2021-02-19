/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 * 2019-10-19     xuzhuoyi     add stm32f429-st-disco bsp
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <dfs.h>
#include <dfs_ramfs.h>
#include <dfs_fs.h>
#include <Thread.h>
using rtthread::Thread;

#include "DigitalInOut.h"
using mbed::DigitalInOut;
#include "basic_utility.h"

DigitalInOut green(GET_PIN(B, 0), PIN_MODE_OUTPUT, PIN_LOW), &yellow = green;
DigitalInOut blue(GET_PIN(B, 7), PIN_MODE_OUTPUT, PIN_LOW);
DigitalInOut red(GET_PIN(B, 14), PIN_MODE_OUTPUT, PIN_LOW);

void task1_func(void *)
{
	//init process
	while (1)
	{
		blue = !blue;
		msdelay(600);
	}
}
Thread task1(task1_func, 0, 1024, '\025', 20, "task1");

rt_device_t vcp;
char rxbuf[128];
rt_err_t Serial_RxCallback(rt_device_t vcp, rt_size_t len)
{
    rt_size_t rxsize = rt_device_read(vcp, 0, rxbuf, len);
    rt_size_t txsize = rt_device_write(vcp, 0, rxbuf, len);
    rt_kprintf("size rx=%d,tx=%d", rxsize, txsize);
//    return rxsize != txsize;
    return 0;
}

#define RAMFS_SIZE (2048)
uint8_t RamFS_Buf[RAMFS_SIZE];
int ramfs_init(void)
{
    struct dfs_ramfs *ramfs = dfs_ramfs_create(RamFS_Buf, RAMFS_SIZE);
    const char fsroot_str[] = "/";
    int res = dfs_mount(RT_NULL, fsroot_str, "ram", 0, ramfs);
    if (!res)
    {
        print("ramfs mount succeed\n");
    }
    else
    {
        print("ramfs mount failed\n");
    }
    return 0;
}
INIT_ENV_EXPORT(ramfs_init);

volatile timestamp_t stamp = 0;
rt_thread_t thread_main;
int main(void)
{
	task1.start();

    vcp = rt_device_find("vcom"); //find name by "list_device" in msh
    if(vcp != NULL)
    {
        rt_err_t status = rt_device_open(vcp, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
        if (status == RT_EOK)
            rt_device_set_rx_indicate(vcp, Serial_RxCallback);
    }

    thread_main = rt_thread_find("main");
    print("main thread stack_addr=0x%08X\n", thread_main->stack_addr);
    msdelay(500);
    while (1)
    {
        green = !green;
        msdelay(1000);
        stamp = us_tick();
        print("us_tick:0x%08X\n", us_tick());
        puts("std::printf\n");
        auto p = malloc(0x200);
        memset(p, 0xAA, 0x200);
//        printf("std::print\n");//还有问题
    }
}

#include <stdio.h>
#include <sys/stat.h>
 
extern "C"
{
    int _fstat(int fd, struct stat *pStat)
    {
        pStat->st_mode = S_IFCHR;
        return 0;
    }
 
    int _close(int)
    {
        return -1;
    }
 
    int _write(int fd, char *pBuffer, int size)
    {
        rt_device_t _console_device = rt_console_get_device();
        if (!pBuffer) return 0;
#ifdef RT_USING_DEVICE
        if (_console_device == RT_NULL)
        {
            rt_hw_console_output(pBuffer);
        }
        else
        {
            rt_uint16_t old_flag = _console_device->open_flag;

            _console_device->open_flag |= RT_DEVICE_FLAG_STREAM;
            rt_device_write(_console_device, 0, pBuffer, size);
            _console_device->open_flag = old_flag;
        }
#else
        rt_hw_console_output(str);
#endif

        return size;
    }
 
    int _isatty(int fd)
    {
        return 1;
    }
 
    int _lseek(int, int, int)
    {
        return -1;
    }
 
    int _read(int fd, char *pBuffer, int size)
    {
        //...
//        for (int i = 0; i < size; i++)
//        {
//            while ((USART1->SR & USART_SR_RXNE) == 0)
//            {
//            }
// 
//            pBuffer[i] = USART_ReceiveData(USART1);
//        }
        return size;
    }
 
    char* _sbrk(int increment)
    {
        extern char end asm("end");
        register char *pStack asm("sp");
 
        static char *s_pHeapEnd;
 
        if (!s_pHeapEnd)
            s_pHeapEnd = &end;
 
        if (s_pHeapEnd + increment > pStack)
            return (char*) - 1;
 
        char *pOldHeapEnd = s_pHeapEnd;
        s_pHeapEnd += increment;
        return (char*)pOldHeapEnd;
    }
}
