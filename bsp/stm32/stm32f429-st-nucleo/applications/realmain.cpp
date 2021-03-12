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
#include "basic_utility.hpp"
#include "utility_test.hpp"

DigitalInOut green(GET_PIN(B, 0), PIN_MODE_OUTPUT, PIN_LOW);
DigitalInOut blue(GET_PIN(B, 7), PIN_MODE_OUTPUT, PIN_LOW);
DigitalInOut red(GET_PIN(B, 14), PIN_MODE_OUTPUT, PIN_LOW);
DigitalInOut &yellow = green;

void task1_func(void *)
{
	//init process
	while (1)
	{
		blue = !blue;
		delayms(600);
	}
}
Thread task1(task1_func, 0, 1024, '\025', 20, "task1");

rt_device_t vcp;
char rxbuf[128];
//echo data received back to vcp and print in console
rt_err_t Serial_RxCallback(rt_device_t vcp, rt_size_t len)
{
    rt_size_t rxsize = rt_device_read(vcp, 0, rxbuf, len);
    rt_size_t txsize = rt_device_write(vcp, 0, rxbuf, len);
    string s(rxbuf, 0, len);
    printf("received from vcp:%s\n", s.c_str());
    
    if (rxsize != txsize)
        printf("echo failed\n");
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
        LOG_I("ramfs mount succeed");
    }
    else
    {
        LOG_I("ramfs mount failed");
    }
    return 0;
}
INIT_ENV_EXPORT(ramfs_init);

volatile uint64_t stamp = 0;
rt_thread_t thread_main;
int main(void)
{
    task1.start();
    
    //find name by "list_device" in msh
    vcp = rt_device_find("vcom");
    if (vcp != NULL)
    {
        rt_err_t status = rt_device_open(vcp, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
        if (status == RT_EOK)
            rt_device_set_rx_indicate(vcp, Serial_RxCallback);
    }

    delayms(200);
    thread_main = rt_thread_find("main");
    printf("main thread stack_addr=0x%08X\n", (int)thread_main->stack_addr);
    
    
    delayms(100);
    int failbehavior = TESTFAIL_BREAKPOINT;
    utility::software_test(failbehavior);
    utility::hardware_test(failbehavior);
//    MConfGenerate_RM35();
//    BrushedDCM::Test::test();
    
    while (1)
    {
        green = !green;
        delayms(1000);
    }
}
