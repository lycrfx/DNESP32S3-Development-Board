/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       ͼƬ��ʾ ʵ��
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ESP32-S3 ������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "led.h"
#include "my_spi.h"
#include "myiic.h"
#include "xl9555.h"
#include "spilcd.h"
#include "spi_sd.h"
#include "sdmmc_cmd.h"
#include "text.h"
#include "fonts.h"
#include "key.h"
#include "piclib.h"
#include "exfuns.h"
#include <stdio.h>


/**
 * @brief       �õ�path·����,Ŀ���ļ����ܸ���
 * @param       path : ·��
 * @retval      ����Ч�ļ���
 */
uint16_t pic_get_tnum(char *path)
{
    uint8_t res;
    uint16_t rval = 0;
    FF_DIR tdir;                                    /* ��ʱĿ¼ */
    FILINFO *tfileinfo;                             /* ��ʱ�ļ���Ϣ */
    tfileinfo = (FILINFO *)malloc(sizeof(FILINFO)); /* �����ڴ� */
    res = f_opendir(&tdir, (const TCHAR *)path);    /* ��Ŀ¼ */

    if (res == FR_OK && tfileinfo)
    {
        while (1)                                   /* ��ѯ�ܵ���Ч�ļ��� */
        {
            res = f_readdir(&tdir, tfileinfo);      /* ��ȡĿ¼�µ�һ���ļ� */

            if (res != FR_OK || tfileinfo->fname[0] == 0)break; /* ������/��ĩβ��,�˳� */
            res = exfuns_file_type(tfileinfo->fname);

            if ((res & 0X0F) != 0X00)               /* ȡ����λ,�����ǲ���ͼƬ�ļ� */
            {
                rval++;                             /* ��Ч�ļ�������1 */
            }
        }
    }

    free(tfileinfo);                                /* �ͷ��ڴ� */
    return rval;
}

/**
 * @brief       ת��
 * @param       fs:�ļ�ϵͳ����
 * @param       clst:ת��
 * @retval      =0:�����ţ�0:ʧ��
 */
static LBA_t atk_clst2sect(FATFS *fs, DWORD clst)
{
    clst -= 2;  /* Cluster number is origin from 2 */

    if (clst >= fs->n_fatent - 2)
    {
        return 0;   /* Is it invalid cluster number? */
    }

    return fs->database + (LBA_t)fs->csize * clst;  /* Start sector number of the cluster */
}

/**
 * @brief       ƫ��
 * @param       dp:ָ��Ŀ¼����
 * @param       Offset:Ŀ¼���ƫ����
 * @retval      FR_OK(0):�ɹ���!=0:����
 */
FRESULT atk_dir_sdi(FF_DIR *dp, DWORD ofs)
{
    DWORD clst;
    FATFS *fs = dp->obj.fs;

    if (ofs >= (DWORD)((FF_FS_EXFAT && fs->fs_type == FS_EXFAT) ? 0x10000000 : 0x200000) || ofs % 32)
    {
        /* Check range of offset and alignment */
        return FR_INT_ERR;
    }

    dp->dptr = ofs;         /* Set current offset */
    clst = dp->obj.sclust;  /* Table start cluster (0:root) */

    if (clst == 0 && fs->fs_type >= FS_FAT32)
    {	/* Replace cluster# 0 with root cluster# */
        clst = (DWORD)fs->dirbase;

        if (FF_FS_EXFAT)
        {
            dp->obj.stat = 0;
        }
        /* exFAT: Root dir has an FAT chain */
    }

    if (clst == 0)
    {	/* Static table (root-directory on the FAT volume) */
        if (ofs / 32 >= fs->n_rootdir)
        {
            return FR_INT_ERR;  /* Is index out of range? */
        }

        dp->sect = fs->dirbase;

    }
    else
    {   /* Dynamic table (sub-directory or root-directory on the FAT32/exFAT volume) */
        dp->sect = atk_clst2sect(fs, clst);
    }

    dp->clust = clst;   /* Current cluster# */

    if (dp->sect == 0)
    {
        return FR_INT_ERR;
    }

    dp->sect += ofs / fs->ssize;             /* Sector# of the directory entry */
    dp->dir = fs->win + (ofs % fs->ssize);   /* Pointer to the entry in the win[] */

    return FR_OK;
}

/**
 * @brief       �������
 * @param       ��
 * @retval      ��
 */
