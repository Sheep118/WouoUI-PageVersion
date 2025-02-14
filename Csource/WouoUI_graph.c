#include "WouoUI_graph.h"

static Screen cur_screen; //当前操作的屏幕对象，是个指针集合(所以没有必要使用指针)
static Pen* p_cur_pen; //当前画笔的指针

void WouoUI_GraphSetSendBuffFun(FunSendScreenBuff fun){
    cur_screen.p_fun_send_buff = fun; 
}

void WouoUI_GraphSetBuff(ScreenBuff* buff) {
    cur_screen.p_buff = buff;
}

void WouoUI_GraphSetPen(Pen* pen) {
    p_cur_pen = pen;
}

/**
 * @brief : void OLED_SetPointColor(uint8_t color)
 * @param : 设置画笔颜色(即在缓存点上写1对应的颜色)，color：0=灭,1=亮,2=反色
 * @attention : None
 */
void WouoUI_GraphSetPenColor(uint8_t color) {
    switch (color) {
    case 0:
        p_cur_pen->color_mode = PEN_MODE_NORMAL;
        p_cur_pen->color = p_cur_pen->rev_color_flag ? PEN_COLOR_WHITE : PEN_COLOR_BLACK;
        break;
    case 1:
        p_cur_pen->color_mode = PEN_MODE_NORMAL;
        p_cur_pen->color = p_cur_pen->rev_color_flag ? PEN_COLOR_BLACK : PEN_COLOR_WHITE;
        break;
    case 2:
        p_cur_pen->color_mode = PEN_MODE_XOR;
        break;
    default:break;
    }
}

/**
 * @brief 使能反显
 *
 * @param reverse 是否反显
 */
void WouoUI_GraphReversePenColor(bool reverse) {
    p_cur_pen->rev_color_flag = reverse;
}

#if HARDWARE_DYNAMIC_REFRESH
void WouoUI_GraphSetDynamicBuff(ScreenBuff* buff){
   cur_screen.p_buff_dynamic = buff;
}
#endif

/**
 * @brief : void WouoUI_BuffClear(void)
 * @param : Nonw
 * @attention : 清空整个buff
 */
void WouoUI_BuffClear(void) {
    if(p_cur_pen->color == PEN_COLOR_WHITE) //前景色为白色，背景刷黑色
        memset(*(cur_screen.p_buff), 0x00, sizeof(ScreenBuff));
    else 
        memset(*(cur_screen.p_buff), 0xFF, sizeof(ScreenBuff));
}
/**
 * @brief : void WouoUI_BuffSend(void)
 * @param : 更新整个buff到oled
 */
void WouoUI_BuffSend(void) {
    cur_screen.p_fun_send_buff(*(cur_screen.p_buff));
}


#if HARDWARE_DYNAMIC_REFRESH
/**
 * @brief : void WouoUI_BuffSendDynamic(void)
 * @param : 动态更新整个buff到oled：画面无变化时不更新
 */
void WouoUI_BuffSendDynamic(void) {
    if (memcmp(*(cur_screen.p_buff_dynamic), *(cur_screen.p_buff), sizeof(ScreenBuff))) {
        memcpy(*(cur_screen.p_buff_dynamic), *(cur_screen.p_buff), sizeof(ScreenBuff));
        cur_screen.p_fun_send_buff(*(cur_screen.p_buff_dynamic));
    }
}
#endif


/*
函数：void WouoUI_BuffWriteByte(uint8_t x, uint8_t y , uint8_t val)
参数：x y 写入buff的位置，一整个字节的起始位置 coverORadd 对应的字节与\或还是直接覆盖
注意'='是直接将输入赋值给buff，'|'是将输入为1的位在buff中置1(写白点)，'&'内部会自动取反的，及将输入的val中为1的位在buff中置0（写黑点）
返回值：无
说明：主要是用于数组写入时防止越界
*/
static void WouoUI_BuffWriteByte(int16_t x, int16_t y, uint8_t val) {
    if (x > (WOUOUI_BUFF_WIDTH - 1) || y > (WOUOUI_BUFF_HEIGHT_BYTE_NUM - 1) || x < 0 || y < 0)
        return;
    if (p_cur_pen->color_mode == PEN_MODE_NORMAL) {
        if (p_cur_pen->color == PEN_COLOR_WHITE)
            (*(cur_screen.p_buff))[y][x] |= val;
        else
            (*(cur_screen.p_buff))[y][x] &= (~val);
    } else
        (*(cur_screen.p_buff))[y][x] ^= val;
}

