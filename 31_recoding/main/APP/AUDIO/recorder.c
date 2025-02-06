/**
 ****************************************************************************************************
 * @file        recorder.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       wav���� ����
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

#include "recorder.h"


uint32_t g_wav_size = 0;    /* wav���ݴ�С(�ֽ���,�������ļ�ͷ!!) */

uint8_t g_rec_sta = 0;  /**
                         * ¼��״̬
                         * [7]:0,û�п���¼��;1,�Ѿ�����¼��;
                         * [6:1]:����
                         * [0]:0,����¼��;1,��ͣ¼��;
                         */

SemaphoreHandle_t i2s_mutex;
static QueueSetHandle_t xQueueSet;  /* ������м� */
SemaphoreHandle_t key0_xSemaphore;  /* �����ֵ�ź��� */
SemaphoreHandle_t key1_xSemaphore;  /* �����ֵ�ź��� */
SemaphoreHandle_t key2_xSemaphore;  /* �����ֵ�ź��� */
__WaveHeader *wavhead = NULL;       /* wavͷ��ָ�� */
uint32_t recsec = 0;                /* ¼��ʱ�� */
/* ˫������� */
uint8_t *current_buffer = NULL;     /* ��ǰ���� */
uint8_t *next_buffer = NULL;        /* д���� */


/**
 * @brief       ����PCM ¼��ģʽ
 * @param       ��
 * @retval      ��
 */
void recoder_enter_rec_mode(void)
{
    myi2s_init();
    es8388_adda_cfg(0, 1);          /* ����ADC */
    es8388_input_cfg(0);            /* ��������ͨ��(ͨ��1,MIC����ͨ��) */
    es8388_mic_gain(8);             /* MIC��������Ϊ��� */
    es8388_alc_ctrl(3, 4, 4);       /* ����������ALC����,�����¼������ */
    es8388_output_cfg(0, 0);        /* �ر�ͨ��1��2����� */
    es8388_spkvol_set(0);           /* �ر�����. */
    es8388_i2s_cfg(0, 3);           /* �����ֱ�׼,16λ���ݳ��� */
    i2s_set_samplerate_bits_sample(I2S_SAMPLE_RATE,I2S_BITS_PER_SAMPLE_16BIT);    /* ��ʼ��I2S */
    i2s_trx_start();                /* ����I2S */
    recoder_remindmsg_show(0);
}

/**
 * @brief       ����PCM ����ģʽ
 * @param       ��
 * @retval      ��
 */
void recoder_enter_play_mode(void)
{
    i2s_trx_stop();                 /* ֹͣ¼�� */
    i2s_deinit();                   /* ж��I2S */
    recoder_remindmsg_show(1);      /* ��ʾ��ʾ��Ϣ */
}

/**
 * @brief       ��ʼ��WAVͷ
 * @param       wavhead : wav�ļ�ͷָ��
 * @retval      ��
 */
void recoder_wav_init(__WaveHeader *wavhead)
{
    wavhead->riff.ChunkID = 0x46464952;                  /* "RIFF" */
    wavhead->riff.ChunkSize = 0;                         /* ��δȷ��,�����Ҫ���� */
    wavhead->riff.Format = 0x45564157;                   /* "WAVE" */
    wavhead->fmt.ChunkID = 0x20746D66;                   /* "fmt " */
    wavhead->fmt.ChunkSize = 16;                         /* PCM��ʽ���СΪ16���ֽ� */
    wavhead->fmt.AudioFormat = 0x01;                     /* 0x01,��ʾPCM; 0x00,��ʾIMA ADPCM */
    wavhead->fmt.NumOfChannels = 2;                      /* ˫���� */
    wavhead->fmt.SampleRate = 16000;                     /* ��������Ϊ16000Hz */
    wavhead->fmt.BitsPerSample = 16;                     /* 16λPCM */
    wavhead->fmt.BlockAlign = wavhead->fmt.NumOfChannels * (wavhead->fmt.BitsPerSample / 8);  /* �����=ͨ����*ÿ���������ֽ��� */
    wavhead->fmt.ByteRate = wavhead->fmt.SampleRate * wavhead->fmt.BlockAlign; /* �ֽ�����=������*����� */
    wavhead->data.ChunkID = 0x61746164;                  /* "data" */
    wavhead->data.ChunkSize = 0;                         /* ���ݴ�С,�����Ҫ���� */
}

/**
 * @brief       ��ʾ¼��ʱ�������
 * @param       tsec : ʱ�䣨��λ : �룩
 * @param       kbps : ����
 * @retval      ��
 */
