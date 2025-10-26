/**
 ****************************************************************************************************
 * @file        font.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       �ֿ� ����
 *              �ṩfonts_update_font��fonts_init�����ֿ���ºͳ�ʼ��
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

#include "fonts.h"


/* �ֿ�����ռ�õ�����������С(3���ֿ�+unigbk��+�ֿ���Ϣ=3238700 �ֽ�,Լռ791������,һ������4K�ֽ�) */
#define FONTSECSIZE         791
/* ÿ�β��������� 4K ֮�� */
#define SECTOR_SIZE         0X1000

/* �������ֿ�����ʼ��ַ
 * UNIGBK.BIN,�ܴ�С3.09M, 791������,���ֿ�ռ����,���ܶ�!
 */
#define FONTINFOADDR        0


/* ���������ֿ������Ϣ����ַ����С�� */
_font_info ftinfo;

static const char *fonts_tag = "fonts";
const esp_partition_t *storage_partition;

/* �ֿ����ڴ����е�·�� */
const char *FONT_GBK_PATH[4] =
{
    "/SYSTEM/FONT/UNIGBK.BIN",      /* UNIGBK.BIN�Ĵ��λ�� */
    "/SYSTEM/FONT/GBK12.FON",       /* GBK12�Ĵ��λ�� */
    "/SYSTEM/FONT/GBK16.FON",       /* GBK16�Ĵ��λ�� */
    "/SYSTEM/FONT/GBK24.FON",       /* GBK24�Ĵ��λ�� */
};

/* ����ʱ����ʾ��Ϣ */
const char *FONT_UPDATE_REMIND_TBL[4] =
{
    "Updating UNIGBK.BIN",          /* ��ʾ���ڸ���UNIGBK.bin */
    "Updating GBK12.FON ",          /* ��ʾ���ڸ���GBK12 */
    "Updating GBK16.FON ",          /* ��ʾ���ڸ���GBK16 */
    "Updating GBK24.FON ",          /* ��ʾ���ڸ���GBK24 */
};

/**
 * @brief       �������ȡ����
 * @param       buffer    : ��ȡ���ݵĴ洢��
 * @param       offset    : ��ȡ���ݵ���ʼ��ַ
 * @param       length    : ��ȡ��С
 * @retval      ESP_OK:��ʾ�ɹ�;����:��ʾʧ��
 */
esp_err_t fonts_partition_read(void *buffer, uint32_t offset, uint32_t length)
{
    esp_err_t err;

    if (buffer == NULL || (length > SECTOR_SIZE))
    {
        ESP_LOGE(fonts_tag, "ESP_ERR_INVALID_ARG");
        return ESP_ERR_INVALID_ARG;
    }

    err = esp_partition_read(storage_partition, offset, buffer, length);
    
    if (err != ESP_OK)
    {
        ESP_LOGE(fonts_tag, "Flash read failed.");
        return err;
    }
    
    return err;
}

/**
 * @brief       ������д������
 * @param       buffer    : д�����ݵĴ洢��
 * @param       offset    : д�����ݵ���ʼ��ַ
 * @param       length    : д���С
 * @retval      ESP_OK:��ʾ�ɹ�;����:��ʾʧ��
 */
esp_err_t fonts_partition_write(void *buffer, uint32_t offset, uint32_t length)
{
    esp_err_t err;

    if (buffer == NULL || (length > SECTOR_SIZE))
    {
        ESP_LOGE(fonts_tag, "ESP_ERR_INVALID_ARG");
        return ESP_ERR_INVALID_ARG;
    }

    err = esp_partition_write(storage_partition, offset, buffer, length);

    if (err != ESP_OK)
    {
        ESP_LOGE(fonts_tag, "Flash write failed.");
        return err;
    }

    return err;
}

/**
 * @brief       ����ĳ������
 * @param       offset:������ʼ��ַ
 * @retval      ESP_OK:��ʾ�ɹ�;����:��ʾʧ��
 */
esp_err_t fonts_partition_erase_sector(uint32_t offset)
{
    esp_err_t err;

    err = esp_partition_erase_range(storage_partition, offset, SECTOR_SIZE);
    
    if (err != ESP_OK)
    {
        ESP_LOGE(fonts_tag, "Flash erase failed.");
        return err;
    }

    return err;
}

/**
 * @brief       ��ʾ��ǰ������½���
 * @param       x, y    : ����
 * @param       size    : �����С
 * @param       totsize : �����ļ���С
 * @param       pos     : ��ǰ�ļ�ָ��λ��
 * @param       color   : ������ɫ
 * @retval      ��
 */
