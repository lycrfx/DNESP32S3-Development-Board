/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       内部温度传感器实验
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
#include "sensor.h"
#include <stdio.h>


/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;
    float tsens_temperature = 0;
    uint32_t data_tmp = 0;
    
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
    sensor_inter_init();        /* 温度传感器初始化 */

    spilcd_show_string(30, 50,  200, 16, 16, "ESP32-S3", RED);
    spilcd_show_string(30, 70,  200, 16, 16, "Temperature TEST", RED);
    spilcd_show_string(30, 90,  200, 16, 16, "ATOM@ALIENTEK", RED);
    spilcd_show_string(30, 110, 200, 16, 16, "Temperature: 00.00C", BLUE);

    while (1)
    {
        tsens_temperature = sensor_inter_get_temperature();     /* 得到温度值 */

        if (tsens_temperature < 0)
        {
            tsens_temperature = -tsens_temperature;
            spilcd_show_char(126, 110, '-', 16, 0, BLUE);          /* 显示负符号 */    
        }
        else
        {
            spilcd_show_char(126, 110, ' ', 16, 0, BLUE);          /* 无符号 */
        }

        data_tmp = (uint32_t)tsens_temperature;                 /* 取整数部分 */
        spilcd_show_num(134, 110, data_tmp, 2, 16, BLUE);          /* 显示整数部分 */ 
        data_tmp = (tsens_temperature - data_tmp) * 100;        /* 取2位小数部分 */
        spilcd_show_xnum(158, 110, data_tmp, 2, 16, 0x80, BLUE);   /* 显示小数部分 */
        
        LED0_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}