/**
 * @brief : WouoUI_BuffAllBlur(BLUR_DEGREE blur)
 * @param : blur为模糊度 BLUR_0_4, BLUR_1_4, BLUR_2_4, BLUR_3_4, BLUR_4_4
 * @attention :
 */
void WouoUI_BuffAllBlur(BLUR_DEGREE blur) {
    if (blur == BLUR_0_4)
        return;
    static const uint8_t BLUR_PATTERN[5][2] = {
        // 定义模糊度查找表
        {0x00, 0x00}, // BLUR_0_4
        {0x55, 0x00}, // BLUR_1_4
        {0x55, 0xAA}, // BLUR_2_4
        {0xFF, 0xAA}, // BLUR_3_4
        {0xFF, 0xFF}  // BLUR_4_4
    };
    uint8_t pattern_even = BLUR_PATTERN[blur][0];
    uint8_t pattern_odd = BLUR_PATTERN[blur][1];
    for (uint8_t page = 0; page < WOUOUI_BUFF_HEIGHT_BYTE_NUM; page++) { // 每8行一组进行处理
        for (uint16_t col = 0; col < WOUOUI_BUFF_WIDTH; col += 2) { // 一次处理两列
            WouoUI_BuffWriteByte(col, page, pattern_even);
            WouoUI_BuffWriteByte(col + 1, page, pattern_odd);
        }
    }
}

/**
 * @brief : static void WouoUI_CanvasWriteByte(Canvas * canvas ,int16_t x, int16_t y, uint8_t val)
 * @param : canvas 画图窗口，x相对于画图原点的横坐标，y相对于画图原点的纵坐标，写入的一个字节
 * @attention : OLED_OK/OUT/ERR
 */
static void WouoUI_CanvasWriteByte(Canvas *canvas, int16_t x, int16_t y, uint8_t val) {
    uint8_t n = 0, m = 0, temp1 = 0, temp2 = 0;
    int16_t real_y = 0, over_bit1 = 0, over_bit2 = 0; 
    if (x > canvas->w || y > canvas->h || x < 0 || y < -7)
        return; // 超出窗口大小
    if (y + 7 > (canvas->h - 1)) val &= (~(0xFF << (canvas->h - y))); // 超过窗口的部分不显示
    if (y >= -7 && y < 0) over_bit1 = (-1 * y); //没到窗口的部分也不显示
    real_y = canvas->start_y + y; //增加了对canvas_y为负的时候的处理
    if (real_y >= -7 && real_y < 0) { 
        over_bit2 = (-1 * real_y);
        val >>= MAX(over_bit1,over_bit2); //处理超过窗口和buff部分的值
        n = 0;m = 0;
    } else {
        val &= (0xFF << over_bit1); //去掉低的溢出位
        n = real_y / 8;
        m = real_y % 8;
    }
    temp1 = val << m;
    temp2 = (val >> (8 - m));
    // 在全局buff中第几个字节的第几位 ,取出低字节的位,取出高字节的位
    if (m == 0)
        WouoUI_BuffWriteByte(canvas->start_x + x, n, val); // 恰好是整字节
    else if (m != 0) {
        WouoUI_BuffWriteByte(canvas->start_x + x, n, temp1);
        WouoUI_BuffWriteByte(canvas->start_x + x, n + 1, temp2);
    }
}

