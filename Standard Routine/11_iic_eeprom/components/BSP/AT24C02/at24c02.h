/**
 ****************************************************************************************************
 * @file        at24c02.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       AT24C02驱动代码
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
 
#ifndef __AT24C02_H
#define __AT24C02_H

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "myiic.h"
#include "string.h"

/* 24c02设备地址 */
#define AT_ADDR         0x50
#define AT24C02         255

/* 函数声明 */
esp_err_t at24c02_init(void);                                       /* 初始化IIC */
uint8_t at24c02_check(void);                                        /* 检查器件 */
uint8_t at24c02_read_one_byte(uint8_t addr);                       /* 指定地址读取一个字节 */
void at24c02_write_one_byte(uint8_t addr,uint8_t data);            /* 指定地址写入一个字节 */
void at24c02_write(uint8_t addr, uint8_t *pbuf, uint8_t datalen); /* 从指定地址开始写入指定长度的数据 */
void at24c02_read(uint8_t addr, uint8_t *pbuf, uint8_t datalen);  /* 从指定地址开始读出指定长度的数据 */

#endif
