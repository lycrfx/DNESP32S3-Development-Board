/**
 ****************************************************************************************************
 * @file        timg.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       TIMG驱动代码
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

#include "timg.h"


/**
 * @brief       定时器回调函数
 * @param       timer:通用定时器对象
 * @param       edata:通用定时器警报事件数据
 * @param       user_data:用户数据
 * @retval      布尔类型
 */
static bool IRAM_ATTR timeout_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    if (timer->group->group_id == 0)        /* 定时器组0 */
    {
        if ((timer->timer_id))              /* 定时器1 */
        {
            /* 执行定时器1定时操作 */
            ESP_DRAM_LOGI("TimerGroup0", "timer1 time out");
        }
        else                                /* 定时器0 */
        {
            /* 执行定时器0定时操作 */
            ESP_DRAM_LOGI("TimerGroup0", "timer0 time out");
        }
    }
    else                                    /* 定时器组1 */
    {
        if ((timer->timer_id))              /* 定时器1 */
        {
            /* 执行定时器1定时操作 */
            ESP_DRAM_LOGI("TimerGroup1", "timer1 time out");
        }
        else                                /* 定时器0 */
        {
            /* 执行定时器0定时操作 */
            ESP_DRAM_LOGI("TimerGroup1", "timer0 time out");
        }
    }

    return pdTRUE;
}

/**
 * @brief       初始化定时器
 * @param       alarm_count:触发警报事件的目标计数值
 * @param       resolution: 定时器的分辨率
 * @retval      定时器的ID
 */
uint8_t timg_init(uint64_t alarm_count, uint32_t resolution)
{
    gptimer_handle_t gptimer = NULL;                    /* 定义一个通用定时器实例 */
    uint8_t group_index, timer_index, timer_id = 0;

    gptimer_config_t timer_config =                     /* 配置定时器参数 */
    {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,             /* 选择定时器的时钟源 */
        .direction = GPTIMER_COUNT_UP,                  /* 设置定时器的计数方向 */
        .resolution_hz = resolution,                    /* 设置内部计数器的分辨率,若为1000000即1MHz,1tick = 1us */ 
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));    /* 实例化定时器 */
    
    /* 获取定时器ID信息 */
    group_index = gptimer->group->group_id;
    timer_index = gptimer->timer_id;
    timer_id |= (group_index << 1) | timer_index;
    ESP_LOGI("timer", "group_index:%d  timer_index:%d  timer_id:%d", group_index, timer_index, timer_id); 

    gptimer_event_callbacks_t callback_func =                 
    {
        .on_alarm = timeout_callback,                   /* 设置警报事件的回调函数 */
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &callback_func, NULL));  /* 将函数挂载到中断服务例程 (ISR) */

    gptimer_enable(gptimer);                            /* 使能定时器 */

    gptimer_alarm_config_t alarm_config = 
    {
        .alarm_count = alarm_count,                     /* 报警目标计数值 */
        .reload_count = 0,                              /* 重载计数值为0 */
        .flags.auto_reload_on_alarm = true,             /* 开启重加载 */
    };   
    gptimer_set_alarm_action(gptimer, &alarm_config);   /* 设置触发报警条件 */

    gptimer_start(gptimer);                             /* 定时器开始工作 */

    return timer_id;
}

