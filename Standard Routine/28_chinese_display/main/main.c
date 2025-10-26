/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       汉字显示实验
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
#include "spi_sd.h"
#include "sdmmc_cmd.h"
#include "text.h"
#include "fonts.h"
#include "key.h"
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
    uint8_t key = 0;
    uint32_t fontcnt = 0;
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t fontx[2] = {0};

    ret = nvs_flash_init();     /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    led_init();                 /* LED初始化 */
    my_spi_init();              /* SPI初始化 */
    key_init();                 /* KEY初始化 */
    myiic_init();               /* MYIIC初始化 */
    xl9555_init();              /* XL9555初始化 */
    spilcd_init();              /* SPILCD初始化 */

    while (sd_spi_init())       /* 检测不到SD卡 */
    {
        spilcd_show_string(30, 110, 200, 16, 16, "SD Card Error!", RED);
        vTaskDelay(pdMS_TO_TICKS(500));
        spilcd_show_string(30, 130, 200, 16, 16, "Please Check! ", RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    while (fonts_init())        /* 检查字库 */
    {
UPD:
        spilcd_clear(WHITE);

        spilcd_show_string(30, 30, 200, 16, 16, "ESP32-S3", RED);
        
        key = fonts_update_font(30, 50, 16, (uint8_t *)"0:", RED);  /* 更新字库 */

        while (key)             /* 更新失败 */
        {
            spilcd_show_string(30, 50, 200, 16, 16, "Font Update Failed!", RED);
            vTaskDelay(pdMS_TO_TICKS(200));
            spilcd_fill(20, 50, 200 + 20, 90 + 16, WHITE);
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        spilcd_show_string(30, 50, 200, 16, 16, "Font Update Success!            ", RED);
        vTaskDelay(pdMS_TO_TICKS(1000));
        spilcd_clear(WHITE);
    }
   
    text_show_string(30, 30,  200, 16, "正点原子ESP32-S3开发板", 16, 0, RED);
    text_show_string(30, 50,  200, 16, "GBK字库测试程序", 16, 0, RED);
    text_show_string(30, 70,  200, 16, "ATOM@ALIENTEK", 16, 0, RED);
    text_show_string(30, 90,  200, 16, "BOOT: 更新字库", 16, 0, RED);
    
    text_show_string(30, 110, 200, 16, "内码高字节:", 16, 0, BLUE);
    text_show_string(30, 130, 200, 16, "内码低字节:", 16, 0, BLUE);
    text_show_string(30, 150, 200, 16, "汉字计数器:", 16, 0, BLUE);
    
    text_show_string(30, 180, 200, 24, "对应汉字为:", 24, 0, BLUE);
    text_show_string(30, 204, 200, 16, "对应汉字为:", 16, 0, BLUE);
    text_show_string(30, 220, 200, 12, "对应汉字为", 12, 0, BLUE);

    while (1)
    {
        fontcnt = 0;
        
        for (i = 0x81; i < 0xFF; i++)               /* GBK内码高字节范围为0x81~0xFE */
        {
            fontx[0] = i;
            spilcd_show_num(118, 110, i, 3, 16, BLUE); /* 显示内码高字节 */
            
            for (j = 0x40; j < 0xFE; j ++)          /* GBK内码低字节范围为0x40~0x7E、0x80~0xFE) */
            {
                if (j == 0x7F)
                {
                    continue;
                }
                
                fontcnt++;
                spilcd_show_num(118, 130, j, 3, 16, BLUE);        /* 显示内码低字节 */
                spilcd_show_num(118, 150, fontcnt, 5, 16, BLUE);  /* 汉字计数显示 */
                fontx[1] = j;
                text_show_font(30 + 132, 180, fontx, 24, 0, BLUE);
                text_show_font(30 + 144, 204, fontx, 16, 0, BLUE);
                text_show_font(30 + 108, 220, fontx, 12, 0, BLUE);
                
                t = 200;
                
                while ((t--) != 0)      /* 延时，同时扫描按键 */
                {
                    vTaskDelay(pdMS_TO_TICKS(1));
                    
                    key = key_scan(0);
                    
                    if (key == BOOT_PRES)
                    {
                        goto UPD;       /* 跳转到UPD位置（强制更新字库） */
                    }
                }
                
                LED0_TOGGLE();
            }
        }
    }
}