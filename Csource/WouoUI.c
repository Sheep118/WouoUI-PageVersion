#include "WouoUI.h"

// 静态函数声明(方便全局变量初始化函数指针)
//从page文件引入函数的声明，以保证这些函数不会被外部使用
// TitlePage 的类方法
void WouoUI_TitlePageInParaInit(PageAddr page_addr);
bool WouoUI_TitlePageIn(PageAddr page_addr);
void WouoUI_TitlePageShow(PageAddr page_addr);
void WouoUI_TitlePageIndicatorCtrl(PageAddr page_addr);
bool WouoUI_TitlePageReact(PageAddr page_addr);
//ListPage 的类方法
void WouoUI_ListPageInParaInit(PageAddr page_addr);
bool WouoUI_ListPageIn(PageAddr page_addr);
void WouoUI_ListPageShow(PageAddr page_addr);
void WouoUI_ListPageIndicatorCtrl(PageAddr page_addr);
void WouoUI_ListPageScrollBarCtrl(PageAddr page_addr);
bool WouoUI_ListPageReact(PageAddr page_addr);
//WavePage 的类方法
void WouoUI_WavePageInParaInit(PageAddr page_addr);
void WouoUI_WavePageShow(PageAddr page_addr);
bool WouoUI_WavePageReact(PageAddr page_addr);
void WouoUI_WavePageIndicatorCtrl(PageAddr page_addr);

// MsgWin 的类方法
void WouoUI_MsgWinPageInParaInit(PageAddr page_addr); 
bool WouoUI_MsgWinPageIn(PageAddr page_addr); 
void WouoUI_MsgWinPageShow(PageAddr page_addr);
bool WouoUI_MsgWinPageReact(PageAddr page_addr);
void WouoUI_MsgWinPageIndicatorCtrl(PageAddr page_addr);

//ConfWin的类方法
void WouoUI_ConfWinPageInParaInit(PageAddr page_addr);
bool WouoUI_ConfWinPageIn(PageAddr page_addr);
void WouoUI_ConfWinPageShow(PageAddr page_addr);
bool WouoUI_ConfWinPageReact(PageAddr page_addr);
void WouoUI_ConfWinPageIndicatorCtrl(PageAddr page_addr);

//SlideVarWin的类方法
bool WouoUI_ValWinPageIn(PageAddr page_addr);
void WouoUI_ValWinPageInParaInit(PageAddr page_addr);
void WouoUI_ValWinPageShow(PageAddr page_addr);
bool WouoUI_ValWinPageReact(PageAddr page_addr);
void WouoUI_ValWinPageIndicatorCtrl(PageAddr page_addr);

//SpinWin的类方法
bool WouoUI_SpinWinPageIn(PageAddr page_addr);
void WouoUI_SpinWinPageInParaInit(PageAddr page_addr);
void WouoUI_SpinWinPageShow(PageAddr page_addr);
bool WouoUI_SpinWinPageReact(PageAddr page_addr);
void WouoUI_SpinWinPageIndicatorCtrl(PageAddr page_addr);

//ListWin的类方法
bool WouoUI_ListWinPageIn(PageAddr page_addr);
void WouoUI_ListWinPageInParaInit(PageAddr page_addr);
void WouoUI_ListWinPageInParaInit(PageAddr page_addr);
void WouoUI_ListWinPageShow(PageAddr page_addr);
bool WouoUI_ListWinPageReact(PageAddr page_addr);
void WouoUI_ListWinPageIndicatorCtrl(PageAddr page_addr);
void WouoUI_ListWinPageScrollBarCtrl(PageAddr page_addr);

//空函数用于赋值一些页面不需要的方法，防止调用到NULL,这样调用时可以不用考虑NULL指针检查
void WouoUI_FuncDoNothing(PageAddr page_addr){UNUSED_PARAMETER(page_addr);}
bool WouoUI_FuncDoNothingRetTrue(PageAddr page_addr){UNUSED_PARAMETER(page_addr);return true;}
bool WouoUI_FuncDoNothingRetFalse(PageAddr page_addr){UNUSED_PARAMETER(page_addr);return false;}