/*
函数：void WouoUI_CanvasDrawASCII(Canvas *canvas,int16_t x, int16_t y ,uint8_t size, char c)
参数：[in] win， x，y ,size(12,16,24) c
返回值：目前递增到的x的位置
说明：绘制单个字符
*/
int16_t WouoUI_CanvasDrawASCII(Canvas *canvas, int16_t x, int16_t y, sFONT font, char c) {
    c = c - ' '; // 得到偏移值
    switch (font.WidthHeight) {
    case 68: // 8号字6*8
        for (uint8_t i = 0; i < 6; i++) {
            WouoUI_CanvasWriteByte(canvas, x, y, font.table[c * 6 + i]);
            x++;
            if (x > canvas->w)
                break; // 已经超出边框没必要再写了
        }
        break;

    case 612: // 12号字6*12
        for (uint8_t i = 0; i < 6; i++) {
            WouoUI_CanvasWriteByte(canvas, x, y, font.table[c * 2 * 6 + i]);
            WouoUI_CanvasWriteByte(canvas, x, y + 8, font.table[(c * 2 + 1) * 6 + i]);
            x++;
            if (x > canvas->w)
                break; // 已经超出边框没必要再写了
        }
        break;

    case 712: // 12号字7*12
        for (uint8_t i = 0; i < 7; i++) {
            WouoUI_CanvasWriteByte(canvas, x, y, font.table[c * 2 * 7 + i]);
            WouoUI_CanvasWriteByte(canvas, x, y + 8, font.table[(c * 2 + 1) * 7 + i]);
            x++;
            if (x > canvas->w)
                break; // 已经超出边框没必要再写了
        }
        break;

    case 816: // 16号字8x16
        for (uint8_t i = 0; i < 8; i++) {
            WouoUI_CanvasWriteByte(canvas, x, y, font.table[c * 2 * 8 + i]);
            WouoUI_CanvasWriteByte(canvas, x, y + 8, font.table[(c * 2 + 1) * 8 + i]);
            x++;
            if (x > canvas->w)
                break; // 已经超出边框没必要再写了
        }
        break;

    case 1224: // 24号字12*24
        for (uint8_t i = 0; i < 12; i++) {
            WouoUI_CanvasWriteByte(canvas, x, y, font.table[c * 3 * 12 + i]);
            WouoUI_CanvasWriteByte(canvas, x, y + 8, font.table[(c * 3 + 1) * 12 + i]);
            WouoUI_CanvasWriteByte(canvas, x, y + 16, font.table[(c * 3 + 2) * 12 + i]);
            x++;
            if (x > canvas->w)
                break; // 已经超出边框没必要再写了
        }
        break;
    default:
        break;
    }
    return x;
}

/**
 * @brief : void WouoUI_CanvasDrawStr(Canvas *canvas, int16_t x, int16_t y, sFONT font, uint8_t *str)
 * @param : win指定窗口，x，y相对于窗口的坐标，str 字符串
 */
void WouoUI_CanvasDrawStr(Canvas *canvas, int16_t x, int16_t y, sFONT font, uint8_t *str) {
    int16_t cur_x = x, cur_y = y;
    while (*str != '\0') {
        WouoUI_CanvasDrawASCII(canvas, cur_x, cur_y, font, *str);
        cur_x += font.Width;
        str++;
        if (cur_x > canvas->w || cur_y > canvas->h)
            break; // 已经到边了没必要再写了
    }
}

void WouoUI_CanvasDrawSlideStr(SlideStr* ss,int16_t y,sFONT font)
{
    WouoUI_CanvasDrawStr(&(ss->canvas), ss->str_start_x, y, font, (uint8_t*)(ss->str));
    if(ss->canvas.w >= WouoUI_GetStrWidth(ss->str, font)){
        ss->slide_enable = false; //失能滚动
        ss->slide_is_finish = true; //不需要滚动时，单次滚动标记完成
    }
    if(ss->slide_enable && ss->slide_mode != SSM_STOP){ //静止模式就算使能也不会进入运动
        ss->slide_is_finish = false; //滚动中一定是未完成的
        if(0 == ss->start_delay){
            ss->str_start_x -= ss->step;
            switch (ss->slide_mode){
                case SSM_HEAD_RESTART:
                    if(ss->str_start_x + WouoUI_GetStrWidth(ss->str, font) == 0) //到头了
                        ss->str_start_x = 0;
                    break;
                case SSM_TIAL_RESTART:
                    if(ss->str_start_x + WouoUI_GetStrWidth(ss->str, font) == ss->canvas.w)
                        ss->str_start_x = 0;
                    break;
                case SSM_TAIL_STOP:
                    if(ss->str_start_x + WouoUI_GetStrWidth(ss->str, font) == ss->canvas.w){
                        ss->slide_is_finish = true; //单次滚动结束
                        ss->slide_enable = false; //停止移动
                    }
                default: break;
            }
        }else {
            ss->start_delay--;
        }
    }
}

/**
 * @brief : Resets the sliding string parameters, 可用于页面切换时使ss重新等待enable滚动
 * @param : ss - Pointer to the SlideStr structure to reset.
 * @return : None
 */
