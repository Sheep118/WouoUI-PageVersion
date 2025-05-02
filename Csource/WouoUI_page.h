#ifndef __WOUOUI_PAGE__
#define __WOUOUI_PAGE__

#ifdef __cplusplus
extern "C" {
#endif

#include "WouoUI_common.h"
#include "WouoUI_graph.h"
#include "WouoUI_anim.h"
#include "WouoUI_msg.h"

//===================================页面共用的类型参数方法==================================
// 定义字符串类型
typedef char *String;
//--------页面类型枚举
typedef enum {
    type_none = 0x00,   //非页面类型,用于在jump函数判断一个页面是否经过初始化,如果没有,则没有绑定过方法,避免状态机访问方法报错
    type_title,         // 磁贴类
    type_list,         // 列表类
    type_wave,         // 波形类
    type_sep,         // 类型分割线，以上是全页面类型，以下是弹窗页面类型
    type_slidevalwin, // 滑动数值弹窗
    type_msgwin,      // 消息弹窗
    type_confwin,      // 确认弹窗
    type_spinwin,      // 微调数值弹窗
    type_listwin,      // 列表弹窗
} PageType;            // 页面类型，用于标志传入的每个页面类型，方便调用对应的proc函数
typedef enum { //定点数的小数点类型
    DecimalNum_0 = 0x00,
    DecimalNum_1 = 0x01,
    DecimalNum_2 = 0x02,
    DecimalNum_3 = 0x03,
} DecimalNum;
//-------页面成员类型
typedef struct Option {
    String text;
    // 这个列表项显示的字符串,通过选项的第一个字符判断为:
    // 无效果(-)/数值显示(=)/
    // 弹窗类:滑动数值弹窗(~)/微调数字弹窗(%)/提示弹窗(!)/二值确认弹窗(#)/侧边列表(>)
    // 列表内:二值选框(@)
    // 跳转:跳转页面(+)/跳转波形页面($)/跳转文本页面({)
    String content;        // 文本内容
    int32_t val;           // 这个列表项关联的显示的值(可以用于设置初值) 
    uint8_t order;         // 该选项在列表/磁贴中的排序(0-255)
    DecimalNum decimalNum; // 小数位数
} Option; // 通用选项类型
// 选择类界面的回调函数类型定义,参数为确认选中项的指针。
typedef struct _page Page; // 页面基类的声明,方便作为回调函数参数
// 更改回调函数直接传入当前页面的页面地址
typedef bool (*CallBackFunc)(const Page *cur_page_addr, InputMsg msg); // 页面回调函数类型定义(会传入当前页面的地址和当前页面收到的操作消息)
// 页面地址类型，用于声明全局页面数组存放页面使用
#define PageAddr void *
// 每个页面基本方法的定义
typedef bool (*PageIn)(PageAddr);  // 进入页面的过度动画函数(返回值为True表示这个状态已经可以结束了，切换下一个状态)
typedef void (*PageInParaInit)(PageAddr);  // 进入页面过度动画参数初始化函数
typedef void (*PageShow)(PageAddr); // 页面的展示函数
typedef bool (*PageReact)(PageAddr); // 页面的响应函数(返回值为True表示这个状态已经可以结束了，切换下一个状态)
typedef void (*PageIndicatorCtrl)(PageAddr); // 页面与UI指示器交互的控制函数
typedef void (*PageScrollBarCtrl)(PageAddr); // 页面与UI滚动条交互的控制函数
typedef struct
{
    PageIn in; // 进入页面的过度动画函数
    PageInParaInit in_para_init; // 进入页面过度动画参数初始化函数
    PageShow show; // 页面的展示函数
    PageReact react; // 页面的响应函数
    PageIndicatorCtrl indicator_ctrl; // 页面与UI指示器交互的控制函数
    PageScrollBarCtrl scrollbar_ctrl; // 页面与UI滚动条交互的控制函数
} PageMethods; // 方法集合
struct _page {
    PageType page_type; // 页面类型，以便在处理时调用不同函数绘制
    PageAddr last_page; // 上级页面的地址
    CallBackFunc cb;    // 页面的回调函数
    PageMethods* methods; // 页面方法集合的指针，其实例在UI对象上
    bool auto_deal_with_msg ; //页面是否会自动处理消息,默认会,如果置False,需要自己在回调函数中调用提供的接口进行页面操作
}; // 最基本的页面类型(所有页面类型的基类和结构体的**第一个成员**)
// 页面的通用方法

/**
 * @brief 页面类型及相关检查宏定义
 * 
 * @def WouoUI_CheckPageType(page_addr)
 * @brief 获取指定页面的类型
 * @param page_addr 页面地址指针
 * @return 返回页面类型
 */
#define WouoUI_CheckPageType(page_addr) (((Page *)page_addr)->page_type)
/**
 * @def WouoUI_CheckPageTypeisWin(page_addr) 
 * @brief 检查指定页面是否为窗口类型
 * @param page_addr 页面地址指针
 * @return 如果是窗口类型返回true，否则返回false
 */ 
#define WouoUI_CheckPageTypeisWin(page_addr) ((uint8_t)WouoUI_CheckPageType(page_addr) > type_sep)
/**
 * @def WouoUI_CheckPageIsInit(page_addr)
 * @brief 检查指定页面是否已初始化
 * @param page_addr 页面地址指针
 * @return 如果已初始化返回true，否则返回false
 */
#define WouoUI_CheckPageIsInit(page_addr) (type_none != WouoUI_CheckPageType(page_addr))
/**
 * @brief 返回到指定页面
 * @param page_addr 目标页面地址
 * @attention 这个函数用于二次开发
 */
void WouoUI_PageReturn(PageAddr page_addr);
/**
 * @brief 初始化指定页面
 * @param page_addr 页面地址
 * @param call_back 页面回调函数
 * @attention 这个函数用于二次开发
 */
void WouoUI_PageInit(PageAddr page_addr, CallBackFunc call_back);
/**
 * @brief 设置页面是否自动处理消息(默认开启)
 * @attention 需要注意，如果开启自动处理，大多数弹窗页面的click都是默认返回的(只有spinwin除外)，
 *            如果需要click事件跳转的话，需要失能自动消息处理，并自己在回调中调用对应的处理函数
 * @param page 目标页面结构体指针
 * @param open true开启自动处理,false关闭自动处理
 */
void WouoUI_SetPageAutoDealWithMsg(Page* page, bool open);

//===================================================Title页面====================================================
// 磁贴宏参数,所有磁贴页面都使用同一套参数
#define TILE_B_TITLE_FONT             DEFAULT_TILE_B_TITLE_FNOT   // 磁贴大标题字体
#define TILE_BAR_H                    DEFAULT_TILE_BAR_H // 磁贴装饰条高度(初始化时会检查与字体的高度大小)
#define TILE_BAR_W                    DEFAULT_TILE_BAR_W // 磁贴装饰条宽度
#define TILE_BAR_D                    DEFAULT_TILE_BAR_D // 磁贴装饰条到页面底部的距离
#define TILE_BAR_U                    (WOUOUI_BUFF_HEIGHT-DEFAULT_TILE_BAR_D-DEFAULT_TILE_BAR_H)// 磁贴装饰条到页面顶部的距离
#define TILE_ICON_W                   DEFAULT_TILE_ICON_W         // 磁贴图标宽度
#define TILE_ICON_H                   DEFAULT_TILE_ICON_H         // 磁贴图标高度
#define TILE_ICON_IND_U               DEFAULT_TILE_ICON_IND_U   //磁贴指示器与磁贴的上边距
#define TILE_ICON_IND_D               DEFAULT_TILE_ICON_IND_D   //磁贴指示器与磁贴的下边距
#define TILE_ICON_IND_L               DEFAULT_TILE_ICON_IND_L   //磁贴指示器与磁贴的左边距
#define TILE_ICON_IND_R               DEFAULT_TILE_ICON_IND_R   //磁贴指示器与磁贴的右边距
#define TILE_ICON_IND_W               (TILE_ICON_IND_L+TILE_ICON_W+TILE_ICON_IND_R)  // 磁贴指示器宽度
#define TILE_ICON_IND_H               (TILE_ICON_IND_U+TILE_ICON_H+TILE_ICON_IND_D)  // 磁贴指示器高度
#define TILE_ICON_IND_SL              DEFAULT_TILE_ICON_IND_SL    // 磁贴指示器边长
#define TILE_ICON_S                   (DEFAULT_TILE_ICON_S+DEFAULT_TILE_ICON_W) // 磁贴图标间距(中心到中心)
#define TILE_ICON_U                   (WOUOUI_BUFF_HEIGHT-TILE_BAR_D-TILE_BAR_H-TILE_ICON_H)/2  // 磁贴图标上边距(自动居中)
#define TILE_SLIDESTR_MODE            DEFAULT_TILE_SLIDESTR_MODE  //磁贴标题文本的滚动模式
#define ICON_BUFFSIZE                 (TILE_ICON_W * (TILE_ICON_H / 8 + 1))
#if (TILE_BAR_D+TILE_BAR_H+TILE_ICON_IND_H+1 > WOUOUI_BUFF_HEIGHT) //磁贴参数的宏检查(检查icon+bar是否会大于整个Height)
#warning "The sum of TITLE Macro Height large than BUFF Height"
#endif
#define TILE_ASSERT_IND_H              (TILE_ICON_IND_R+TILE_ICON_IND_L > DEFAULT_TILE_ICON_S) //检查指示器水平大小
#define TILE_ASSERT_IND_V              ((TILE_ICON_U-TILE_ICON_IND_U < 0)||(TILE_ICON_U+TILE_ICON_H+TILE_ICON_IND_D > TILE_BAR_U))//检查指示器竖直大小
#if (TILE_ASSERT_IND_H || TILE_ASSERT_IND_V) //检查指示器大小超过icon间隔
#warning "The Indicator large than the size of icon "
#endif
#define TILE_MACRO_ASSERT             (TILE_BAR_H<GET_FNOT_H(TILE_B_TITLE_FONT)) 
    //BAR的高度必须大于字体高度，上面的检查才有意义，这个检查在Init中执行，返回True需要打印log提示
//TitlePage的类型定义
    //TitlePage 需要的共用变量集合(类变量)
struct TitlePageVar { // 因为Title选中项一直在中间，所以不需要有每个页面变量记录它真实页面的偏移位置
    AnimPos iconX;    // 图标的x坐标距选中目标的间距的变量
    AnimPos iconY;    // 图标的y坐标
    AnimPos barX;     // 指示器的x坐标
    AnimPos titleY;   // 标题的y坐标
    SlideStr title_ss;// 滑动标题
};
typedef const uint8_t Icon[ICON_BUFFSIZE]; // 定义图标类型
typedef struct TitlePage {    // 磁贴页面
    Page page;                // 基础页面信息
    uint8_t item_num;         // 页面选项个数，option_array和icon_array个数需与此一致
    Option *option_array;     // 选项类型的数组(由于数组大小不确定，使用指针代替)
    Icon *icon_array;         // 图标数组(由于数组大小不确定，使用指针代替)
    uint8_t select_item;      // 选中选项
} TitlePage;                  // 磁帖页面类型(所有类型页面，类型成员为第一个，方便查看)
//TitlePage的接口函数
/**
 * @brief 初始化标题页面
 * @param title_page 标题页面结构体指针
 * @param item_num 选项数量(需要确保与下面两个数组大小一致)
 * @param option_array 选项数组指针
 * @param icon_array 图标数组指针
 * @param call_back 回调函数指针
 * @attention 数组大小需要使用者传入，确保传入的数组大小正确
 */
void WouoUI_TitlePageInit(TitlePage *title_page, uint8_t item_num, Option *option_array, Icon *icon_array, CallBackFunc call_back);
/**
 * @brief 切换到上一个选项
 * @param tp 标题页面结构体指针
 * @attention 此函数只建议在回调函数中使用 
 */
void WouoUI_TitlePageLastItem(TitlePage *tp);
/**
 * @brief 切换到下一个选项
 * @param tp 标题页面结构体指针
 * @attention 此函数只建议在回调函数中使用 
 */
void WouoUI_TItlePageNextItem(TitlePage *tp);


//===================================================List页面====================================================
// 列表参数,所有列表使用同一套参数
#define LIST_TEXT_FONT                DEFAULT_LIST_TEXT_FONT   // 列表每行文字字体
#define LIST_TEXT_U_S                 DEFAULT_LIST_TEXT_U_S   // 列表每行文字的上边距
#define LIST_TEXT_D_S                 DEFAULT_LIST_TEXT_D_S   // 列表每行文字的上边距
#define LIST_LINE_H                   (GET_FNOT_H(LIST_TEXT_FONT)+LIST_TEXT_U_S+LIST_TEXT_D_S) // 列表单行高度
#define LIST_TEXT_L_S                 DEFAULT_LIST_TEXT_L_S   // 列表每行文字的左边距
#define LIST_IND_VAL_S                DEFAULT_LIST_IND_VAL_S  //指示器和val文本的间距
#define LIST_BAR_W                    DEFAULT_LIST_BAR_W        // 列表进度条宽度，需要是奇数，因为正中间有1像素宽度的线
#define LIST_TEXT_R_S                 (DEFAULT_LIST_TEXT_R_S+LIST_BAR_W)  // 列表每行结尾的数值的右边距（包括进度条宽度 LIST_BAR_W）
#define LIST_TEXT_MAX_LEN             (DEFAULT_LIST_TEXT_LEN_PER_MAX*WOUOUI_BUFF_WIDTH/100) //列表文字的最大宽度
#define LIST_VAL_MAX_LEN              (WOUOUI_BUFF_WIDTH-LIST_TEXT_MAX_LEN-LIST_TEXT_R_S-LIST_TEXT_L_S*2-LIST_IND_VAL_S) //这里左间距*2是为了给val和text之间留空间
#define LIST_VAL_BUFF_SIZE            DEFAULT_LIST_VAL_BUFF_SIZE //列表值文本缓冲区的大小
#define LIST_IND_BOX_R                DEFAULT_LIST_IND_BOX_R    // 列表指示器盒子倒角
    // 列表行尾选择选项参数
#define CHECK_BOX_U_S                 LIST_TEXT_U_S // 选择框在每行的上边距(与文字保持同高)
#define CHECK_BOX_R_S                 LIST_TEXT_R_S // 选择框的右边距（包括进度条宽度 LIST_BAR_W）(与值文字对齐)
#define CHECK_BOX_F_H                 GET_FNOT_H(LIST_TEXT_FONT) // 选择框外框高度(与文字高度保持一致)
#define CHECK_BOX_F_W                 CHECK_BOX_F_H // 选择框外框宽度(方形，与高度保持一致)
#define CHECK_BOX_D_S                 DEFAULT_LIST_CHECK_BOX_D_S // 选择框里面的点距离外框的边距
#define CHECK_BOX_D_W                 CHECK_BOX_F_W-2*(CHECK_BOX_D_S+1) // 选择框里面的点宽度(这里加的常量1是外框的线条宽)
#define CHECK_BOX_D_H                 CHECK_BOX_F_H-2*(CHECK_BOX_D_S+1) // 选择框里面的点高度(这里加的常量1是外框的线条宽)
#define CHECK_BOX_R                   DEFAULT_LIST_CHECK_BOX_R     //列表确认选择框倒角
#define CHECK_BOX_D_R                 DEFAULT_LIST_CHECK_BOX_D_R   //列表确认选择框内点的倒角
#define LIST_TXT_SLIDESTR_MODE        DEFAULT_LIST_TXT_SLIDESTR_MODE  //磁贴标题文本的滚动模式
#define LIST_VAL_SLIDESTR_MODE        DEFAULT_LIST_VAL_SLIDESTR_MODE  //磁贴标题文本的滚动模式
#define LIST_LINETAIL_VAL_PREFIX    "~="       //listtext中使用这些字符其中一个，行尾会把val显示出来
#define LIST_LINETAIL_TXT_PREFIX    "><"        //listtext中使用这些字符其中一个，行尾会把content显示出来
#define LIST_LINETAIL_SPIN_PREFIX   "%$"        //listtext中使用这些字符其中一个，行尾会把val按照decimal显示为定点数
#define LIST_LINETAIL_CONF_PREFIX   "@#"        //listtext中使用这些字符其中一个，行尾会显示一个单选框(其中第一个作为二值选项框会自动处理,如果开启自动处理的话)
#if (LIST_VAL_MAX_LEN < 6) //Val的空间是由Text长度宏计算来的，因此如果这个长度不足最小字符宽度6的话，报一个warning
#warning "LIST_VAL_MAX_LEN is too small, the reason may be that DEFAULT_LIST_TEXT_LEN_PER_MAX is too large OR the WOUOUI_BUFF_WIDTH is too small"
#endif
#define LIST_MACRO_ASSERT             ((CHECK_BOX_R*2 > CHECK_BOX_F_W) || (CHECK_BOX_D_R*2 >CHECK_BOX_D_W) || (LIST_VAL_MAX_LEN < CHECK_BOX_F_W))
    //检查check box的倒角是否符合大小(倒角太大会自动取到能取的最大值的，但还是提示一下)，这个检查在Init中执行，返回True需要打印log提示
//List页面类型定义
// ListPage 需要的共用变量集合(类变量)
struct ListPageVar {
    bool radio_click_flag; // 标志位，用于标记在单选项页面内是否单击了Click
    int16_t indicator_w_temp; //用于暂时存储指示器长度，统一在ind_ctrl中赋值给指示器
    AnimPos optInt;        // 列表中每个选项的间隔值
    SlideStr opt_text_ss;       // 滑动选项文字
    SlideStr opt_val_ss;       // 滑动选项数值
};
typedef enum {
    Setting_none = 0x00, // 页面无设置
    Setting_radio,       // 设置为单选页面
} PageSetting;           // 页面设置
//ListPage页面类
typedef struct ListPage {     // 列表页面
    Page page;                // 基础页面信息
    PageSetting page_setting; // 页面设置
    uint8_t item_num;         // 页面选项个数，title和icon个数需与此一致
    uint8_t select_item;      // 选中选项
    Option *option_array;     // 选项类型的数组(由于数组大小不确定，使用指针代替)
    uint8_t line_n;           // = DISP_H / LIST_LINE_H; 屏幕内有多少行选
    int16_t ind_y_tgt;        // 存储指示器y目标坐标
} ListPage;                   // 列表页面类型(所有类型页面，类型成员为第一个，方便查看)
// ListPage的接口函数
/**
 * @brief 初始化列表页面
 * @param lp 列表页面指针
 * @param item_num 列表项目数量
 * @param option_array 选项数组指针
 * @param page_setting 页面设置
 * @param call_back 回调函数
 * @note 用于创建和初始化一个新的列表页面
 */
void WouoUI_ListPageInit(ListPage *lp, uint8_t item_num, Option *option_array, PageSetting page_setting, CallBackFunc call_back);
/**
 * @brief 移动到列表的上一个项目
 * @param lp 列表页面指针
 * @attention 此函数只建议在回调函数中使用 
 */
void WouoUI_ListPageLastItem(ListPage *lp);
/**
 * @brief 移动到列表的下一个项目
 * @param lp 列表页面指针
 * @attention 此函数只建议在回调函数中使用 
 */
void WouoUI_ListPageNextItem(ListPage* lp);

/**
 * @brief 获取标题列表页当前选中的选项
 * 
 * @param cur_page_addr 当前页面地址
 * @return Option* 返回选中的选项指针，如果没有选中项则返回NULL
 * 
 * @note 此函数用于获取标题列表页中用户当前选中的选项(该函数TitlePage和ListPage都可以作为参数传入)
 */
Option* WouoUI_ListTitlePageGetSelectOpt(const Page* cur_page_addr);


//===================================================Wave页面====================================================
// 波形相关参数
#define WAVE_FONT                     DEFAULT_WAVE_FONT       // 波形字体
#define WAVE_BOX_L_S                  DEFAULT_WAVE_BOX_L_S              // 波形边框左边距
#define WAVE_BOX_R_S                  DEFAULT_WAVE_BOX_R_S              // 波形边框右边距
#define WAVE_BOX_U_S                  DEFAULT_WAVE_BOX_U_S              // 波形边框上边距
#define WAVE_BOX_D_S                  DEFAULT_WAVE_BOX_D_S              // 波形边框下边距
#define WAVE_BOX_W                    (WOUOUI_BUFF_WIDTH-DEFAULT_WAVE_Y_AXIS_LABEL_WIDTH-1) // 波形边框宽度
#define WAVE_BOX_H                    (WOUOUI_BUFF_HEIGHT-GET_FNOT_H(WAVE_FONT)-DEFAULT_WAVE_BOX_D_S-DEFAULT_WAVE_TEXT_D_S)             // 波形边框高度
#define WAVE_STOP_SIG_HW              (GET_FNOT_H(WAVE_FONT)-1)             // 波形停止标志高度/宽度，为方形保持与字体同高 
#define WAVE_Y_AXIS_LABEL_WIDTH       DEFAULT_WAVE_Y_AXIS_LABEL_WIDTH //y轴标签的宽度
#define WAVE_TEXT_L_S                 DEFAULT_WAVE_D_TXT_VAL_S          // 波形文本左边距
#define WAVE_TEXT_R_S                 DEFAULT_WAVE_D_TXT_VAL_S          // 波形文本右边距
#define WAVE_TEXT_D_S                 DEFAULT_WAVE_TEXT_D_S             // 波形边框下边距
#define WAVE_TEXT_W                   DEFAULT_WAVE_TEXT_W             // 文字背景框宽度
#define WAVE_SHOW_WIDTH               WAVE_BOX_W - 2 // 波形展示的宽度
#define WAVE_DEPTH                    DEFAULT_WAVE_DEPTH//波形的储存深度
#define WAVE_TXT_SLIDESTR_MODE        DEFAULT_WAVE_TXT_SLIDESTR_MODE  //磁贴标题文本的滚动模式
#define WAVE_VAL_SLIDESTR_MODE        DEFAULT_WAVE_VAL_SLIDESTR_MODE  //磁贴标题文本的滚动模式
#if(WAVE_DEPTH < WAVE_SHOW_WIDTH+1) //波形深度必须于宽度+1,不然显示上有缺失，因为深度是数组的大小，报一个warning
#warning "WAVE_DEPTH is too small, the wave will be lost some point, not zoom to fill the screen"
#endif
// 类型定义
// 波形页面的类变量
struct WavePageVar {
    SlideStr title_ss; // 滑动标题
    SlideStr val_ss;   // 滑动数值
    SlideStr max_val_ss; //最大值滑动窗口
    SlideStr min_val_ss; //最小值滑动窗口
    SlideStr mid_val_ss; //中值滑动窗口
    AnimPos text_y;   // 标题的y坐标
    AnimPos y_axis_val_x;     // 数值的y坐标
    uint16_t last_y;           // 上一次绘制的y坐标
    uint16_t cur_y;           // 本次绘制的y坐标
};
// 波形类型
typedef enum {
    WaveType_Solid = 0x00, // 实线波形
    WaveType_Dash,         // 虚线波形                             
    WaveType_Fill,         // 填充波形
} WaveType;                // 波形类型
// 波形数据类
typedef struct WaveData {
    char * text;              // 波形标题
    WaveType waveType;  // 波形类型
    int16_t data[WAVE_DEPTH]; // 用于记录波形全程队列
    uint16_t idx_head;           // 队列头部指针/下标(最旧数据的位置)
    uint16_t idx_tail;           // 队列尾部指针/下标(最新数据的位置)
    uint16_t idx_show_max;       // 波形队列的最大指针/下标
    uint16_t idx_show_min;       // 波形队列的最小指针/下标
    uint16_t idx_show_head;    // 显示的波形数据的头指针(下标)
    uint16_t idx_show_tail;    // 显示的波形数据的尾指针(下标)
    int16_t rangeMax;         // 波形最大值
    int16_t rangeMin;         // 波形最小值
    DecimalNum decimal_num:4; // 小数位数
    bool stop_flag:1;       // 波形停止滚动标志
} WaveData;
// 波形页面类
typedef struct WavePage {  // 列表页面
    Page page;             // 基础页面信息
    WaveData* wave_data_array; //波形数据数组
    uint8_t wave_data_num; //波形数据个数(上面数组的大小)
    uint8_t wave_data_select; //选中的波形数据
} WavePage;   
// ------波形页面接口函数
/**
 * @brief 初始化波形页面
 * @param wp 波形页面控制器指针
 * @param wave_num 波形数量
 * @param wave_data_array 波形数据数组
 * @param call_back 回调函数
 */
void WouoUI_WavePageInit(WavePage *wp, uint8_t wave_num, WaveData * wave_data_array, CallBackFunc call_back); 
/**
 * @brief 更新指定波形的数据值
 * @param wp 波形页面控制器指针
 * @param wave_num 波形编号
 * @param new_data 新的波形数据
 */
void WouoUI_WavePageUpdateVal(WavePage* wp, uint8_t wave_num, int16_t new_data);
/**
 * @brief 显示波形数组中上一组波形数据
 * @param wp 波形页面控制器指针
 */
void WouoUI_WavePageShowLastWaveData(WavePage* wp);
/**
 * @brief 显示波形数组中下一组波形数据
 * @param wp 波形页面控制器指针
 * @attention 此函数只建议在回调函数中使用
 */
void WouoUI_WavePageShowNextWaveData(WavePage* wp);
/**
 * @brief 检查指定波形是否可以向左或向右移动
 * @param wp 波形页面控制器指针
 * @param wave_num 波形编号
 * @param left0ORright1 移动方向(0:左移, 1:右移)
 * @return true 可以移动
 * @return false 不能移动
 * @attention 此函数只建议在回调函数中使用
 */
bool WouoUI_WavePageCanShiftWave(WavePage * wp, uint8_t wave_num, uint8_t left0ORright1);//返回true,表示能够移动
/**
 * @brief 波形左移操作
 * @param wp 波形页面控制器指针
 * @param wave_num 波形编号
 * @return true 移动成功
 * @return false 已到最左端
 * @attention 此函数只建议在回调函数中使用
 */
bool WouoUI_WavePageLeftShiftWave(WavePage* wp, uint8_t wave_num); //返回值表示是否已经到头了(即有没有成功shift波形)
/**
 * @brief 波形右移操作
 * @param wp 波形页面控制器指针
 * @param wave_num 波形编号
 * @return true 移动成功
 * @return false 已到最右端
 * @attention 此函数只建议在回调函数中使用
 */
bool WouoUI_WavePageRightShiftWave(WavePage* wp, uint8_t wave_num);
/**
 * @brief 停止或重启波形显示
 * @param wp 波形页面控制器指针
 * @param wave_num 波形编号
 * @param stop 停止标志(true:停止, false:重启)
 * @attention 此函数只建议在回调函数中使用
 */
void WouoUI_WavePageStopRestartWave(WavePage* wp, uint8_t wave_num, bool stop);
/**
 * @brief 检查波形是否处于停止状态
 * @param wd 波形数据指针
 * @return true 已停止
 * @return false 正在运行
 * @attention 此函数只建议在回调函数中使用
 */
#define WouoUI_WaveDataIsStop(wd) ((wd)->stop_flag)


#ifdef __cplusplus
}
#endif

#endif