// 全局窗口变量，所有的绘制都在这个窗口内进行

//===================================全局变量==================================
// 全局UI参数集合对象(同时初始化)，这个UI的相关参数都在这个集合中定义
UiPara g_default_ui_para = {
    .ani_param = {
        [IND_ANI] = 150,  // 指示器动画速度
        [TILE_ANI] = 100, // 磁贴动画速度
        [TAG_ANI] = 100,  // 标签动画速度
        [LIST_ANI] = 100, // 列表动画速度
        [WAVE_ANI] = 100, // 波形动画速度
        [WIN_ANI] = 100,  // 弹窗动画速度
        [FADE_ANI] = 20,  // 页面渐变退出速度
    },
    .ufd_param = {
        [TILE_UFD] = false, // 磁贴图标从头展开开关
        [LIST_UFD] = false, // 菜单列表从头展开开关
    },
    .loop_param = {
        [TILE_LOOP] = true, // 磁贴图标循环模式开关
        [LIST_LOOP] = true, // 菜单列表循环模式开关
        [LIST_WIN_LOOP] = true, // 菜单弹窗循环模式开关
        },
    .slidestrstep_param ={
        [TILE_SSS] = 2,    // 磁贴字符串滚动步进
        [LIST_TEXT_SSS] = 1, // 列表文本滚动步进
        [LIST_VAL_SSS] = 1,  // 列表数值滚动步进
        [WAVE_TEXT_SSS] = 1, // 波形文本滚动步进
        [WAVE_VAL_SSS] = 1,  // 波形数值滚动步进 
        [WIN_TXT_SSS] = 1,   // 弹窗文本滚动步进
        [WIN_VAL_SSS] = 1,   // 弹窗数值滚动步进
    },
    .slidestrmode_param = {
        [TILE_SSS] = (SlideStrMode)2,      // 磁贴字符串滚动模式
        [LIST_TEXT_SSS] = (SlideStrMode)2,  // 列表文本滚动模式
        [LIST_VAL_SSS] = (SlideStrMode)2,   // 列表数值滚动模式
        [WAVE_TEXT_SSS] = (SlideStrMode)2,  // 波形文本滚动模式
        [WAVE_VAL_SSS] = (SlideStrMode)2,   // 波形数值滚动模式
        [WIN_TXT_SSS] = (SlideStrMode)2,    // 弹窗文本滚动模式
        [WIN_VAL_SSS] = (SlideStrMode)2,    // 弹窗数值滚动模式
    },
    .winbgblur_param = {
        [MGS_WBB] = (BLUR_DEGREE)2,    // 消息弹窗背景模糊程度
        [CONF_WBB] = (BLUR_DEGREE)2,   // 确认弹窗背景模糊程度
        [VAL_WBB] = (BLUR_DEGREE)2,    // 数值弹窗背景模糊程度
        [SPIN_WBB] = (BLUR_DEGREE)2,   // 微调弹窗背景模糊程度
        [LIST_WBB] = (BLUR_DEGREE)0,   // 列表弹窗背景模糊程度
    },
};

