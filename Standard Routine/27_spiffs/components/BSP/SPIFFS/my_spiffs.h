/**
 ****************************************************************************************************
 * @file        my_spiffs.h
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

#ifndef __MY_SPIFFS_H
#define __MY_SPIFFS_H

#include "esp_spiffs.h"
#include "esp_vfs.h"
#include "esp_log.h"

#define DEFAULT_FD_NUM          5           /* 默认最大可打开文件数量 */
#define DEFAULT_MOUNT_POINT     "/spiffs"   /* 文件系统名称 */

/* 函数声明 */
esp_err_t spiffs_init(char *partition_label, char *mount_point, size_t max_files);      /* spiffs初始化 */

#endif
