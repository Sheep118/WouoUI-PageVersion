#ifndef __WOUOUI_GRAPH_H__
#define __WOUOUI_GRAPH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "WouoUI_font.h"
#include "WouoUI_common.h"

#define WOUOUI_MIDDLE_H             (WOUOUI_BUFF_WIDTH/2)  // 屏幕水平中间值
#define WOUOUI_MIDDLE_V             (WOUOUI_BUFF_HEIGHT/2) // 屏幕垂直中间值
#define WOUOUI_BUFF_HEIGHT_BYTE_NUM UINT_DIVISION_CELL(WOUOUI_BUFF_HEIGHT,8) //BUFF高度所需字节数

#define SCREEN_BUFF_SIZE (WOUOUI_BUFF_HEIGHT_BYTE_NUM * WOUOUI_BUFF_WIDTH)

#define PEN_MODE_NORMAL 0x00
#define PEN_MODE_XOR    (!PEN_MODE_NORMAL)
#define PEN_COLOR_BLACK  0x00               //写1的点显示为黑色(灭)
#define PEN_COLOR_WHITE (!PEN_COLOR_BLACK)  //写1的点显示为白色(亮)

typedef struct
{
    bool color_mode:1; //颜色模式，正常(|//&)还是异或 normal=0;xor=1;
    bool color:1; //画笔颜色(指前景色，背景色默认与前景色反色)(1白色，0黑色)
    bool rev_color_flag:1; //是否反色绘制标志，0不反色，1反色。
} Pen;

typedef uint8_t ScreenBuff[WOUOUI_BUFF_HEIGHT_BYTE_NUM][WOUOUI_BUFF_WIDTH];
typedef void FunSendScreenBuff(ScreenBuff);
typedef struct 
{
    ScreenBuff* p_buff; //主缓冲区
#if HARDWARE_DYNAMIC_REFRESH
    ScreenBuff* p_buff_dynamic; //用于动态刷新的对比缓冲区
#endif
    FunSendScreenBuff* p_fun_send_buff; //将缓冲区刷新到屏幕上的函数指针
} Screen;

typedef struct
{
    int16_t start_x;   // 画布起始点x坐标
    int16_t start_y;   // 画布起始点y坐标
    int16_t w;         // 画布宽度
    int16_t h;         // 画布高度
} Canvas;

typedef enum
{
    SSM_HEAD_RESTART = 0x00, //到头后重新开始
    SSM_TIAL_RESTART,        //到尾后重新开始
    SSM_TAIL_STOP,           //到尾后停止
    SSM_STOP,                //完全停止不滚动的模式
}SlideStrMode;

typedef struct 
{
    uint8_t slide_enable:1 ;  //是否使能字符串滚动
    uint8_t slide_is_finish:1 ; //单次滚动是否结束标志位
    SlideStrMode slide_mode:2;     //滚动模式
    uint8_t step;                  //每次移动的步进
    uint8_t start_delay;      //开始运动后的延迟
    int16_t str_start_x;      //用于滚动时记录字符串的起始位置
    Canvas  canvas;          //滚动画布
    char*   str;             //要滚动的字符串
}SlideStr; //自动滚动字符串对象


typedef enum {
    BLUR_0_4 = 0x00, //完全显示
    BLUR_1_4,
    BLUR_2_4,
    BLUR_3_4,
    BLUR_4_4, //完全虚化
} BLUR_DEGREE;

void WouoUI_GraphSetSendBuffFun(FunSendScreenBuff fun);
void WouoUI_GraphSetBuff(ScreenBuff* buff);
void WouoUI_GraphSetPen(Pen* pen);
void WouoUI_BuffClear(void);
void WouoUI_GraphSetPenColor(uint8_t color);
void WouoUI_BuffSend(void);
#if HARDWARE_DYNAMIC_REFRESH
    void WouoUI_BuffSendDynamic(void);
    void WouoUI_GraphSetDynamicBuff(ScreenBuff* buff);
#endif

int16_t WouoUI_CanvasDrawASCII(Canvas *canvas, int16_t x, int16_t y, sFONT font, char c);
void WouoUI_CanvasDrawStr(Canvas *canvas, int16_t x, int16_t y, sFONT font, uint8_t *str);
void WouoUI_CanvasDrawSlideStr(SlideStr* ss, int16_t y, sFONT font);
void WouoUI_CanvasSlideStrReset(SlideStr* ss);
void WouoUI_CanvasDrawStrWithNewline(Canvas *canvas, int16_t x, int16_t y, sFONT font, uint8_t *str, uint8_t lineSpacing);
void WouoUI_CanvasDrawStrAutoNewline(Canvas *canvas, int16_t x, int16_t y, sFONT font, uint8_t *str);
uint16_t WouoUI_GetStrWidth(const char *str, sFONT font);
uint16_t WouoUI_GetStrHeight(const char *str, sFONT font);
uint16_t WouoUI_GetStrHeightAutoNewLine(int16_t canvas_w, const char* str, sFONT font);
void WouoUI_CanvasDrawLine_V(Canvas *canvas, int16_t x, int16_t y_start, int16_t y_end);
void WouoUI_CanvasDrawDashedLine_V(Canvas *canvas, int16_t x, int16_t y_start, int16_t y_end, uint8_t DashedStyle, uint8_t Unit_Length);
void WouoUI_CanvasDrawLine_H(Canvas *canvas, int16_t x_start, int16_t x_end, int16_t y);
void WouoUI_GraphDrawDashedLine_H(Canvas *canvas, int16_t x_start, int16_t x_end, int16_t y, uint8_t DashedStyle, uint8_t Unit_Length);
void WouoUI_CanvasDrawRBox(Canvas *canvas, int16_t x_start, int16_t y_start, int16_t width, int16_t height, uint8_t r);
void WouoUI_CanvasDrawRBoxEmpty(Canvas *canvas, int16_t x_start, int16_t y_start, int16_t width, int16_t height, uint8_t r);
void WouoUI_CanvasDrawBoxRightAngle(Canvas *canvas, int16_t x_start, int16_t y_start, int16_t width, int16_t height, uint8_t r);
void WouoUI_CanvasDrawBMP(Canvas *canvas, int16_t x, int16_t y, int16_t width, int16_t height, const uint8_t *BMP, uint8_t color);
void WouoUI_CanvasDrawPoint(Canvas *canvas, int16_t x, int16_t y);
void WouoUI_CanvasDrawLine(Canvas *canvas, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void WouoUI_BuffAllBlur(BLUR_DEGREE blur);

#ifdef __cplusplus
}
#endif

#endif
