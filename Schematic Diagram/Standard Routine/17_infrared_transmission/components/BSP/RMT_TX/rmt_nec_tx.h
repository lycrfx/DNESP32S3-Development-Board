/**
 ****************************************************************************************************
 * @file        rmt_nec_tx.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       RMT红外发射驱动代码
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

#ifndef __RMT_NEC_TX_H
#define __RMT_NEC_TX_H

#include "driver/rmt_tx.h"
#include "ir_nec_encoder.h"

/* 引脚定义 */
#define RMT_TX_PIN                  GPIO_NUM_8      /* 连接RMT_TX_IN的GPIO端口 */
#define RMT_TX_HZ                   1000000         /* 1MHz 频率, 1 tick = 1us */

/* 外部调用 */
extern rmt_encoder_handle_t nec_encoder;
extern rmt_transmit_config_t transmit_config;
extern rmt_channel_handle_t tx_channel;

/* 函数声明 */
esp_err_t rmt_nec_tx_init(void);

#endif
