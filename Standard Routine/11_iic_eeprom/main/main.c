/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       EEPROM实验
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
#include "at24c02.h"
#include "myiic.h"
#include "xl9555.h"
#include <stdio.h>


const uint8_t g_text_buf[] = {"ESP32-S3 IIC TEST"};     /* 要写入到24c02的字符串数组 */
#define TEXT_SIZE   sizeof(g_text_buf)                  /* TEXT字符串长度 */

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;
    uint8_t key = 0;
    uint16_t i = 0;
    uint8_t datatemp[TEXT_SIZE];

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
    at24c02_init();             /* 初始化24CXX */
    spilcd_init();              /* LCD屏初始化 */

    spilcd_show_string(30, 50,  200, 16, 16, "ESP32-S3", RED);
    spilcd_show_string(30, 70,  200, 16, 16, "EEPROM TEST", RED);
    spilcd_show_string(30, 90,  200, 16, 16, "ATOM@ALIENTEK", RED);
    spilcd_show_string(30, 110, 200, 16, 16, "KEY1:Write  KEY0:Read", RED);

    while (at24c02_check())     /* 检测不到24c02 */
    {
        spilcd_show_string(30, 130, 200, 16, 16, "24C02 Check Failed!", RED);
        vTaskDelay(pdMS_TO_TICKS(500));
        spilcd_show_string(30, 130, 200, 16, 16, "Please Check!      ", RED);
        vTaskDelay(pdMS_TO_TICKS(500));
        LED0_TOGGLE();          
    }

    spilcd_show_string(30, 130, 200, 16, 16, "24C02 Ready!", RED);

    while (1)
    {
        key = xl9555_key_scan(0);   /* 按键扫描  */

        if (key == KEY1_PRES)       /* KEY1按下,写入24C02 */
        {                                /* 清除数据显示行 */
            spilcd_show_string(30, 150, 200, 16, 16, "Start Write 24C02....", BLUE);
            at24c02_write(0, (uint8_t *)g_text_buf, TEXT_SIZE);                     /* 写数据到24C02 */
            spilcd_show_string(30, 150, 200, 16, 16, "24C02 Write Finished!", BLUE);   
        }

        if (key == KEY0_PRES)       /* KEY0按下,读取字符串并显示 */
        {
            spilcd_show_string(30, 150, 200, 16, 16, "Start Read 24C02.... ", BLUE);
            at24c02_read(0, datatemp, TEXT_SIZE);                                   /* 从24C02读取数据 */
            spilcd_show_string(30, 150, 200, 16, 16, "The Data Readed Is:  ", BLUE);   
            spilcd_show_string(30, 170, 200, 16, 16, (char *)datatemp, BLUE);          /* 显示读到的字符串 */
        }

        i++;

        if (i == 20)
        {
            LED0_TOGGLE();
            i = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