void WouoUI_CanvasSlideStrReset(SlideStr* ss)
{
    ss->str_start_x = 0;
    ss->slide_is_finish = false;
    ss->slide_enable = false; 
    ss->start_delay = WOUOUI_SLIDESTR_START_DELAY;
}

/**
 * @brief : WouoUI_CanvasDrawStrWithNewline(Canvas *canvas, int16_t x, int16_t y, sFONT font, uint8_t *str, uint8_t lineSpacing)
 * @param : win指定窗口，x，y相对于窗口的坐标，str 字符串,lineSpacing 行间距
 */
void WouoUI_CanvasDrawStrWithNewline(Canvas *canvas, int16_t x, int16_t y, sFONT font, uint8_t *str, uint8_t lineSpacing) {
    int16_t cur_x = x, cur_y = y;
    while (*str != '\0') {
        if (*str == '\n') {
            str++;
            cur_x = x;
            cur_y += (font.Height + lineSpacing);
        }
        WouoUI_CanvasDrawASCII(canvas, cur_x, cur_y, font, *str);
        cur_x += font.Width;
        str++;
        if (cur_x > canvas->w || cur_y > canvas->h)
            continue; // 已经到边了没必要再写了
    }
}

void WouoUI_CanvasDrawStrAutoNewline(Canvas *canvas, int16_t x, int16_t y, sFONT font, uint8_t *str) {
    int16_t cur_x = x, cur_y = y;
    while (*str != '\0') {
        // Handle explicit newline
        if (*str == '\n') {
            str++;
            cur_x = x;
            cur_y += (font.Height + WOUOUI_STR_LINE_SPACING);
            continue;
        }
        // Check if next character would exceed canvas width
        if (cur_x + font.Width > canvas->w) {
            cur_x = x;
            cur_y += (font.Height + WOUOUI_STR_LINE_SPACING);
        }
        // Stop if we've exceeded canvas height
        if (cur_y > canvas->h) break;
        // Draw character and advance
        WouoUI_CanvasDrawASCII(canvas, cur_x, cur_y, font, *str);
        cur_x += font.Width;
        str++;
    }
}


/**
 * @brief : void WouoUI_CanvasDrawLine_V(Canvas *canvas,int16_t x, int16_t y_start, int16_t y_end)
 * @param : 往指定窗口中画线，注意，y_end 必须大于y_start
 * @attention : None
 */
void WouoUI_CanvasDrawLine_V(Canvas *canvas, int16_t x, int16_t y_start, int16_t y_end) {
    if (y_start < 0)
        y_start = 0;
    if (y_end < 0)
        y_end = 0;
    if (y_start > canvas->w)
        y_start = canvas->w;
    if (y_end > canvas->w)
        y_end = canvas->w;
    if (x > canvas->w || x < 0)
        return;
    if (y_start > y_end) {
        int16_t index = y_end;
        y_end = y_start;
        y_start = index;
    }
    uint8_t n = 0, m = 0;
    if ((y_end - y_start) < 7)
        WouoUI_CanvasWriteByte(canvas, x, y_start, ~(0xFF << (y_end - y_start + 1)));
    else {
        uint8_t i = 0;
        n = (y_end - y_start) / 8;
        m = (y_end - y_start) % 8;
        for (i = 0; i < n; i++)
            WouoUI_CanvasWriteByte(canvas, x, y_start + i * 8, 0xFF);
        WouoUI_CanvasWriteByte(canvas, x, y_start + i * 8, ~(0xFF << (m + 1)));
    }
}

/**
 * @brief : void WouoUI_CanvasDrawDashedLine_V(Canvas *canvas,int16_t x, int16_t y_start, int16_t y_end)
 * @param : 往指定窗口中画虚线，注意，y_end 必须大于y_start
 * @attention : None
 */
