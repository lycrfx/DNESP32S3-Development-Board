/**
 ****************************************************************************************************
 * @file        rmt_nec_tx.c
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

#include "rmt_nec_tx.h"


rmt_encoder_handle_t nec_encoder;
rmt_transmit_config_t transmit_config;
rmt_channel_handle_t tx_channel;

/**
 * @brief       RMT红外发送初始化
 * @param       无
 * @retval      ESP_OK:初始化成功
 */
esp_err_t rmt_nec_tx_init(void)
{
    /* 配置发送通道 */
    rmt_tx_channel_config_t tx_channel_cfg = {
        .gpio_num           = RMT_TX_PIN,               /* RMT发送通道引脚 */
        .clk_src            = RMT_CLK_SRC_DEFAULT,      /* RMT发送通道时钟源 */
        .resolution_hz      = RMT_TX_HZ,                /* RMT发送通道时钟分辨率 */
        .mem_block_symbols  = 64,                       /* 通道一次可以存储的RMT符号数量 */
        .trans_queue_depth  = 4,                        /* 允许在后台挂起的事务数，本例不会对多个事务进行排队，因此队列深度>1就足够了 */
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_channel_cfg, &tx_channel));  /* 创建一个RMT发送通道 */

    /* 配置载波与占空比 */
    rmt_carrier_config_t carrier_cfg = {
        .frequency_hz = 38000,                                          /* 载波频率，0表示禁用载波 */
        .duty_cycle = 0.33,                                             /* 载波占空比33% */
    };
    ESP_ERROR_CHECK(rmt_apply_carrier(tx_channel, &carrier_cfg));       /* 对发送信道应用调制功能 */

    /* 不会在循环中发送NEC帧 */
    transmit_config.loop_count = 0;                                     /* 0为不循环，-1为无限循环 */

    /* 配置编码器 */
    ir_nec_encoder_config_t nec_encoder_cfg = {
        .resolution = RMT_TX_HZ,                                        /* 编码器分辨率 */
    };
    ESP_ERROR_CHECK(rmt_new_ir_nec_encoder(&nec_encoder_cfg, &nec_encoder));    /* 配置编码器 */

    ESP_ERROR_CHECK(rmt_enable(tx_channel));                            /* 使能发送通道 */

    return ESP_OK;
}