void app_main(void)
{
    esp_err_t ret;
    FF_DIR picdir; 
    FILINFO *picfileinfo;
    char *pname;
    uint16_t totpicnum;
    uint16_t curindex = 0;
    uint8_t key = 0; 
    uint8_t t;
    uint16_t temp;
    uint32_t *picoffsettbl;

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    led_init(); 
    my_spi_init();
    key_init();
    myiic_init();
    xl9555_init();
    spilcd_init();

    while (sd_spi_init())
    {
        spilcd_show_string(30, 110, 200, 16, 16, "SD Card Error!", RED);
        vTaskDelay(pdMS_TO_TICKS(500));
        spilcd_show_string(30, 130, 200, 16, 16, "Please Check! ", RED);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    ret = exfuns_init();

    while (fonts_init())
    {
        spilcd_clear(WHITE);
        spilcd_show_string(30, 30, 200, 16, 16, "ESP32-S3", RED);
        
        key = fonts_update_font(30, 50, 16, (uint8_t *)"0:", RED);

        while (key)
        {
            spilcd_show_string(30, 50, 200, 16, 16, "Font Update Failed!", RED);
            vTaskDelay(pdMS_TO_TICKS(200));
            spilcd_fill(20, 50, 200 + 20, 90 + 16, WHITE);
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        spilcd_show_string(30, 50, 200, 16, 16, "Font Update Success!   ", RED);
        vTaskDelay(pdMS_TO_TICKS(1000));
        spilcd_clear(WHITE); 
    }

    text_show_string(30, 50, 200, 16, "����ԭ��ESP32-S3������",16,0, RED);
    text_show_string(30, 70, 200, 16, "ͼƬ��ʾ ʵ��", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "����ԭ��@ALIENTEK", 16, 0, RED);
    text_show_string(30, 110, 200, 16, "KEY0:NEXT KEY1:PREV", 16, 0, RED);
    text_show_string(30, 130, 200, 16, "KEY_UP:PAUSE:", 16, 0, RED);

    while (f_opendir(&picdir, "0:/PICTURE")) 
    {
        text_show_string(30, 150, 240, 16, "PICTURE�ļ��д���!", 16, 0, RED);
        vTaskDelay(pdMS_TO_TICKS(200));
        spilcd_fill(30, 150, 240, 186, WHITE);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    totpicnum = pic_get_tnum("0:/PICTURE");
    while (totpicnum == 0) 
    {
        text_show_string(30, 150, 240, 16, "û��ͼƬ�ļ�", 16, 0, RED);
        vTaskDelay(pdMS_TO_TICKS(200));
        spilcd_fill(30, 150, 240, 186, WHITE);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    picfileinfo = (FILINFO *)malloc(sizeof(FILINFO));
    pname = malloc(255 * 2 + 1);
    picoffsettbl = malloc(4 * totpicnum);

    while (!picfileinfo || !pname || !picoffsettbl)
    {
        text_show_string(30, 150, 240, 16, "û��ͼƬ�ļ�!", 16, 0, RED);
        vTaskDelay(pdMS_TO_TICKS(200));
        spilcd_fill(30, 150, 240, 186, WHITE);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    ret = f_opendir(&picdir, "0:/PICTURE");

    if (ret == FR_OK)
    {
        curindex = 0;

        while (1)
        {
            temp = picdir.dptr;
            ret = f_readdir(&picdir, picfileinfo); 
            if (ret != FR_OK || picfileinfo->fname[0] == 0) break;

            ret = exfuns_file_type(picfileinfo->fname);

            if ((ret & 0X0F) != 0X00)
            {
                picoffsettbl[curindex] = temp; 
                curindex++;
            }
        }

        ESP_LOGI("main", "0:/PICTURE pic_num:%d", curindex);
    }

    text_show_string(30, 150, 240, 16, "ͼƬ��ʼ��ʾ......", 16, 0, RED);
    vTaskDelay(pdMS_TO_TICKS(1000));
    piclib_init();
    curindex = 0; 
    ret = f_opendir(&picdir, (const TCHAR *)"0:/PICTURE");

    while (ret == FR_OK)                                                        
    {
        atk_dir_sdi(&picdir, picoffsettbl[curindex]);
        ret = f_readdir(&picdir, picfileinfo);                                  

        if (ret != FR_OK || picfileinfo->fname[0] == 0)
        {
            picdir.dptr = picoffsettbl[curindex];
            ret = f_readdir(&picdir, picfileinfo);                              
            
            if (ret != FR_OK || picfileinfo->fname[0] == 0)
            {
                ESP_LOGE(__FUNCTION__, "Read Failed");
                break;                                                          
            }
        }

        strcpy((char *)pname, "0:/PICTURE/");                                   
        strcat((char *)pname, (const char *)picfileinfo->fname);                
        spilcd_clear(BLACK);
        
        piclib_ai_load_picfile(pname, 0, 0, spilcddev.width, spilcddev.height);       
        text_show_string(2, 2, spilcddev.width, 16, (char *)pname, 16, 0, RED);    
        t = 0;

        while (1)
        {
            t ++;
            key = xl9555_key_scan(0);

            if (t > 250) key = KEY0_PRES;

            if ((t % 20) == 0)
            {
                LED0_TOGGLE();
            }

            if (key == KEY1_PRES)
            {
                if (curindex)
                {
                    curindex--;
                }
                else
                {
                    curindex = totpicnum - 1;
                }
                
                break;
            }
            else if (key == KEY0_PRES)
            {
                curindex++;

                if (curindex >= totpicnum)
                {
                    curindex = 0;
                }

                break;
            }

            vTaskDelay(pdMS_TO_TICKS(10));
        }

        ret = 0;
    }

    free(picfileinfo);
    free(pname);
    free(picoffsettbl);
}