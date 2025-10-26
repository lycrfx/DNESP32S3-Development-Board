/**
 ****************************************************************************************************
 * @file        sensor.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       内部温度传感器驱动代码
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

#ifndef __SENSOR_H
#define __SENSOR_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/temperature_sensor.h"


/* 函数声明 */
esp_err_t sensor_inter_init(void);          /* 初始化内部温度传感器 */
float sensor_inter_get_temperature(void);   /* 获取内部温度传感器温度值 */

#endif