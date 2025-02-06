/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       RMT TX实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * @attention
 * 
 * 实验平台:正点原子 ESP32-S3 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 ******************************************************************************
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "led.h"
#include "myiic.h"
#include "xl9555.h"
#include "my_spi.h"
#include "spilcd.h"
#include "rmt_nec_rx.h"
#include "rmt_nec_tx.h"
#include <stdio.h>


uint8_t tbuf[40];
const char *TAG = "rmt_rx";

/**
 * @brief       根据NEC编码解析红外协议并打印指令结果
 * @param       rmt_nec_symbols : 数据帧
 * @param       symbol_num      : 数据帧大小
 * @retval      无
 */
void rmt_rx_scan(rmt_symbol_word_t *rmt_nec_symbols, size_t symbol_num)
{
    switch (symbol_num)     /* 解码RMT接收数据 */
    {
        case 34:            /* 正常NEC数据帧 */
        {
            if (rmt_nec_parse_frame(rmt_nec_symbols) )
            {
                spilcd_fill(110, 130, 200, 150, WHITE);
                sprintf((char *)tbuf, "%d", s_nec_code_command);
                ESP_LOGI(TAG,"RX KEYCNT = %d", s_nec_code_command);
                spilcd_show_string(110, 130, 200, 16, 16, (char *)tbuf, BLUE);
            }
            break;
        }
        case 2:         /* 重复NEC数据帧 */
        {
            if (rmt_nec_parse_frame_repeat(rmt_nec_symbols))
            {
                ESP_LOGI(TAG,"RX KEYCNT = %d, repeat", s_nec_code_command);
            }
            break;
        }
        default:        /* 未知NEC数据帧 */
        {
            ESP_LOGI(TAG,"Unknown NEC frame");
            break;
        }
    }
}

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;
    uint8_t t = 0;
    rmt_rx_done_event_data_t rx_data;

    ret = nvs_flash_init();     /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    led_init();                 /* LED初始化 */
    my_spi_init();              /* SPI初始化 */
    myiic_init();               /* IIC初始化 */
    xl9555_init();              /* 初始化按键 */
    spilcd_init();              /* LCD屏初始化 */
    rmt_nec_rx_init();          /* 红外接收器件初始化 */
    rmt_nec_tx_init();          /* 红外发送器件初始化 */

    spilcd_show_string(30,  50, 200, 16, 16, "ESP32-S3", RED);
    spilcd_show_string(30,  70, 200, 16, 16, "RMT TX TEST", RED);
    spilcd_show_string(30,  90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    spilcd_show_string(30, 110, 200, 16, 16, "TX KEYVAL:", RED);
    spilcd_show_string(30, 130, 200, 16, 16, "RX KEYVAL:", RED);

    while (1)
    {
        if (xQueueReceive(receive_queue, &rx_data, pdMS_TO_TICKS(1000)) == pdPASS)
        {
            rmt_rx_scan(rx_data.received_symbols, rx_data.num_symbols);                                     /* 解析接收符号并打印结果 */

            ESP_ERROR_CHECK(rmt_receive(rx_channel, raw_symbols, sizeof(raw_symbols), &receive_config));    /* 重新开始接收 */
        }
        else    /* 超时，没有消息到消息队列，传输预定义的IR NEC数据包 */
        {
            t++;

            if (t == 0)
            {
                t = 1;
            }

            const ir_nec_scan_code_t scan_code = {
                .command = t,
            };

            spilcd_fill(110, 110, 200, 150, WHITE);
            sprintf((char *)tbuf, "%d", scan_code.command);
            ESP_LOGI(TAG, "TX KEYVAL = %d", scan_code.command);
            spilcd_show_string(110, 110, 200, 16, 16, (char *)tbuf, BLUE);
            ESP_ERROR_CHECK(rmt_transmit(tx_channel, nec_encoder, &scan_code, sizeof(scan_code), &transmit_config));    /* 通过RMT发送信道传输数据 */
        }
    
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