void fonts_progress_show(uint16_t x, uint16_t y, uint8_t size, uint32_t totsize, uint32_t pos, uint32_t color)
{
    float prog;
    uint8_t t = 0XFF;

    prog = (float)pos / totsize;
    prog *= 100;

    if (t != prog)
    {
        spilcd_show_string(x + 3 * size / 2, y, 240, 320, size, "%", color);
        t = prog;

        if (t > 100) t = 100;

        spilcd_show_num(x, y, t, 3, size, color); 
    }
}

/**
 * @brief       ����ĳһ���ֿ�
 * @param       x, y    : ��ʾ��Ϣ����ʾ��ַ
 * @param       size    : ��ʾ��Ϣ�����С
 * @param       fpath   : ����·��
 * @param       fx      : ���µ�����
 *   @arg                 0, ungbk;
 *   @arg                 1, gbk12;
 *   @arg                 2, gbk16;
 *   @arg                 3, gbk24;
 * @param       color   : ������ɫ
 * @retval      0, �ɹ�; ����, �������;
 */
static uint8_t fonts_update_fontx(uint16_t x, uint16_t y, uint8_t size, uint8_t *fpath, uint8_t fx, uint32_t color)
{
    uint32_t flashaddr = 0;
    FIL *fftemp;
    uint8_t *tempbuf;
    uint8_t res;
    uint16_t bread;
    uint32_t offx = 0;
    uint8_t rval = 0;
    fftemp = (FIL *)malloc(sizeof(FIL));  /* �����ڴ� */

    if (fftemp == NULL)rval = 1;

    tempbuf = malloc(4096);               /* ����4096���ֽڿռ� */

    if (tempbuf == NULL) rval = 1;

    res = f_open(fftemp, (const TCHAR *)fpath, FA_READ);
    if (res) rval = 2;  /* ���ļ�ʧ�� */

    if (rval == 0)
    {
        switch (fx)
        {
            case 0:                                                 /* ���� UNIGBK.BIN */
                ftinfo.ugbkaddr = FONTINFOADDR + sizeof(ftinfo);    /* ��Ϣͷ֮�󣬽���UNIGBKת����� */
                ftinfo.ugbksize = fftemp->obj.objsize;              /* UNIGBK��С */
                flashaddr = ftinfo.ugbkaddr;
                break;

            case 1:                                                 /* ���� GBK12.BIN */
                ftinfo.f12addr = ftinfo.ugbkaddr + ftinfo.ugbksize; /* UNIGBK֮�󣬽���GBK12�ֿ� */
                ftinfo.gbk12size = fftemp->obj.objsize;             /* GBK12�ֿ��С */
                flashaddr = ftinfo.f12addr;                         /* GBK12����ʼ��ַ */
                break;

            case 2:                                                 /* ���� GBK16.BIN */
                ftinfo.f16addr = ftinfo.f12addr + ftinfo.gbk12size; /* GBK12֮�󣬽���GBK16�ֿ� */
                ftinfo.gbk16size = fftemp->obj.objsize;             /* GBK16�ֿ��С */
                flashaddr = ftinfo.f16addr;                         /* GBK16����ʼ��ַ */
                break;

            case 3:                                                 /* ���� GBK24.BIN */
                ftinfo.f24addr = ftinfo.f16addr + ftinfo.gbk16size; /* GBK16֮�󣬽���GBK24�ֿ� */
                ftinfo.gbk24size = fftemp->obj.objsize;             /* GBK24�ֿ��С */
                flashaddr = ftinfo.f24addr;                         /* GBK24����ʼ��ַ */
                break;
        }

        while (res == FR_OK)            /* ��ѭ��ִ�� */
        {
            res = f_read(fftemp, tempbuf, 4096, (UINT *)&bread);    /* ��ȡ���� */

            if (res != FR_OK) break;    /* ִ�д��� */

            fonts_partition_write(tempbuf, offx + flashaddr, bread);            /* ��0��ʼд��bread������ */
            offx += bread;
            fonts_progress_show(x, y, size, fftemp->obj.objsize, offx, color);  /* ������ʾ */

            if (bread != 4096) break;   /* ������ */
        }

        f_close(fftemp);
    }

    free(fftemp);     /* �ͷ��ڴ� */
    free(tempbuf);    /* �ͷ��ڴ� */

    return res;
}

/**
 * @brief       ���������ļ�
 *   @note      �����ֿ�һ�����(UNIGBK,GBK12,GBK16,GBK24)
 * @param       x, y    : ��ʾ��Ϣ����ʾ��ַ
 * @param       size    : ��ʾ��Ϣ�����С
 * @param       src     : �ֿ���Դ����
 *   @arg                 "0:", SD��;
 *   @arg                 "1:", FLASH��
 * @param       color   : ������ɫ
 * @retval      0, �ɹ�; ����, �������;
 */
