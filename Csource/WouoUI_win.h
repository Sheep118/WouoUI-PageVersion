#ifndef __WOUOUI_WIN_H__
#define __WOUOUI_WIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "WouoUI_page.h"

//-------------------------------------MagWin页面----------------------------------------------
//宏参数
#define MSG_WIN_FONT          DEFAULT_MSG_WIN_FONT // 弹窗字体
#define MSG_WIN_W             DEFAULT_MSG_WIN_W// 弹窗宽度
#define MSG_WIN_H_MAX         (WOUOUI_BUFF_HEIGHT-2*DEFAULT_MSG_WIN_V_S_MIN)// 弹窗最大高度
#define MSG_WIN_R             DEFAULT_MSG_WIN_R        // 弹窗倒角
#define MSG_WIN_FONT_MARGIN   DEFAULT_MSG_WIN_FONT_MARGIN  //MSG弹窗文字到框的边距
#if (DEFAULT_MSG_WIN_W > WOUOUI_BUFF_WIDTH) //边框太宽了
#warning "the width of msg_win is too large, gt the width of buff"
#endif
#if (MSG_WIN_H_MAX < 24)
#warning "the MSG_WIN_V_S_MIN is too large"
#endif
//类型定义
struct MsgWinVar
{
    Canvas  canvas;      //弹窗画布
    int16_t indicator_h_temp; //暂时保存指示器的高度值在ind_ctrl中统一给指示器赋值
    uint16_t autonewline_need_h;    //弹窗文本是否需要滑动
};
typedef struct 
{
    Page page;      //页面基础信息
    Option* bg_opt; //自动获取到的背景页面的选项的指针
    char* content;  //显示的消息内容
    int16_t str_start_y; //字符串书写的起始位置
    uint16_t str_move_step; //字符串移动时的步长
    bool  auto_get_bg_opt:1;  //弹窗页面是否自动获得lastPage中的select_opt
} MsgWin;
/**
 * @brief 初始化消息窗口页面
 * 
 * @param mw 指向消息窗口的指针
 * @param content 消息内容
 * @param auto_get_bg_opt 是否自动获取背景选项
 * @param move_step 移动步长
 * @param cb 回调函数
 * 
 * @attention auto_get_bg_opt 若为true，会自动获取背景页面选中项的content覆盖弹窗内的content文本
 */
void WouoUI_MsgWinPageInit(MsgWin* mw, char* content, bool auto_get_bg_opt, uint16_t move_step, CallBackFunc cb);
/**
 * @brief 设置消息窗口的显示内容
 * 
 * @param mw 指向消息窗口的指针
 * @param content 要显示的消息内容
 * @return true 设置成功
 * @return false 设置失败
 * 
 * @attention 确保消息窗口页面初始化过(建议只在回调函数中调用此函数)
 */
bool WouoUI_MsgWinPageSetContent(MsgWin *mw, char* content);
/**
 * @brief 向上滑动消息窗口文本
 * 
 * @param mw 指向消息窗口的指针
 * @return true 如果成功滑动
 * @return false 如果无法滑动
 * 
 * @attention 确保消息窗口页面初始化过(建议只在回调函数中调用此函数)
 */
bool WouoUI_MsgWinPageSlideUpTxt(MsgWin *mw); //返回值表示能否成功滑动
/**
 * @brief 向下滑动消息窗口文本
 * 
 * @param mw 指向消息窗口的指针
 * @return true 如果成功滑动
 * @return false 如果无法滑动
 * 
 * @attention 确保消息窗口页面初始化过(建议只在回调函数中调用此函数)
 */
bool WouoUI_MsgWinPageSlideDownTxt(MsgWin * mw);


