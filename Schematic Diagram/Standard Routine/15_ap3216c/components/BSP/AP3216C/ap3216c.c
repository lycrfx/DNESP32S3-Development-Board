/**
 ****************************************************************************************************
 * @file        ap3216c.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       AP3216C驱动代码
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

#include "ap3216c.h"


const char* ap3216c_tag = "ap3216c";
i2c_master_dev_handle_t ap3216c_handle = NULL;

/**
 * @brief       AP3216C读取一个字节
 * @param       reg: 寄存器地址
 * @retval      读到的数据
 */
static uint8_t ap3216c_read_one_byte(uint8_t reg)
{
    uint8_t rec_data = 0;
    i2c_master_transmit_receive(ap3216c_handle, &reg, 1, &rec_data, 1, -1);
    return rec_data;
}

/**
 * @brief       AP3216C写入一个字节
 * @param       reg   :寄存器地址
 * @param       data  :要写入的数据
 * @retval      ESP_OK:写入成功; 其他异常
 */
static esp_err_t ap3216c_write_one_byte(uint8_t reg, uint8_t data)
{
    uint8_t *buf = malloc(2);
    if (buf == NULL)
    {
        ESP_LOGE(ap3216c_tag, "%s memory failed", __func__);
        return ESP_ERR_NO_MEM; 
    }

    buf[0] = reg;                  
    buf[1] = data;

    esp_err_t ret = i2c_master_transmit(ap3216c_handle, buf, 2, -1);

    free(buf);

    return ret;
}

/**
 * @brief       读取AP3216C的数据
 * @note        读取原始数据，包括ALS,PS和IR
 *              如果同时打开ALS,IR+PS的话两次数据读取的时间间隔要大于112.5ms
 * @param       ir  :IR传感器值指针
 * @param       ps  :PS传感器值指针
 * @param       als :ALS传感器值指针
 * @retval      无
 */
void ap3216c_read_data(uint16_t *ir, uint16_t *ps, uint16_t *als)
{
    uint8_t buf[6] = {0};
    uint8_t i;

    for (i = 0; i < 6; i++)
    {
        buf[i] =  ap3216c_read_one_byte(0X0A + i);  /* 循环读取所有传感器数据 */
    }

    if (buf[0] & 0X80)      /* IR_OF位为1,则数据无效 */
    {
        *ir = 0;    
    }
    else 
    {
        *ir = ((uint16_t)buf[1] << 2) | (buf[0] & 0X03);    /* 读取IR传感器的数据   */
    }

    *als = ((uint16_t)buf[3] << 8) | buf[2];                /* 读取ALS传感器的数据   */ 

    if (buf[4] & 0x40)      /* IR_OF位为1,则数据无效 */
    {
        *ps = 0;    
    }
    else
    {
        *ps = ((uint16_t)(buf[5] & 0X3F) << 4) | (buf[4] & 0X0F);   /* 读取PS传感器的数据 */
    }
}

/**
 * @brief       初始化AP3216C
 * @param       无
 * @retval      ESP_OK:初始化成功; ESP_FAIL:初始化失败
 */
esp_err_t ap3216c_init(void)
{
    uint8_t temp = 0x00;

    /* 未调用myiic_init初始化IIC */
    if (bus_handle == NULL)
    {
        ESP_ERROR_CHECK(myiic_init());
    }

    i2c_device_config_t ap3216c_i2c_dev_conf = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,  /* 从机地址长度 */
        .scl_speed_hz    = IIC_SPEED_CLK,       /* 传输速率 */
        .device_address  = AP3216C_ADDR,        /* 从机7位的地址 */
    };
    /* I2C总线上添加ap3216c设备 */
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &ap3216c_i2c_dev_conf, &ap3216c_handle));

    ap3216c_write_one_byte(0x00, 0X04);     /* 复位AP3216C */
    vTaskDelay(pdMS_TO_TICKS(50));          /* AP3216C复位至少10ms */
    ap3216c_write_one_byte(0x00, 0X03);     /* 开启ALS、PS+IR */
    temp = ap3216c_read_one_byte(0X00);     /* 读取刚刚写进去的0X03 */

    if (temp == 0X03)
    {
        ESP_LOGI(ap3216c_tag, "AP3216C success!!!");
        return ESP_OK;                      /* AP3216C正常 */
    }
    else
    {
        ESP_LOGE(ap3216c_tag, "AP3216C init fail!!!");
        return ESP_FAIL;                    /* AP3216C失败 */
    }
}
