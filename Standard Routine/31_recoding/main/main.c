/**
 ******************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       ���ֲ�����ʵ��
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
#include "my_spi.h"
#include "led.h"
#include "myiic.h"
#include "xl9555.h"
#include "spilcd.h"
#include "spi_sd.h"
#include "sdmmc_cmd.h"
#include "text.h"
#include "fonts.h"
#include "key.h"
#include "text.h"
#include "fonts.h"
#include "es8388.h"
#include "myi2s.h"
#include "exfuns.h"
#include "audioplay.h"
#include "recorder.h"
#include <stdio.h>


/**
 * @brief       �������
 * @param       ��
 * @retval      ��
 */
void app_main(void)
{
    esp_err_t ret;
    uint8_t key = 0;

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

    while (es8388_init())       /* ES8388��ʼ�� */
    {
        spilcd_show_string(30, 110, 200, 16, 16, "ES8388 Error", RED);
        vTaskDelay(pdMS_TO_TICKS(200));
        spilcd_fill(30, 110, 239, 126, WHITE);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    xl9555_pin_write(SPK_EN_IO, 0);     /* ������ */

    while (sd_spi_init())       /* ��ⲻ��SD�� */
    {
        spilcd_show_string(30, 110, 200, 16, 16, "SD Card Error!", RED);
        vTaskDelay(pdMS_TO_TICKS(500));
        spilcd_show_string(30, 130, 200, 16, 16, "Please Check! ", RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    ret = exfuns_init();    /* Ϊfatfs��ر��������ڴ� */

    while (fonts_init())    /* ����ֿ� */
    {
        spilcd_clear(WHITE);
        spilcd_show_string(30, 30, 200, 16, 16, "ESP32-S3", RED);
        
        key = fonts_update_font(30, 50, 16, (uint8_t *)"0:", RED);  /* �����ֿ� */
        
        while (key)         /* ����ʧ�� */
        {
            spilcd_show_string(30, 50, 200, 16, 16, "Font Update Failed!", RED);
            vTaskDelay(pdMS_TO_TICKS(200));
            spilcd_fill(20, 50, 200 + 20, 90 + 16, WHITE);
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        spilcd_show_string(30, 50, 200, 16, 16, "Font Update Success!   ", RED);
        vTaskDelay(pdMS_TO_TICKS(1000));
        spilcd_clear(WHITE);   
    }

    text_show_string(30, 50, 200, 16, "����ԭ��ESP32-S3������", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "WAV ¼���� ʵ��", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "����ԭ��@ALIENTEK", 16, 0, RED);

    vTaskDelay(pdMS_TO_TICKS(1000));

    while (1)
    {
        wav_recorder();       /* ¼�� */
    }
}