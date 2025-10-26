/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       DS18B20数字温度传感器实验
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
#include "my_spi.h"
#include "myiic.h"
#include "xl9555.h"
#include "spilcd.h"
#include "ds18b20.h"
#include <stdio.h>


/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;
    uint8_t t = 0;
    short temperature = 0;
    
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

    spilcd_show_string(30, 50, 200, 16, 16, "ESP32-S3", RED);
    spilcd_show_string(30, 70, 200, 16, 16, "DS18B20 TEST", RED);
    spilcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    
    while (ds18b20_init())      /* 初始化DS18B20数字温度传感器 */
    {
        spilcd_show_string(30, 110, 200, 16, 16, "DS18B20 Error", RED);
        vTaskDelay(pdMS_TO_TICKS(200));
        spilcd_fill(30, 110, 239, 126, WHITE);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    spilcd_show_string(30, 110, 200, 16, 16, "DS18B20 OK", RED);
    spilcd_show_string(30, 130, 200, 16, 16, "Temp: 00.0C", BLUE);

    while (1)
    {
        if (t % 10 == 0)    /* 每100ms读取一次温度 */
        {
            temperature = ds18b20_get_temperature();

            if (temperature < 0)
            {
                spilcd_show_char(70, 130, '-', 16, 0, BLUE);   /* 显示负号 */
                temperature = -temperature;                 /* 转为正数 */
            }
            else
            {
                spilcd_show_char(70, 130, ' ', 16, 0, BLUE);   /* 无符号 */
            }

            spilcd_show_num(78,  130, temperature / 10, 2, 16, BLUE);  /* 显示整数部分 */
            spilcd_show_num(102, 130, temperature % 10, 1, 16, BLUE);  /* 显示小数部分 */
        }

        vTaskDelay(pdMS_TO_TICKS(10));
        t++;

        if (t == 20)
        {
            t = 0;
            LED0_TOGGLE();
        }
    }
}
