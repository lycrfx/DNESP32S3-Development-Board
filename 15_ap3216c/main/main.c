/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       光环境传感器实验
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
#include "spilcd.h"
#include "ap3216c.h"
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
    uint16_t ir, als, ps;

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
    spilcd_show_string(30, 70, 200, 16, 16, "AP3216C TEST", RED);
    spilcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

    while (ap3216c_init())  /* 初始化AP3216C */
    {
        spilcd_show_string(30, 110, 200, 16, 16, "AP3216C Check Failed!", RED);
        vTaskDelay(pdMS_TO_TICKS(200));
        spilcd_fill(30, 110, 239, 126, WHITE);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    spilcd_show_string(30, 110, 200, 16, 16, "AP3216C Ready!", RED);
    spilcd_show_string(30, 130, 200, 16, 16, " IR:", RED);
    spilcd_show_string(30, 160, 200, 16, 16, " PS:", RED);
    spilcd_show_string(30, 190, 200, 16, 16, "ALS:", RED);

    while(1)
    {
        ap3216c_read_data(&ir, &ps, &als);              /* 读取数据  */
        spilcd_show_num(62, 130, ir,  5, 16, BLUE);     /* 显示IR数据 */
        spilcd_show_num(62, 160, ps,  5, 16, BLUE);     /* 显示PS数据 */
        spilcd_show_num(62, 190, als, 5, 16, BLUE);     /* 显示ALS数据  */

        LED0_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