void recoder_msg_show(uint32_t tsec, uint32_t kbps)
{
    spilcd_show_string(30, 210, 200, 16, 16, "TIME:", RED);        /* ��ʾ¼��ʱ�� */
    spilcd_show_xnum(30 + 40, 210, tsec / 60, 2, 16, 0x80, RED);   /* ���� */
    spilcd_show_char(30 + 56, 210, ':', 16, 0, RED);
    spilcd_show_xnum(30 + 64, 210, tsec % 60, 2, 16, 0x80, RED);   /* ���� */

    spilcd_show_string(140, 210, 200, 16, 16, "KPBS:", RED);   /* ��ʾ���� */
    spilcd_show_num(140 + 40, 210, kbps / 1000, 4, 16, RED);   /* ������ʾ */
}

/**
 * @brief       ��ʾ��Ϣ
 * @param       mode : ����ģʽ
 *   @arg       0, ¼��ģʽ
 *   @arg       1, ����ģʽ
 * @retval      ��
 */
void recoder_remindmsg_show(uint8_t mode)
{
    spilcd_fill(30, 120, spilcddev.width, spilcddev.height, WHITE);            /* ���ԭ������ʾ */

    if (mode == 0)  /* ¼��ģʽ */
    {
        spilcd_show_string(30, 120, 200, 16, 16, "KEY0:REC/PAUSE", BLUE);
        spilcd_show_string(30, 140, 200, 16, 16, "KEY1:STOP&SAVE", BLUE);
        spilcd_show_string(30, 160, 200, 16, 16, "KEY2:PLAY", BLUE);
    }
    else            /* ����ģʽ */
    {
        spilcd_show_string(30, 120, 200, 16, 16, "KEY0:STOP Play", BLUE);
        spilcd_show_string(30, 140, 200, 16, 16, "KEY2:PLAY/PAUSE", BLUE);
    }
} 

/**
 * @brief       ͨ��ʱ���ȡ�ļ���
 * @note        ������SD������,��֧��FLASH DISK����
 * @note        ��ϳ�:����"0:RECORDER/REC00001.wav"���ļ���
 * @param       pname : �ļ�·��
 * @retval      ��
 */
void recoder_new_pathname(uint8_t *pname)
{
    uint8_t res;
    uint16_t index = 0;
    FIL *ftemp;
    ftemp = (FIL *)malloc(sizeof(FIL));           /* ����FIL�ֽڵ��ڴ����� */

    if (ftemp == NULL) 
    {
        return;  /* �ڴ�����ʧ�� */
    }

    while (index < 0xFFFF)
    {
        sprintf((char *)pname, "0:RECORDER/REC%05d.wav", index);
        res = f_open(ftemp, (const TCHAR *)pname, FA_READ); /* ���Դ�����ļ� */

        if (res == FR_NO_FILE)
        {
            break;              /* ���ļ���������=����������Ҫ��. */
        }

        index++;
    }

    free(ftemp);
}

/**
 * @brief       WAV¼������
 * @param       arg:����������
 * @retval      ��
 */
