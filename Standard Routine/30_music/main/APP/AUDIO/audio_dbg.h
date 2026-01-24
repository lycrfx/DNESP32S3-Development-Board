#pragma once

// 1 = 打开调试log，0 = 关闭
#ifndef AUDIO_DBG
#define AUDIO_DBG 0
#endif

#if AUDIO_DBG
  #include "esp_log.h"
  #define ALOGI(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)
  #define ALOGW(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)
  #define ALOGE(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)
#else
  #define ALOGI(tag, fmt, ...) ((void)0)
  #define ALOGW(tag, fmt, ...) ((void)0)
  #define ALOGE(tag, fmt, ...) ((void)0)
#endif