//-----------------------------------------------ConfWin页面-----------------------------------------------------------
//宏参数
#define CONF_WIN_FONT          DEFAULT_CONF_WIN_FONT // 弹窗字体
#define CONF_WIN_W             DEFAULT_CONF_WIN_W// 弹窗宽度
#define CONF_WIN_HORIZON_S     ((WOUOUI_BUFF_WIDTH - DEFAULT_CONF_WIN_W)>>1) //弹窗水平上与两侧的边距
#define CONF_WIN_H_MAX         (WOUOUI_BUFF_HEIGHT-2*DEFAULT_CONF_WIN_V_S_MIN)// 弹窗最大高度
#define CONF_WIN_R             DEFAULT_CONF_WIN_R        // 弹窗倒角
#define CONF_WIN_FONT_MARGIN   DEFAULT_CONF_WIN_FONT_MARGIN  //CONF弹窗文字到框的边距
#define CONF_WIN_IND_BTN_S     DEFAULT_CONF_WIN_IND_BTN_S  //弹窗的按键内文字到边框的距离
#define CONF_WIN_BTN_H         (GET_FNOT_H(CONF_WIN_FONT)+2*DEFAULT_CONF_WIN_IND_BTN_S) // 按键高度
#define CONF_WIN_BTN_R         DEFAULT_CONF_WIN_BTN_R        // 按键倒角
#define CONF_TEXT_BTN_S        DEFAULT_CONF_WIN_TEXT_BTN_S       //文本和按键的竖直间距
#define CONF_BTN_MAX_W         (WOUOUI_MIDDLE_H-CONF_WIN_HORIZON_S-2*DEFAULT_CONF_WIN_FONT_MARGIN) //按键文本的最大宽度
#define CONF_BTN_SLIDESTR_MODE DEFAULT_CONF_BTN_SLIDESTR_MODE   //按键文本的移动模式
#if (DEFAULT_CONF_WIN_W > WOUOUI_BUFF_WIDTH)
#warning "the width of Conf win is too large"
#endif
#if (CONF_WIN_H_MAX < 24)
#warning "the CONF_WIN_V_S_MIN is too large"
#endif
//类型定义
struct ConfWinVar
{
    AnimPos win_y;       //弹窗的竖直动画
    Canvas  canvas;      //弹窗画布
    SlideStr btn_ss;        //按键滑动字符
    uint16_t autonewline_need_h;    //弹窗文本是否需要滑动
};
typedef struct 
{
    Page page;      //页面基础信息
    Option* bg_opt; //自动获取到的背景页面的选项的指针
    char* content;  //显示的消息内容
    char* str_left; //左侧按键内容
    char* str_right; //右侧按键内容
    int16_t str_start_y; //字符串书写的起始位置
    uint16_t str_move_step; //字符串移动时的步长
    bool conf_ret:1;        //确认弹窗的结果，如果使能自动获取的话，这个成员和content的值都会自动被置成lastPage的选中项
    bool auto_get_bg_opt:1;  //弹窗页面是否自动获得lastPage中的select_opt
    bool auto_set_bg_opt:1;  //弹窗页面是否自动设置lastPage中的select_opt相关的值
} ConfWin;
/**
 * @brief 初始化配置窗口页面
 * 
 * @param cw 配置窗口对象指针
 * @param content 页面内容
 * @param str_left 左侧按钮文本(NULl指针输入默认为Yes)
 * @param str_right 右侧按钮文本(NULl指针输入默认为No)
 * @param conf_ret 配置返回值
 * @param auto_get_bg_opt 自动获取背景选项
 * @param auto_set_bg_opt 自动设置背景选项
 * @param move_step 移动步长
 * @param cb 回调函数
 * @attention auto_get_bg_opt 为true,会自动获取背景页面选中项的val和content覆盖conf_ret和content。auto_set_bg_opt同理
 */
void WouoUI_ConfWinPageInit(ConfWin* cw, char* content, char* str_left, char* str_right, bool conf_ret, bool auto_get_bg_opt, bool auto_set_bg_opt, uint16_t move_step, CallBackFunc cb);
/**
 * @brief 向上滑动配置窗口页面文本
 * 
 * @param cw 配置窗口对象指针
 * @return true 成功滑动
 * @return false 失败
 * @attention 确保消息窗口页面初始化过(建议只在回调函数中调用此函数)
 */
