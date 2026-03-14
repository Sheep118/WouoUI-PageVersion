#ifndef __WOUOUI_FONT_H
#define __WOUOUI_FONT_H

// /*最大字体微软雅黑24 (32x41) */
// #define MAX_HEIGHT_FONT 41
// #define MAX_WIDTH_FONT 32
// #define OFFSET_BITMAP

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

// !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
// ASCII
typedef struct _tFont {
    const uint8_t* table;
    uint16_t Width;
    uint16_t Height;
} sFONT;

extern const sFONT Font_6_8;
extern const sFONT Font_6_12;
extern const sFONT Font_7_12;
extern const sFONT Font_8_16;
extern const sFONT Font_12_24;

#define GET_FNOT_H(fnot) (fnot.Height)
#define GET_FNOT_W(fnot) (fnot.Width)

// 有关字体的设置
#define WOUOUI_SUPPORT_CHINESE_SYMBOL 1 // 是否支持中文符号显示

#if (WOUOUI_SUPPORT_CHINESE_SYMBOL)

#    define WOUOUI_SUPPORT_CNSYMBOL_UNICODE 1 // 是否支持UNICODE编码输入
#    define WOUOUI_SUPPORT_CNSYMBOL_GB2312 1  // 是否支持GB2312编码输入
#    define WOUOUI_CN_FALLBACK_SHOW 1         // 中文字符未命中时是否显示占位框（动态空心矩形）

#    if (WOUOUI_SUPPORT_CNSYMBOL_UNICODE)
typedef char CNCodeIndexType[3]; // 汉字内码索引（UTF-8 3字节，GB2312 2字节）
#    elif (WOUOUI_SUPPORT_CNSYMBOL_GB2312)
typedef char CNCodeIndexType[2]; // 汉字内码索引（UTF-8 3字节，GB2312 2字节）
#    endif

typedef struct {
    const char* matrix;    // 点阵码数据
    CNCodeIndexType index; // 汉字内码索引
    uint8_t width;         // 字符宽度
    uint8_t height;        // 字符高度
} CH_CN;
#endif
typedef struct {
    const char* table;                  // 所有生成的汉字字模的数组
    const CNCodeIndexType* index_table; // 汉字内码索引表，按照生成汉字编码大小排列方便二分查找
    const uint16_t count;               // 汉字数量(编码表长度)
    const uint16_t Width;
    const uint16_t Height;
} cFONT;

#ifdef __cplusplus
}
#endif

#endif
