/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-08-26
 * @brief       SD卡实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 ESP32-S3 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "led.h"
#include "myiic.h"
#include "xl9555.h"
#include "my_spi.h"
#include "spilcd.h"
#include "spi_sd.h"
#include "sdmmc_cmd.h"
#include "exfuns.h"
#include <stdio.h>


extern sdmmc_card_t *card;  /* SD / MMC卡结构 */

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;
    size_t bytes_total, bytes_free;                     /* SD卡的总空间与剩余空间 */

    ret = nvs_flash_init();     /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    led_init();                 /* LED初始化 */
    my_spi_init();              /* SPI初始化 */
    myiic_init();               /* MYIIC初始化 */
    xl9555_init();              /* XL9555初始化 */
    spilcd_init();              /* SPILCD初始化 */

    spilcd_show_string(30, 50, 200, 16, 16, "ESP32-S3", RED);
    spilcd_show_string(30, 70, 200, 16, 16, "SD TEST", RED);
    spilcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

    while (sd_spi_init())       /* 检测不到SD卡 */
    {
        spilcd_show_string(30, 110, 200, 16, 16, "SD Card Error!", RED);
        vTaskDelay(500);
        spilcd_show_string(30, 130, 200, 16, 16, "Please Check! ", RED);
        vTaskDelay(500);
    }

    spilcd_show_string(30, 110, 200, 16, 16, "SD Card OK!", RED);
    spilcd_show_string(30, 130, 200, 16, 16, "Total:      MB", RED);
    spilcd_show_string(30, 150, 200, 16, 16, "Free :      MB", RED);

    sd_get_fatfs_usage(&bytes_total, &bytes_free);
    spilcd_show_num(80, 130,(int)bytes_total / 1024,5,16,BLUE);
    spilcd_show_num(80, 150,(int)bytes_free / 1024,5,16,BLUE);

    while(1)
    {
        LED0_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
