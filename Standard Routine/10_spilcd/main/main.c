/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       SPILCD实验
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
#include "driver/spi_master.h"
#include "my_spi.h"
#include <stdio.h>

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    const uint8_t tx_data[] = {1, 2, 3};

    ESP_ERROR_CHECK(my_spi_init());    /* 初始化SPI总线 */

    spi_transaction_t t = {
        .length    = 8 * sizeof(tx_data), /* 一次发送3个字节(24位) */
        .tx_buffer = tx_data,
    };

    while (1)
    {
        ESP_ERROR_CHECK(spi_device_transmit(MY_SD_Handle, &t));
     //   printf("SPI sent: 1 2 3\r\n");
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
