/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       SPIFFS实验
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
#include "my_spiffs.h"
#include <stdio.h>


#define WRITE_DATA              "ALIENTEK ESP32-S3"     /* 写入数据 */

/**
 * @brief       测试spiffs
 * @param       无
 * @retval      无
 */
void spiffs_test(void)
{
    ESP_LOGI("spiffs_test", "Opening file");
    
    FILE *file_obj = fopen("/spiffs/hello.txt", "w");   /* 建立一个名为/spiffs/hello.txt的只写文件 */
    if (file_obj == NULL)
    {
        ESP_LOGE("spiffs_test", "Failed to open file for writing");
    }

    fprintf(file_obj, WRITE_DATA);      /* 写入字符 */
    fclose(file_obj);                   /* 关闭文件 */
    ESP_LOGI("spiffs_test", "File written");

    /* 重命名之前检查目标文件是否存在 */
    struct stat st;
    if (stat("/spiffs/foo.txt", &st) == 0)  /* 获取文件信息，获取成功返回0 */
    {
        /*  从文件系统中删除一个名称。
            如果名称是文件的最后一个连接，并且没有其它进程将文件打开，
            名称对应的文件会实际被删除。 */
        unlink("/spiffs/foo.txt");
    }
 
    /* 重命名创建的文件 */
    ESP_LOGI("spiffs_test", "Renaming file");
    if (rename("/spiffs/hello.txt", "/spiffs/foo.txt") != 0)
    {
        ESP_LOGE("spiffs_test", "Rename failed");
    }
 
    /* 打开重命名的文件并读取 */
    ESP_LOGI("spiffs_test", "Reading file");
    file_obj = fopen("/spiffs/foo.txt", "r");
    if (file_obj == NULL)
    {
        ESP_LOGE("spiffs_test", "Failed to open file for reading");
    }

    char line[64];
    fgets(line, sizeof(line), file_obj);    /* 从指定的流中读取数据 */
    fclose(file_obj);
    
    char *pos = strchr(line, '\n'); /* 指针pos指向第一个找到‘\n’ */
    if (pos)
    {
        *pos = '\0';                /* 将‘\n’替换为‘\0’ */
    }

    ESP_LOGI("spiffs_test", "Read from file: '%s'", line);

    spilcd_show_string(110, 130, 200, 16, 16, line, BLUE);
}

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;
    
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
    ESP_ERROR_CHECK(spiffs_init("storage", DEFAULT_MOUNT_POINT, DEFAULT_FD_NUM));    /* SPIFFS初始化 */

    spilcd_show_string(30, 50,  200, 16, 16, "ESP32-S3", RED);
    spilcd_show_string(30, 70,  200, 16, 16, "SPIFFS TEST", RED);
    spilcd_show_string(30, 90,  200, 16, 16, "ATOM@ALIENTEK", RED);
    spilcd_show_string(30, 130, 200, 16, 16, "Read Text:", RED);

    spiffs_test();              /* SPIFFS测试 */

    while (1)
    {
        LED0_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
