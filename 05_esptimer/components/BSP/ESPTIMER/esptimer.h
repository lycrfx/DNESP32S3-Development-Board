/**
 ****************************************************************************************************
 * @file        esptim.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       高分辨率定时器（ESP定时器）驱动代码
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

#ifndef __ESPTIMER_H
#define __ESPTIMER_H

#include "esp_timer.h"


/* 函数声明 */
void esptimer_init(uint64_t tps);   /* 初始化高分辨率定时器 */

#endif