static void recorder_task(void * arg)
{
    QueueSetMemberHandle_t activate_member = NULL;
    UINT bw;
    size_t bytes_read = 0;                  /* ��ȡд��¼���ļ���С */
    uint8_t file_write_res = 0;             /* ��һ��д����ɱ�־ */
    uint8_t rval = 0;                       /* ��ȡTF״̬ */
    FF_DIR recdir;                          /* Ŀ¼ */
    FIL *f_rec = NULL;                      /* ¼���ļ� */
    static uint8_t *pdatabuf = NULL;        /* ���ݻ���ָ�� */
    static uint8_t *pdatabuf1 = NULL;       /* ���ݻ���ָ�� */
    uint8_t *pname = NULL;                  /* �ļ����ƴ洢buf */
    FSIZE_t recorder_file_read_pos = 0;     /* ��¼��ǰ¼���ļ���ȡλ�� */
    uint32_t g_wav_size_tatol = 0;          /* wav���ݴ�С(�ֽ���,�������ļ�ͷ!!) */

    /* ��Դ�����ڴ� */
    pdatabuf = malloc(REC_RX_BUF_SIZE);                     /* �洢¼������ */
    pdatabuf1 = malloc(REC_RX_BUF_SIZE);                     /* �洢¼������ */
    f_rec = (FIL*)malloc(sizeof(FIL));                      /* �ļ�ָ�� */
    wavhead = (__WaveHeader *)malloc(sizeof(__WaveHeader)); /* wavͷ�� */
    pname = malloc(255);                                    /* �洢�ļ�����buf */

    if (!f_rec || !wavhead || !pname || !pdatabuf || !pdatabuf1)
    {
        goto exit;
    }
    
    memset(pdatabuf,0,REC_RX_BUF_SIZE);
    memset(pdatabuf1,0,REC_RX_BUF_SIZE);

    current_buffer = pdatabuf;
    next_buffer = pdatabuf1;

    /* ���ļ��У���û�У����Զ����� */
    while (f_opendir(&recdir, "0:/RECORDER"))
    {
        f_mkdir("0:/RECORDER"); /* ������Ŀ¼ */
    }

    xTaskNotifyGive(arg);
    /* ��������������ֹ���ݴ洢���� */
    i2s_mutex = xSemaphoreCreateRecursiveMutex();
    xSemaphoreGiveRecursive(i2s_mutex);

    recoder_enter_rec_mode();   /* ����¼��ģʽ */
    pname[0] = 0;               /* pnameû���κ��ļ��� */

    while (rval == 0)
    {
        activate_member = xQueueSelectFromSet(xQueueSet, 10);/* �ȴ����м��еĶ��н��յ���Ϣ */

        if (activate_member == key0_xSemaphore)
        {
            xSemaphoreTake(activate_member, portMAX_DELAY);

            if (g_rec_sta & 0x01 && recorder_file_read_pos != 0 && file_write_res == 1) /* ��ͣ¼�� */
            {
                g_rec_sta &= 0xFE;          /* �ָ�¼�� */
            }
            else if (g_rec_sta & 0x80 && file_write_res == 1)   /* ����¼�� */
            {
                g_rec_sta |= 0x01;          /* ��ͣ¼�� */
                recorder_file_read_pos = f_tell(f_rec);         /* ��¼��ǰ�ļ�д���Ǹ�λ�� */
            }
            else                            /* δ��ʼ¼�� */
            {
                recsec = 0;
                recoder_new_pathname(pname);
                text_show_string(30, 190, spilcddev.width, 16, "¼��:", 16, 0, RED);
                text_show_string(30 + 40, 190, spilcddev.width, 16, (char *)pname + 11, 16, 0, RED);
                recoder_wav_init(wavhead);  /* ��ʼ��wavͷ */
                i2s_set_samplerate_bits_sample(wavhead->fmt.SampleRate,I2S_BITS_PER_SAMPLE_16BIT);    /* ��ʼ��I2S */
                i2s_trx_start();            /* ����I2S */
                rval = f_open(f_rec, (const TCHAR *)pname, FA_CREATE_ALWAYS | FA_WRITE);
                /* ���ļ�ʧ�� */
                if (rval != FR_OK)
                {
                    g_rec_sta = 0;          /* �ر�¼�� */
                    rval = 0xFE;            /* ��ʾSD������ */
                }
                else
                {
                    f_lseek(f_rec, sizeof(__WaveHeader));   /* ƫ�Ƶ����ݴ洢��ַ */
                    recoder_msg_show(0, 0); /* ��ʾ¼��ʱ�� */
                    g_rec_sta |= 0x80;      /* ��ʼ¼�� */
                }
            }
        }
        else if (activate_member == key1_xSemaphore)
        {
            xSemaphoreTake(activate_member, portMAX_DELAY);
            if ((g_rec_sta & 0x80 )&& (file_write_res == 1) &&(g_wav_size_tatol == g_wav_size)) /* ��¼������һ��¼��д����� */
            {
                xSemaphoreTakeRecursive(i2s_mutex, portMAX_DELAY);
                f_lseek(f_rec, 0);                          /* ƫ�Ƶ��ļ�ͷ */
                g_rec_sta = 0;
                wavhead->riff.ChunkSize = g_wav_size + 36;  /* �����ļ��Ĵ�С-8; */
                wavhead->data.ChunkSize = g_wav_size;       /* ���ݴ�С */
                f_write(f_rec, (const void *)wavhead, sizeof(__WaveHeader), (UINT *)&bw); /* д��ͷ���� */
                f_close(f_rec);
                g_wav_size_tatol = 0;
                g_wav_size = 0;
                xSemaphoreGiveRecursive(i2s_mutex);
            }
            g_rec_sta = 0;
            recsec = 0;
            spilcd_fill(30, 190, spilcddev.width, spilcddev.height, WHITE); /* �����ʾ */
        }
        else if (activate_member == key2_xSemaphore)
        {
            xSemaphoreTake(activate_member, portMAX_DELAY);
            if (g_rec_sta == 0 && file_write_res == 1)
            {
                text_show_string(30, 190, spilcddev.width, 16, "����:", 16, 0, RED);
                text_show_string(30 + 40, 190, spilcddev.width, 16, (char *)pname + 11, 16, 0, RED);
                recoder_enter_play_mode();  /* ���벥��ģʽ */
                audio_play_song(pname);     /* ����¼�� */
                spilcd_fill(30, 190, spilcddev.width, spilcddev.height, WHITE); /* �����ʾ */
                recoder_enter_rec_mode();   /* �ص�¼��ģʽ */
                g_rec_sta = 0;
                bytes_read = 0;
                memset(current_buffer,0,REC_RX_BUF_SIZE);
            }
        }

        /* ����¼��������д����״̬��ʾ */
        if (g_rec_sta & 0x80 && !(g_rec_sta & 0x01))
        {
            xSemaphoreTakeRecursive(i2s_mutex, portMAX_DELAY);
            file_write_res = 0;
            bytes_read = i2s_rx_read(current_buffer, REC_RX_BUF_SIZE);    /* ��ȡ¼���ļ� */
            g_wav_size_tatol += bytes_read;

            if (bytes_read == REC_RX_BUF_SIZE)
            {
                if (recorder_file_read_pos != 0)
                {
                    f_lseek(f_rec, recorder_file_read_pos);
                    recorder_file_read_pos = 0;
                }
                /* д�뵽�ļ����� */
                rval = f_write(f_rec, current_buffer, REC_RX_BUF_SIZE, (UINT*)&bw);

                if (rval != FR_OK)
                {
                    printf("write error:%d\r\n", rval);
                }
                else
                {
                    file_write_res = 1;
                    g_wav_size += bytes_read;
                    /* �л������� */
                    uint8_t *temp = current_buffer;
                    current_buffer = next_buffer;
                    next_buffer = temp;
                }
            }

            xSemaphoreGiveRecursive(i2s_mutex);
        }

    }

exit:
    /* ������Դ */
    free(pdatabuf);
    free(f_rec);
    free(wavhead);
    free(pname);
    /* ɾ������ */
    vTaskDelete(NULL);
}