// 默认UI对象（同时进行初始化）
WouoUI default_ui = {
#if SOFTWARE_DYNAMIC_REFRESH
    .is_motionless = false,     // 初始时开启运动
#endif
    .home_page = NULL,         // 初始化当前页面和主页面均是NULL
    .current_page = NULL,
    .pen = {.color_mode = PEN_MODE_NORMAL, .color = PEN_COLOR_BLACK, .rev_color_flag = 0},   // 全局画笔变量清零
    .screen_buff = {0},                             // 屏幕缓冲区对象清零
    .pfun_sendbuff = NULL,                          //这个函数指针必须要外界赋值
    .w_all = {0, 0, WOUOUI_BUFF_WIDTH, WOUOUI_BUFF_HEIGHT},       // 全局画布变量清零
    .state = ui_page_in,         // 从没页面进入主页面，所以是lay_in
    .upara = &g_default_ui_para, // 将默认参数赋值给default_ui
    .msg_queue = {// 消息队列相关的初始化
        .queue = {msg_none},
        .font = 0,
        .rear = 0,
    },
    .indicator = {
        .x = {0, 0, 0},
        .y = {0, 0, 0},
        .w = {WOUOUI_BUFF_WIDTH, 0, 0},
        .h = {WOUOUI_BUFF_HEIGHT, 0, 0},
    },
    .scrollBar = {
        .display = true,
        .y = {0, 0, 0},
    },
    .ui_blur = {
        .blur_cur = 0,
        .blur_tgt = 0,
        .blur_end = true,
        .timer = 0,
        .blur_time = 0,
    },
    .time = 20, //整个UI的时间尺度参数
#if HARDWARE_DYNAMIC_REFRESH
    .screen_dynamic_buff = {0}, 
#endif
    //注意每个方法都需要初始化，因为调用前没有NULL检查
    .tp_var = {
        .title_ss = {
            .slide_mode = (SlideStrMode)TILE_SLIDESTR_MODE,
            .canvas = {.w = WOUOUI_BUFF_WIDTH - TILE_BAR_W,},
        },
    },
    .tp_mth = {
        .in = WouoUI_TitlePageIn,
        .in_para_init = WouoUI_TitlePageInParaInit,
        .show = WouoUI_TitlePageShow,
        .react = WouoUI_TitlePageReact,
        .indicator_ctrl = WouoUI_TitlePageIndicatorCtrl,
        .scrollbar_ctrl = WouoUI_FuncDoNothing,
    },
    .lp_var = {
        .radio_click_flag = false,
        .opt_text_ss = {.slide_mode = (SlideStrMode)LIST_TXT_SLIDESTR_MODE,},
        .opt_val_ss = {.slide_mode = (SlideStrMode)LIST_VAL_SLIDESTR_MODE,},
    },
    .lp_mth = {
        .in = WouoUI_ListPageIn,
        .in_para_init = WouoUI_ListPageInParaInit,
        .show = WouoUI_ListPageShow,
        .react = WouoUI_ListPageReact,
        .indicator_ctrl = WouoUI_ListPageIndicatorCtrl,
        .scrollbar_ctrl = WouoUI_ListPageScrollBarCtrl,
    },
    .wt_var = {
        .title_ss = {.slide_mode = (SlideStrMode)WAVE_TXT_SLIDESTR_MODE},
        .val_ss = {.slide_mode = (SlideStrMode)WAVE_VAL_SLIDESTR_MODE},
        .max_val_ss = {.slide_mode = (SlideStrMode)WAVE_VAL_SLIDESTR_MODE},
        .min_val_ss = {.slide_mode = (SlideStrMode)WAVE_VAL_SLIDESTR_MODE},
        .mid_val_ss = {.slide_mode = (SlideStrMode)WAVE_VAL_SLIDESTR_MODE},
    },
    .wt_mth = {
        .in = WouoUI_FuncDoNothingRetTrue,
        .in_para_init = WouoUI_WavePageInParaInit,
        .show = WouoUI_WavePageShow,
        .react = WouoUI_WavePageReact,
        .indicator_ctrl = WouoUI_WavePageIndicatorCtrl,
        .scrollbar_ctrl = WouoUI_FuncDoNothing,
    },
    .mw_mth = {
        .in = WouoUI_MsgWinPageIn,
        .in_para_init = WouoUI_MsgWinPageInParaInit,
        .show = WouoUI_MsgWinPageShow,
        .react = WouoUI_MsgWinPageReact,
        .indicator_ctrl = WouoUI_MsgWinPageIndicatorCtrl,
        .scrollbar_ctrl = WouoUI_FuncDoNothing,
    },
    .cw_var = {
        .btn_ss = {.slide_mode = (SlideStrMode)CONF_BTN_SLIDESTR_MODE},
    },
    .cw_mth = {
        .in = WouoUI_ConfWinPageIn,
        .in_para_init = WouoUI_ConfWinPageInParaInit,
        .show = WouoUI_ConfWinPageShow,
        .react = WouoUI_ConfWinPageReact,
        .indicator_ctrl = WouoUI_ConfWinPageIndicatorCtrl,
        .scrollbar_ctrl = WouoUI_FuncDoNothing,
    },
    .vw_var = {
        .text_ss = {.slide_mode = (SlideStrMode)CONF_BTN_SLIDESTR_MODE},
        .val_ss = {.slide_mode = (SlideStrMode)CONF_BTN_SLIDESTR_MODE},
        .min_ss = {.slide_mode = (SlideStrMode)CONF_BTN_SLIDESTR_MODE},
        .max_ss = {.slide_mode = (SlideStrMode)CONF_BTN_SLIDESTR_MODE},
    },
    .vw_mth ={
        .in = WouoUI_ValWinPageIn,
        .in_para_init = WouoUI_ValWinPageInParaInit,
        .show = WouoUI_ValWinPageShow,
        .react = WouoUI_ValWinPageReact,
        .indicator_ctrl = WouoUI_ValWinPageIndicatorCtrl,
        .scrollbar_ctrl = WouoUI_FuncDoNothing,
    },
    .spw_var = {
        .text_ss = {.slide_mode = (SlideStrMode)SPIN_WIN_SLI_TXT_MODE},
        .min_ss = {.slide_mode = (SlideStrMode)SPIN_WIN_SLI_VAL_MODE},
        .max_ss = {.slide_mode = (SlideStrMode)SPIN_WIN_SLI_VAL_MODE},
    },
    .spw_mth = {
        .in = WouoUI_SpinWinPageIn,
        .in_para_init = WouoUI_SpinWinPageInParaInit,
        .show = WouoUI_SpinWinPageShow,
        .react = WouoUI_SpinWinPageReact,
        .indicator_ctrl = WouoUI_SpinWinPageIndicatorCtrl,
        .scrollbar_ctrl = WouoUI_FuncDoNothing,
    },
    .lw_var = {
        .str_ss = {.slide_mode = (SlideStrMode)SPIN_WIN_SLI_TXT_MODE},
    },
    .lw_mth = {
        .in = WouoUI_ListWinPageIn,
        .in_para_init = WouoUI_ListWinPageInParaInit,
        .show = WouoUI_ListWinPageShow,
        .react = WouoUI_ListWinPageReact,
        .indicator_ctrl = WouoUI_ListWinPageIndicatorCtrl,
        .scrollbar_ctrl = WouoUI_ListWinPageScrollBarCtrl,
    },
};

