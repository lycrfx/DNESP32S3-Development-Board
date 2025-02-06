/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       USB模拟U盘实验
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
#include "spilcd.h"
#include "my_spi.h"
#include "spi_sd.h"
#include "tinyusb.h"
#include "diskio_impl.h"
#include "diskio_sdmmc.h"
#include "tusb_msc_storage.h"
#include "esp_console.h"
#include <stdio.h>


extern sdmmc_card_t *card;
/* 挂载名称 */
#define BASE_PATH "/0:" /* 挂载分区大小的基本路径 */
/* TinyUSB 描述符 */
#define EPNUM_MSC       1
#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN)

enum {
    ITF_NUM_MSC = 0,
    ITF_NUM_TOTAL
};

enum {
    EDPT_CTRL_OUT = 0x00,
    EDPT_CTRL_IN  = 0x80,

    EDPT_MSC_OUT  = 0x01,
    EDPT_MSC_IN   = 0x81,
};

/* USB设备描述符 */
static tusb_desc_device_t descriptor_config = {
    .bLength = sizeof(descriptor_config),       /* 描述符的大小 */
    .bDescriptorType = TUSB_DESC_DEVICE,        /* 描述符类型 */
    .bcdUSB = 0x0200,                           /* BUSB规格发布号 */
    .bDeviceClass = TUSB_CLASS_MISC,            /* MSC类 */
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,    /* 子类代码(由USB-IF分配) */
    .bDeviceProtocol = MISC_PROTOCOL_IAD,       /* 协议代码(由USB-IF分配) */
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,  /* 端点0的最大数据包大小(只有8、16、32或64有效) */
    .idVendor = 0x303A,                         /* 供应商ID(由USB-IF分配) */
    .idProduct = 0x4002,                        /* 产品ID(由制造商指定) */
    .bcdDevice = 0x100,                         /* 设备发布号，以二进制编码的十进制表示 */
    .iManufacturer = 0x01,                      /* 描述制造商的字符串描述符的索引 */
    .iProduct = 0x02,                           /* 描述产品的字符串描述符的索引 */
    .iSerialNumber = 0x03,                      /* 描述设备序列号的字符串描述符的索引 */
    .bNumConfigurations = 0x01                  /* 可能的配置数 */
};

/* 全速配置描述符 */
static uint8_t const msc_fs_configuration_desc[] = {
    /* 配置号，接口计数，字符串索引，总长度，属性，功率(mA) */
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    /* 接口编号，字符串索引，EP Out & EP In地址，EP大小 */
    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 0, EDPT_MSC_OUT, EDPT_MSC_IN, 64),
};

/* 字符串描述符 */
static char const *string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 },  /* 0: 支持英文 (0x0409) */
    "TinyUSB",                      /* 1: 生产商 */
    "TinyUSB Device",               /* 2: 产品 */
    "123456",                       /* 3: 序列 */
    "Example MSC",                  /* 4. MSC */
};

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
    /* 显示实验信息 */
    spilcd_show_string(30, 50, 200, 16, 16, "ESP32-S3", RED);
    spilcd_show_string(30, 70, 200, 16, 16, "USB SD TEST", RED);
    spilcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

    ESP_ERROR_CHECK(sd_spi_init());
    /* 配置SDMMC */
    const tinyusb_msc_sdmmc_config_t config_sdmmc = {
        .card = card,                   /* 指向sdmmc卡配置结构的指针 */
        .callback_mount_changed = NULL, /* 注册回调函数，用来初始化子分区表 */
        .mount_config.max_files = 5,    /* 最大文件打开数量 */
    };
    /* 注册存储类型sd卡与tinyusb驱动程序 */
    ESP_ERROR_CHECK(tinyusb_msc_storage_init_sdmmc(&config_sdmmc));
    /* 注册回调的其他方法，即使用单独的API注册 */
    ESP_ERROR_CHECK(tinyusb_msc_register_callback(TINYUSB_MSC_EVENT_MOUNT_CHANGED, NULL));
    // /* 挂载设备 */
    // ESP_ERROR_CHECK(tinyusb_msc_storage_mount(BASE_PATH));
    /* 配置USB */
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = &descriptor_config,    /* 设备描述符 */
        .string_descriptor = string_desc_arr,       /* 字符串描述符 */
        .string_descriptor_count = sizeof(string_desc_arr) / sizeof(string_desc_arr[0]),    /* 字符串描述符大小 */
        .external_phy = false,                      /* 使用内部USB PHY */
        .configuration_descriptor = msc_fs_configuration_desc,      /* 配置描述符 */
    };
    /* 初始化USB */
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    while(1)
    {
        LED0_TOGGLE();
        vTaskDelay(500);
    }
}