void WouoUI_CanvasDrawDashedLine_V(Canvas *canvas, int16_t x, int16_t y_start, int16_t y_end, uint8_t DashedStyle, uint8_t Unit_Length) {
    if (y_start < 0)
        y_start = 0;
    if (y_end < 0)
        y_end = 0;
    if (y_start > canvas->w)
        y_start = canvas->w;
    if (y_end > canvas->w)
        y_end = canvas->w;
    if (x > canvas->w || x < 0)
        return;
    if (y_start > y_end) {
        int16_t index = y_end;
        y_end = y_start;
        y_start = index;
    }
    for (uint8_t i = 0; i < 3; i++) {
        if (Unit_Length == (0x01 << i)) {
            Unit_Length <<= 1;
            DashedStyle |= (DashedStyle << (0x01 << i));
        }
    }
    if (Unit_Length == 8) {
        uint8_t n = 0, m = 0;
        uint8_t i = 0;
        n = (y_end - y_start) / 8;
        m = (y_end - y_start) % 8;
        for (i = 0; i < n; i++)
            WouoUI_CanvasWriteByte(canvas, x, y_start + i * 8, DashedStyle);
        WouoUI_CanvasWriteByte(canvas, x, y_start + i * 8, DashedStyle & (0xFF >> (7 - m)));
    } else {
        for (uint8_t i = 0; i <= (y_end - y_start); i++)
            WouoUI_CanvasWriteByte(canvas, x, y_start + i, (DashedStyle >> (i % Unit_Length)) & 0x01);
    }
}

/**
 * @brief : void WouoUI_CanvasDrawLine_H(Canvas * canvas, int16_t x_start, int16_t x_end, int16_t y)
 * @param : 注意x_end > x_start
 */
void WouoUI_CanvasDrawLine_H(Canvas *canvas, int16_t x_start, int16_t x_end, int16_t y) {
    if (x_start < 0)
        x_start = 0;
    if (x_end < 0)
        x_end = 0;
    if (x_start > canvas->w)
        x_start = canvas->w;
    if (x_end > canvas->w)
        x_end = canvas->w;
    if (y > canvas->h || y < 0)
        return;
    if (x_start > x_end) {
        int16_t index = x_end;
        x_end = x_start;
        x_start = index;
    }
    for (uint8_t i = 0; i <= (x_end - x_start); i++)
        WouoUI_CanvasWriteByte(canvas, x_start + i, y, 0x01);
}

/**
 * @brief : void WouoUI_GraphDrawDashedLine_H(Canvas * canvas, int16_t x_start, int16_t x_end, int16_t y)
 * @param : 注意x_end > x_start
 */
void WouoUI_GraphDrawDashedLine_H(Canvas *canvas, int16_t x_start, int16_t x_end, int16_t y, uint8_t DashedStyle, uint8_t Unit_Length) {
    if (x_start < 0)
        x_start = 0;
    if (x_end < 0)
        x_end = 0;
    if (x_start > canvas->w)
        x_start = canvas->w;
    if (x_end > canvas->w)
        x_end = canvas->w;
    if (y > canvas->h || y < 0)
        return;
    if (x_start > x_end) {
        int16_t index = x_end;
        x_end = x_start;
        x_start = index;
    }
    for (uint8_t i = 0; i <= (x_end - x_start); i++)
        WouoUI_CanvasWriteByte(canvas, x_start + i, y, (DashedStyle >> (i % Unit_Length)) & 0x01);
}

/**
 * @brief : void WouoUI_CanvasDrawRBoxCommon(Canvas *canvas, int16_t x_start, int16_t y_start, int16_t width, int16_t height, uint8_t r, bool fill)
 * @param : r 倒角像素的大小
 * @attention : 绘制倒角矩形,注意倒角不能大于宽或高的1/2(鲁棒性真的好，还带了倒角检查)
 */
void WouoUI_CanvasDrawRBoxCommon(Canvas *canvas, int16_t x_start, int16_t y_start, int16_t width, int16_t height, uint8_t r, bool fill) {
    if (width < 1 || height < 1)
        return;
    uint8_t max_r = width > height ? (height - 1) >> 1 : (width - 1) >> 1;
    if (r > max_r)
        r = max_r;
    uint8_t cir_r = r;
    for (uint8_t i = 0; i < height; i++) {
        if (fill || i == 0 || i == height - 1) {
            WouoUI_CanvasDrawLine_H(canvas, x_start + r, x_start + width - r - 1, y_start + i);
        } else {
            WouoUI_CanvasWriteByte(canvas, x_start + r, y_start + i, 0x01);
            WouoUI_CanvasWriteByte(canvas, x_start + width - r - 1, y_start + i, 0x01);
        }
        if (i < cir_r && r > 0)
            r--;
        if (i >= (height - cir_r - 1) && (r + 1) <= max_r)
            r++;
    }
}

