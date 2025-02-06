/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       TIMG实验
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
#include "esp_log.h"
#include <stdio.h>
#include "led.h"
#include "driver/gptimer.h"
#include "timg.h"


/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;
    timg_config_t *timgr_config = malloc(sizeof(timg_config_t));

    ret = nvs_flash_init();     /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    led_init();                 /* 初始化LED */

    timgr_config->timer_count_value = 0;                            /* 记录计数值 */
    timgr_config->clk_src           = GPTIMER_CLK_SRC_DEFAULT;      /* 时钟源选择（GPTIMER_CLK_SRC_XTAL\GPTIMER_CLK_SRC_PLL_F80M(默认)\GPTIMER_CLK_SRC_RC_FAST） */
    timgr_config->timer_group       = TIMER_GROUP_0;                /* 定时器组0（TIMER_GROUP_0~TIMER_GROUP_1） */
    timgr_config->timer_idx         = TIMER_0;                      /* 定时器组0的定时器0（TIMER_0~TIMER_1） */
    timgr_config->timing_time       = 1 * 1000000;                  /* 定时时间 */
    timgr_config->alarm_value       = timgr_config->timing_time;    /* 设置警报值(us秒为单位)等于定时时间 */
    timgr_config->auto_reload       = TIMER_ALARM_EN;               /* 自动重装载 */
    timg_init(timgr_config);                                        /* 定时器初始化 */

    while(1)
    {
        if (timgr_config->timer_count_value != 0)
        {
            ESP_LOGI("Timer:", "Timer auto reloaded, count value in ISR: %llu", timgr_config->timer_count_value);
            timgr_config->timer_count_value = 0;    /* 清零 */
        }

        vTaskDelay(pdMS_TO_TICKS(10));              /* 延时10ms */
    }
}
