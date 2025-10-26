/**
 ****************************************************************************************************
 * @file        dht11.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       DHT11数字温湿度传感器驱动代码
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

#ifndef __DHT11_H
#define __DHT11_H

#include "driver/gpio.h" 
#include "esp_log.h"

/* 引脚定义 */
#define DHT11_DQ_GPIO_PIN       GPIO_NUM_0

/* IO操作 */
#define DHT11_DQ_IN             gpio_get_level(DHT11_DQ_GPIO_PIN)

/* DHT11端口定义 */
#define DHT11_DQ_OUT(x)         do { x ?                                   \
                                     gpio_set_level(DHT11_DQ_GPIO_PIN, 1): \
                                     gpio_set_level(DHT11_DQ_GPIO_PIN, 0); \
                                   } while(0)

/* 函数声明 */
void dht11_reset(void);                                 /* 复位DHT11 */
uint8_t dht11_init(void);                               /* 初始化DHT11 */
uint8_t dht11_check(void);                              /* 等待DHT11的回应 */
uint8_t dht11_read_data(short *temp, short *humi);      /* 读取温湿度 */

#endif
