/**
 ****************************************************************************************************
 * @file        rmt_nec_rx.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       RMT红外解码驱动代码
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

#include "rmt_nec_rx.h"


/* 保存NEC解码的地址和命令字节 */
uint16_t s_nec_code_address = 0x0000;
uint16_t s_nec_code_command = 0x0000;

QueueHandle_t receive_queue = NULL;
rmt_channel_handle_t rx_channel = NULL;
rmt_symbol_word_t raw_symbols[64];      /* 对于标准NEC框架应该足够 */
rmt_receive_config_t receive_config;


/**
 * @brief       RMT数据接收完成回调函数
 * @param       channel   : 通道
 * @param       edata     : 接收的数据
 * @param       user_data : 传入的参数
 * @retval      返回是否唤醒了任何任务
 */
bool rmt_nec_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_wakeup = pdFALSE;

    QueueHandle_t receive_queue = (QueueHandle_t)user_data;
    xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);     /* 将收到的RMT数据通过消息队列发送到解析任务 */

    return high_task_wakeup == pdTRUE;
}

/**
 * @brief       RMT红外接收初始化
 * @param       无
 * @retval      ESP_OK:初始化成功
 */
esp_err_t rmt_nec_rx_init(void)
{
    ESP_ERROR_CHECK(gpio_reset_pin(RMT_IN_GPIO_PIN));
    /* 配置接收通道 */
    rmt_rx_channel_config_t rx_channel_cfg = {
        .gpio_num           = RMT_IN_GPIO_PIN,          /* 设置红外接收通道管脚 */
        .clk_src            = RMT_CLK_SRC_DEFAULT,      /* 设置RMT时钟源 */
        .resolution_hz      = RMT_RESOLUTION_HZ,        /* 设置时钟分辨率 */
        .mem_block_symbols  = 64,                       /* 通道一次可以存储的RMT符号数量 */
    };
    ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_channel_cfg, &rx_channel));  /* 创建接收通道 */

    /* 配置红外接收完成回调 */
    receive_queue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));  /* 创建消息队列，用于接收红外编码 */
    assert(receive_queue);
    rmt_rx_event_callbacks_t cbs = {
        .on_recv_done = rmt_nec_rx_done_callback,                       /* RMT信号接收完成回调函数 */
    };
    ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx_channel, &cbs, receive_queue));              /* 配置RMT接收通道回调函数 */

    /* NEC协议的时序要求 */
    receive_config.signal_range_min_ns = 1250;          /* NEC信号的最短持续时间为560us，1250ns＜560us，有效信号不会被视为噪声 */
    receive_config.signal_range_max_ns = 12000000;      /* NEC信号的最长持续时间为9000us，12000000ns>9000us，接收不会提前停止 */

    /* 开启RMT通道 */
    ESP_ERROR_CHECK(rmt_enable(rx_channel));            /* 使能RMT接收通道 */
    ESP_ERROR_CHECK(rmt_receive(rx_channel, raw_symbols, sizeof(raw_symbols), &receive_config));    /* 准备接收 */

    return ESP_OK;
}

/**
 * @brief       判断数据时序长度是否在NEC时序时长容差范围内 正负REMOTE_NEC_DECODE_MARGIN的值以内
 * @param       signal_duration:信号持续时间
 * @param       spec_duration:信号的标准持续时间 
 * @retval      true:符合条件;false:不符合条件
 */
inline bool rmt_nec_check_range(uint32_t signal_duration, uint32_t spec_duration)
{
    return (signal_duration < (spec_duration + RMT_NEC_DECODE_MARGIN)) &&
           (signal_duration > (spec_duration - RMT_NEC_DECODE_MARGIN));
}

/**
 * @brief       对比数据时序长度判断是否为逻辑0
 * @param       rmt_nec_symbols:RMT数据帧
 * @retval      true:符合条件;false:不符合条件
 */
bool rmt_nec_logic0(rmt_symbol_word_t *rmt_nec_symbols)
{
    return rmt_nec_check_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ZERO_DURATION_0) &&
           rmt_nec_check_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ZERO_DURATION_1);
}

/**
 * @brief       对比数据时序长度判断是否为逻辑1
 * @param       rmt_nec_symbols:RMT数据帧
 * @retval      true:符合条件;false:不符合条件
 */
bool rmt_nec_logic1(rmt_symbol_word_t *rmt_nec_symbols)
{
    return rmt_nec_check_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ONE_DURATION_0) &&
           rmt_nec_check_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ONE_DURATION_1);
}

/**
 * @brief       将RMT接收结果解码出NEC地址和命令
 * @param       rmt_nec_symbols:RMT数据帧
 * @retval      true成功;false失败
 */
bool rmt_nec_parse_frame(rmt_symbol_word_t *rmt_nec_symbols)
{
    rmt_symbol_word_t *cur = rmt_nec_symbols;
    uint16_t address = 0;
    uint16_t command = 0;

    bool valid_leading_code = rmt_nec_check_range(cur->duration0, NEC_LEADING_CODE_DURATION_0) &&
                              rmt_nec_check_range(cur->duration1, NEC_LEADING_CODE_DURATION_1);

    if (!valid_leading_code) 
    {
        return false;
    }

    cur++;

    for (int i = 0; i < 16; i++)
    {
        if (rmt_nec_logic1(cur)) 
        {
            address |= 1 << i;
        } 
        else if (rmt_nec_logic0(cur))
        {
            address &= ~(1 << i);
        } 
        else 
        {
            return false;
        }
        cur++;
    }

    for (int i = 0; i < 16; i++)
    {
        if (rmt_nec_logic1(cur))
        {
            command |= 1 << i;
        }
        else if (rmt_nec_logic0(cur))
        {
            command &= ~(1 << i);
        }
        else
        {
            return false;
        }
        cur++;
    }

    /* 保存数据地址和命令，用于判断重复按键 */
    s_nec_code_address = address;
    s_nec_code_command = command;

    return true;
}

/**
 * @brief       检查数据帧是否为重复按键：一直按住同一个键
 * @param       rmt_nec_symbols:RMT数据帧
 * @retval      true:符合条件;false:不符合条件
 */
bool rmt_nec_parse_frame_repeat(rmt_symbol_word_t *rmt_nec_symbols)
{
    return rmt_nec_check_range(rmt_nec_symbols->duration0, NEC_REPEAT_CODE_DURATION_0) &&
           rmt_nec_check_range(rmt_nec_symbols->duration1, NEC_REPEAT_CODE_DURATION_1);
}
