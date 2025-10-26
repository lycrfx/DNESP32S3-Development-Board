/**
 ****************************************************************************************************
 * @file        piclib.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       图片解码库 代码
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

#include "piclib.h"


_pic_info picinfo;      /* 图片信息 */
_pic_phy pic_phy;       /* 图片显示物理接口 */

/**
 * @brief       多点填充
 * @param       x, y          : 起始坐标
 * @param       width, height : 宽度和高度
 * @param       color         : 颜色数组
 * @retval      无
 */
static void piclib_multi_color(uint16_t x, uint16_t y, uint16_t size, uint16_t *color)
{
    esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + (size - 1), y + 1, color);
}

/**
 * @brief       画图初始化
 *   @note      在画图之前,必须先调用此函数, 指定相关函数
 * @param       无
 * @retval      无
 */
void piclib_init(void)
{
    pic_phy.draw_point = spilcd_draw_point;         /* 画点函数实现,仅GIF需要 */
    pic_phy.fill = spilcd_fill;                     /* 填充函数实现,仅GIF需要 */
    pic_phy.draw_hline = spilcd_draw_hline;         /* 画线函数实现,仅GIF需要 */
    pic_phy.multicolor = piclib_multi_color;        /* 颜色填充函数实现,JPEG、BMP、PNG需要 */

    picinfo.lcdwidth = spilcddev.width;               /* 得到LCD的宽度像素 */
    picinfo.lcdheight = spilcddev.height;             /* 得到LCD的高度像素 */
}

/**
 * @brief       智能画图
 *   @note      图片仅在x,y和width, height限定的区域内显示.
 *
 * @param       filename      : 包含路径的文件名(.bmp/.jpg/.jpeg/.gif等)
 * @param       x, y          : 起始坐标
 * @param       width, height : 显示区域
 * @param       fast          : 使能快速解码
 *   @arg                       0, 不使能
 *   @arg                       1, 使能
 *   @note                      图片尺寸小于等于液晶分辨率,才支持快速解码
 * @retval      无
 */
uint8_t piclib_ai_load_picfile(char *filename, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    uint8_t	res = 0;/* 返回值 */
    uint8_t temp;

    if ((x + width) > picinfo.lcdwidth)return PIC_WINDOW_ERR;   /* x坐标超范围了 */

    if ((y + height) > picinfo.lcdheight)return PIC_WINDOW_ERR; /* y坐标超范围了 */

    /* 得到显示方框大小 */
    if (width == 0 || height == 0)return PIC_WINDOW_ERR;        /* 窗口设定错误 */

    /* 文件名传递 */
    temp = exfuns_file_type(filename);   /* 得到文件的类型 */
    ESP_LOGI("here","temp:%#x ", temp);
    switch (temp)
    {
        case T_BMP:
            ESP_LOGI("here","enter");
            res = bmp_decode(filename,width, height);           /* 解码BMP */
            break;

        case T_JPG:
        case T_JPEG:
            res = jpeg_decode(filename,width, height);          /* 解码JPG/JPEG */
            break;

        case T_GIF:
            res = gif_decode(filename, x, y, width, height);    /* 解码gif */
            break;

        case T_PNG:
            res = png_decode(filename, width, height);          /* 解码PNG */
            break;

        default:
            res = PIC_FORMAT_ERR;                               /* 非图片格式!!! */
            break;
    }

    return res;
}

/**
 * @brief       动态分配内存
 * @param       size          : 要申请的内存大小(字节)
 * @retval      分配到的内存首地址
 */
void *piclib_mem_malloc (uint32_t size)
{
    return (void *)malloc(size);
}

/**
 * @brief       释放内存
 * @param       paddr         : 内存首地址
 * @retval      无
 */
void piclib_mem_free (void *paddr)
{
    free(paddr);
}