//--------UI对象类型
WouoUI *p_cur_ui = &default_ui; // 当前操作的ui对象的指针(默认使用defaultui)

/**
 * @brief 将无符号32位整数转换为字符串
 *
 * @param num 要转换的无符号32位整数
 * @return char* 转换后的字符串
 */
char *ui_itoa(uint32_t num, char *str) {
    memset(str, 0, strlen(str)); // 将字符数组清零(用sizeof对指针是取指针大小)
    sprintf(str, "%d", num);     // 将无符号32位整数转换为字符串
    return str;                  // 返回转换后的字符串
}

char *ui_itoa_str(uint32_t num, char *str) {
    memset(str, 0, strlen(str)); // 将字符数组清零
    sprintf(str, "%d", num);     // 将无符号32位整数转换为字符串
    return str;                  // 返回转换后的字符串
}

/**
 * @brief 将浮点数转换为字符串
 *
 * @param num 要转换的数
 * @param decimalNum 小数点位数
 * @return char* 转换后的字符串
 */
char *ui_ftoa_g(int32_t num, DecimalNum decimalNum) {
    static char str[16] = {0};   // 定义一个静态字符数组，用于存储转换后的字符串
    memset(str, 0, strlen(str)); // 将字符数组清零
    switch (decimalNum) {
    case DecimalNum_0:
        sprintf(str, "%d", num);
        break;
    case DecimalNum_1:
        sprintf(str, "%g", num / 10.0f);
        break;
    case DecimalNum_2:
        sprintf(str, "%g", num / 100.0f);
        break;
    case DecimalNum_3:
        sprintf(str, "%g", num / 1000.0f);
        break;
    default:
        sprintf(str, "%d", num);
        break;
    }
    return str; // 返回转换后的字符串
}

