#ifndef _ONEWIRE_BUS_H
#define _ONEWIRE_BUS_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "platform.h"

void WT588F_GPIO_Init(void);                  // 初始化 DATA+BUSY 脚
void WT588F_Set_Volume(uint8_t volume_code); // 设置音量
void WT588F_Play_Voice(uint8_t voice_addr);  // 播放语音
void WT588F_Stop_Voice(void);                // 停止播放
#endif /* _ONEWIRE_BUS_H */
void WT588F_Init(void); // 初始化语音模块
