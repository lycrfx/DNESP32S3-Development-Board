/**
 ****************************************************************************************************
 * @file        uart.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       UART0驱动代码
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

#ifndef __UART_H
#define __UART_H

#include "driver/uart.h"
#include "driver/uart_select.h"
#include "driver/gpio.h"

/* 引脚和串口定义 */
#define USART_UX            UART_NUM_0
#define USART_TX_GPIO_PIN   GPIO_NUM_43
#define USART_RX_GPIO_PIN   GPIO_NUM_44

/* 串口接收相关定义 */
#define RX_BUF_SIZE         1024        /* 环形缓冲区大小(单位字节) */

/* 函数声明 */
void uart0_init(uint32_t baudrate);

#endif