bool WouoUI_ConfWinPageSlideUpTxt(ConfWin *cw);

/**
 * @brief 向下滑动配置窗口页面文本
 * 
 * @param cw 配置窗口对象指针
 * @return true 成功滑动
 * @return false 失败
 * @attention 确保消息窗口页面初始化过(建议只在回调函数中调用此函数)
 */
bool WouoUI_ConfWinPageSlideDownTxt(ConfWin *cw);

/**
 * @brief 切换配置窗口页面按钮
 * 
 * @param cw 配置窗口对象指针
 * @attention 确保消息窗口页面初始化过(建议只在回调函数中调用此函数)
 */
void WouoUI_ConfWinPageToggleBtn(ConfWin *cw);

//---------------------------------------------------ValWin页面-----------------------------------------------------------------
//宏定义
#define VAL_WIN_FONT           DEFAULT_VAL_WIN_FONT           // Val弹窗字体
#define VAL_WIN_STR_BUFF_SIZE  DEFAULT_VAL_WIN_STR_BUFF_SIZE  //临时buff的大小
#define VAL_WIN_FONT_MARGIN    DEFAULT_VAL_WIN_FONT_MARGIN    // 弹窗内部元素到边框的边距
#define VAL_WIN_W              DEFAULT_VAL_WIN_W              // Val弹窗的宽度
#define VAL_WIN_H              (2*GET_FNOT_H(DEFAULT_VAL_WIN_FONT)+2*DEFAULT_VAL_WIN_FONT_MARGIN+DEFAULT_VAL_TEXT_BAR_S) //val弹窗的高度
#define VAL_WIN_R              DEFAULT_VAL_WIN_R              // Val弹窗的倒角大小
#define VAL_WIN_X              ((WOUOUI_BUFF_WIDTH-DEFAULT_VAL_WIN_W)>>1) //弹窗的起始x
#define VAL_WIN_Y              ((WOUOUI_BUFF_HEIGHT-VAL_WIN_H)>>1) //弹窗的起始y
#define VAL_WIN_TXT_W_MAX      DEFAULT_VAL_WIN_TXT_W_MAX        //txt可显示最大宽度
#define VAL_WIN_TXTVAL_H       GET_FNOT_H(DEFAULT_VAL_WIN_FONT) //text VAL的高度         
#define VAL_WIN_TXTVAL_X       (VAL_WIN_X+DEFAULT_VAL_WIN_FONT_MARGIN) //text val的起始x   
#define VAL_WIN_TXTVAL_Y_OFS   DEFAULT_VAL_WIN_FONT_MARGIN     // text val相对于弹窗的y偏移
#define VAL_WIN_TXTVAL_S       DEFAULT_VAL_WIN_TXTVAL_S         //弹窗内文本和val间的间距
#define VAL_WIN_VAL_W_MAX      (VAL_WIN_W-VAL_WIN_TXT_W_MAX-VAL_WIN_TXTVAL_S-2*VAL_WIN_FONT_MARGIN) //val可显示的最大宽度
#define VAL_WIN_MMVAL_W_MAX    ((DEFAULT_VAL_WIN_W-2*DEFAULT_VAL_WIN_FONT_MARGIN-2*DEFAULT_VAL_MMVAL_BAR_S-DEFAULT_VAL_WIN_BAR_W)>>1) //minmaxval可显示最大宽度
#define VAL_WIN_MMVAL_H        GET_FNOT_H(DEFAULT_VAL_WIN_FONT) //MINMAXVAL的高度        
#define VAL_WIN_MMVAL_Y_OFS    (VAL_WIN_TXTVAL_Y_OFS+VAL_WIN_TXTVAL_H+DEFAULT_VAL_TEXT_BAR_S)    //mmval相对于弹窗的y偏移
#define VAL_WIN_BAR_W          DEFAULT_VAL_WIN_BAR_W          // 进度条宽度
#define VAL_WIN_BAR_H          DEFAULT_VAL_WIN_BAR_H          // 进度条高度
#define VAL_WIN_BAR_R          DEFAULT_VAL_WIN_BAR_R          // 进度条倒角
#define VAL_WIN_BAR_X          ((WOUOUI_BUFF_WIDTH-DEFAULT_VAL_WIN_BAR_W)>>1) //进度条的起始X
#define VAL_WIN_BAR_Y_OFS      (VAL_WIN_MMVAL_Y_OFS+((VAL_WIN_MMVAL_H-VAL_WIN_BAR_H)>>1)) //进度条相对于边框的Y偏移
#define VAL_WIN_TEXT_BAR_S     DEFAULT_VAL_TEXT_BAR_S         // 下方进度条和上方文本的间距
#define VAL_WIN_TXT_SLISTRMODE DEFAULT_VAL_WIN_TXT_SLISTRMODE  //文本的移动模式
#define VAL_WIN_VAL_SLISTRMODE DEFAULT_VAL_WIN_VAL_SLISTRMODE  //数值的移动模式
#if (DEFAULT_VAL_WIN_W > WOUOUI_BUFF_WIDTH) 
#warning "DEFAULT_VAL_WIN_W is too large ,gt the width of buff"
#endif
#if (VAL_WIN_BAR_W > VAL_WIN_W)
#warning "the width of bar gt the width of win"
#endif
#define VAL_WIN_MACRO_ASSERT    (VAL_WIN_H>WOUOUI_BUFF_HEIGHT)
//类型定义
struct ValWinVar
{
    AnimPos win_y;  //弹窗的竖直动画
    SlideStr text_ss;  //文本的滚动动画
    SlideStr val_ss;  //数值的滚动动画
    SlideStr min_ss; //最小值的滚动动画
    SlideStr max_ss;  //最大值的滚动动画
};
typedef struct 
{
    Page page;  //页面基础信息
    Option* bg_opt; //自动获取到的背景页面的选项的指针
    char* text; //要显示的文本
    int32_t val; //弹窗数值
    int32_t min; //弹窗最小值
    int32_t max; //弹窗最大值
    int32_t step; //弹窗步长
    bool auto_get_bg_opt:1;  //弹窗页面是否自动获得lastPage中的select_opt
    bool auto_set_bg_opt:1;  //弹窗页面是否自动设置lastPage中的select_opt相关的值
} ValWin;
/**
 * @brief Valwin页面初始化
 * @param vw Valwin实例指针
 * @param text 显示的文本
 * @param init_val 初始值
 * @param min 最小值
 * @param max 最大值
 * @param step 步进值
 * @param auto_get_bg_opt 自动获取背景选项
 * @param auto_set_bg_opt 自动设置背景选项
 * @param cb 回调函数
 * @attention auto_get_bg_opt 为true会自动获取背景页面选中项的text作为文本，获取val作为init_val, auto_set_bg_opt同理
 */
