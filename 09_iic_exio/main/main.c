/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       IO扩展实验
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
#include <stdio.h>


/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;
    uint8_t exio_key = 0;
    
    ret = nvs_flash_init();     /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    led_init();                 /* 初始化LED */
    myiic_init();               /* 初始化IIC0 */
    xl9555_init();              /* 初始化XL9555 */

    while(1)
    {
        exio_key = xl9555_key_scan(0);
        
        switch (exio_key)
        {
            case KEY0_PRES: /* 打开蜂鸣器 */
                xl9555_pin_write(BEEP_IO, 0);
                ESP_LOGI("mian","KEY0 has been pressed");
                break;
            case KEY1_PRES: /* 关闭蜂鸣器 */
                xl9555_pin_write(BEEP_IO, 1);
                ESP_LOGI("mian","KEY1 has been pressed");
                break;
            case KEY2_PRES: /* 打开LED */
                LED0(0);
                ESP_LOGI("mian","KEY2 has been pressed");
                break;
            case KEY3_PRES: /* 关闭LED */
                LED0(1);
                ESP_LOGI("mian","KEY3 has been pressed");
                break;
            default:
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