/**
 * @brief       WAV¼��
 * @param       ��
 * @retval      ��
 */
void wav_recorder(void)
{
    uint8_t key = 0;
    uint8_t timecnt = 0;       /* ��ʱ�� */
    /* ����¼���߳� */
    BaseType_t task_created = xTaskCreatePinnedToCore(recorder_task,
                                                     "recorder_task",
                                                     4096,
                                                     xTaskGetCurrentTaskHandle(),
                                                     10, NULL, 0);
    assert(task_created == pdTRUE);
    
    /* �ȴ�¼���̵߳�֪ͨ���� */
    ulTaskNotifyTake(false, portMAX_DELAY);
    /* �������м� */
    xQueueSet = xQueueCreateSet(3);
    /* �����ź��� */
    key0_xSemaphore = xSemaphoreCreateBinary();
    key1_xSemaphore = xSemaphoreCreateBinary();
    key2_xSemaphore = xSemaphoreCreateBinary();
    /* ���ź������뵽���м� */
    xQueueAddToSet(key0_xSemaphore, xQueueSet);
    xQueueAddToSet(key1_xSemaphore, xQueueSet);
    xQueueAddToSet(key2_xSemaphore, xQueueSet);

    while (1)
    {
        key = xl9555_key_scan(0);

        switch (key)
        {
            case KEY0_PRES:
                xSemaphoreGive(key0_xSemaphore);
                break;
            case KEY1_PRES:
                xSemaphoreGive(key1_xSemaphore);
                break;
            case KEY2_PRES:
                xSemaphoreGive(key2_xSemaphore);
                break;
            default:
                break;
        }

        uint32_t current_rec_time = g_wav_size / (16000 * 4);

        timecnt++;

        if (timecnt % 20 == 0)
        {
            LED0_TOGGLE();  /* ��˸��ʾ */
        }

        if (recsec != current_rec_time)
        {
            recsec = current_rec_time;
            recoder_msg_show(recsec, wavhead->fmt.SampleRate * wavhead->fmt.NumOfChannels * wavhead->fmt.BitsPerSample);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