uint8_t fonts_update_font(uint16_t x, uint16_t y, uint8_t size, uint8_t *src, uint32_t color)
{
    uint8_t *pname;
    uint32_t *buf;
    uint8_t res = 0;
    uint16_t i, j;
    FIL *fftemp;
    uint8_t rval = 0;
    res = 0XFF;
    ftinfo.fontok = 0XFF;

    pname = malloc(100);                  /* ����100�ֽ��ڴ� */
    buf = malloc(4096);                   /* ����4K�ֽ��ڴ� */
    fftemp = (FIL *)malloc(sizeof(FIL));  /* �����ڴ� */

    if (buf == NULL || pname == NULL || fftemp == NULL)
    {
        free(fftemp);
        free(pname);
        free(buf);
        return 5;   /* �ڴ�����ʧ�� */
    }

    for (i = 0; i < 4; i++) /* �Ȳ����ļ�UNIGBK,GBK12,GBK16,GBK24 �Ƿ����� */
    {
        strcpy((char *)pname, (char *)src);                     /* copy src���ݵ�pname */
        strcat((char *)pname, (char *)FONT_GBK_PATH[i]);        /* ׷�Ӿ����ļ�·�� */
        res = f_open(fftemp, (const TCHAR *)pname, FA_READ);    /* ���Դ� */

        if (res)
        {
            rval |= 1 << 7; /* ��Ǵ��ļ�ʧ�� */
            break;          /* ������,ֱ���˳� */
        }
    }

    free(fftemp);           /* �ͷ��ڴ� */

    if (rval == 0)          /* �ֿ��ļ�������. */
    {
        spilcd_show_string(x, y, 240, 320, size, "Erasing sectors... ", color);    /* ��ʾ���ڲ������� */

        for (i = 0; i < FONTSECSIZE; i++)       /* �Ȳ����ֿ�����,���д���ٶ� */
        {
            fonts_progress_show(x + 20 * size / 2, y, size, FONTSECSIZE, i, color);             /* ������ʾ */
            fonts_partition_read((uint8_t *)buf, ((FONTINFOADDR / 4096) + i) * 4096, 4096);     /* ������������������ */

            for (j = 0; j < 1024; j++)          /* У������ */
            {
                if (buf[j] != 0XFFFFFFFF) break;/* ��Ҫ���� */
            }

            if (j != 1024)
            {
                fonts_partition_erase_sector(((FONTINFOADDR / 4096) + i) * 4096); /* ��Ҫ���������� */
            }
        }

        for (i = 0; i < 4; i++) /* ���θ���UNIGBK,GBK12,GBK16,GBK24 */
        {
            spilcd_show_string(x, y, 240, 320, size, (char *)FONT_UPDATE_REMIND_TBL[i], color);
            strcpy((char *)pname, (char *)src);                                     /* copy src���ݵ�pname */
            strcat((char *)pname, (char *)FONT_GBK_PATH[i]);                        /* ׷�Ӿ����ļ�·�� */
            res = fonts_update_fontx(x + 20 * size / 2, y, size, pname, i, color);  /* �����ֿ� */

            if (res)
            {
                free(buf);
                free(pname);
                return 1 + i;
            }
        }

        /* ȫ�����º��� */
        ftinfo.fontok = 0XAA;
        fonts_partition_write((uint8_t *)&ftinfo, FONTINFOADDR, sizeof(ftinfo));    /* �����ֿ���Ϣ */
    }

    free(pname);    /* �ͷ��ڴ� */
    free(buf);

    return rval;
}

/**
 * @brief       ��ʼ������
 * @param       ��
 * @retval      0, �ֿ����; ����, �ֿⶪʧ;
 */
uint8_t fonts_init(void)
{
    uint8_t t = 0;

    storage_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
    
    if (storage_partition == NULL)
    {
        ESP_LOGE(fonts_tag, "Flash partition not found.");
        return 1;
    }
    
    while (t < 10)  /* ������ȡ10��,���Ǵ���,˵��ȷʵ��������,�ø����ֿ��� */
    {
        t++;
        fonts_partition_read((uint8_t *)&ftinfo, FONTINFOADDR, sizeof(ftinfo)); /* ����ftinfo�ṹ������ */

        if (ftinfo.fontok == 0XAA)
        {
            break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (ftinfo.fontok != 0XAA)
    {
        return 1;
    }
    
    return 0;
}