void WouoUI_ValWinPageInit(ValWin* vw, char* text, int32_t init_val, int32_t min, int32_t max, int32_t step, bool auto_get_bg_opt, bool auto_set_bg_opt, CallBackFunc cb);
/**
 * @brief 设置Valwin页面的最小值、步进值和最大值
 * @param vw Valwin实例指针
 * @param min 最小值
 * @param step 步进值
 * @param max 最大值
 * @attention 确保min<= step <= max，
 */
void WouoUI_ValWinPageSetMinStepMax(ValWin* vw, int32_t min, int32_t step, int32_t max);
/**
 * @brief 增加Valwin页面的当前值
 * @param vw Valwin实例指针
 * @return 增加是否成功
 * @attention 当前值到达最大值时返回false(建议只在回调函数中使用此函数)
 */
bool WouoUI_ValWinPageValIncrease(ValWin *vw);
/**
 * @brief 减少Valwin页面的当前值
 * @param vw Valwin实例指针
 * @return 减少是否成功
 * @attention 当前值到达最小值时返回false(建议只在回调函数中使用此函数)
 */
bool WouoUI_ValWinPageValDecrease(ValWin *vw);

//---------------------------------------------------------SpinWin页面---------------------------------------------------
//宏定义
#define SPIN_WIN_FONT          DEFAULT_SPIN_WIN_FONT    // 弹窗字体
#define SPIN_WIN_NUM_FONT      DEFAULT_SPIN_WIN_NUM_FONT   // 数字字体
#define SPIN_WIN_STR_BUFF_SIZE DEFAULT_VAL_WIN_STR_BUFF_SIZE  //临时buff的大小
#define SPIN_WIN_FONT_MARGIN   DEFAULT_SPIN_WIN_FONT_MARGIN //弹窗内元素到边框的间距
#define SPIN_WIN_W             DEFAULT_SPIN_WIN_W         // 弹窗宽度      
#define SPIN_WIN_NUM_S         DEFAULT_SPIN_WIN_NUM_S    //每个数字的间距         
#define SPIN_WIN_V_S           DEFAULT_SPIN_WIN_V_S //页面内元素间纵向的间距
#define SPIN_WIN_BOX_R         DEFAULT_SPIN_WIN_BOX_R // 弹窗指示器倒角
#define SPIN_WIN_BOX_H         DEFAULT_SPIN_WIN_BOX_H // 弹窗指示器在数字底部时的高度
#define SPIN_WIN_R             DEFAULT_SPIN_WIN_R            
#define SPIN_WIN_H             (2*SPIN_WIN_FONT_MARGIN+2*GET_FNOT_H(SPIN_WIN_NUM_FONT)+GET_FNOT_H(SPIN_WIN_FONT)+2*SPIN_WIN_V_S+SPIN_WIN_BOX_H) // 弹窗高度
#define SPIN_WIN_Y             ((WOUOUI_BUFF_HEIGHT-SPIN_WIN_H) >>1)
#define SPIN_WIN_X             ((WOUOUI_BUFF_WIDTH-SPIN_WIN_W) >>1)
#define SPIN_WIN_TXT_Y_OFS     SPIN_WIN_FONT_MARGIN  //文本y坐标相对边框的偏移
#define SPIN_WIN_MMVAL_Y_OFS   (SPIN_WIN_TXT_Y_OFS+GET_FNOT_H(SPIN_WIN_FONT)+DEFAULT_SPIN_WIN_V_S)//最大最小值y坐标相对边框的偏移
#define SPIN_WIN_MMVAL_MID_LX  (WOUOUI_MIDDLE_H-(GET_FNOT_W(SPIN_WIN_NUM_FONT)>>1)) //mmval中间间隔符左侧的横坐标
#define SPIN_WIN_MMVAL_MID_RX  (WOUOUI_MIDDLE_H+(GET_FNOT_W(SPIN_WIN_NUM_FONT)>>1)) //mmval中间间隔符左侧的横坐标
#define SPIN_WIN_MMVAL_S       DEFAULT_SPIN_WIN_MMVAL_S  //mmval和中间间隔符之间的间隔
#define SPIN_WIN_MIN_W_MAX     (SPIN_WIN_MMVAL_MID_LX-SPIN_WIN_MMVAL_S-(SPIN_WIN_X+SPIN_WIN_FONT_MARGIN)) //最小值显示的最大宽度
#define SPIN_WIN_MAX_W_MAX     ((SPIN_WIN_X+SPIN_WIN_W-SPIN_WIN_FONT_MARGIN)-(SPIN_WIN_MMVAL_MID_RX+SPIN_WIN_MMVAL_S)) //最大值显示的最大宽度
#define SPIN_WIN_NUM_Y_OFS     (SPIN_WIN_MMVAL_Y_OFS+GET_FNOT_H(SPIN_WIN_NUM_FONT)+DEFAULT_SPIN_WIN_V_S)//最大最小值y坐标相对边框的偏移
#define SPIN_WIN_SLI_TXT_MODE  DEFAULT_SPIN_WIN_SLI_TXT_MODE  // 文本的滑动模式
#define SPIN_WIN_SLI_VAL_MODE  DEFAULT_SPIN_WIN_SLI_VAL_MODE  // 数值的滑动模式
#if (SPIN_WIN_W > WOUOUI_BUFF_WIDTH)
#warning "the width of spin——win is too large,gt the width of buff"
#endif
#define SPIN_WIN_MACRO_ASSERT ( SPIN_WIN_H>WOUOUI_BUFF_HEIGHT || (9*GET_FNOT_W(SPIN_WIN_NUM_FONT)+6*(SPIN_WIN_NUM_S)>SPIN_WIN_W ))
//类型定义
struct SpinWinVar
{
    AnimPos win_y;       // 弹窗的竖直动画
    SlideStr text_ss;    // 文本的滚动动画
    SlideStr min_ss;     // 最小值的滚动动画
    SlideStr max_ss;     // 最大值的滚动动画
    int16_t num_w_temp;  // 数字总宽度暂存
};

