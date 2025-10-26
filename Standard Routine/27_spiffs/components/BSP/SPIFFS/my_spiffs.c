/**
 ****************************************************************************************************
 * @file        my_spiffs.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       SPIFFS驱动测试代码
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

#include "my_spiffs.h"


static const char *spiffs_tag = "spiffs";

/**
 * @brief       spiffs初始化
 * @param       partition_label:分区表的分区名称
 * @param       mount_point:文件系统关联的文件路径前缀
 * @param       max_files:可以同时打开的最大文件数
 * @retval      ESP_OK:成功; ESP_FAIL:失败
 */
esp_err_t spiffs_init(char *partition_label, char *mount_point, size_t max_files)
{
    size_t total = 0;   /* SPIFFS总容量 */
    size_t used = 0;    /* SPIFFS已使用的容量 */
    
    esp_vfs_spiffs_conf_t spiffs_conf = {           /* 配置spiffs文件系统的参数 */
        .base_path              = mount_point,      /* 磁盘路径,比如"0:","1:" */
        .partition_label        = partition_label,  /* 分区表的分区名称 */
        .max_files              = max_files,        /* 最大可同时打开的文件数 */
        .format_if_mount_failed = true,             /* 挂载失败则格式化文件系统 */
    };

    esp_err_t ret = esp_vfs_spiffs_register(&spiffs_conf);  /* 初始化和挂载SPIFFS分区 */
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(spiffs_tag, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(spiffs_tag, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(spiffs_tag, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }

        return ESP_FAIL;
    }

    ret = esp_spiffs_info(spiffs_conf.partition_label, &total, &used);  /* 获取SPIFFS的总容量和已使用的容量 */
    if (ret != ESP_OK)
    {
        ESP_LOGI(spiffs_tag, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(spiffs_tag, "Partition size: total: %d Bytes, used: %d Bytes", total, used);
    }

    return ret;
}

/**
 * @brief       注销spiffs
 * @param       partition_label:分区表的分区名称
 * @retval      ESP_OK:注销成功; 其他:失败
 */
esp_err_t spiffs_deinit(char *partition_label)
{
    return esp_vfs_spiffs_unregister(partition_label);
}

