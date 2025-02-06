/**
 ****************************************************************************************************
 * @file        ledc.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       LEDC驱动代码
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

#ifndef __LEDC_H
#define __LEDC_H

#include "driver/gpio.h"
#include "driver/ledc.h"


/* 引脚以及重要参数定义 */
#define LEDC_PWM_TIMER          LEDC_TIMER_0        /* 使用定时器(0~3) */
#define LEDC_PWM_CH0_GPIO       GPIO_NUM_1          /* LED控制器通道对应GPIO */
#define LEDC_PWM_CH0_CHANNEL    LEDC_CHANNEL_0      /* LED控制器通道号(0~7) */

/* LEDC配置结构体 */
typedef struct
{
    ledc_clk_cfg_t clk_cfg;             /* 时钟源配置（LEDC_USE_XTAL_CLK\LEDC_USE_PLL_DIV_CLK\LEDC_USE_RC_FAST_CLK或者LEDC_AUTO_CLK(自动选择)） */
    ledc_timer_t  timer_num;            /* 定时器（LEDC_TIMER_0~LEDC_TIMER_3） */
    uint32_t freq_hz;                   /* 频率（系统自动计算分频系数） */
    ledc_timer_bit_t duty_resolution;   /* 占空比分辨率 */
    ledc_channel_t channel;             /* 通道（LEDC_CHANNEL_0~LEDC_CHANNEL_7） */
    uint32_t duty;                      /* 初始占空比 */
    int gpio_num;                       /* PWM输出管脚 */
}ledc_config_t;


/* 函数声明 */
void ledc_init(ledc_config_t *ledc_config);                             /* ledc初始化 */
void ledc_pwm_set_duty(ledc_config_t *ledc_config, uint16_t duty);      /* PWM占空比设置 */

#endif
