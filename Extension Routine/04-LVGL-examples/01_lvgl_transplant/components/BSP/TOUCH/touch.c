/**
 ****************************************************************************************************
 * @file        touch.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       触摸屏 驱动代码
 * @note        支持电容式触摸屏
 *
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

#include "touch.h"

/* 触摸屏控制器初始化参数 */
_m_tp_dev tp_dev =
{
    .init = tp_init,
    .scan = 0,
    .x = {0},
    .y = {0},
    .sta = 0,
    .touchtype = 0x00
};

/**
 * @brief       触摸屏初始化
 * @param       无
 * @retval      0,触摸屏初始化成功
 *              1,触摸屏有问题
 */
esp_err_t tp_init(void)
{
    tp_dev.touchtype = 0;                   /* 默认设置竖屏 */
    tp_dev.touchtype |= lcddev.dir & 0X01;  /* 根据LCD判定是横屏还是竖屏 */
    gt9xxx_init();                          /* 初始化gt9xxx器件 */
    tp_dev.scan = gt9xxx_scan;              /* 扫描函数指向GT触摸屏扫描 */
    tp_dev.touchtype |= 0X80;               /* 电容屏 */
    return ESP_OK;
}
