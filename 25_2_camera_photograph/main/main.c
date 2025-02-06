/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       SPILCD实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * @attention
 * 
 * 实验平台:正点原子 ESP32-S3开发板
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
#include "my_spi.h"
#include "xl9555.h"
#include "spilcd.h"
#include "esp_camera.h"
#include "spi_sd.h"
#include "sdmmc_cmd.h"
#include "jpegd2.h"
#include "exfuns.h"
#include <stdio.h>

uint8_t *rgb565 = NULL;
uint8_t sd_check_en = 0;                    /* sd卡检测标志 */
extern sdmmc_card_t *card;  /* SD / MMC卡结构 */
camera_fb_t *fb = NULL;
static const char *TAG = "main";
/* TASK3 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define TASK3_PRIO      4                   /* 任务优先级 */
#define TASK3_STK_SIZE  5*1024              /* 任务堆栈大小 */
TaskHandle_t            Task3Task_Handler;  /* 任务句柄 */
void task3(void *pvParameters);             /* 任务函数 */
SemaphoreHandle_t BinarySemaphore;          /* 二值信号量句柄 */

/* 引脚配置 */
#define CAM_PIN_PWDN    GPIO_NUM_NC
#define CAM_PIN_RESET   GPIO_NUM_NC
#define CAM_PIN_VSYNC   GPIO_NUM_47
#define CAM_PIN_HREF    GPIO_NUM_48
#define CAM_PIN_PCLK    GPIO_NUM_45
#define CAM_PIN_XCLK    GPIO_NUM_NC
#define CAM_PIN_SIOD    GPIO_NUM_39
#define CAM_PIN_SIOC    GPIO_NUM_38
#define CAM_PIN_D0      GPIO_NUM_4
#define CAM_PIN_D1      GPIO_NUM_5
#define CAM_PIN_D2      GPIO_NUM_6
#define CAM_PIN_D3      GPIO_NUM_7
#define CAM_PIN_D4      GPIO_NUM_15
#define CAM_PIN_D5      GPIO_NUM_16
#define CAM_PIN_D6      GPIO_NUM_17
#define CAM_PIN_D7      GPIO_NUM_18


#define CAM_PWDN(x)         do{ x ? \
                                (xl9555_pin_write(OV_PWDN_IO, 1)):       \
                                (xl9555_pin_write(OV_PWDN_IO, 0));       \
                            }while(0)

#define CAM_RST(x)          do{ x ? \
                                (xl9555_pin_write(OV_RESET_IO, 1)):       \
                                (xl9555_pin_write(OV_RESET_IO, 0));       \
                            }while(0)

