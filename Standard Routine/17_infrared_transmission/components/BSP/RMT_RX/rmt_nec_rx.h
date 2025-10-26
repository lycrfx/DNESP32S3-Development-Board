/**
 ****************************************************************************************************
 * @file        rmt_nec_rx.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       RMT红外解码驱动代码
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

#ifndef __RMT_NEC_RX_H
#define __RMT_NEC_RX_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include "driver/rmt_rx.h"
#include "esp_err.h"

/* 引脚定义 */
#define RMT_IN_GPIO_PIN                 GPIO_NUM_2  /* 连接RMT_RX_IN的GPIO端口 */
#define RMT_RESOLUTION_HZ               1000000     /* 1MHz 频率, 1 tick = 1us */
#define RMT_NEC_DECODE_MARGIN           200         /* 判断NEC时序时长的容差值，小于（值+此值），大于（值-此值）为正确 */

/* NEC 协议时序时间，协议头9.5ms 4.5ms 逻辑0两个电平时长，逻辑1两个电平时长，重复码两个电平时长 */
#define NEC_LEADING_CODE_DURATION_0     9000
#define NEC_LEADING_CODE_DURATION_1     4500
#define NEC_PAYLOAD_ZERO_DURATION_0     560
#define NEC_PAYLOAD_ZERO_DURATION_1     560
#define NEC_PAYLOAD_ONE_DURATION_0      560
#define NEC_PAYLOAD_ONE_DURATION_1      1690
#define NEC_REPEAT_CODE_DURATION_0      9000
#define NEC_REPEAT_CODE_DURATION_1      2250

/* 外部调用 */
extern QueueHandle_t receive_queue;
extern rmt_channel_handle_t rx_channel;
extern rmt_symbol_word_t raw_symbols[64];
extern rmt_receive_config_t receive_config;
extern uint16_t s_nec_code_address;
extern uint16_t s_nec_code_command;

/* 函数声明 */
esp_err_t rmt_nec_rx_init(void);                                        /* RMT红外接收初始化 */
bool rmt_nec_parse_frame(rmt_symbol_word_t *rmt_nec_symbols);           /* 将RMT接收结果解码出NEC地址和命令 */
bool rmt_nec_parse_frame_repeat(rmt_symbol_word_t *rmt_nec_symbols);    /* 检查数据帧是否为重复按键 */

#endif
