/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       LED实验
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
#include "led.h"
#include "esp_log.h"

static const char *TAG = "MAIN";


/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;
    ESP_LOGI(TAG, "Application start");

    ret = nvs_flash_init();     /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_LOGI(TAG, "NVS initialized");

    led_init();                 /* 初始化LED */
    ESP_LOGI(TAG, "LED initialized");

    int cnt = 0;
    while(1)
    {
        LED0_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(200));    /* 延时100ms */
        cnt++;
        /* 每秒打印一次运行提示，避免串口被大量日志淹没 */
        if (cnt >= 10) {
            ESP_LOGI(TAG, "LED toggled %d times", cnt);
            cnt = 0;
        }
    }
}