typedef struct 
{
    Page page;           // 页面基础信息
    Option* bg_opt; //自动获取到的背景页面的选项的指针
    char* text;          // 要显示的文本
    int32_t val;         // 弹窗数值
    int32_t min;         // 弹窗最小值
    int32_t max;         // 弹窗最大值
    uint8_t sel_bit:4;   //选中位0-7
    DecimalNum dec_num:3; // 定点数的类型
    uint8_t sel_flag:1;  //spin弹窗是否有选中
    bool auto_get_bg_opt:1;  // 弹窗页面是否自动获得lastPage中的select_opt
    bool auto_set_bg_opt:1;  // 弹窗页面是否自动设置lastPage中的select_opt相关的值
} SpinWin;
/**
 * @brief 设置数值选择窗口的最小值、最大值和小数位数
 * @param spw 指向SpinWin结构体的指针
 * @param min 最小值
 * @param max 最大值
 * @param decnum 小数位数
 * @attention 如果该页面使能自动获取背景页面opt的话，小数位数建议与opt的位数保持一致
 */
void WouoUI_SpinWinPageSetMinMaxDecimalnum(SpinWin *spw, int32_t min, int32_t max, DecimalNum decnum);
/**
 * @brief 初始化数值选择窗口
 * @param spw 指向SpinWin结构体的指针
 * @param text 显示文本
 * @param init_val 初始值
 * @param dec_num 小数位数
 * @param min 最小值
 * @param max 最大值
 * @param auto_get_bg_opt 自动获取背景选项
 * @param auto_set_bg_opt 自动设置背景选项
 * @param cb 回调函数
 * @attention auto_get_bg_opt 为true会自动获取背景页面选中项的text作为文本，获取val作为init_val, 获取opt的小数位数作为dec_num, auto_set_bg_opt同理
 */
