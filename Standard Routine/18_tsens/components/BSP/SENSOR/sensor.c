/**
 ****************************************************************************************************
 * @file        sensor.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       内部温度传感器驱动代码
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

#include "sensor.h"


temperature_sensor_handle_t temp_handle = NULL; /* 温度传感器句柄 */

/**
 * @brief       初始化内部温度传感器
 * @param       无
 * @retval      ESP_OK:初始化成功
 */
esp_err_t sensor_inter_init(void)
{
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);     /* 设置测试温度范围10~50 */

    ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temp_handle));     /* 创建温度传感器模块 */

    ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));                            /* 启动温度传感器 */

    return ESP_OK;
}

/**
 * @brief       获取内部温度传感器温度值
 * @param       无
 * @retval      返回内部温度值
 */
float sensor_inter_get_temperature(void)
{
    float temperature = 0;

    ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &temperature));    /* 获取当前测量的温度值 */ 

    return temperature;
}