/* 摄像头配置 */
static camera_config_t camera_config = {
    /* 引脚配置 */
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    /* XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental) */
    .xclk_freq_hz = 24000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,     /* YUV422,GRAYSCALE,RGB565,JPEG */
    .frame_size = FRAMESIZE_QVGA,       /* QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates */

    .jpeg_quality = 12,                 /* 0-63, for OV series camera sensors, lower number means higher quality */
    .fb_count = 2,                      /* When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode */
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

/**
 * @brief       摄像头初始化
 * @param       无
 * @retval      esp_err_t
 */
static esp_err_t init_camera(void)
{
    if (CAM_PIN_PWDN == GPIO_NUM_NC)
    {
        CAM_PWDN(0);
    } 

    if (CAM_PIN_RESET == GPIO_NUM_NC)
    { 
        CAM_RST(0);
        vTaskDelay(pdMS_TO_TICKS(20));
        CAM_RST(1);
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    /* 摄像头初始化 */
    esp_err_t err = esp_camera_init(&camera_config);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    sensor_t * s = esp_camera_sensor_get();

    /* 如果摄像头模块是OV3660或者是OV5640，则需要以下配置 */
    if (s->id.PID == OV3660_PID)
    {
        s->set_vflip(s, 1);         /* 向后翻转 */
        s->set_brightness(s, 1);    /* 亮度提高 */
        s->set_saturation(s, -2);   /* 降低饱和度 */
    }
    else if (s->id.PID == OV5640_PID)
    {
        s->set_vflip(s, 1);         /* 向后翻转 */
    }

    return ESP_OK;
}

/**
 * @brief       得到path路径下,目标文件的总个数
 * @param       path : 路径
 * @retval      总有效文件数
 */
uint16_t pic_get_tnum(char *path)
{
    uint8_t res;
    uint16_t rval = 0;
    FF_DIR tdir;                                    /* 临时目录 */
    FILINFO *tfileinfo;                             /* 临时文件信息 */
    tfileinfo = (FILINFO *)malloc(sizeof(FILINFO)); /* 申请内存 */
    res = f_opendir(&tdir, (const TCHAR *)path);    /* 打开目录 */

    if (res == FR_OK && tfileinfo)
    {
        while (1)                                   /* 查询总的有效文件数 */
        {
            res = f_readdir(&tdir, tfileinfo);      /* 读取目录下的一个文件 */

            if (res != FR_OK || tfileinfo->fname[0] == 0)break; /* 错误了/到末尾了,退出 */
            res = exfuns_file_type(tfileinfo->fname);

            if ((res & 0X0F) != 0X00)               /* 取低四位,看看是不是图片文件 */
            {
                rval++;                             /* 有效文件数增加1 */
            }
        }
    }

    free(tfileinfo);                                /* 释放内存 */
    return rval;
}

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;
    uint8_t key = 0;

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
    init_camera();              /* 初始化摄像头 */

    spilcd_show_string(30, 50, 200, 16, 16, "ESP32-S3", RED);
    spilcd_show_string(30, 70, 200, 16, 16, "CAMERA TEST", RED);
    spilcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

    while (sd_spi_init())       /* 检测不到SD卡 */
    {
        spilcd_show_string(30, 110, 200, 16, 16, "SD Card Error!", RED);
        vTaskDelay(pdMS_TO_TICKS(500));
        spilcd_show_string(30, 130, 200, 16, 16, "Please Check! ", RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    sd_check_en = 1;
    
    rgb565 = malloc(spilcddev.pheight * spilcddev.pwidth * 2);

    if (NULL == rgb565)
    {
        ESP_LOGE(TAG, "can't alloc memory for rgb565 buffer");
    }

    BinarySemaphore = xSemaphoreCreateBinary();
    /* 创建任务3 */
    xTaskCreatePinnedToCore((TaskFunction_t )task3,                 /* 任务函数 */
                            (const char*    )"task3",               /* 任务名称 */
                            (uint16_t       )TASK3_STK_SIZE,        /* 任务堆栈大小 */
                            (void*          )NULL,                  /* 传入给任务函数的参数 */
                            (UBaseType_t    )TASK3_PRIO,            /* 任务优先级 */
                            (TaskHandle_t*  )&Task3Task_Handler,    /* 任务句柄 */
                            (BaseType_t     ) 0);                   /* 该任务哪个内核运行 */

    vTaskDelay(pdMS_TO_TICKS(1500));

    while(1)
    {
        key = xl9555_key_scan(0);
        fb = esp_camera_fb_get();

        if (fb)
        {
            mjpegdraw(fb->buf, fb->len, (uint8_t *)rgb565, NULL);

            if (key == KEY0_PRES)
            {
                xSemaphoreGive(BinarySemaphore);                    /* 释放二值信号量 */
            }

            /* 处理SD卡释放挂载 */
            if (sd_check_en == 1)
            {
                if (sdmmc_get_status(card) != ESP_OK)
                {
                    sd_check_en = 0;
                }
            }
            else
            {
                if (sd_spi_init() == ESP_OK)
                {
                    if (sdmmc_get_status(card) == ESP_OK)
                    {
                        sd_check_en = 1;
                    }
                }
            }

            esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, fb->width, fb->height, rgb565);
            esp_camera_fb_return(fb);
        }
        else
        {
            ESP_LOGE(TAG, "Get frame failed");
        }
    }
}

/**
 * @brief       task3
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
void task3(void *pvParameters)
{
    pvParameters = pvParameters;
    char file_name[30];
    uint32_t pictureNumber = 0;
    uint8_t res = 0;
    size_t writelen = 0;
    FIL *fftemp;
    res = exfuns_init();                                            /* 为fatfs相关变量申请内存 */
    pictureNumber = pic_get_tnum("0:/PICTURE");                     /* 得到总有效文件数 */
    pictureNumber = pictureNumber + 1;

    while (1)
    {
        xSemaphoreTake(BinarySemaphore, portMAX_DELAY);             /* 获取二值信号量 */
        
        /* SD卡挂载了，才能拍照 */
        if (sd_check_en == 1)
        {
            sprintf(file_name, "0:/PICTURE/img%ld.jpg", pictureNumber);
            fftemp = (FIL *)malloc(sizeof(FIL));                    /* 分配内存 */
            res = f_open(fftemp, (const TCHAR *)file_name, FA_WRITE | FA_CREATE_NEW);   /* 尝试打开 */

            if (res != FR_OK)
            {
                ESP_LOGE(TAG, "img open err\r\n");
            }

            f_write(fftemp, (const void *)fb->buf, fb->len, &writelen); /* 写入头数据 */

            if (writelen != fb->len)
            {
                ESP_LOGE(TAG, "img Write err");
            }
            else
            {
                ESP_LOGI(TAG, "write buff len %d byte", writelen);
                pictureNumber++;
            }

            f_close(fftemp);
            free(fftemp);
        }
    }
}