void WouoUI_SpinWinPageInit(SpinWin* spw, char* text, int32_t init_val, DecimalNum dec_num, int32_t min, int32_t max, bool auto_get_bg_opt, bool auto_set_bg_opt, CallBackFunc cb);
/**
 * @brief 移动数值选择窗口的选择位
 * @param spw 指向SpinWin结构体的指针
 * @param left0ORright1 移动方向(0表示向左,1表示向右)
 * @attention (建议只在回调函数中使用此函数) 
 */
void WouoUI_SpinWinPageShiftSelbit(SpinWin *spw, bool left0ORright1);
/**
 * @brief 切换数值选择窗口的选择状态
 * @param spw 指向SpinWin结构体的指针
 * @attention (建议只在回调函数中使用此函数)
 */
void WouoUI_SpinWinPageToggleSelState(SpinWin* spw);
/**
 * @brief 改变数值选择窗口当前选择位的值
 * @param spw 指向SpinWin结构体的指针
 * @param Inc1OrDec_1 增加或减少(1表示增加,-1表示减少)
 * @return true 改变成功
 * @return false 改变失败
 * @attention (建议只在回调函数中使用此函数)
 */
bool WouoUI_SpinWinPageChangeSelbit(SpinWin* spw, int32_t Inc1OrDec_1);