/**
 * @brief 将浮点数转换为字符串
 *
 * @param num 要转换的数
 * @param decimalNum 小数点位数
 * @param str 转换后的字符串
 * @return char* 转换后的字符串
 */
char *ui_ftoa_g_str(int32_t num, DecimalNum decimalNum, char *str) {
    memset(str, 0, strlen(str));               // 将字符数组清零
    char *result = ui_ftoa_g(num, decimalNum); // 返回转换后的字符串
    strcpy(str, result);
    return str;
}

/**
 * @brief 将浮点数转换为字符串
 *
 * @param num 要转换的数
 * @param decimalNum 小数点位数
 * @return char* 转换后的字符串
 */
char *ui_ftoa_f(int32_t num, DecimalNum decimalNum) {
    static char str[16] = {0};   // 定义一个静态字符数组，用于存储转换后的字符串
    memset(str, 0, strlen(str)); // 将字符数组清零
    switch (decimalNum) {
    case DecimalNum_0:
        sprintf(str, "%d", num);
        break;
    case DecimalNum_1:
        sprintf(str, "%.1f", num / 10.0f);
        break;
    case DecimalNum_2:
        sprintf(str, "%.2f", num / 100.0f);
        break;
    case DecimalNum_3:
        sprintf(str, "%.3f", num / 1000.0f);
        break;
    default:
        sprintf(str, "%d", num);
        break;
    }
    return str; // 返回转换后的字符串
}

/**
 * @brief 将浮点数转换为字符串
 *
 * @param num 要转换的数
 * @param decimalNum 小数点位数
 * @param str 转换后的字符串
 * @return char* 转换后的字符串
 */
char *ui_ftoa_f_str(int32_t num, DecimalNum decimalNum, char *str) {
    memset(str, 0, sizeof(str));               // 将字符数组清零
    char *result = ui_ftoa_f(num, decimalNum); // 返回转换后的字符串
    strcpy(str, result);
    return str;
}

/**
 * @brief 获取Option项目的浮点值
 *
 * @param option 目标项目
 * @return float val的浮点值
 */
float WouoUI_GetOptionFloatVal(Option *option) {
    switch (option->decimalNum) {
    case DecimalNum_0:
        return option->val;
    case DecimalNum_1:
        return option->val / 10.0f;
    case DecimalNum_2:
        return option->val / 100.0f;
    case DecimalNum_3:
        return option->val / 1000.0f;
    default:
        return option->val;
    }
}

//-------UI相关函数
// 设置当前正在操作的UI对象
void WouoUI_SetCurrentUI(WouoUI *ui) {
    WouoUI_GraphSetBuff(&(ui->screen_buff));// 设置默认UI的屏幕缓冲区
#if HARDWARE_DYNAMIC_REFRESH
    WouoUI_GraphSetDynamicBuff(&(ui->screen_dynamic_buff));
#endif
    WouoUI_GraphSetPen(&(ui->pen));// 设置默认UI的屏幕缓冲区
    WouoUI_GraphSetSendBuffFun(ui->pfun_sendbuff);
    p_cur_ui = ui;
}
// 将当前操作的UI对象选择为默认UI对象
void WouoUI_SelectDefaultUI(void) {
    WouoUI_SetCurrentUI(&default_ui);
}


void WouoUI_AttachSendBuffFun(FunSendScreenBuff fun){
    p_cur_ui->pfun_sendbuff = fun;
    WouoUI_GraphSetSendBuffFun(p_cur_ui->pfun_sendbuff);
}



/**
 * @brief UI画面渐变消失动画初始化
 *
 * @param blur_tgt 目标渐变程度：0-4
 * @param blur_time 渐变动画间隔时间
 */
void WouoUI_BlurParaInit(uint8_t blur_tgt, uint16_t blur_time) {
    p_cur_ui->ui_blur.blur_tgt = blur_tgt;
    p_cur_ui->ui_blur.blur_end = false;
    p_cur_ui->ui_blur.blur_time = blur_time;
    p_cur_ui->ui_blur.timer = 0;
}

/**
 * @brief UI画面渐变动画处理函数
 *
 * @param time 函数轮询间隔时间
 * @return 返回true表示任务完成，通知UI可以切换状态
 */