/**
 * @brief : void WouoUI_CanvasDrawRBox(Canvas *canvas, int16_t x_start, int16_t y_start, int16_t width, int16_t height, uint8_t r)
 * @param : r 倒角像素的大小
 * @attention : 绘制倒角矩形,注意倒角不能大于宽或高的1/2
 */
void WouoUI_CanvasDrawRBox(Canvas *canvas, int16_t x_start, int16_t y_start, int16_t width, int16_t height, uint8_t r) {
    WouoUI_CanvasDrawRBoxCommon(canvas, x_start, y_start, width, height, r, true);
}

/**
 * @brief : void WouoUI_CanvasDrawRBoxEmpty(Canvas *canvas, int16_t x_start, int16_t y_start, int16_t width, int16_t height, uint8_t r)
 * @param :  在窗口内绘制空心的倒角矩形,r 倒角像素的大小
 * @attention : 绘制倒角矩形,注意倒角不能大于宽或高的1/2
 */
void WouoUI_CanvasDrawRBoxEmpty(Canvas *canvas, int16_t x_start, int16_t y_start, int16_t width, int16_t height, uint8_t r) {
    WouoUI_CanvasDrawRBoxCommon(canvas, x_start, y_start, width, height, r, false);
}

/**
 * @brief 画矩形的四个直角
 *
 * @param canvas 绘制的窗口
 * @param x_start 矩形左上角x坐标
 * @param y_start 矩形左上角y坐标
 * @param width 矩形宽度
 * @param height 矩形高度
 * @param r 直角长度
 */
void WouoUI_CanvasDrawBoxRightAngle(Canvas *canvas, int16_t x_start, int16_t y_start, int16_t width, int16_t height, uint8_t r) {
    if (width < 1 || height < 1 || r < 1)
        return;
    uint8_t max_r = width > height ? (height - 1) >> 1 : (width - 1) >> 1;
    if (r > max_r)
        r = max_r;
    WouoUI_CanvasDrawLine_H(canvas, x_start, x_start + r - 1, y_start);
    WouoUI_CanvasDrawLine_H(canvas, x_start + width - r, x_start + width - 1, y_start);
    WouoUI_CanvasDrawLine_H(canvas, x_start, x_start + r - 1, y_start + height - 1);
    WouoUI_CanvasDrawLine_H(canvas, x_start + width - r, x_start + width - 1, y_start + height - 1);

    WouoUI_CanvasDrawLine_V(canvas, x_start, y_start, y_start + r - 1);
    WouoUI_CanvasDrawLine_V(canvas, x_start, y_start + height - r, y_start + height - 1);
    WouoUI_CanvasDrawLine_V(canvas, x_start + width - 1, y_start, y_start + r - 1);
    WouoUI_CanvasDrawLine_V(canvas, x_start + width - 1, y_start + height - r, y_start + height - 1);
}

/**
 * @brief : void WouoUI_CanvasDrawBMP(Canvas * canvas, int16_t x, int16_t y, int16_t width, int16_t height,const uint8_t * BMP, uint8_t color)
 * @param : heigh must be a total times of 8, the color is convenient for draw inverse-color BMP
 */
void WouoUI_CanvasDrawBMP(Canvas *canvas, int16_t x, int16_t y, int16_t width, int16_t height, const uint8_t *BMP, uint8_t color) {
    if (BMP == NULL)
        return; // 如果是NULL，直接返回
    uint8_t n = height / 8, m = height % 8;
    for (uint8_t i = 0; i < n; i++) {
        for (uint8_t j = 0; j < width; j++) {
            if (x + j > canvas->w)
                break;
            if (y + i * 8 > canvas->h)
                return;
            if (color == 1)
                WouoUI_CanvasWriteByte(canvas, x + j, y + i * 8, (BMP[i * width + j]));
            else
                WouoUI_CanvasWriteByte(canvas, x + j, y + i * 8, ~(BMP[i * width + j]));
        }
    }
    if (m != 0) {
        for (uint8_t j = 0; j < width; j++) {
            if (x + j > canvas->w)
                return;
            if (color == 1)
                WouoUI_CanvasWriteByte(canvas, x + j, y + n * 8, (BMP[n * width + j] & (~(0xFF << m))));
            else
                WouoUI_CanvasWriteByte(canvas, x + j, y + n * 8, ~(BMP[n * width + j] & (~(0xFF << m))));
        }
    }
}

