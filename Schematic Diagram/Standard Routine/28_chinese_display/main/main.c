/**
 ******************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       ������ʾʵ��
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ******************************************************************************
 * @attention
 * 
 * ʵ��ƽ̨:����ԭ�� ESP32-S3 ������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
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
 * @brief       �������
 * @param       ��
 * @retval      ��
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

    ret = nvs_flash_init();     /* ��ʼ��NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    led_init();                 /* LED��ʼ�� */
    my_spi_init();              /* SPI��ʼ�� */
    key_init();                 /* KEY��ʼ�� */
    myiic_init();               /* MYIIC��ʼ�� */
    xl9555_init();              /* XL9555��ʼ�� */
    spilcd_init();              /* SPILCD��ʼ�� */

    while (sd_spi_init())       /* ��ⲻ��SD�� */
    {
        spilcd_show_string(30, 110, 200, 16, 16, "SD Card Error!", RED);
        vTaskDelay(pdMS_TO_TICKS(500));
        spilcd_show_string(30, 130, 200, 16, 16, "Please Check! ", RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    while (fonts_init())        /* ����ֿ� */
    {
UPD:
        spilcd_clear(WHITE);

        spilcd_show_string(30, 30, 200, 16, 16, "ESP32-S3", RED);
        
        key = fonts_update_font(30, 50, 16, (uint8_t *)"0:", RED);  /* �����ֿ� */

        while (key)             /* ����ʧ�� */
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
   
    text_show_string(30, 30,  200, 16, "����ԭ��ESP32-S3������", 16, 0, RED);
    text_show_string(30, 50,  200, 16, "GBK�ֿ���Գ���", 16, 0, RED);
    text_show_string(30, 70,  200, 16, "ATOM@ALIENTEK", 16, 0, RED);
    text_show_string(30, 90,  200, 16, "BOOT: �����ֿ�", 16, 0, RED);
    
    text_show_string(30, 110, 200, 16, "������ֽ�:", 16, 0, BLUE);
    text_show_string(30, 130, 200, 16, "������ֽ�:", 16, 0, BLUE);
    text_show_string(30, 150, 200, 16, "���ּ�����:", 16, 0, BLUE);
    
    text_show_string(30, 180, 200, 24, "��Ӧ����Ϊ:", 24, 0, BLUE);
    text_show_string(30, 204, 200, 16, "��Ӧ����Ϊ:", 16, 0, BLUE);
    text_show_string(30, 220, 200, 12, "��Ӧ����Ϊ", 12, 0, BLUE);

    while (1)
    {
        fontcnt = 0;
        
        for (i = 0x81; i < 0xFF; i++)               /* GBK������ֽڷ�ΧΪ0x81~0xFE */
        {
            fontx[0] = i;
            spilcd_show_num(118, 110, i, 3, 16, BLUE); /* ��ʾ������ֽ� */
            
            for (j = 0x40; j < 0xFE; j ++)          /* GBK������ֽڷ�ΧΪ0x40~0x7E��0x80~0xFE) */
            {
                if (j == 0x7F)
                {
                    continue;
                }
                
                fontcnt++;
                spilcd_show_num(118, 130, j, 3, 16, BLUE);        /* ��ʾ������ֽ� */
                spilcd_show_num(118, 150, fontcnt, 5, 16, BLUE);  /* ���ּ�����ʾ */
                fontx[1] = j;
                text_show_font(30 + 132, 180, fontx, 24, 0, BLUE);
                text_show_font(30 + 144, 204, fontx, 16, 0, BLUE);
                text_show_font(30 + 108, 220, fontx, 12, 0, BLUE);
                
                t = 200;
                
                while ((t--) != 0)      /* ��ʱ��ͬʱɨ�谴�� */
                {
                    vTaskDelay(pdMS_TO_TICKS(1));
                    
                    key = key_scan(0);
                    
                    if (key == BOOT_PRES)
                    {
                        goto UPD;       /* ��ת��UPDλ�ã�ǿ�Ƹ����ֿ⣩ */
                    }
                }
                
                LED0_TOGGLE();
            }
        }
    }
}