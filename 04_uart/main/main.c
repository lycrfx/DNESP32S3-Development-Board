/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       UART实验
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
#include <stdio.h>
#include <string.h>
#include "led.h"
#include "uart.h"


/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;
    uint16_t len = 0;
    char data[10] = {0};
    char *a = "LED_ON";
    char *b = "LED_OFF";

    ret = nvs_flash_init();         /* 初始化NVS */

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    led_init();                     /* 初始化LED */
    uart0_init(115200);             /* 初始化串口0 */

    while(1)
    {
        uart_get_buffered_data_len(USART_UX, (size_t*) &len);

        if (len > 0)
        {
            uart_read_bytes(USART_UX, data, len, 100);
            data[len] = '\0';

            if (strcmp(a, data) == 0)
            {
                LED0(0);
            }
            else if (strcmp(b, data) == 0)
            {
                LED0(1);
            }

            memset(data, 0, 10);
        }

        vTaskDelay(pdMS_TO_TICKS(10));    /* 延时10ms */
    }
}
