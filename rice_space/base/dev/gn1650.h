#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef GN1650_H_
#define GN1650_H_
 
 
//显示参数
#define GN1650_BRIGHT1       0x11   /*一等级亮度，打开LED显示*/
#define GN1650_BRIGHT2       0x21   /*二等级亮度，打开LED显示*/
#define GN1650_BRIGHT3       0x31   /*三等级亮度，打开LED显示*/
#define GN1650_BRIGHT4       0x41   /*四等级亮度，打开LED显示*/
#define GN1650_BRIGHT5       0x51   /*五等级亮度，打开LED显示*/
#define GN1650_BRIGHT6       0x61   /*六等级亮度，打开LED显示*/
#define GN1650_BRIGHT7       0x71   /*七等级亮度，打开LED显示*/
#define GN1650_BRIGHT8       0x01   /*八等级亮度，打开LED显示*/
#define GN1650_DSP_OFF       0x00   /*关闭LED显示*/
 
//数码管位选择
#define GN1650_DIG1     0
#define GN1650_DIG2     1
#define GN1650_DIG3     2
#define GN1650_DIG4     3
 
void GN1650_init(void);
void GN1650_cfg_display(uint8_t param);
void GN1650_clear(void);
void GN1650_print(uint8_t dig,uint8_t seg_data);
 
#endif //GN1650_H_
