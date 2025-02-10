/*
版本更新日志：
Version-1.0.0[2025.02.09]:
1. 适配多尺寸屏幕，可以通过WouoUI_conf.h中的宽长的宏定义更改屏幕宽长，所有页面的元素都会自动居中
2. 字体自适应，同样在WouoUI_conf.h中的修改页面使用的字体，页面会自适应字体的宽高
3. 长文本自动滚动，长文本在选中时会自动滚动，同时滚动模式、速度和起始延时都有参数可以调整
4. 基于anim、slide动画监视的软件动态刷新，相比硬件buff对比动态刷新：
    - 软件刷新：基于动画监视，静止时完全停止状态机，只会轮询msg输入，通用性不如硬件动态刷新。
    - 硬件刷新：基于buff对比，通用性强，但需要多一个buff作为对比，同时静止时，内部buff的clear和重新写入一直在运行，只是没有将buff发送
5. 主状态机优化，将弹窗抽象为页面(弹窗状态机和页面状态机融合)，支持任意页面的弹窗调用和弹窗自身的嵌套调用
6. 修改了回调函数类型，回调函数会将该页面msg传入，方便使用者进行更灵活的开发，且所有页面回调函数调用机制统一————只要有该页面有msg输入就会调用
7. 宏参数检查和LOG提示
版本致谢：
@(bilibili叻叻菌)[https://space.bilibili.com/86391945] 为这个版本提供了
    - 指示器连贯的丝滑动画
    - 波形页面的优化
    - 用于浮点数调整的Spin弹窗
    - 使用渐进整形的动画参数代替浮点运算的神来之笔(👍)
Todo List：
[ ] 将中文接口完善，支持中文
[ ] 加入图片显示页面和钟表页面，完成宏定义裁剪，适配前一个版本内存小的air001😭
[ ] 实现一个C++版本，使其能作为arduino的库使用(正好在学C++🤣)
[ ] 实现在freetros中线程安全的版本，这个版本中对全局变量的分类和整理也是为了这个做准备😀
本项目地址和开发者：
@ (项目地址)[https://github.com/Sheep118/WouoUI-PageVersion]
@ (本人bilibili)[https://space.bilibili.com/679703519]
@ (叻叻菌bilibili)[https://space.bilibili.com/86391945]
*/

#ifndef __WOUOUI_UI_H__
#define __WOUOUI_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "WouoUI_common.h"
#include "WouoUI_anim.h"
#include "WouoUI_graph.h"
#include "WouoUI_page.h"
#include "WouoUI_msg.h"
#include "WouoUI_win.h"