//--------------------------------------------------ListWin页面------------------------------------------------------------------
//宏定义
#define LIST_WIN_FONT         DEFAULT_LIST_WIN_FONT   // 弹窗字体
#define LIST_WIN_W            DEFAULT_LIST_WIN_W         // 弹窗宽度
#define LIST_WIN_R            DEFAULT_LIST_WIN_R         // 弹窗倒角
#define LIST_WIN_BOX_R        DEFAULT_LIST_WIN_BOX_R     // 列表选择框倒角
#define LIST_WIN_L_S          DEFAULT_LIST_WIN_L_S       // 列表左边距
#define LIST_WIN_R_S          (DEFAULT_LIST_WIN_R_S+DEFAULT_LIST_WIN_BAR_W) // 列表右边距不含进度条
#define LIST_WIN_TEXT_U_S     DEFAULT_LIST_WIN_TEXT_U_S          // 列表每行文字的上边距
#define LIST_WIN_TEXT_D_S     DEFAULT_LIST_WIN_TEXT_D_S          // 列表每行文字的下边距
#define LIST_WIN_LINE_H       (GET_FNOT_H(LIST_WIN_FONT)+LIST_WIN_TEXT_U_S+LIST_WIN_TEXT_D_S) // 列表单行高度 
#define LIST_WIN_BAR_W        DEFAULT_LIST_WIN_BAR_W //默认进度条的宽度
//类型声明
struct ListWinVar
{
    AnimPos win_x; //弹窗的横坐标
    AnimPos list_y;     // 列表y值动画
    SlideStr str_ss; // 滑动字符串
    uint8_t line_n;  // = DISP_H / LIST_LINE_H; 屏幕内有多少行选
    int16_t ind_y_temp; //指示器的y坐标暂存
};
typedef struct 
{
    Page page;  
    Option* bg_opt; //自动获取到的背景页面的选项的指针
    String* str_array; //显示字符串的数组
    uint8_t array_num; //数组的大小
    uint8_t sel_str_index; //选中字符串在数组中的下标
    bool auto_set_bg_opt:1;  // 弹窗页面是否自动设置lastPage中的select_opt相关的值
} ListWin;
/**
 * @brief 初始化列表窗口页面
 * @param lw 列表窗口指针
 * @param array_num 字符串数组的数量
 * @param str_array 字符串数组指针 
 * @param auto_set_bg_opt 是否自动设置背景选项
 * @param cb 回调函数指针
 * @attention auto_set_bg_opt 为true会自动将选中项的内容设置到背景页面选中选项的content中，注意，数组大小需要正确传入
 */
void WouoUI_ListWinPageInit(ListWin* lw, uint8_t array_num, String * str_array, bool auto_set_bg_opt, CallBackFunc cb);

/**
 * @brief 移动到列表窗口的上一个项目
 * @param lw 列表窗口指针
 * @attention 确保列表窗口页面初始化过(建议只在回调函数中调用此函数)
 */
void WouoUI_ListWinPageLastItem(ListWin *lw);

/**
 * @brief 移动到列表窗口的下一个项目
 * @param lw 列表窗口指针
 * @attention 确保列表窗口页面初始化过(建议只在回调函数中调用此函数)
 */
void WouoUI_ListWinPageNextItem(ListWin *lw);

#ifdef __cplusplus
}
#endif

#endif // __WOUOUI_WIN_H__
