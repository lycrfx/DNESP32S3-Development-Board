/**
 ****************************************************************************************************
 * @file        ap3216c.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       AP3216C驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 ESP32-S3 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */
 
#ifndef __AP3216C_H
#define __AP3216C_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "myiic.h"
#include "string.h"

/* 相关参数定义 */
#define AP3216C_INT     xl9555_pin_read(AP_INT_IO)                  /* 读取AP3216C中断状态 */
#define AP3216C_ADDR    0X1E                                        /* AP3216C器件地址 */

/* 函数声明 */
esp_err_t ap3216c_init(void);                                       /* 初始化AP3216C */
void ap3216c_read_data(uint16_t *ir, uint16_t *ps, uint16_t *als);  /* 读取AP3216C的数据 */

#endif