/*============================================常量定义=========================================*/
// 定义字符串类型
typedef char *String;
typedef enum UIState {
    ui_page_out = 0x00, // ui层级退出时
    ui_page_in,         // ui层级深入时
    ui_page_proc,       // ui页面处理时
} UIState;              // UI状态机
//------类别下标声明。用于UI参数数组中做索引
enum _ani_kind // 动画速度类别(数组中的下标)
{
    IND_ANI = 0x00, // 指示器动画速度
    TILE_ANI,       // 磁贴动画速度
    TAG_ANI,        // 磁贴页面标签动画速度
    LIST_ANI,       // 列表动画速度
    WAVE_ANI,       // 波形动画速度
    WIN_ANI,        // 弹窗动画速度
    FADE_ANI,       // 页面渐变(模糊)退出速度
    AIN_ALL_NUM,    // 动画速度参数的数目，用于数组初始化
};
enum _ufd_kind // 展开方式类别(数组中的下标)
{
    TILE_UFD = 0x00, // 磁贴图标从头展开开关
    LIST_UFD,        // 菜单列表从头展开开关
    UFD_ALL_NUM,     // 展开方式类别数目
};
enum _loop_kind // 循环模式类别(数组中的下标)
{
    TILE_LOOP = 0x00, // 磁贴图标循环模式开关
    LIST_LOOP,        // 菜单列表循环模式开关
    LIST_WIN_LOOP,    // 列表弹窗循环开关
    LOOP_ALL_NUM,     // 循环模式类别数目
};
enum _SSS_kind
{
    TILE_SSS = 0x00,  //磁贴文本滚动步长
    LIST_TEXT_SSS,    //列表文本滚动步长
    LIST_VAL_SSS,     //列表值滚动步长
    WAVE_TEXT_SSS,    //波形页面文本滚动步长
    WAVE_VAL_SSS,    //波形页面数值滚动步长
    WIN_TXT_SSS,     //弹窗内文本的滚动步长
    WIN_VAL_SSS,     //弹窗内数值的滚动步长
    SSS_ALL_NUM,        
};
enum _wbb_kind{
    MGS_WBB = 0x00,  //msgwin背景模糊程度下标
    CONF_WBB,  //msgwin背景模糊程度下标
    VAL_WBB,  //msgwin背景模糊程度下标
    SPIN_WBB,  //msgwin背景模糊程度下标
    LIST_WBB,  //msgwin背景模糊程度下标
    WBB_ALL_NUM, 
};
typedef struct UiPara {
    uint16_t ani_param[AIN_ALL_NUM];  // 动画参数数组
    uint8_t ufd_param[UFD_ALL_NUM];   // 展开参数数组
    uint8_t loop_param[LOOP_ALL_NUM]; // 循环参数数组
    uint8_t slidestrstep_param[SSS_ALL_NUM]; //文本滚动步长参数数组
    SlideStrMode slidestrmode_param[SSS_ALL_NUM]; //文本滚动步长参数数组
    BLUR_DEGREE winbgblur_param[WBB_ALL_NUM]; //背景模糊参数的数组
} UiPara;                             // UI参数集合类型
extern UiPara g_default_ui_para;      // 共外部使用的全局UI参数变量

// 指示器
typedef struct Indicator {
    AnimPos x;
    AnimPos y;
    AnimPos w;
    AnimPos h;
} Indicator;

typedef struct ScrollBar {
    bool display; // 是否显示滚动条
    AnimPos y;    // 滚动条y坐标
} ScrollBar;


typedef struct UIBlur {
    uint8_t blur_cur : 3;
    uint8_t blur_tgt : 3;
    bool blur_end;
    uint16_t timer; //用于演示虚化函数，
    uint16_t blur_time;
} UIBlur;

// WouoUI类型，整个UI类型
typedef struct WouoUI {
#if SOFTWARE_DYNAMIC_REFRESH
    uint8_t is_motionless:1;                  //soft sendbuff的标志位
#endif
    uint8_t slide_is_finish:1;                //滚动动画是否结束
    uint8_t anim_is_finish;                   // 用于查看非线性动画是否结束的标志位
    PageAddr home_page;                       // 主页面的地址
    PageAddr current_page;                    // 当前页面的地址
    PageAddr in_page;                         // 记录fade out后要进入的页面地址(在jump中暂存下页面，之后才能给cur_page)
    Pen pen;                                  // 画笔
    ScreenBuff screen_buff;                   // 屏幕缓冲区
    FunSendScreenBuff* pfun_sendbuff;         // 刷新屏幕的函数指针
    Canvas w_all;                             // 全局画布变量，所有的绘制都在这个窗口内进行
    UIState state;                            // ui状态变量
    UiPara *upara;                            // ui参数集合
    InputMsgQue msg_queue;                    // 消息队列
    Indicator indicator;                      // 指示器
    ScrollBar scrollBar;                      // 滚动条
    UIBlur ui_blur;                            // UI模糊
    BLUR_DEGREE win_bg_blur;                   //弹窗存在时背景模糊的程度
    uint16_t time;                            // UI时间尺度参数
#if HARDWARE_DYNAMIC_REFRESH
    ScreenBuff screen_dynamic_buff; //用于动态刷新的对比缓冲区
#endif
    struct TitlePageVar tp_var; // TitlePage共用变量集合
    PageMethods tp_mth;         // TitlePage的方法集合
    struct ListPageVar lp_var;  // List页面的共用变量集合
    PageMethods lp_mth;         // List页面的方法集合
    struct WavePageVar wt_var;  // 波形显示区域的共用变量集合
    PageMethods wt_mth;         // 波形显示区域的方法集合

    struct MsgWinVar mw_var;    // 消息弹窗共用变量集合
    PageMethods mw_mth;         // 消息弹窗方法集合
    struct ConfWinVar cw_var;   //确认弹窗的共用变量集合
    PageMethods cw_mth;         //确认弹窗的方法集合
    struct ValWinVar vw_var;    //数值条弹窗共用变量集合
    PageMethods vw_mth;         //数值跳弹窗的方法集合
    struct SpinWinVar spw_var;  // 微调数值弹窗共用变量集合
    PageMethods spw_mth;        // 微调数值弹窗方法集合
    struct ListWinVar lw_var;   // 列表弹窗共用变量集合
    PageMethods lw_mth;         // 列表弹窗方法集合                                                                               
} WouoUI;