/**
 * @brief : void WouoUI_CanvasDrawPoint(Canvas * canvas, int16_t x, int16_t y)
 * @param : //画点函数尚未测试
 */
void WouoUI_CanvasDrawPoint(Canvas *canvas, int16_t x, int16_t y) {
    WouoUI_CanvasWriteByte(canvas, x, y, 0x01);
}


/**
 * @brief : void WouoUI_CanvasDrawLine(Canvas* canvas,int16_t x1, int16_t y1, int16_t x2, int16_t y2)
 * @param : (x1,y1)为起点，(x2,y2)为终点
 * @attention : 使用Bresenham算法进行画直线,注意，x1 != x2
 * @author : Sheep
 * @date : 23/10/31
 */
void WouoUI_CanvasDrawLine(Canvas *canvas, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    int16_t x_small = 0, x_big = 0, temp = 0, y_small = 0, y_big = 0;
    int32_t dx = 0, dy = 0, p = 0; // deltax 和deltay
    if (x2 == x1)
        return;
    else
        p = (y2 - y1) / (x2 - x1);
    p = (p > 0) ? p : (-p); // 求斜率的绝对值,后面作误差变量使用
    if (p < 1)              // p>0 && p<1
    {
        if (x2 > x1) {
            x_small = x1;
            x_big = x2;
        } else {
            x_small = x2;
            x_big = x1;
            temp = y1;
            y1 = y2;
            y2 = temp;
        } // 默认与 xsmall对应的y是y1
        dx = x_big - x_small;
        dy = ((y1 - y2) > 0) ? (y1 - y2) : (y2 - y1);
        p = (dy << 1) - dx;
        while (x_small <= x_big) {
            WouoUI_CanvasWriteByte(canvas, x_small, y1, 0x01);
            if (p > 0) {
                if (y1 < y2)
                    y1++;
                else
                    y1--;
                p = p + (dy << 1) - (dx << 1);
            } else {
                p = p + (dy << 1);
            }
            x_small++;
        }
    } else // p>=1
    {
        if (y2 > y1) {
            y_small = y1;
            y_big = y2;
        } else {
            y_small = y2;
            y_big = y1;
            temp = x1;
            x1 = x2;
            x2 = temp;
        } // 默认与y_small对应的是x1;
        dx = (x1 > x2) ? (x1 - x2) : (x2 - x1);
        dy = y_big - y_small;
        p = (dx << 1) - dy;
        while (y_small <= y_big) {
            WouoUI_CanvasWriteByte(canvas, x1, y_small, 0x01);
            if (p > 0) {
                if (x1 < x2)
                    x1++;
                else
                    x1--;
                p = p + (dx << 1) - (dy << 1);
            } else {
                p = p + (dx << 1);
            }
            y_small++;
        }
    }
}

/**
 * @brief : uint8_t WouoUI_GetStrWidth(const char * str, sFONT font)
 * @param : 得到字符串的宽度
 * @attention : len
 */
uint16_t WouoUI_GetStrWidth(const char *str, sFONT font) {
    return strlen(str) * font.Width;
}

/**
 * @brief : uint8_t WouoUI_GetStrHeight(const char * str, sFONT font)
 * @param : 得到字符串的高度
 * @attention : len
 */
uint16_t WouoUI_GetStrHeight(const char *str, sFONT font) {
    uint8_t lines = 1;
    if (str == NULL)
        return 0;
    while (*str != '\0') {
        if (*str == '\n')
            lines++;
        str++;
    }
    return lines * font.Height + (lines - 1) * WOUOUI_STR_LINE_SPACING;
}

uint16_t WouoUI_GetStrHeightAutoNewLine(int16_t canvas_w, const char* str, sFONT font)
{
    uint8_t lines = 1;
    uint16_t x_in_line = 0;
    if (str == NULL)
        return 0;
    while (*str != '\0') {
        x_in_line+=font.Width;
        if(x_in_line >= canvas_w || *str == '\n'){
            lines++;
            x_in_line = 0;
        }
        str++;
    }
    return lines * font.Height + (lines - 1) * WOUOUI_STR_LINE_SPACING;
}