bool WouoUI_BlurProc(uint16_t time) {
    if(false == p_cur_ui->ui_blur.blur_end){ //fade完成就不用fade了，可以作为一个开关，要开启fade的时候置false
        if (p_cur_ui->ui_blur.timer <= p_cur_ui->ui_blur.blur_time) { //用于虚化过程的延时
            p_cur_ui->ui_blur.timer += time; // 计时器累加
        } else {
            if (p_cur_ui->ui_blur.blur_cur < p_cur_ui->ui_blur.blur_tgt) {
                p_cur_ui->ui_blur.blur_cur++;
                p_cur_ui->ui_blur.timer = 0;
            }
            if (p_cur_ui->ui_blur.blur_cur > p_cur_ui->ui_blur.blur_tgt) {
                p_cur_ui->ui_blur.blur_cur--;
                p_cur_ui->ui_blur.timer = 0;
            }
        }
        p_cur_ui->ui_blur.blur_end = (p_cur_ui->ui_blur.blur_cur == p_cur_ui->ui_blur.blur_tgt);
    }
    WouoUI_GraphSetPenColor(0);
    WouoUI_BuffAllBlur((BLUR_DEGREE)(p_cur_ui->ui_blur.blur_cur));
    WouoUI_GraphSetPenColor(1);
    return p_cur_ui->ui_blur.blur_end;
}

#define PAGE_USE_METHOD(page, method) (((Page*)page)->methods->method(page)) //使用这个宏调用方法，避免传错指针

void WouoUI_IndicatorProc(Page *p)
{ //同样为了提高对消息的响应能力，tgt和cur的坐标设置需要在Animation函数调用前，
// 这样可以保证在soft动态刷新时一有消息进入，就可以接力到anim_is_finish到false使状态机全速运行
    PAGE_USE_METHOD(p, indicator_ctrl);
    // indicator anim
    WouoUI_Animation(&p_cur_ui->indicator.x, p_cur_ui->upara->ani_param[IND_ANI], p_cur_ui->time,&(p_cur_ui->anim_is_finish));
    WouoUI_Animation(&p_cur_ui->indicator.y, p_cur_ui->upara->ani_param[IND_ANI], p_cur_ui->time,&(p_cur_ui->anim_is_finish));
    WouoUI_Animation(&p_cur_ui->indicator.w, p_cur_ui->upara->ani_param[IND_ANI], p_cur_ui->time,&(p_cur_ui->anim_is_finish));
    WouoUI_Animation(&p_cur_ui->indicator.h, p_cur_ui->upara->ani_param[IND_ANI], p_cur_ui->time,&(p_cur_ui->anim_is_finish));
}

void WouoUI_ScrollBarProc(Page* p)
{//同样为了提高对消息的响应能力，tgt和cur的坐标设置需要在Animation函数调用前，
// 这样可以保证在soft动态刷新时一有消息进入，就可以接力到anim_is_finish到false使状态机全速运行
    if(WouoUI_CheckPageTypeisWin(p) && WouoUI_FuncDoNothing == p->methods->scrollbar_ctrl){//如果当前页面是弹窗页面同时弹窗页面没有对scrollbar操作的话
        PAGE_USE_METHOD(p->last_page,scrollbar_ctrl); //做背景页面的bar函数(因为部分页面有bar但弹窗不带bar函数)
    } 
    PAGE_USE_METHOD(p,scrollbar_ctrl);
    WouoUI_Animation(&p_cur_ui->scrollBar.y, p_cur_ui->upara->ani_param[LIST_ANI], p_cur_ui->time,&(p_cur_ui->anim_is_finish));
}                                      

