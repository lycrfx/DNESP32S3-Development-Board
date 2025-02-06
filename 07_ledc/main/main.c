/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       LEDC PWM输出实验
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
#include <stdio.h>
#include "ledc.h"


/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;
    uint8_t dir = 1;
    uint16_t ledpwmval = 0;

    ledc_config_t *ledc_config = malloc(sizeof(ledc_config_t));

    ret = nvs_flash_init();     /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ledc_config->clk_cfg         = LEDC_AUTO_CLK;           /* 启动定时器时，根据给出的分辨率和占空率参数自动选择ledc源时钟 */
    ledc_config->timer_num       = LEDC_TIMER_0;            /* 选择哪个定时器计数（LEDC_TIMER_0~LEDC_TIMER_3） */
    ledc_config->freq_hz         = 1000;                    /* 1KHz（系统自动计算分配系数，并提供freq_hz频率给到定时器） */
    ledc_config->duty_resolution = LEDC_TIMER_14_BIT;       /* 设置定时器最大计数值（请看技术手册表32.4.1） */
    ledc_config->channel         = LEDC_CHANNEL_0;          /* 设置输出通道（LEDC_CHANNEL_0 ~ LEDC_CHANNEL_7） */
    ledc_config->duty            = 0;                       /* 一个周期内占高电平时间(占空比) */
    ledc_config->gpio_num        = LEDC_PWM_CH0_GPIO;       /* PWM信号输出那个管脚 */
    ledc_init(ledc_config);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));

        if (dir == 1)
        {
            ledpwmval += 5; /* dir==1 ledpwmval递增 */
        }
        else
        {
            ledpwmval -= 5; /* dir==0 ledpwmval递减 */
        }

        if (ledpwmval > 95)
        {
            dir = 0;        /* ledpwmval到达100后，方向为递减 */
        }

        if (ledpwmval < 5)
        {
            dir = 1;        /* ledpwmval递减到5后，方向改为递增 */
        }

        /* 设置占空比 */
        ledc_pwm_set_duty(ledc_config, ledpwmval);
    }
}