//============================================全局变量的外界声明================================
extern WouoUI *p_cur_ui;
/*============================================接口函数=========================================*/
/**
 * @brief 将整数转换为字符串
 * @param num 要转换的整数
 * @param str 存储结果字符串的缓冲区
 * @return 转换后的字符串
 */
char *ui_itoa_str(uint32_t num, char *str);

/**
 * @brief 将整数转换为浮点数字符串（通用格式）
 * @param num 要转换的整数
 * @param decimalNum 小数位数
 * @return 转换后的字符串
 */
char *ui_ftoa_g(int32_t num, DecimalNum decimalNum);

/**
 * @brief 将整数转换为浮点数字符串（通用格式）
 * @param num 要转换的整数
 * @param decimalNum 小数位数
 * @param str 存储结果字符串的缓冲区
 * @return 转换后的字符串
 */
char *ui_ftoa_g_str(int32_t num, DecimalNum decimalNum, char *str);

/**
 * @brief 将整数转换为浮点数字符串（定点格式）
 * @param num 要转换的整数
 * @param decimalNum 小数位数
 * @return 转换后的字符串
 */
char *ui_ftoa_f(int32_t num, DecimalNum decimalNum);

/**
 * @brief 将整数转换为浮点数字符串（定点格式）
 * @param num 要转换的整数
 * @param decimalNum 小数位数
 * @param str 存储结果字符串的缓冲区
 * @return 转换后的字符串
 */
char *ui_ftoa_f_str(int32_t num, DecimalNum decimalNum, char *str);

/**
 * @brief 获取选项的浮点值
 * @param option 选项指针
 * @return 选项的浮点值
 */
float WouoUI_GetOptionFloatVal(Option *option);

/**
 * @brief 发送消息到消息队列
 * @param msg 要发送的消息
 */
#define WOUOUI_MSG_QUE_SEND(msg) WouoUI_MsgQueSend(&(p_cur_ui->msg_queue), msg)

/**
 * @brief 读取消息队列中的消息
 * @return 读取到的消息
 */
#define WOUOUI_MSG_QUE_READ() WouoUI_MsgQueRead(&(p_cur_ui->msg_queue))

/**
 * @brief 清空消息队列
 */
#define WOUOUI_MSG_QUE_CLEAR() WouoUI_MsgQueClear(&(p_cur_ui->msg_queue))

/**
 * @brief 选择默认UI
 */
void WouoUI_SelectDefaultUI(void);

/**
 * @brief 设置当前UI
 * @param ui 要设置的UI指针
 */
void WouoUI_SetCurrentUI(WouoUI *ui);

/**
 * @brief 附加发送缓冲区函数
 * @param fun 发送缓冲区的函数指针
 */
void WouoUI_AttachSendBuffFun(FunSendScreenBuff fun);

/**
 * @brief 处理UI逻辑
 * @param time 时间参数
 */
void WouoUI_Proc(uint16_t time);

/**
 * @brief 跳转到指定页面
 * @param self_page 当前页面地址
 * @param terminate_page 目标页面地址
 */
void WouoUI_JumpToPage(PageAddr self_page, PageAddr terminate_page);

/**
 * @brief 获取当前页面
 * @return 当前页面指针
 */
Page *WouoUI_GetCurrentPage(void);

#ifdef __cplusplus
}
#endif

#endif