// 注意，在以下状态机中，in out状态中，cur_page都指向当前页面，in_page都指向要进入的页面，
// 在进入page_proc状态之前，完成in到cur_page的交接，在不同情况下切换p_all的值，表示当前与ui中全局元素交互的应该时哪个页面
// 所以，对页面方法有要求就是，每个页面的in,inparainit,show,react函数内只能对自己的类变量和变量进行操作，
// 如果要操作到全局ui对象里的变量，像indicator和ScrollerBar的话，需要使用类变量暂存，再在对应的indicator_ctrl和scrollbar_ctrl方法中进行操作
void WouoUI_Proc(uint16_t time){
    p_cur_ui->time = time; // 设置UI时间尺度参数
#if SOFTWARE_DYNAMIC_REFRESH
    if(!WouoUI_MsgQueIsEmpty(&(p_cur_ui->msg_queue)))p_cur_ui->is_motionless = false; //有消息输入时开启动画
    if(p_cur_ui->current_page != NULL && !(p_cur_ui->is_motionless)){
#else 
    if(p_cur_ui->current_page != NULL){
#endif
        bool ret_in = false, ret_blur = false;//由于短路原则不直接在if中使用两个函数(存放in和 blur函数的返回值)
        Page *p_all = (Page*)p_cur_ui->current_page; // 把当前使用的与全局指示器和bar交互的页面是哪个(默认是当前页面)
        WouoUI_BuffClear();
        switch (p_cur_ui->state){
            case ui_page_in: // 进入动画(全页面渐显，弹窗是同时做渐隐)
            {
                if(WouoUI_CheckPageTypeisWin(p_cur_ui->in_page)){ //将要跳转的页面如果是弹窗的话，渐隐背景的同时要做in动画，
                    p_all = (Page *)(p_cur_ui->in_page); //弹窗的in动画内指示器动画应该是弹窗的in动画
                    PAGE_USE_METHOD(p_cur_ui->current_page, show); //绘制背景页面
                    ret_blur = WouoUI_BlurProc(p_cur_ui->time); //渐隐
                    ret_in = PAGE_USE_METHOD(p_cur_ui->in_page, in); //同时做弹窗的IN动画
                } else {
                    p_all = (Page *)(p_cur_ui->in_page); //普通页面的交互函数就是即将要进入的函数
                    ret_in= PAGE_USE_METHOD(p_cur_ui->in_page,in);// 调用页面进入函数
                    ret_blur = (WouoUI_BlurProc(p_cur_ui->time));//同时进行渐显操作
                }
                if(ret_in && ret_blur){
                    p_cur_ui->current_page = p_cur_ui->in_page; //在page_proc过程中，in cur page保持一致
                    p_cur_ui->state = ui_page_proc;  //两者都完成时进入页面显示和React
                } 
                break;
            }
            case ui_page_proc: // 页面显示
                p_cur_ui->anim_is_finish = true; //假设每一次show过程的动画都是完成的，如果有动画未完成就会被置成false.
                p_cur_ui->slide_is_finish = true; //同样假设每一次show中的滚动动画都是完成的，如果有动画未完成就会被置成false.
                //提高消息响应能力，先react再show
                p_all = (Page*)p_cur_ui->current_page; //page显示与总页面交互的也是当前页面
                if(PAGE_USE_METHOD(p_cur_ui->current_page, react))// 页面react(退出时返回true)
                {
                    if(WouoUI_CheckPageTypeisWin(p_cur_ui->current_page))WouoUI_BlurParaInit(0,p_cur_ui->upara->ani_param[FADE_ANI]);//弹窗退出是做渐显参数初始化
                    else WouoUI_BlurParaInit(4,p_cur_ui->upara->ani_param[FADE_ANI]); //渐隐参数初始化
                    p_cur_ui->state = ui_page_out;  
                }
                PAGE_USE_METHOD(p_cur_ui->current_page, show); // 页面处理任务函数
            break;
            case ui_page_out: // 页面退出函数(全页面是做渐隐，弹窗页面做渐显)
                if(WouoUI_CheckPageTypeisWin(p_cur_ui->current_page)){
                    p_all = (Page*)p_cur_ui->in_page; //与指示器交互的页面应该是背景页面
                    PAGE_USE_METHOD(p_cur_ui->in_page,show); //绘制背景页面
                    ret_blur = WouoUI_BlurProc(p_cur_ui->time); 
                    ret_in = PAGE_USE_METHOD(p_cur_ui->current_page, in);// 这个是弹窗out动画函数，起始参数不一样而已，绘制与in完全一样，所以直接用in
                    if(ret_blur && ret_in){ //渐显和out动画完成
                        p_cur_ui->current_page = p_cur_ui->in_page;
                        p_cur_ui->state = ui_page_proc; //弹窗退出时不用做in动画
                    }
                }else { //普通全页面只需要完全渐隐即可
                    p_all = (Page*)p_cur_ui->in_page; //渐隐时的指示器动画换成即将要进入页面的动画
                    PAGE_USE_METHOD(p_cur_ui->current_page, show);//由于clearbuff一直清空buff，所以需要重新绘制上一个页面用于渐隐
                    if(WouoUI_BlurProc(p_cur_ui->time)){ //渐隐函数调用，返回true时，渐隐完成
                        WouoUI_BlurParaInit(0,p_cur_ui->upara->ani_param[FADE_ANI]); //渐显参数初始化
                        PAGE_USE_METHOD(p_cur_ui->in_page, in_para_init);//调用要进入页面的IN动画参数初始化
                        p_cur_ui->state = ui_page_in;
                    }
                }
            break;
        }
        WouoUI_ScrollBarProc(p_all); //侧边条处理
        WouoUI_IndicatorProc(p_all); // 指示器处理
//软件刷新和硬件刷新的判断都必须放在所有动画绘制之后
#if SOFTWARE_DYNAMIC_REFRESH
        if(p_cur_ui->state == ui_page_in || p_cur_ui->state == ui_page_out)
            p_cur_ui->is_motionless = false; //in/out动画阶段不可能静止(动画结束就跳转show动画了)
        else if(p_cur_ui->state == ui_page_proc) //show阶段判断两种动画是否完成
            p_cur_ui->is_motionless = p_cur_ui->anim_is_finish && p_cur_ui->slide_is_finish; //anim和slide动画都完成了
#endif
#if HARDWARE_DYNAMIC_REFRESH
        if (memcmp(p_cur_ui->screen_dynamic_buff, p_cur_ui->screen_buff, sizeof(ScreenBuff))) {
            memcpy(p_cur_ui->screen_dynamic_buff, p_cur_ui->screen_buff, sizeof(ScreenBuff));
                              WouoUI_BuffSend();
        }
#else 
           WouoUI_BuffSend();
#endif
    }
}


/**
 * @brief 跳转到指定页面(用于关联确认上下级页面关系)
 *
 * @param self_page 当前页面对象的地址 
 * @param terminate_page 目标页面地址
  */
void WouoUI_JumpToPage(PageAddr self_page_addr, PageAddr terminate_page) {
    // 关联上级页面并跳转页面
    if (terminate_page != NULL) {
        if(WouoUI_CheckPageIsInit(terminate_page)){ //检查页面是否初始化过
            Page *p_ter = (Page *)terminate_page;
            p_ter->last_page = self_page_addr; //给要跳转的页面绑定上级页面
            p_cur_ui->in_page = terminate_page;// 暂存要跳转的页面，等待渐隐结束赋值给cur_page跳转
            if(WouoUI_CheckPageTypeisWin(terminate_page)){ //如果要跳转的页面是弹窗的话
                PAGE_USE_METHOD(p_cur_ui->in_page, in_para_init); //弹窗in动画目标参数初始化
                WouoUI_BlurParaInit(p_cur_ui->win_bg_blur,p_cur_ui->upara->ani_param[FADE_ANI]); //渐隐参数初始化(是弹窗的话上一个页面只消失一半)
                p_cur_ui->state = ui_page_in;     // 弹窗页面在in渐隐中同时做in动画
            }
            else {
                WouoUI_BlurParaInit(4,p_cur_ui->upara->ani_param[FADE_ANI]); //渐隐参数初始化(不是弹窗的话上一个页面完全消失)
                p_cur_ui->state = ui_page_out;     // 改变页面，启动渐隐
            }
        }else {
            WOUOUI_LOG_E("The Page will jump is not inited!!");
        }
    }
}

/**
 * @brief 得到当前页面的地址
 *
 * @param terminate_page The address of the page to change to.
 */
Page *WouoUI_GetCurrentPage(void) {
    return (Page *)p_cur_ui->current_page;
}
