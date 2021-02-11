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

/* defined the LED1 pin: PG13 */
#define LED1_PIN    GET_PIN(B, 0)

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
        rt_kprintf("ramfs mount succeed\n");
    }
    else
    {
        rt_kprintf("ramfs mount failed\n");
    }
    return 0;
}
INIT_ENV_EXPORT(ramfs_init);

#if 0
#include "cmain.h"
int cmain(void)
#else
int main(void)
#endif
{
    int count = 1;
    /* set LED1 pin mode to output */
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
    
    vcp = rt_device_find("vcom"); //find name by "list_device" in msh
    if(vcp != NULL)
    {
        rt_err_t status = rt_device_open(vcp, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
        if (status == RT_EOK)
            rt_device_set_rx_indicate(vcp, Serial_RxCallback);
    }
    
    while (count++)
    {
        rt_pin_write(LED1_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED1_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }

    return RT_EOK;
}
