#ifndef __WOUOUI_FONT_H
#define __WOUOUI_FONT_H

/*最大字体微软雅黑24 (32x41) */
#define MAX_HEIGHT_FONT 41
#define MAX_WIDTH_FONT 32
#define OFFSET_BITMAP

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

// !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
// ASCII
typedef struct _tFont {
    const uint8_t *table;
    uint16_t WidthHeight;
    uint16_t Width;
    uint16_t Height;
} sFONT;

// GB2312
typedef struct // 汉字字模数据结构
{
    unsigned char index[2];                                  // 汉字内码索引
    const char matrix[MAX_HEIGHT_FONT * MAX_WIDTH_FONT / 8]; // 点阵码数据
} CH_CN;

typedef struct
{
    const CH_CN *table;
    uint16_t size;
    uint16_t ASCII_Width;
    uint16_t Width;
    uint16_t Height;
} cFONT;

// extern sFONT Font_5_8;
extern const sFONT Font_6_8;
extern const sFONT Font_6_12;
extern const sFONT Font_7_12;
extern const sFONT Font_8_16;
// extern sFONT Font_14_20;
extern const sFONT Font_12_24;

extern cFONT Font12CN;
extern cFONT Font24CN;

#define GET_FNOT_H(fnot) (fnot.Height)
#define GET_FNOT_W(fnot) (fnot.Width)


#ifdef __cplusplus
}
#endif

#endif
