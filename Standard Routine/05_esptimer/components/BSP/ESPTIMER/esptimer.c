#include "esptimer.h"
#include "led.h"
#include "esp_log.h"
#include "esp_timer.h"


static const char *TAG = "ESPTIMER";
static int32_t trigger_count = 0;


/**
 * @brief       定时器回调函数
 * @param       arg: 不携带参数
 * @retval      无
 */
void esptimer_callback(void *arg)
{
    LED0_TOGGLE();

    int64_t us = esp_timer_get_time();
    ESP_LOGI(TAG, "运行时间: %lld us, 函数调用次数：%d", (long long)us, trigger_count++);

}

/**
 * @brief       初始化高分辨率定时器(ESP_TIMER)
 * @param       tps: 定时器周期,以微秒为单位(μs).
 *              若以一秒为定时器周期来执行一次定时器中断,那此处tps = 1s = 1000000μs
 * @retval      无
 */

void esptimer_init(uint64_t tps)
{
    esp_timer_handle_t esp_tim_handle;                      /* 定义定时器句柄  */

    /* 定义一个定时器结构体设置定时器配置参数 */
    esp_timer_create_args_t timer_arg = {
        .callback = &esptimer_callback,                     /* 计时时间到达时执行的回调函数 */
        .arg = NULL,                                        /* 传递给回调函数的参数 */
        .dispatch_method = ESP_TIMER_TASK,                  /* 进入回调方式,从定时器任务进入 */
        .name = "Timer",                                    /* 定时器名称 */
    };

    ESP_ERROR_CHECK(esp_timer_create(&timer_arg, &esp_tim_handle));     /* 创建定时器 */
    ESP_ERROR_CHECK(esp_timer_start_periodic(esp_tim_handle, tps));     /* 启动周期性定时器,tps设置定时器周期(us单位) */
}
