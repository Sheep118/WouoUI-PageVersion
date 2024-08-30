#include "oled_ui.h"
#include "string.h"
#include "stdio.h"
//===============================类型声明(内部静态函数声明)===================================
//弹窗类型
typedef struct _Win 
{
  uint8_t state:2; //弹窗的状态 不再弹窗内0，参数初始化1,弹窗动画轮询2 
  uint8_t l; // = (DISP_W - WIN_W) / 2;
  uint8_t u; // = (DISP_H - WIN_H) / 2;
  float bar; //进度条长度值(按键位置x值)btn_x
  float bar_trg; //进度条目标值(按键位置x值目标)btn_x_trg
  float y;    //弹窗y值
  float y_trg; //弹窗y植目标值
  PageAniInit anim_init; //动画初始化方法
  void (*show)(Option * select_item); //展示方法(函数指针，参数为选中项)
  void (*react)(PageAddr bg,Option * select_item); //响应方法(函数指针，参数为背景页面和选中项)
} Win; //数值(确认)弹窗类型
//有一个全局数值弹窗变量vwin在ui结构体中，用于每次弹窗使用(对于数值弹窗来说,不论是上下、还是确认，都会调用一次回调，方便数值实时更新) 
//有一个全局确认弹窗变量cwin在ui结构体中，用于每次确认弹窗使用（对于确认弹窗来说，只有按下确认键，才会触发回调,确认的意义所在）

//Listpage 需要的共用变量集合
struct TitlePageVar 
{ //因为Title选中项一直在中间，所以不需要有每个页面变量记录它真实页面的偏移位置
    float   icon_x; //图标的x坐标距选中目标的间距的变量
    float   icon_x_trg;//图标的x坐标距选中目标的间距的变量目标
    float   icon_y;//图标的y坐标
    float   icon_y_trg;//图标的y坐标目标
    float   indi_x; //指示器的x坐标
    float   indi_x_trg;//指示器的x坐标目标值
    float   title_y;//标题的y坐标
    float   title_y_trg;//标题的y坐标目标值
};

//Listpage 需要的共用变量集合
struct ListPageVar  
{
    uint8_t slip_flag;   //切换动画是否完成的标志位
    float y;            //列表中每个选项的间隔值(过度动画后作为正真页面的起始y坐标值，以屏幕原点为0)
    float y_trg;        //列表中每个选项间隔的目标值
    float box_x;        //选中框x
    float box_x_trg;    //选中框x的目标值
    float box_y;        //选中框y(以屏幕原点为0的box_y值)
    // float box_y_trg;    //选中框y的目标值(这个变量放在每一个页面中，用于推算正真的页面的起始y坐标值)
    float bar_y;        //进度条的y值
    float bar_y_trg;    //进度条的y目标值
};

#if PAGE_WAVE_ENABLE
//波形曲线标签全局变量，用于记录波形页面中标签的移动
struct WaveTag
{
  int16_t data_que[WAVE_W-1]; //用于记录波形全程队列
  uint8_t p_head; //队列头部指针，表征队列最起始的位置
  uint8_t p_rear; //出队列指针
  uint8_t p_font; //入队列指针
  uint8_t change_flag; // 切换过的标志
  float   text_bg_r;   //文本框动态宽度
  float   text_bg_r_trg; 
} ; 
#endif

#if PAGE_RADIO_ENABLE
//单选项页面共用变量集合
struct RadioPageVar
{
    uint8_t radio_click_flag:1; //标志位，用于标记在单选项页面内是否单击了Click
};
#endif

#if PAGE_RADERPIC_ENABLE
//RaderPage的共用变量集合
struct RaderPicPageVar 
{
    float move_x; 
    float move_y;// 图片绘制x y坐标的移动倍率(-1~0)
};
#endif

#if PAGE_DIGITAL_ENABLE
//只有单个位的数据,用于存储单个位的数据
typedef struct _SigleBit
{
    uint8_t num:4; 
} SingleBit;
//Digital页面变量集合
struct DigitalPageVar
{
    uint8_t digital_num_pos; //0表示没有选中位置，1-7bit每一位置1表示该位被选中
    uint8_t temp_ripple_index; //用于在ripple时记录选中位置(选中最低位位置开始，慢慢往上增)
    DigitalDirect dir:2;  //用于Digital Show和React函数间传递信号
    window w_digtal[2]; //用限制数字时钟的窗口(1个数字窗口+1个label窗口) 2个窗口
    float rect_y; //移动外框的顶点坐标
    float rect_y_trg; //移动外框的顶点坐标目标
    float label_y; //移动外框的顶点坐标
    float label_y_trg; //移动外框的顶点坐标目标
    float num_y;   //移动数字的顶点坐标
    float num_y_trg;   //移动数字的顶点坐标目标
    uint8_t digital_num_xindex[DIGITAL_NUM_INDEX_MAX];//6个单位数字的横坐标存储数组
    uint8_t Gap_shine_time; //全局闪烁时间递减变量 
    uint8_t Uline_shine_time; //全局闪烁时间递减变量 
    SingleBit num_array[6]; //显示的6个单个位数据
    uint8_t last_or_next_label_index;  //用来记录上次显示的label的index
    uint8_t temp_num_pos; //用于在show函数内暂时装载num_pos变量，实现同时运动动画和ripple动画的切换
    DigitalDirect temp_dir; //记录下运动方向
    DigitalDirect temp_label_dir; //记录下label运动方向，防止数字和label同时需要运动的情况出现
};
#endif

//WouoUI类型，整个UI类型
struct _WouoUI
{
    uint8_t init_finish_flag ; //页面是否正在初始化完成的标志位，即是否完成过度页面的初始化
    PageAddr home_page;    //主页面的地址
    PageAddr current_page; //当前页面的地址
    UIState state ;           //ui状态变量
    UiPara* upara;   //ui参数集合
    InputMsg msg_queue[INPUT_MSG_QUNEE_SIZE];//消息队列
    uint8_t msg_que_font ;
    uint8_t msg_que_rear; //消息队列的头尾指针
    struct TitlePageVar tp_var;   //TitlePage共用变量集合
    struct ListPageVar lp_var;    //List页面的共用变量集合(包括Radio和Wave的列表的变量也使用这个共用变量)
    struct DigitalPageVar dp_var; //digital页面的共用变量集合
    Win vwin; //数值弹窗插件
#if PAGE_RADIO_ENABLE
    struct RadioPageVar rp_var;   //Radio页面共用变量集合
#endif
#if PAGE_WAVE_ENABLE
    struct WaveTag     wt_var;   //波形显示区域的共用变量集合
#endif
#if PAGE_RADERPIC_ENABLE
    struct RaderPicPageVar rrp_var;   //RaderPicPage的共用变量集合
#endif
#if UI_CONWIN_ENABLE
    Win cwin; //确认弹窗插件
#endif
}; 

//静态函数声明(方便全局变量初始化函数指针)
static void OLED_ValWinAnimInit(PageAddr bg);
static void OLED_ValWinShow(Option* win);
static void OLED_ValWinReact(PageAddr bg, Option* select_win);
#if UI_CONWIN_ENABLE
static void OLED_ConWinAnimInit(PageAddr bg);
static void OLED_ConWinShow(Option* win);
static void OLED_ConWinReact(PageAddr bg,Option* win);
#endif

//===================================全局变量==================================
//全局UI参数集合对象(同时初始化)，这个UI的相关参数都在这个集合中定义
UiPara g_default_ui_para = 
{
    .ani_param = {
        [TILE_ANI] = 120, //磁贴动画速度
        [LIST_ANI] = 120, //列表动画速度
        [WIN_ANI] = 120,  //弹窗动画速度
        [TAG_ANI] = 120,  //标签动画速度
        [DIGI_ANI] = 100, //数字动画滚动速度
    },
    .ufd_param = {
        [TILE_UFD] = True, //磁贴图标从头展开开关
        [LIST_UFD] = True, //菜单列表从头展开开关
    },
    .loop_param = {
        [TILE_LOOP] = True, //磁贴图标循环模式开关
        [LIST_LOOP] = True, //菜单列表循环模式开关
    },
    .valwin_broken = True, //弹窗背景虚化开关
    .conwin_broken = True, //确认弹窗背景虚化开关
    .digital_ripple = True, //digital页面波纹递增动画开关
    .raderpic_scan_mode = False, //镭射文字只扫描亮处 
    .raderpic_scan_rate = 4,  //镭射文字扫描速度
    .raderpic_move_rate = 50, //镭射文字移动速度
};  
//默认UI对象（同时进行初始化）
WouoUI default_ui = {
    .init_finish_flag = False, //初始时需要先开启过度动画
    .current_page = NULL,
    .home_page = NULL,  //初始化当前页面和主页面均是NULL    
    .state = ui_layer_in, //从没页面进入主页面，所以是lay_in
    .msg_queue = {msg_none},
    .msg_que_rear = 0,
    .msg_que_font = 0, //消息队列相关的初始化
    .vwin = { //弹窗里的默认参数
        .l = (OLED_WIDTH - WIN_W)/2,
        .u = (OLED_HEIGHT - WIN_H)/2,
        .state = 0, //默认弹窗是关闭的
        .anim_init = OLED_ValWinAnimInit,
        .show = OLED_ValWinShow,
        .react = OLED_ValWinReact,//关联相关的处理函数
    },
#if UI_CONWIN_ENABLE
    .cwin = {
        .l = (OLED_WIDTH - WIN_W)/2, //不管选框宽度，以字体宽度为准
        .u = (OLED_HEIGHT - WIN_H)/2, //确认弹窗里的默认参数
        .state = 0, //默认弹窗是关闭的
        .anim_init = OLED_ConWinAnimInit,
        .show = OLED_ConWinShow,
        .react = OLED_ConWinReact, //关联相关的处理函数
    },
#endif
#if PAGE_DIGITAL_ENABLE 
    .upara = &g_default_ui_para,  //将默认参数赋值给default_ui
    .dp_var = { //初始化Digital页面共用变量的初始化
        .Gap_shine_time = 0,
        .Uline_shine_time = 0,
        .last_or_next_label_index = 0,
        .temp_dir = Digital_Direct_None,
        .temp_label_dir = Digital_Direct_None,
        .temp_num_pos = 0,
        .w_digtal = { //数字Label窗口的坐标设置
            [0] = {.start_x = 16, .h = 24, .w = 96}, //确定数字窗口的一些参数
            [1] = {.start_x = 0,  .h = 16, .w = 128}, //确定标签窗口的一些参数
        },
        .num_array = {
            {0},{0},{0},{0},{0},{0}
        },
        .digital_num_xindex = {12+24+12+24+12, 12+24+12+24, 12+24+12,12+24, 12,0}
        //数字框的横坐标数组{12+24+12+24+12, 12+24+12+24, 12+24+12,12+24, 12,0}
    },
#endif
#if PAGE_RADIO_ENABLE //初始化radio页面共用变量的初始化
    .rp_var = {.radio_click_flag = False}
#endif
};
WouoUI* p_current_ui = &default_ui; //当前操作的ui对象的指针(默认使用defaultui)
// PageAddr page_array[UI_MAX_PAGE_NUM] = {NULL}; //页面地址的数组，方便记录不同页面用于切换
window w_all = {0,0,OLED_WIDTH,OLED_HEIGHT}; //全局窗口变量，所有的绘制都在这个窗口内进行
//===================================对应函数==================================

//--------通用页面函数
#define UNUSED_PARAMETER(x)    ((void)x) //对未使用参数的处理，防止警告
//通用的页面返回上一个页面函数
static void OLED_PageReturn(PageAddr page_addr)
{
    Page* p = (Page*)page_addr;
    p_current_ui->current_page = p->last_page; //将UI页面设置为上一级页面
    p_current_ui->state = ui_layer_in; //将状态机置为页面初始化状态
    p_current_ui->init_finish_flag = False; //启用过度页面动画
}

//最基本的页面成员初始化，并加入到页面队列中，注意，没有进行页面类型初始化，因为这个由各自初始化函数执行
static void OLED_PageInit(PageAddr page_addr, CallBackFunc call_back)
{
    Page* p = (Page*)page_addr;
    if(p_current_ui->home_page == NULL) //第一个页面初始化
    {
        p_current_ui->home_page = page_addr; //将其初始化为主页面
        p_current_ui->current_page = page_addr; //同时作为当前页面
    }
    p->last_page = p_current_ui->home_page; 
    //页面初始化时没有上一级页面,只有在页面跳转时才确定页面的上下级关系,默认将主页面(第一个初始化的页面)作为上一级页面
    p->cb = call_back;
}

/**
 * @brief OLED_WinFSM函数用于处理窗口的状态机
 * 
 * @param w 指向窗口结构体的指针
 * @param bg 页面地址
 * @param select_item 指向选项结构体的指针
 */
static void OLED_WinFSM(Win * w, PageAddr bg, Option * select_item)
{
    switch (w->state)
    {
        case 1: w->anim_init(bg); w->state = 2; break; // 初始化只用在弹窗最开始时做—次弹窗参数初始化
        case 2: w->show(select_item); w->react(bg, select_item); break; // 弹窗动画和交互
        default: break;
    }
}
//检查页面类型，返回PageType类型
#define OLED_CheckPageType(page_addr) (((Page*)page_addr)->page_type)
/**
 * @brief 将无符号32位整数转换为字符串
 * 
 * @param num 要转换的无符号32位整数
 * @return char* 转换后的字符串
 */
char * itoa(uint32_t num)
{
  static char str[10] = {0}; // 定义一个静态字符数组，用于存储转换后的字符串
  memset(str, 0, sizeof(str)); // 将字符数组清零
  sprintf(str, "%d", num); // 将无符号32位整数转换为字符串
  return str; // 返回转换后的字符串
}
/**
 * @brief 将浮点数转换为字符串
 * 
 * @param num 要转换的浮点数
 * @return char* 转换后的字符串
 */
char * ftoa(float num)
{
    static char str[10] = {0}; // 定义一个静态字符数组，用于存储转换后的字符串
    memset(str, 0, sizeof(str)); // 将字符数组清零
    sprintf(str, "%.2f", num); // 将浮点数转换为字符串，并保留两位小数
    return str; // 返回转换后的字符串
}


//--------msg_que处理相关函数
#define OLED_MsgQueIsEmpty  (p_current_ui->msg_que_font == p_current_ui->msg_que_rear) //队列空
#define OLED_MsgQueIsFull   ((p_current_ui->msg_que_rear + 1)%INPUT_MSG_QUNEE_SIZE == p_current_ui->msg_que_font) //队列满
//向msg队列发送消息和读msg队列函数，参数和返回值均是InputMsg类型
void OLED_MsgQueSend(InputMsg msg)
{
  if(!OLED_MsgQueIsFull)
  {
    p_current_ui->msg_queue[p_current_ui->msg_que_rear] = msg; 
    p_current_ui->msg_que_rear++;
    if(p_current_ui->msg_que_rear == INPUT_MSG_QUNEE_SIZE )p_current_ui->msg_que_rear = 0;
  }
}
InputMsg OLED_MsgQueRead(void)
{
  InputMsg msg = msg_none;
  if(!OLED_MsgQueIsEmpty)
  {
    msg = p_current_ui->msg_queue[p_current_ui->msg_que_font];
    p_current_ui->msg_que_font++;
    if(p_current_ui->msg_que_font == INPUT_MSG_QUNEE_SIZE)p_current_ui->msg_que_font = 0;
  }
  return msg;
}


//**********************每个以Page为基类的类都有对应的AnimInit、show、React方法
//--------Title页面相关函数

static void OLED_TitlePageAnimInit(PageAddr page_addr)
{
  p_current_ui->tp_var.icon_x = 0;
  p_current_ui->tp_var.icon_x_trg = TILE_ICON_S;
  p_current_ui->tp_var.icon_y = -TILE_ICON_H; 
  p_current_ui->tp_var.icon_y_trg = 0;
  p_current_ui->tp_var.indi_x = 0;
  p_current_ui->tp_var.indi_x_trg = TILE_INDI_W;
  p_current_ui->tp_var.title_y = TILE_INDI_S + (TILE_INDI_H - TILE_B_TITLE_H) / 2 + TILE_B_TITLE_H;
  p_current_ui->tp_var.title_y_trg = TILE_INDI_S + (TILE_INDI_H - TILE_B_TITLE_H) / 2;
}

static void OLED_TitlePageShow(PageAddr page_addr)
{
  TitlePage * tp = (TitlePage *)page_addr;
  float temp = 0; //用于存放临时的icon的x坐标
  //计算动画参数
  OLED_Animation(&p_current_ui->tp_var.icon_x,&p_current_ui->tp_var.icon_x_trg,p_current_ui->upara->ani_param[TILE_ANI]);
  OLED_Animation(&p_current_ui->tp_var.icon_y,&p_current_ui->tp_var.icon_y_trg,p_current_ui->upara->ani_param[TILE_ANI]);
  OLED_Animation(&p_current_ui->tp_var.indi_x,&p_current_ui->tp_var.indi_x_trg,p_current_ui->upara->ani_param[TILE_ANI]);
  OLED_Animation(&p_current_ui->tp_var.title_y,&p_current_ui->tp_var.title_y_trg,p_current_ui->upara->ani_param[TILE_ANI]);
  String show_str = tp->option_array[tp->select_item].text;
  //绘制title
  OLED_WinDrawStr(&w_all, ((OLED_WIDTH - TILE_INDI_W) - OLED_GetStrWidth(&(show_str[2]),TILE_B_TITLE_H)) / 2 + TILE_INDI_W,
                 p_current_ui->tp_var.title_y,TILE_B_TITLE_H,(uint8_t*)&(show_str[2]));
  //绘制title指示器
  OLED_WinDrawRBox(&w_all,0,TILE_ICON_S,p_current_ui->tp_var.indi_x,TILE_INDI_H,0);
  //绘制图标
  if(!p_current_ui->init_finish_flag) //过度动画尚未完成
    {
        for (uint8_t i = 0; i < tp->item_num; i++)
        {
            if(p_current_ui->upara->ufd_param[TILE_UFD])
                temp = (OLED_WIDTH - TILE_ICON_W) / 2 + i * p_current_ui->tp_var.icon_x - TILE_ICON_S * tp->select_item;
                //从第一个选项开始展开，最终保持选中在中间
            else
                temp = (OLED_WIDTH - TILE_ICON_W) / 2 + (i - tp->select_item) * p_current_ui->tp_var.icon_x; 
                //保证选中的选项在中间，向两侧展开
            OLED_WinDrawBMP(&w_all,(int16_t)temp,(int16_t)(p_current_ui->tp_var.icon_y),TILE_ICON_W,TILE_ICON_H,tp->icon_array[i],1);
        }
        if(p_current_ui->tp_var.icon_x == p_current_ui->tp_var.icon_x_trg)
        {
            p_current_ui->init_finish_flag = True; //设置过度动画已经结束
            p_current_ui->tp_var.icon_x = p_current_ui->tp_var.icon_x_trg = -1*tp->select_item * TILE_ICON_S; 
        }
    }
  else
      for(uint8_t i = 0; i < tp->item_num; i++) //过度动画完成后一般选择时的切换动画
        OLED_WinDrawBMP(&w_all,(OLED_WIDTH - TILE_ICON_W)/2 + (int16_t)(p_current_ui->tp_var.icon_x) + i*TILE_ICON_S,
        0,TILE_ICON_W,TILE_ICON_H,tp->icon_array[i],1);
}


static void OLED_TitlePageReact(PageAddr page_addr)
{
    TitlePage * tp = (TitlePage *)page_addr;
    String selcet_string = NULL;
    InputMsg msg = OLED_MsgQueRead(); //空时读出msg_none
    switch (msg)
    {
        case msg_up:
            if(p_current_ui->init_finish_flag) //已经完成过渡动画了
            {
                if(tp->select_item > 0)
                {
                    //第一个icon与中心的距离减小S
                    tp->select_item --;
                    p_current_ui->tp_var.icon_x_trg += TILE_ICON_S; 
                    //切换动画动作，动indi 和title
                    p_current_ui->tp_var.indi_x = 0;
                    p_current_ui->tp_var.title_y = TILE_INDI_S + (TILE_INDI_H - TILE_B_TITLE_H) / 2 + TILE_B_TITLE_H;
                }
                else
                {
                    if(p_current_ui->upara->loop_param[TILE_LOOP]) //开启循环的话
                    {
                        tp->select_item = tp->item_num -1;
                        p_current_ui->tp_var.icon_x_trg = -1*TILE_ICON_S*(tp->item_num -1);
                        //切换动画动作，动indi 和title
                        p_current_ui->tp_var.indi_x = 0;
                        p_current_ui->tp_var.title_y = TILE_INDI_S + (TILE_INDI_H - TILE_B_TITLE_H) / 2 + TILE_B_TITLE_H;
                    }
                }
            }
          break;
        case msg_down:
            if(p_current_ui->init_finish_flag)
            {
                if(tp->select_item < (tp->item_num - 1))
                {
                    tp->select_item ++;
                    p_current_ui->tp_var.icon_x_trg -= TILE_ICON_S;
                    //切换动画动作，动indi 和title
                    p_current_ui->tp_var.indi_x = 0;
                    p_current_ui->tp_var.title_y = TILE_INDI_S + (TILE_INDI_H - TILE_B_TITLE_H) / 2 + TILE_B_TITLE_H;
                }
                else
                {
                    if(p_current_ui->upara->loop_param[TILE_LOOP])
                    {
                        tp->select_item = 0;
                        p_current_ui->tp_var.icon_x_trg = 0;
                        //切换动画动作，动indi 和title
                        p_current_ui->tp_var.indi_x = 0;
                        p_current_ui->tp_var.title_y = TILE_INDI_S + (TILE_INDI_H - TILE_B_TITLE_H) / 2 + TILE_B_TITLE_H;
                    }
                }
            }
          break;
        case msg_return: OLED_PageReturn(page_addr);break;
        case msg_home : break;
        case msg_click:
            selcet_string = tp->option_array[tp->select_item].text;
            if(selcet_string[0] == '~')
            {//数值弹窗的话，进入数值弹窗这个动作不触发回调，但弹窗内不论点击什么都会实时触发回调
                //当数值弹窗step设为0为只读是，进入数值弹窗前会调用一次回调，以便外界更改val值
                p_current_ui->vwin.state = 1; //进入弹窗初始化
                if((tp->option_array[tp->select_item].step == 0) && (tp->page.cb != NULL))
                    tp->page.cb(&(tp->page), &(tp->option_array[tp->select_item]));
            }
#if UI_CONWIN_ENABLE
            else if(selcet_string[0] == '$') p_current_ui->cwin.state = 1; //进入确认弹窗初始化
                //确认弹窗内只有确认弹窗内的click才会触发回调
#endif
            else if(tp->page.cb !=NULL)tp->page.cb(&(tp->page),&(tp->option_array[tp->select_item])); 
                //当前页面id和确认选中项的指针传入
          break;
        default:
          break;
    }
}

void OLED_TitlePageInit(
      TitlePage * title_page, //磁贴页面对象
      uint8_t item_num,       //选项个数，需与title数组大小，icon数组大小一致
      Option *option_array,   //整个页面的选项数组(数组大小需与item_num一致)
      Icon   *icon_array,     //整个页面的icon数组(数组大小需与item_num一致)
      CallBackFunc call_back) //回调函数，参数为确认选中项index（1-256）0表示未确认哪个选项
{
    //缺少选项个数检查，Icon个数检查，
    title_page->page.page_type = type_title;
    OLED_PageInit((PageAddr)title_page, call_back);
    title_page->page.ani_init = OLED_TitlePageAnimInit; 
    title_page->page.show = OLED_TitlePageShow;
    title_page->page.react = OLED_TitlePageReact; //关联处理函数(方法)
    title_page->select_item = 0;
    title_page->item_num = item_num;
    title_page->option_array = option_array;
    title_page->icon_array = icon_array;
    for (uint8_t i = 0; i < title_page->item_num; i++)
        title_page->option_array[i].order = i;//选项序号标号
}




//------------数值弹窗相关函数

static void OLED_ValWinAnimInit(PageAddr bg)
{
    UNUSED_PARAMETER(bg);
    p_current_ui->vwin.bar = 0;
    p_current_ui->vwin.y = WIN_Y;
    p_current_ui->vwin.y_trg = p_current_ui->vwin.u;
}

static void OLED_ValWinShow(Option* win)
{ 
    if(p_current_ui->upara->valwin_broken) //如果要背景虚化的话
    {
        OLED_SetPointColor(0);
        OLED_AllSrcFade(0,0x55);
        OLED_AllSrcFade(1,0xAA);
        OLED_SetPointColor(1);
    }
    p_current_ui->vwin.bar_trg = (float)(win->val - win->item_min)/(float)(win->item_max - win->item_min)*(WIN_BAR_W - 4);

    OLED_Animation(&p_current_ui->vwin.bar, &p_current_ui->vwin.bar_trg,p_current_ui->upara->ani_param[WIN_ANI]);
    OLED_Animation(&p_current_ui->vwin.y, &p_current_ui->vwin.y_trg,p_current_ui->upara->ani_param[WIN_ANI]);

    OLED_SetPointColor(0);
    OLED_WinDrawRBox(&w_all, p_current_ui->vwin.l, (int16_t)p_current_ui->vwin.y, WIN_W, WIN_H, 2); //弹窗背景黑边
    OLED_SetPointColor(1);
    OLED_WinDrawRBoxEmpty(&w_all, p_current_ui->vwin.l, (int16_t)p_current_ui->vwin.y, WIN_W, WIN_H, 2); //弹窗外框
    OLED_WinDrawRBoxEmpty(&w_all, p_current_ui->vwin.l + 5, (int16_t)p_current_ui->vwin.y + 20,WIN_BAR_W, WIN_BAR_H, 1); //进度条外框
    OLED_WinDrawRBox(&w_all, p_current_ui->vwin.l+7, (int16_t)p_current_ui->vwin.y+22, p_current_ui->vwin.bar, WIN_BAR_H - 4,1); //绘制进度条
    OLED_WinDrawStr(&w_all, p_current_ui->vwin.l + 5,(int16_t)p_current_ui->vwin.y+2, WIN_FNOT_H, (uint8_t*)&(win->text[2])); //提示文本跳过“~ ”
    OLED_WinDrawStr(&w_all, p_current_ui->vwin.l + 6 + OLED_GetStrWidth((char*)&(win->text[2]),WIN_FNOT_H),
                    (int16_t)p_current_ui->vwin.y+2,WIN_FNOT_H, (uint8_t*)(itoa(win->val))); //数值
}

static void OLED_ValWinReact(PageAddr bg, Option* select_win)
{
    if(p_current_ui->vwin.y == p_current_ui->vwin.y_trg && p_current_ui->vwin.y != WIN_Y_TRG) //弹窗进场动画结束时
    {
        InputMsg msg =  OLED_MsgQueRead();
        switch (msg)
        {
            case msg_up: 
                if(select_win->val < select_win->item_max)
                    select_win->val += select_win->step;
              break;
            case msg_down: 
                if(select_win->val > select_win->item_min)
                    select_win->val -= select_win->step;
              break;
            case msg_click:  //点击确认也会退出弹窗，不过，点击确认是会触发一次回调，点击返回则不会     
            case msg_return:
                p_current_ui->vwin.y_trg = WIN_Y_TRG; //弹窗退场
                p_current_ui->vwin.state = 0; //状态标志，退出弹窗
                break;
            default:break;
        }
        if (msg != msg_none && msg != msg_return) //只有弹窗内有动作，就会实时触发回调
        {
            Page * p = (Page*)bg;
            if(p->cb != NULL)p->cb(p, select_win);
        }
    }
}

//----------确认弹窗相关函数
#if UI_CONWIN_ENABLE

static void OLED_ConWinAnimInit(PageAddr bg)
{
    UNUSED_PARAMETER(bg);
    p_current_ui->cwin.y = CON_WIN_Y;
    p_current_ui->cwin.y_trg = p_current_ui->cwin.u;
    p_current_ui->cwin.bar = p_current_ui->cwin.l; 
} 

static void OLED_ConWinShow(Option* win)
{
    if(p_current_ui->upara->conwin_broken) //如果要背景虚化的话
    {
        OLED_SetPointColor(0);
        OLED_AllSrcFade(0,0x55);
        OLED_AllSrcFade(1,0xAA);
        OLED_SetPointColor(1);
    }
    //对应的动画
    OLED_Animation(&p_current_ui->cwin.y, &p_current_ui->cwin.y_trg,p_current_ui->upara->ani_param[WIN_ANI]);
    // OLED_Animation(&p_current_ui->cwin.bar, &p_current_ui->cwin.bar_trg,p_current_ui->upara->ani_param[WIN_ANI]); //ConWin的选项不用横向移动

    OLED_SetPointColor(0);//绘制外围黑框
    OLED_WinDrawRBox(&w_all, p_current_ui->cwin.l, (int16_t)p_current_ui->cwin.y, CON_WIN_W, CON_WIN_H, CON_WIN_R); //弹窗背景黑边
    OLED_SetPointColor(1);
    //绘制外框
    OLED_WinDrawRBoxEmpty(&w_all, p_current_ui->cwin.l, (int16_t)p_current_ui->cwin.y, CON_WIN_W,CON_WIN_H,CON_WIN_R);
    OLED_WinDrawStr(&w_all, (OLED_WIDTH - OLED_GetStrWidth((const char*)&(win->text[2]),CON_WIN_FONT_H) )>>1, (int16_t)p_current_ui->cwin.y,
                    CON_WIN_FONT_H,(uint8_t*)&(win->text[2]));// 《/2》
    OLED_WinDrawStr(&w_all, p_current_ui->cwin.l+15, (int16_t)p_current_ui->cwin.y+20,CON_WIN_FONT_H,(uint8_t*)"Yes");
    OLED_WinDrawStr(&w_all, OLED_WIDTH-p_current_ui->cwin.l-30, (int16_t)p_current_ui->cwin.y+20,CON_WIN_FONT_H,(uint8_t*)"No");
    OLED_WinDrawRBoxEmpty(&w_all, p_current_ui->cwin.l, (int16_t)p_current_ui->cwin.y+18, CON_WIN_W/2,CON_WIN_BTN_H,CON_WIN_R); //绘制选项框《/2》
    OLED_WinDrawRBoxEmpty(&w_all, p_current_ui->cwin.l+CON_WIN_W/2, (int16_t)p_current_ui->cwin.y+18, CON_WIN_W/2,CON_WIN_BTN_H,CON_WIN_R); //《/2》
    OLED_SetPointColor(2); //反色绘制
    if(win->val == True) //选中的话
        OLED_WinDrawRBox(&w_all, p_current_ui->cwin.bar, (int16_t)p_current_ui->cwin.y+18, CON_WIN_W/2, CON_WIN_BTN_H, CON_WIN_R);//绘制选中背景 《/2》
    else 
        OLED_WinDrawRBox(&w_all, p_current_ui->cwin.bar+CON_WIN_W/2, (int16_t)p_current_ui->cwin.y+18, CON_WIN_W/2, CON_WIN_BTN_H, CON_WIN_R); //《/2》
    OLED_SetPointColor(1);//改回实色绘制
} 

static void OLED_ConWinReact(PageAddr bg,Option* win)
{
    if(p_current_ui->cwin.y == p_current_ui->cwin.y_trg && p_current_ui->cwin.y != CON_WIN_Y_TRG) //弹窗进场动画结束
    {
        InputMsg msg = OLED_MsgQueRead();
        Page* p = (Page*)bg;
        switch (msg)
        {
            case msg_up:
            case msg_down: //上下键在确认弹窗时用于切换
                win->val = !(win->val);
                p_current_ui->cwin.bar_trg = p_current_ui->cwin.l; //触发选项移动动画
              break;
            case msg_click: //对于确认弹窗来说，只有按下确认键，才会触发回调
                if((p->cb) != NULL)p->cb(p, win);
                //这儿不用break;因为确认结束，直接退出弹窗
            case msg_return:
                p_current_ui->cwin.y_trg = CON_WIN_Y_TRG; //弹窗退场
                p_current_ui->cwin.state = 0; //标志退出弹窗状态
                break;
            default:
              break;
        }
    }
}

#endif

//-------List页面相关函数

//绘制列表项文字和对应的行末数字或选框
static void OLED_ListDrawText_CheckBox(float start_y ,Option* item)
{ 
  //绘制文字
  OLED_WinDrawStr(&w_all,LIST_TEXT_S,start_y + LIST_TEXT_S,LIST_TEXT_H,(uint8_t *)(item->text));
  //绘制表尾
  switch (item->text[0])
  {
      case '~': //数值弹窗的话 // 画行结尾的数值
        OLED_WinDrawStr(&w_all,CHECK_BOX_L_S, start_y + LIST_TEXT_S, LIST_TEXT_H,(uint8_t*)(itoa(item->val)));
        break;
      case '+': //二值选择框的话 //画结尾外框 
      case '=': //如果是单选框页面内的单选选项的话
      case '$': //如果是确认弹窗的话
        OLED_WinDrawRBoxEmpty(&w_all, CHECK_BOX_L_S, start_y + CHECK_BOX_U_S, CHECK_BOX_F_W, CHECK_BOX_F_H, 2);
        if(item->val != 0)//非0即是选中，画框内的点
          OLED_WinDrawRBox(&w_all,CHECK_BOX_L_S+CHECK_BOX_D_S+1, start_y+CHECK_BOX_U_S+CHECK_BOX_D_S+1, 
              CHECK_BOX_F_W - (CHECK_BOX_D_S +1) * 2 +1,  CHECK_BOX_F_H - (CHECK_BOX_D_S + 1) * 2 , 1);
        break;
  default:
    break;
  }
}

static void OLED_ListPageAnimInit(PageAddr page_addr)
{
    p_current_ui->lp_var.y = 0;
    p_current_ui->lp_var.y_trg = LIST_LINE_H;
    p_current_ui->lp_var.box_x = 0;
    p_current_ui->lp_var.box_y = 0;
    p_current_ui->lp_var.bar_y = 0;
}
static void OLED_ListPageShow(PageAddr page_addr)
{
  ListPage * lp = (ListPage *)page_addr;
  static float temp = 0; //用于临时存放列表每一项的y坐标 
  //计算选中框目标值和进度条y值目标
  p_current_ui->lp_var.box_x_trg = OLED_GetStrWidth(lp->option_array[lp->select_item].text,LIST_TEXT_H) + LIST_TEXT_S * 2;
  p_current_ui->lp_var.bar_y_trg = (int16_t)(lp->select_item * OLED_HEIGHT/(lp->item_num-1))+1;

  //计算动画过渡值
  OLED_Animation(&(p_current_ui->lp_var.y), &(p_current_ui->lp_var.y_trg), p_current_ui->upara->ani_param[LIST_ANI]);
  OLED_Animation(&(p_current_ui->lp_var.box_x), &(p_current_ui->lp_var.box_x_trg), p_current_ui->upara->ani_param[LIST_ANI]);
  OLED_Animation(&(p_current_ui->lp_var.box_y), &(lp->box_y_trg), p_current_ui->upara->ani_param[LIST_ANI]);
  OLED_Animation(&(p_current_ui->lp_var.bar_y), &(p_current_ui->lp_var.bar_y_trg), p_current_ui->upara->ani_param[LIST_ANI]);

  //绘制进度条
  OLED_WinDrawHLine(&w_all,OLED_WIDTH - LIST_BAR_W,0,LIST_BAR_W);
  OLED_WinDrawHLine(&w_all,OLED_WIDTH - LIST_BAR_W,OLED_HEIGHT-1,LIST_BAR_W);
  OLED_WinDrawVLine(&w_all,OLED_WIDTH - ((LIST_BAR_W/2)+1),0,OLED_HEIGHT);
  OLED_WinDrawRBox(&w_all,OLED_WIDTH - LIST_BAR_W,0,LIST_BAR_W,p_current_ui->lp_var.bar_y,0);

  if(p_current_ui->lp_var.slip_flag && p_current_ui->lp_var.box_y == lp->box_y_trg) p_current_ui->lp_var.slip_flag = False;
  //该标志位是为了防止 切换动画导致的box_y移动对过度动画造成影响
  if(!p_current_ui->init_finish_flag) //过度动画
  {
    for (uint8_t i = 0; i < (lp->item_num); i++)
    {
      if(p_current_ui->upara->ufd_param[LIST_UFD]) //从头展开
          temp = i*(p_current_ui->lp_var.y) - LIST_LINE_H*(lp->select_item) + (lp->box_y_trg);
      else //选中项展开
          temp = (i - (lp->select_item))*(p_current_ui->lp_var.y) + lp->box_y_trg;
      OLED_ListDrawText_CheckBox(temp, &(lp->option_array[i]));
    }
    if(p_current_ui->lp_var.y == p_current_ui->lp_var.y_trg) //动画抵达目标位置
    {
        p_current_ui->init_finish_flag = True;//过度动画完成
        p_current_ui->lp_var.y = p_current_ui->lp_var.y_trg = -LIST_LINE_H * (lp->select_item) + (lp->box_y_trg);
        //第一个选项到光标(盒子)所在的距离确定下来
    }
  }
  else //过度后，一般上下切换选项时的动画，但目标参数不在这儿设置
    for (uint8_t i = 0; i < (lp->item_num); i++)
    {
      temp = LIST_LINE_H *i + (p_current_ui->lp_var.y); 
      OLED_ListDrawText_CheckBox(temp, &(lp->option_array[i]));
    }
  //绘制文字选中框，框需要在文字前绘制，否则文字无法显色
  OLED_SetPointColor(2);//反色绘制
  OLED_WinDrawRBox(&w_all, 0, p_current_ui->lp_var.box_y, p_current_ui->lp_var.box_x, LIST_LINE_H, LIST_BOX_R);
  OLED_SetPointColor(1);//实色绘制
}


static void OLED_ListPageReact(PageAddr page_addr)
{
    ListPage * lp = (ListPage *)page_addr;
    String selcet_string = NULL;
    int16_t select_val = 0; 
  //int16_t contorl_flip = 0; //控制取反信号(这儿不能使用取反信号进行异或，对数组参数的赋值无法实现？？)
    InputMsg msg = OLED_MsgQueRead(); 
    switch (msg)
    { 
        case msg_up: //上一个
          if(p_current_ui->init_finish_flag && !(p_current_ui->lp_var.slip_flag)) //初始化动画已完成
          {
#if PAGE_WAVE_ENABLE
                if(OLED_CheckPageType(lp) == type_wave) p_current_ui->wt_var.change_flag = True;//如果是波形页面的话，标志波形切换
#endif
               if(lp->select_item == 0) //选中第一个的话
               {
                  if(p_current_ui->upara->loop_param[LIST_LOOP]) //同时loop参数开的话
                  {
                      p_current_ui->lp_var.slip_flag = True;
                      lp->select_item = lp->item_num - 1; //选中最后一个
                      if(lp->item_num > lp->line_n) //数目超出一页的最大数目
                      {//更改box到最底
                          lp->box_y_trg = OLED_HEIGHT - LIST_LINE_H;
                        //同时更改文字到最底
                          p_current_ui->lp_var.y_trg = OLED_HEIGHT - (lp->item_num)*LIST_LINE_H;
                      }
                      else // 没有超出数目则是到最后一个
                        lp->box_y_trg = (lp->item_num - 1)*LIST_LINE_H;
                  }
               }
               else //没有选中第一个
               {
                    lp->select_item -= 1; //选中减1
                    if(lp->select_item < -((p_current_ui->lp_var.y_trg) / LIST_LINE_H))//光标盒子到页面顶了????
                    {
                       if(!(OLED_HEIGHT % LIST_LINE_H)) //上面剩余完整的行
                        p_current_ui->lp_var.y_trg += LIST_LINE_H; //文字下移
                      else //上面的行不完整(LIST_LINE_H该项用于页面和行高非整除时)
                      {
                          if(lp->box_y_trg == OLED_HEIGHT-LIST_LINE_H*lp->line_n)
                          {//文字往下走一行，且光标盒子置于0处，把上面的非整行去掉
                              p_current_ui->lp_var.y_trg += (lp->line_n+1)*LIST_LINE_H - OLED_HEIGHT;
                              lp->box_y_trg = 0;
                          }
                          else if(lp->box_y_trg == LIST_LINE_H)
                              lp->box_y_trg = 0; //上面整行直接移动光标盒子
                          else p_current_ui->lp_var.y_trg += LIST_LINE_H; //顶页整行，文字直接往下走
                      }
                    }
                    else //光标盒子没到页面顶
                        lp->box_y_trg -= LIST_LINE_H; //直接光标盒子往上走即可。
               }
          }
          break;
        case msg_down: //下一个
            if(p_current_ui->init_finish_flag && !(p_current_ui->lp_var.slip_flag)) //初始化动作已完成
            {
#if PAGE_WAVE_ENABLE
                if(OLED_CheckPageType(lp) == type_wave) p_current_ui->wt_var.change_flag = True;//如果是波形页面的话，标志波形切换
#endif
                if(lp->select_item == (lp->item_num) -1) //到最后一个选项了
                {
                    if(p_current_ui->upara->loop_param[LIST_LOOP]) //loop开关开
                    {//全部回到顶部
                        p_current_ui->lp_var.slip_flag = True; 
                        lp->select_item = 0;
                        p_current_ui->lp_var.y_trg = 0;
                        lp->box_y_trg = 0;
                    }
                }
                else //不是最后一个选项
                {
                    lp->select_item += 1;
                    if((lp->select_item+1) > ((lp->line_n) - (p_current_ui->lp_var.y_trg)/LIST_LINE_H))
                    {//光标到页面底
                        if(!(OLED_HEIGHT % LIST_LINE_H)) p_current_ui->lp_var.y_trg -= LIST_LINE_H;
                        else //非整行的情况
                        {
                            if(lp->box_y_trg == LIST_LINE_H*(lp->line_n-1))
                            {
                                p_current_ui->lp_var.y_trg -= (lp->line_n+1)*LIST_LINE_H - OLED_HEIGHT; 
                                //取出要向下移动的距离，对y_trg来说就是向上移动的距离
                                lp->box_y_trg = OLED_HEIGHT - LIST_LINE_H;
                            }
                            else if(lp->box_y_trg == OLED_HEIGHT - LIST_LINE_H*2) 
                            //这种情况什么时候会出现呢嗯?
                                lp->box_y_trg = OLED_HEIGHT - LIST_LINE_H;
                            else
                              p_current_ui->lp_var.y_trg -= LIST_LINE_H;
                        }
                    }
                    else lp->box_y_trg += LIST_LINE_H;
                }
            }
          break;

        case msg_return: OLED_PageReturn((PageAddr)lp); break;

        case msg_click :
            selcet_string = lp->option_array[lp->select_item].text;
            select_val = lp->option_array[lp->select_item].val;
            //contorl_flip = (!!(lp->option_array[lp->select_item].step)); //使用step作为是否取反的信号
            if(selcet_string[0] == '+' || selcet_string[0] == '=') ////二值选框或者单选框
            {
                if(lp->option_array[lp->select_item].step)lp->option_array[lp->select_item].val = !select_val; //将对应值取反
                //lp->option_array[lp->select_item].val ^= contorl_flip; 
                //将对应值取反(用step的是否非0控制val是否取反)
#if PAGE_RADIO_ENABLE
                if(selcet_string[0] == '=')p_current_ui->rp_var.radio_click_flag = True; //标记为1，用于OLED_RadioReact进行轮询判断单选
#endif
            }

//为没有使能确认弹窗的宏定义时，打个补丁，$标识的确认弹窗和一般的二值选择框一样，将值取反，并调用回调
#if !UI_CONWIN_ENABLE
            if(selcet_string[0] == '$') 
                if(lp->option_array[lp->select_item].step)lp->option_array[lp->select_item].val = !select_val; //将对应值取反
                //lp->option_array[lp->select_item].val ^=  contorl_flip; 
                //将对应值取反(用step的是否非0控制val是否取反)
#endif
            //回调函数和状态机部分
            if(selcet_string[0] == '~')
            {//数值弹窗的话，进入数值弹窗这个动作不触发回调，但弹窗内不论点击什么都会实时触发回调
                //当数值弹窗step设为0为只读是，进入数值弹窗前会调用一次回调，以便外界更改val值
                p_current_ui->vwin.state = 1; //进入弹窗初始化
                if((lp->option_array[lp->select_item].step == 0) && (lp->page.cb != NULL))
                    lp->page.cb(&(lp->page), &(lp->option_array[lp->select_item]));
            }
#if UI_CONWIN_ENABLE
            else if(selcet_string[0] == '$') p_current_ui->cwin.state = 1; //进入确认弹窗初始化
                //确认弹窗内只有确认弹窗内的click才会触发回调
#endif
            else if(lp->page.cb != NULL)lp->page.cb(&(lp->page), &(lp->option_array[lp->select_item]));
          break;
    default:break;
    }
}

void OLED_ListPageInit(
      ListPage * lp,          //列表页面对象
      uint8_t item_num,       //选项个数，需与title数组大小，icon数组大小一致
      Option *option_array,   //整个页面的选项数组(数组大小需与item_num一致)
      CallBackFunc call_back) //回调函数，参数为确认选中项index（1-256）0表示未确认哪个选项
{
    //缺少选项个数检查，检查，
    lp->page.page_type = type_list;
    OLED_PageInit((PageAddr)lp, call_back);
    lp->page.ani_init = OLED_ListPageAnimInit;
    lp->page.show = OLED_ListPageShow;
    lp->page.react = OLED_ListPageReact;//关联相关处理函数
    lp->select_item = 0;
    lp->item_num = item_num;
    lp->option_array = option_array;
    lp->line_n = OLED_HEIGHT/LIST_LINE_H;
    for (uint8_t i = 0; i < lp->item_num; i++)
        lp->option_array[i].order = i;//选项序号标号
}


// // ------终端波形页面
#if PAGE_WAVE_ENABLE

void OLED_WavePageAnimInit(PageAddr page_addr)
{
    WavePage * wp = (WavePage *)page_addr;
    OLED_ListPageAnimInit(wp);
    p_current_ui->wt_var.p_rear = 0;
    p_current_ui->wt_var.p_head = 0; //重新进入页面时，会重新开始显示波形
    p_current_ui->wt_var.text_bg_r = 0;
    p_current_ui->wt_var.text_bg_r_trg = WAVE_TEXT_BG_W;
}

void OLED_WavePageShow(PageAddr page_addr)
{
    WavePage * wp = (WavePage *)page_addr;
    int16_t temp; //临时变量用于存放波形的点，暂时显示
    String temp_str = NULL;
    OLED_ListPageShow(wp);
    OLED_WinDrawRBoxEmpty(&w_all, WAVE_BOX_L_S, 0 ,WAVE_BOX_W , WAVE_BOX_H,0);
    OLED_WinDrawRBoxEmpty(&w_all, WAVE_BOX_L_S+1, 1 ,WAVE_BOX_W -2, WAVE_BOX_H-2,0);

    //数据入队列
    if(wp->option_array[wp->select_item].step == 1) //有数据更新，才将数据入队
    {
        p_current_ui->wt_var.p_rear++; p_current_ui->wt_var.p_rear %= (WAVE_W-1);
        p_current_ui->wt_var.data_que[p_current_ui->wt_var.p_rear] = wp->option_array[wp->select_item].val; //放入数据
        wp->option_array[wp->select_item].step = 0; //将数据更新的标志位置0
    }
    if((p_current_ui->wt_var.p_rear+1)%(WAVE_W-1) == p_current_ui->wt_var.p_head) //队列满了
    {  
        p_current_ui->wt_var.p_head++; p_current_ui->wt_var.p_head %= (WAVE_W-1); //环状前进
    }   
    p_current_ui->wt_var.p_font = p_current_ui->wt_var.p_head; //获取起始地址
    if(p_current_ui->lp_var.box_y == wp->box_y_trg && p_current_ui->lp_var.y == p_current_ui->lp_var.y_trg)//完成过度动画
    {
        for (uint8_t i = 1; i < WAVE_W -1; i++)
        {
            if(p_current_ui->wt_var.p_rear != p_current_ui->wt_var.p_font) //队列非空
            {
                temp = p_current_ui->wt_var.data_que[p_current_ui->wt_var.p_font];
                p_current_ui->wt_var.p_font++;p_current_ui->wt_var.p_font %= (WAVE_W -1); //出队指针前移
                //画点或者差值画斜线函数
                OLED_WinDrawPoint(&w_all,WAVE_L + i , 
                        WAVE_BOX_H-2 - (temp - wp->option_array[wp->select_item].item_min)*(WAVE_BOX_H-4)/(wp->option_array[wp->select_item].item_max - wp->option_array[wp->select_item].item_min));
                //因为0会占掉一个像素点，所以波形没有完全对称。
            }
            else break;
        }
        //绘制文字
        temp_str = itoa(wp->option_array[wp->select_item].val);
        OLED_WinDrawStr(&w_all, WAVE_TEXT_BG_L_S + (WAVE_TEXT_BG_W - OLED_GetStrWidth(temp_str,WAVE_FONT_H))/2,
                             OLED_HEIGHT-28, WAVE_FONT_H, (uint8_t*)temp_str);
        //绘制选框
        OLED_SetPointColor(2); //反色选框
        //注意这个动态宽度在过度动画完成后才执行，且从0到宽度，不是从中间开始的
        OLED_Animation(&p_current_ui->wt_var.text_bg_r, &p_current_ui->wt_var.text_bg_r_trg, p_current_ui->upara->ani_param[TAG_ANI]);
        OLED_WinDrawRBox(&w_all, WAVE_TEXT_BG_L_S, OLED_HEIGHT - WAVE_TEXT_BG_H, p_current_ui->wt_var.text_bg_r, WAVE_TEXT_BG_H, 0);
        OLED_SetPointColor(1); //恢复实色
    }
}

void OLED_WavePageReact(PageAddr page_addr)
{
    WavePage * wp = (WavePage *)page_addr;
    OLED_ListPageReact(wp);
    if(p_current_ui->wt_var.change_flag)
    {
        p_current_ui->wt_var.p_rear = 0;
        p_current_ui->wt_var.p_head = 0; //切换选项时，重新开始显示波形
        p_current_ui->wt_var.text_bg_r = 0;
        p_current_ui->wt_var.text_bg_r_trg = WAVE_TEXT_BG_W; //切换选项时唤醒tag动画
        p_current_ui->wt_var.change_flag = False;
    }
}

void OLED_WavePageInit(
      WavePage * wp,            //波形页面对象
      uint8_t item_num,       //选项个数，需与option_array数组大小，icon数组大小一致
      Option *option_array,   //整个页面的选项数组(数组大小需与item_num一致)
      CallBackFunc call_back) //回调函数，参数为 self_id 和 选中项option指针
{
    OLED_ListPageInit(wp,item_num,option_array,call_back); //先初始化为list
    wp->page.page_type = type_wave; //指定为波形页面
    wp->page.ani_init = OLED_WavePageAnimInit;
    wp->page.show = OLED_WavePageShow;
    wp->page.react = OLED_WavePageReact;
}

//更新波形选项的显示值(后期会加上断言)，以防止输入NULL
void OLED_UIWaveUpdateVal(Option * op, int16_t val)
{
    //这里可以添加断言
    op->step = 1; //wave中的step用于指示是否更新这个波形的数值
    op->val = val;
}

#endif

// // -------单选框页面
#if PAGE_RADIO_ENABLE
//单选框的AnimInit 和show页面与list页面一致
void OLED_RadioPageReact(PageAddr page_addr)
{
    RadioPage * rp = (RadioPage *)page_addr;
    OLED_ListPageReact(rp);
    if(p_current_ui->rp_var.radio_click_flag)
    {
        for(uint8_t i = 0; i < rp->item_num; i++)
        {
            if(rp->option_array[i].text[0] == '=') //确保这个选项是单选项
            {
                if(i != rp->select_item)rp->option_array[i].val = False;
                else rp->option_array[i].val = True; //确保单选
            }
        }
        p_current_ui->rp_var.radio_click_flag = False;
    }
}

void OLED_RadioPageInit(      
        RadioPage * rp,          //Radio单选页面对象
        uint8_t item_num,       //选项个数，option_array 大小一致
        Option *option_array,   //整个页面的选项数组(数组大小需与item_num一致)，需要注意单选项 的 text 需要以“= ”开头，一般标识符则不需要
        CallBackFunc call_back) //回调函数，参数为 self_id 和 选中项option指针
{
    OLED_ListPageInit(rp, item_num, option_array, call_back);
    rp->page.page_type = type_radio; //标记为单选终端页面
    rp->page.react = OLED_RadioPageReact; //只有单选的react函数不一样,其他处理函数和list的一样
}

#endif

//-----镭射文字界面相关函数
#if PAGE_RADERPIC_ENABLE

void OLED_RaderPicAnimInit(PageAddr page_addr)
{
    RaderPicPage* rpp = (RaderPicPage*)page_addr;
    if(rpp->cur_pic_index == rpp->pic_num) //所有的图片已经绘制完成,且从pic页面退出重新进入(因为在layIn中调用该函数)
        rpp->cur_pic_index = 0; //重新开始绘制图片页面的绘制
    p_current_ui->rrp_var.move_x = -1.0;
    p_current_ui->rrp_var.move_y = -1.0;
}

void OLED_RaderPicShow(PageAddr page_addr)
{
    RaderPicPage* rpp = (RaderPicPage*)page_addr;
    RaderPic *cur_pic = NULL;
    Option temp_op;
    float target = 0;
    OLED_Animation(&(p_current_ui->rrp_var.move_x), &target, p_current_ui->upara->raderpic_move_rate);
    OLED_Animation(&(p_current_ui->rrp_var.move_y), &target, p_current_ui->upara->raderpic_move_rate);
    if(rpp->cur_pic_index < rpp->pic_num) //当前序号在数组大小内的话
    {
        uint8_t pic_finish_flag = 0;
        cur_pic = &(rpp->pic_array[rpp->cur_pic_index]);
        if(rpp->cur_pic_index > 0) //将已经绘制完成的pic快速绘制完
            for(uint8_t i = 0; i < rpp->cur_pic_index ; i++)
            {
                RaderPic * temp_pic = &(rpp->pic_array[i]);
                OLED_WinDrawBMP(&w_all, temp_pic->start_x*(1+p_current_ui->rrp_var.move_x), temp_pic->start_y*(1+p_current_ui->rrp_var.move_y), temp_pic->w, temp_pic->h, temp_pic->pic,1);//实色显示
            }
        pic_finish_flag = OLED_WinDrawRaderPic(&w_all,cur_pic->pic, cur_pic->start_x*(1+p_current_ui->rrp_var.move_x), cur_pic->start_y*(1+p_current_ui->rrp_var.move_y),
                        cur_pic->w,cur_pic->h, &(rpp->pic_end_point), cur_pic->rd,p_current_ui->upara->raderpic_scan_mode,p_current_ui->upara->raderpic_scan_rate);
        if(pic_finish_flag) //一张图片绘制完成切换下一张
        {
            rpp->cur_pic_index++; //默认自增(1 ~ pic_num)
            if(rpp->page.cb != NULL)
            {
                temp_op.order = rpp->cur_pic_index;
                rpp->page.cb(&(rpp->page),&temp_op); //传入回调函数第i(1 ~ pic_num)张图像已经绘制完成
            }
        }
    }
    else //全部图片绘制完成
    {
        switch (rpp->rader_pic_mode)
        {
            case Rader_Pic_Mode_Loop: rpp->cur_pic_index = 0; break;
            case Rader_Pic_Mode_Hold:             
                for(uint8_t i = 0; i < rpp->pic_num ; i++)
                {
                    RaderPic * temp_pic = &(rpp->pic_array[i]);
                    OLED_WinDrawBMP(&w_all, temp_pic->start_x, temp_pic->start_y, temp_pic->w, temp_pic->h, temp_pic->pic,1);//实色显示
                }
                //hold保持模式的话，在所有图像绘制完成后会持续调用回调
                if(rpp->page.cb != NULL)
                {
                    temp_op.order = rpp->cur_pic_index;
                    rpp->page.cb(&(rpp->page),&temp_op); //传入回调函数第i(0 ~ pic_num-1)张图像已经绘制完成
                }
             break;
            default: break;
        }
    }
}
void OLED_RaderPicReact(PageAddr page_addr)
{
    RaderPicPage* rpp = (RaderPicPage*)page_addr;
    InputMsg msg = OLED_MsgQueRead();
    switch (msg)        
    {
        case msg_click :
        case msg_return : //镭射文字页面内上up下down键没有用，按下click,或return均是返回
            OLED_PageReturn((PageAddr)rpp);
            break;
        default:break;
    }
}

void OLED_RaderPicPageInit(
    RaderPicPage* rpp,          //镭射页面对象
    uint8_t pic_num,            //页面内的图片数量，pic_array的数组大小
    RaderPic * pic_array,       //镭射图片数组
    RaderPicMode mode,          //所有图片播放完后的操作模式选择(clear,hold,loop)
    CallBackFunc cb)    //回调函数，在每绘制完一个图片前会调用一次    
{
    rpp->page.page_type = type_rader_pic;
    OLED_PageInit((PageAddr)rpp, cb);
    rpp->pic_num = pic_num;
    rpp->pic_array = pic_array;
    rpp->cur_pic_index = 0;//默认第一张显示的序号为数组下标为0的图片
    rpp->rader_pic_mode = mode;
    rpp->page.ani_init = OLED_RaderPicAnimInit;
    rpp->page.show = OLED_RaderPicShow;
    rpp->page.react = OLED_RaderPicReact;
}


#endif


#if PAGE_DIGITAL_ENABLE

/**
 * @brief 在数字标签上绘制下划线(光标)
 * 
 * @param dp 数字页面指针
 * @param color 颜色值
 */
static void OLED_DigitalDrawUline(DigitalPage *dp, uint8_t color) 
{
    //标签的下划线偏移值不同
    uint8_t end_x = 0, start_y = 0, start_x = 0;
    if(dp->select_index == Digital_Pos_IndexLabel)
    {
        start_x = (OLED_WIDTH-OLED_GetStrWidth(dp->label_array[dp->select_label_index],DIGITAL_Label_SIZE))>>1; 
        end_x = start_x + OLED_GetStrWidth(dp->label_array[dp->select_label_index],DIGITAL_Label_SIZE);
        start_y = p_current_ui->dp_var.w_digtal[1].start_y + DIGITAL_Label_SIZE;
    } else if(dp->select_index < Digital_Pos_IndexLabel) //是数字的话
    {
        start_y = p_current_ui->dp_var.w_digtal[0].start_y + DIGITAL_NUM_SIZE+2;
        start_x = 16 + p_current_ui->dp_var.digital_num_xindex[(dp->select_index)*2+1]; //012->135 《*2》
        end_x = start_x + DIGITAL_NUM_SIZE;   //两个字符宽
    }
    if(dp->select_index != Digital_Pos_Complete)
    {
        if(color == 0)
        {
            OLED_SetPointColor(0); //更改颜色
            OLED_WinDrawHLine(&w_all, start_x, end_x, start_y); //如果在python中，应该是套个装饰器
            OLED_SetPointColor(1);
        }
        else 
            OLED_WinDrawHLine(&w_all, start_x, end_x, start_y);
    }
}

/*
函数：static void OLED_DigitalDirChangey(DigitalPage *dp, DigitalDirect dir)
参数：dp 数字页面指针 ， dir方向枚举类型
返回值：None
说明：根据输入的方向，设置数字运动的目标值
作者:Sheep
日期：24/02/10
*/
static void OLED_DigitalDirChangey(DigitalPage *dp, DigitalDirect dir)
{   
    p_current_ui->dp_var.num_y = 0; 
    switch (dir)
    {
        case Digital_Direct_Increase : p_current_ui->dp_var.num_y_trg = -24; break;
        case Digital_Direct_Decrease : p_current_ui->dp_var.num_y_trg = 24; break;
        default:break;
    }
}
//创建过度动画使用的string
static char* OLED_DigitalCreateText(DigitalPage * dp)
{
    static char str[10] = {0};
	memset(str, 0, sizeof(str));
	sprintf(str, "%02d%c%02d%c%02d", dp->option_array[Digital_Pos_IndexLeft].val, dp->gap_char, 
                                    dp->option_array[Digital_Pos_IndexMid].val, dp->gap_char, 
                                    dp->option_array[Digital_Pos_IndexRight].val);
    return str;
}

/**
* @brief : static void OLED_DigitalSetSingleBit(DigitalPage* dp, uint8_t index, SingleBit sbit_array[6])
            将设置的数字写入到显示在屏幕上6个单个位的数组中
* @param : dp 数字页面指针, index 下标，sbit_array 十位和个位的数组(共6个位)
* @attention : None
* @author : Sheep
* @date : 24/02/10
*/
static void OLED_DigitalSetSingleBit(DigitalPage* dp, uint8_t index, SingleBit sbit_array[6])
{
    if(index > 5)return ; //防止数组越界
    if(index%2) //个位
        sbit_array[index].num = ((dp->option_array[index>>1].val)%100)/10; //取出十位
    else 
        sbit_array[index].num = (dp->option_array[index>>1].val)%10 ; //取出个位
}

/**
* @brief : static void OLED_DigitalSetValAnim(DigitalPage * dp, DigitalPosIndex bit, uint8_t val, DigitalDirect dir)
* @param : dp 数字页面指针, bit 下标，val 更新的值，dir 方向枚举类型
* @attention : 这个函数将更新的值设置在选项中，并触发一次更新动画，让show函数中按模式要求更新显示
* @todo : 后期有一处细节可以更新，将根据输入方向，决定先更新十位还是个位，这样，可以使上下的ripple动画有区分。
* @author : Sheep
* @date : 24/02/10
*/
// 建议可以判断两个位更新时再等待运动结束
static void OLED_DigitalSetValAnim(DigitalPage * dp, DigitalPosIndex bit, uint8_t val, DigitalDirect dir)
{
    uint8_t last_val = dp->option_array[bit].val;
    if((val >= dp->option_array[bit].item_min) && (val <= dp->option_array[bit].item_max) && (last_val != val)) //更新的数据符合要求 
    {
        dp->option_array[bit].val = val;
        p_current_ui->dp_var.dir = dir;
        if((val%100)/10 != (last_val%100)/10) //十位发生变化
        {
            p_current_ui->dp_var.digital_num_pos |= (1<<(bit*2+1)); //012->135
            p_current_ui->dp_var.temp_ripple_index = (bit*2+1); //记下这个十位
        }
        if(val%10 != (last_val%10)) //个位发生变化
        {
            p_current_ui->dp_var.digital_num_pos |= (1<<(bit*2)); //012->024
            p_current_ui->dp_var.temp_ripple_index = (bit*2); //记下这个个位(个位在十位后，所以最低位会被记录)
        }
    }
}


/**
 * @brief 在数字页面上绘制标签
 * 
 * @param dp 数字页面指针
 * @param y 标签的纵坐标
 * @param label 标签的文本
 * @brief 为了减少代码量而专门为digital[1] 绘制标签设置的函数
 */
static void OLED_DigitalDrawLabel(DigitalPage * dp, int16_t y , String label)
{
    int16_t x = (OLED_WIDTH-OLED_GetStrWidth(label,DIGITAL_Label_SIZE))>>1; // 计算起始横坐标
    OLED_WinDrawStr(&(p_current_ui->dp_var.w_digtal[1]), x, y, DIGITAL_Label_SIZE, (uint8_t*)(label)); // 绘制标签
}
static void OLED_DigitalShow(PageAddr page_addr)
{
    DigitalPage* dp = (DigitalPage*)page_addr;
    if(!p_current_ui->init_finish_flag) //绘制过度动画
    {
        OLED_Animation(&(p_current_ui->dp_var.rect_y), &(p_current_ui->dp_var.rect_y_trg), p_current_ui->upara->ani_param[WIN_ANI]); //使用弹窗的速度
        OLED_Animation(&(p_current_ui->dp_var.label_y), &(p_current_ui->dp_var.label_y_trg), p_current_ui->upara->ani_param[WIN_ANI]); //使用弹窗的速度
        OLED_WinDrawRBoxEmpty(&w_all,DIGITAL_RECT_X, p_current_ui->dp_var.rect_y, DIGITAL_RECT_W, DIGITAL_RECT_H, DIGITAL_RECT_R);
        p_current_ui->dp_var.w_digtal[0].start_y = p_current_ui->dp_var.rect_y + DIGITAL_NUM_T_S;
        OLED_WinDrawStr(&(p_current_ui->dp_var.w_digtal[0]), 0, 0, DIGITAL_NUM_SIZE,(uint8_t*)OLED_DigitalCreateText(dp));
       //这儿可以将label的移动改为label窗口的移动，再将label绘制在label窗口中
        p_current_ui->dp_var.w_digtal[1].start_y = p_current_ui->dp_var.label_y; //更新label窗口的纵坐标
        OLED_DigitalDrawLabel(dp, 0, dp->label_array[dp->select_label_index]);
        if(p_current_ui->dp_var.rect_y == p_current_ui->dp_var.rect_y_trg && p_current_ui->dp_var.label_y == p_current_ui->dp_var.label_y_trg) p_current_ui->init_finish_flag = True;
    }
    else //绘制切换动画
    {
        //绘制不需要移动的框
        OLED_WinDrawRBoxEmpty(&w_all,DIGITAL_RECT_X, DIGITAL_RECT_Y, DIGITAL_RECT_W, DIGITAL_RECT_H, DIGITAL_RECT_R);
        //闪烁 
        if(p_current_ui->dp_var.Gap_shine_time <= ((dp->gap_shine_time)>>1)) //恒定为0是显示的内容
        {
            OLED_WinDrawASCII(&w_all, p_current_ui->dp_var.digital_num_xindex[4]+28, p_current_ui->dp_var.w_digtal[0].start_y, DIGITAL_NUM_SIZE, dp->gap_char);
            OLED_WinDrawASCII(&w_all, p_current_ui->dp_var.digital_num_xindex[2]+28, p_current_ui->dp_var.w_digtal[0].start_y, DIGITAL_NUM_SIZE, dp->gap_char);
        }
        else
        {
            OLED_WinDrawASCII(&w_all, p_current_ui->dp_var.digital_num_xindex[4]+28, p_current_ui->dp_var.w_digtal[0].start_y, DIGITAL_NUM_SIZE, ' ');
            OLED_WinDrawASCII(&w_all, p_current_ui->dp_var.digital_num_xindex[2]+28, p_current_ui->dp_var.w_digtal[0].start_y, DIGITAL_NUM_SIZE, ' ');
        }
        if(p_current_ui->dp_var.Gap_shine_time == dp->gap_shine_time)p_current_ui->dp_var.Gap_shine_time = 0;
        else p_current_ui->dp_var.Gap_shine_time++;
        //根据模式横杠闪烁提示
        switch(dp->mod)
        {
            case Digital_Mode_Edit:
                if(p_current_ui->dp_var.Uline_shine_time <= ((dp->uline_shine_time)>>1)) //恒定为0是显示的内容
                    OLED_DigitalDrawUline(dp,0);
                else 
                    OLED_DigitalDrawUline(dp,1);
                if(p_current_ui->dp_var.Uline_shine_time == dp->uline_shine_time)p_current_ui->dp_var.Uline_shine_time = 0;
                else p_current_ui->dp_var.Uline_shine_time++;
            break;
            case Digital_Mode_Observe: //未进入编辑模式时，没有横杠
                OLED_DigitalDrawUline(dp,0);
            break;
            case Digital_Mode_Singlebit: //编辑单个位时，常亮
                OLED_DigitalDrawUline(dp,1);
            break;
            default:break;
        }
        //数字上下移动
        if(p_current_ui->dp_var.dir != Digital_Direct_None) 
        {
            if((p_current_ui->dp_var.digital_num_pos & 0x3F) && !(p_current_ui->dp_var.digital_num_pos & 0x40)) 
            {
                p_current_ui->dp_var.temp_dir = p_current_ui->dp_var.dir; //记录数字传入的方向，
                //ripple模式和一起滚动模式的区别只对数字移动有效(对标签滚动无效)
                //排除标签是为了防止在数字运动是标签运动重复进入
                if(p_current_ui->upara->digital_ripple) //第一次接收到运动消息时，
                {
                    OLED_DigitalDirChangey(dp,p_current_ui->dp_var.temp_dir);
                    p_current_ui->dp_var.temp_num_pos = (p_current_ui->dp_var.digital_num_pos & 0x3F) & (1<<p_current_ui->dp_var.temp_ripple_index);
                    OLED_DigitalSetSingleBit(dp , p_current_ui->dp_var.temp_ripple_index, p_current_ui->dp_var.num_array); //更新第一个需要运动的位的数据
                    (p_current_ui->dp_var.temp_ripple_index)++;
                }
                else //一起滚动的话
                {
                    p_current_ui->dp_var.temp_num_pos = (p_current_ui->dp_var.digital_num_pos & 0x3F); //设置需要滚动的位
                    for(uint8_t i = 0; i < 6; i++)OLED_DigitalSetSingleBit(dp ,i , p_current_ui->dp_var.num_array);
                    //更新选中位的值
                    OLED_DigitalDirChangey(dp,p_current_ui->dp_var.temp_dir);
                }
            }
            else if(p_current_ui->dp_var.digital_num_pos & 0x40) //如果是标签
            {
                p_current_ui->dp_var.temp_label_dir = p_current_ui->dp_var.dir;
            }
            p_current_ui->dp_var.dir = Digital_Direct_None; //保证只有接收到运动消息的第一次进来
        }
        if(p_current_ui->dp_var.temp_dir != Digital_Direct_None && (p_current_ui->dp_var.digital_num_pos & 0x3F)) //持续运动//移动的是数字
                OLED_Animation(&(p_current_ui->dp_var.num_y),&(p_current_ui->dp_var.num_y_trg),p_current_ui->upara->ani_param[DIGI_ANI]);  //从0-24
        if(p_current_ui->dp_var.temp_label_dir != Digital_Direct_None && (p_current_ui->dp_var.digital_num_pos & 0x40)) //移动的是标签
                OLED_Animation(&(p_current_ui->dp_var.label_y),&(p_current_ui->dp_var.label_y_trg),p_current_ui->upara->ani_param[DIGI_ANI]);
        for(uint8_t i = 0; i < DIGITAL_NUM_INDEX_MAX; i++) //绘制每个数字win的位置
        {
            if((p_current_ui->dp_var.temp_num_pos & (1<<i)) && p_current_ui->dp_var.temp_dir == Digital_Direct_Decrease) //选中了数字且收到下的消息
            {
                OLED_WinDrawASCII(&(p_current_ui->dp_var.w_digtal[0]), p_current_ui->dp_var.digital_num_xindex[i],p_current_ui->dp_var.num_y - DIGITAL_NUM_SIZE,DIGITAL_NUM_SIZE,((p_current_ui->dp_var.num_array[i].num)%10)+'0'); //上一个数字
                OLED_WinDrawASCII(&(p_current_ui->dp_var.w_digtal[0]), p_current_ui->dp_var.digital_num_xindex[i],p_current_ui->dp_var.num_y,DIGITAL_NUM_SIZE,((p_current_ui->dp_var.num_array[i].num+1)%10)+'0'); //这个数字
            }
            else if((p_current_ui->dp_var.temp_num_pos & (1<<i)) && p_current_ui->dp_var.temp_dir == Digital_Direct_Increase) //选中数字且收到上的消息
            {
                OLED_WinDrawASCII(&(p_current_ui->dp_var.w_digtal[0]), p_current_ui->dp_var.digital_num_xindex[i],p_current_ui->dp_var.num_y + DIGITAL_NUM_SIZE,DIGITAL_NUM_SIZE,((p_current_ui->dp_var.num_array[i].num)%10)+'0'); //下一个数字
                OLED_WinDrawASCII(&(p_current_ui->dp_var.w_digtal[0]), p_current_ui->dp_var.digital_num_xindex[i],p_current_ui->dp_var.num_y,DIGITAL_NUM_SIZE,((p_current_ui->dp_var.num_array[i].num-1)%10)+'0'); //这个
            }
            else //直接绘制这个数字就可以
            {    OLED_WinDrawASCII(&(p_current_ui->dp_var.w_digtal[0]), p_current_ui->dp_var.digital_num_xindex[i],0,DIGITAL_NUM_SIZE,(p_current_ui->dp_var.num_array[i].num%10)+'0'); //因为只有单位数字
            }
        }
        if(p_current_ui->dp_var.num_y == p_current_ui->dp_var.num_y_trg && (p_current_ui->dp_var.digital_num_pos&0x3F)) //单次字符运动结束时
        {
            if(p_current_ui->upara->digital_ripple)
            {
                do{
                    p_current_ui->dp_var.temp_num_pos = (p_current_ui->dp_var.digital_num_pos &0x3F) & (1<<p_current_ui->dp_var.temp_ripple_index);
                    if(p_current_ui->dp_var.temp_num_pos)
                    {
                        OLED_DigitalDirChangey(dp,p_current_ui->dp_var.temp_dir); //设置运动坐标 
                        OLED_DigitalSetSingleBit(dp, p_current_ui->dp_var.temp_ripple_index ,p_current_ui->dp_var.num_array);//更新下一位数据
                        (p_current_ui->dp_var.temp_ripple_index)++; //准备检测下一位
                        break;
                    }
                    p_current_ui->dp_var.temp_ripple_index++; //准备检测下一位
                    if(p_current_ui->dp_var.temp_ripple_index >= 6)//所有位到扫描完，才完全退出
                    {
                        p_current_ui->dp_var.temp_dir = Digital_Direct_None;p_current_ui->dp_var.digital_num_pos &= 0xC0;
                        p_current_ui->dp_var.temp_num_pos = 0; //全部数字运动完清零
                        break;//所有位检测完了break;
                    }
                }while(1); //不需要运动继续往下检测
            }
            else
            {p_current_ui->dp_var.temp_dir = Digital_Direct_None; p_current_ui->dp_var.digital_num_pos &= 0xC0;}
        }
        //标签上下移动
        //绘制移动标签
        if((p_current_ui->dp_var.digital_num_pos&0x40) && p_current_ui->dp_var.temp_label_dir == Digital_Direct_Increase) 
        {
            OLED_DigitalDrawLabel(dp, p_current_ui->dp_var.label_y + DIGITAL_Label_SIZE, dp->label_array[dp->select_label_index]);
            OLED_DigitalDrawLabel(dp, p_current_ui->dp_var.label_y, dp->label_array[p_current_ui->dp_var.last_or_next_label_index]);
        }
        else if (((p_current_ui->dp_var.digital_num_pos)&0x40) && p_current_ui->dp_var.temp_label_dir == Digital_Direct_Decrease)
        {
            OLED_DigitalDrawLabel(dp, p_current_ui->dp_var.label_y - DIGITAL_Label_SIZE, dp->label_array[dp->select_label_index]);
            OLED_DigitalDrawLabel(dp, p_current_ui->dp_var.label_y, dp->label_array[p_current_ui->dp_var.last_or_next_label_index]);
        }
        else 
            OLED_DigitalDrawLabel(dp, 0, dp->label_array[dp->select_label_index]);
        //运动完成
        if(p_current_ui->dp_var.label_y == p_current_ui->dp_var.label_y_trg  && ((p_current_ui->dp_var.digital_num_pos)&0x40))
        {  p_current_ui->dp_var.temp_label_dir = Digital_Direct_None;  p_current_ui->dp_var.digital_num_pos &= 0x3F;}
    }
}

static void OLED_DigitalAnimInit(PageAddr page_addr)
{
    DigitalPage* dp = (DigitalPage*)page_addr;
    dp->mod = Digital_Mode_Observe; //默认观察模式
    dp->select_index = Digital_Pos_Complete; //默认光标位置为完成状态(即没有选中任何位)
    for(uint8_t i = 0; i < 6; i++)
        OLED_DigitalSetSingleBit(dp, i,p_current_ui->dp_var.num_array); //设置显示数字
    p_current_ui->dp_var.rect_y = - DIGITAL_RECT_Y - DIGITAL_RECT_H;
    p_current_ui->dp_var.rect_y_trg = DIGITAL_RECT_Y ;
    p_current_ui->dp_var.label_y = OLED_HEIGHT;
    p_current_ui->dp_var.label_y_trg = OLED_HEIGHT- DIGITAL_Label_SIZE-4;
    p_current_ui->dp_var.dir = Digital_Direct_None; //默认进入页面没有动作
    p_current_ui->dp_var.digital_num_pos = 0; //默认没有选中数字
    p_current_ui->dp_var.temp_ripple_index = 0; //默认没有开始滚动
}


/**
 * @brief 设置数字页面选项和回调函数
 * 
 * @param dp 数字页面指针
 * @param op 选项指针
 */
static void OLED_DigitalSetOptionAndCallback(DigitalPage *dp, Option *op)
{
    // 如果选择索引小于标签索引，则设置选项为选项数组中的值
    if(dp->select_index < Digital_Pos_IndexLabel)
        *op = dp->option_array[dp->select_index];
    else 
    {
        // 设置选项的顺序和文本为标签数组中的值
        op->order = dp->select_index;
        op->text = dp->label_array[dp->select_label_index];
    }
    // 如果页面回调函数不为空，则调用回调函数并传入页面ID和选项指针
    if (dp->page.cb != NULL)
        dp->page.cb(&(dp->page), op);
}



//Digital页面仅在每一位(数字/标签)发生变化并单击click或者返回到observe模式时，才调用回调，其他情况下不会调用回调
static void OLED_DigitalReact(PageAddr page_addr)
{
    DigitalPage* dp = (DigitalPage*)page_addr;
    InputMsg msg = OLED_MsgQueRead(); 
    Option output_option = {0};
    switch (dp->mod)
    {
        case Digital_Mode_Observe:
            if(msg == msg_up || msg == msg_click) 
            {
                dp->mod = Digital_Mode_Edit; dp->select_index = Digital_Pos_IndexRight;
            }
            else if(msg == msg_down)
            {
                dp->mod = Digital_Mode_Edit; dp->select_index = Digital_Pos_IndexLabel;
            }
            else if(msg == msg_return)
            {
                p_current_ui->dp_var.temp_dir = Digital_Direct_None;
                p_current_ui->dp_var.temp_num_pos = 0; //退出时将临时变量全部清零，防止动画运行一半退出时，在其他DigitalPage页面造成影响。
                OLED_PageReturn((PageAddr)dp);
            }
        break;

        case Digital_Mode_Edit:
            if(msg == msg_return) 
            {
                dp->select_index = Digital_Pos_Complete;
                OLED_DigitalSetOptionAndCallback(dp, &output_option); //要退回到观察模式时会有一次complete回调
                dp->mod = Digital_Mode_Observe;
            }
            else if(msg == msg_click) {dp->mod = Digital_Mode_Singlebit;}   
            else if(msg == msg_up){dp->select_index++; if(dp->select_index > Digital_Pos_IndexLabel) dp->select_index = Digital_Pos_IndexRight;}
            else if(msg == msg_down)
            {
                if(dp->select_index == Digital_Pos_IndexRight) dp->select_index = Digital_Pos_IndexLabel;
                else dp->select_index--; 
            }
        break;
        
        case Digital_Mode_Singlebit:
            if(msg == msg_click)dp->mod = Digital_Mode_Edit;
            if(msg == msg_return)
            {
                dp->select_index = Digital_Pos_Complete;
                OLED_DigitalSetOptionAndCallback(dp, &output_option); //要退回到观察模式时会有一次complete回调
                dp->mod = Digital_Mode_Observe;
            }
            else if (msg == msg_up)
            {
                if(dp->select_index < Digital_Pos_IndexLabel) //数字的话
                    OLED_DigitalSetValAnim(dp, dp->select_index, 
                                dp->option_array[dp->select_index].val + dp->option_array[dp->select_index].step, Digital_Direct_Increase);
                else //在eidt页面做了selcet_index的上限限制，输入这边一定是index_label
                {
                    OLED_DigitalPage_UpdateLabelAnimation(dp, (dp->select_label_index)+1, Digital_Direct_Increase);
                }
            }
            else if (msg == msg_down)
            {
                if(dp->select_index < Digital_Pos_IndexLabel) //数字的话
                    OLED_DigitalSetValAnim(dp, dp->select_index, 
                                dp->option_array[dp->select_index].val - dp->option_array[dp->select_index].step, Digital_Direct_Decrease);
                else //在eidt页面做了selcet_index的上限限制，输入这边一定是index_label
                {
                    OLED_DigitalPage_UpdateLabelAnimation(dp, (dp->select_label_index)-1, Digital_Direct_Decrease);
                }
            }
            if(msg == msg_click) //编辑值完成后click时会调用回调，将对应选项的值输出给回调
                OLED_DigitalSetOptionAndCallback(dp, &output_option);
        break;
        default:
            break;
    }
}

void OLED_DigitalPageInit(
    DigitalPage* dp,                            //数字显示页面
    Option * option_array,                      //选项数组，必须包含3个选项，因为要显示3个数字
    uint8_t  label_array_num,                   //标签数组的大小
    String * label_array,                       //标签数组，方便标签滚动,数组大小必须和label_array_num一致
    char    gap_char,                           //间隔字符
    uint8_t gap_shine_time,                     //间隔字符闪烁时间间隔
    uint8_t uline_shine_time,                   //下划线标识符闪烁时间间隔
    CallBackFunc cb                             //回调函数
)
{
    dp->page.page_type = type_digital;   //赋值页面类型
    OLED_PageInit((PageAddr)dp, cb);
    //对方法赋值
    dp->page.ani_init = OLED_DigitalAnimInit;
    dp->page.show = OLED_DigitalShow;
    dp->page.react = OLED_DigitalReact;

    dp->option_array = option_array;
    dp->label_array_num = label_array_num;
    dp->select_label_index = 0; //默认选中第一个标签
    dp->select_index = Digital_Pos_Complete; //默认没有选中任何一个数字/标签，即是选择完成状态。
    dp->label_array = label_array;
    dp->gap_char = gap_char;
    dp->gap_shine_time = gap_shine_time;
    dp->uline_shine_time = uline_shine_time;
}


/**
 * @brief 更新数字页面标签动画
 * 
 * @param dp 数字页面指针
 * @param label_index 标签索引
 * @param dir 运动方向
 */
void OLED_DigitalPage_UpdateLabelAnimation(DigitalPage * dp, uint8_t label_index, DigitalDirect dir)
{
    if(label_index == 255) label_index = dp->label_array_num -1; // 如果标签索引为255，则设置为最后一个标签索引
    else if(label_index >= dp->label_array_num)label_index = 0; // 如果标签索引大于等于标签数组长度，则设置为第一个标签索引
    if(dp == (DigitalPage*)(p_current_ui->current_page)) //只有当前页面是要更新的页面时，才需要更新动画，否则直接更新值就可以
    {
        // 这里需要考虑下，给外界调用需不需要等待运动完成？？？？？？if(digital_num_pos == 0)
        p_current_ui->dp_var.dir = dir; // 确定方向
        p_current_ui->dp_var.label_y = 0; // 设置标签的初始位置
        p_current_ui->dp_var.digital_num_pos |= 0x40; // 设置数字位置标志位
        if(dir == Digital_Direct_Increase)p_current_ui->dp_var.label_y_trg = - DIGITAL_Label_SIZE; // 如果方向为增加，则设置运动目标值为负数
        else p_current_ui->dp_var.label_y_trg = DIGITAL_Label_SIZE; // 如果方向为减少，则设置运动目标值为正数
        p_current_ui->dp_var.last_or_next_label_index = dp->select_label_index; // 存储上一次的值
    }
    dp->select_label_index = label_index; // 更新选中项
}



/**
 * @brief 更新数字页面的数字动画
 * 
 * @param dp 数字页面指针
 * @param leftval 左侧数字值
 * @param midval 中间数字值
 * @param rightval 右侧数字值
 * @param dir 数字方向
 */
void OLED_DigitalPage_UpdateDigitalNumAnimation(DigitalPage * dp, uint8_t leftval, uint8_t midval, uint8_t rightval, DigitalDirect dir)
{
    uint8_t val_array[3] = {rightval, midval, leftval};
    if(dp == (DigitalPage*)(p_current_ui->current_page))//当当前页面是要更新的页面时，才触发动画更新，否则直接更新数据就可以
    {
        if((p_current_ui->dp_var.digital_num_pos&0x3F) == 0) //数字运动完成就可以更新 
        //运动结束赋值发生变化,才能从外部更新数据,(是否需要处于obeserve模式由外界自己决定，这样就可以在回调函数中使用这个函数了)
        {
            //触发数字的动画(调用时倒着循环调用)(保证能让temp_ripplr_index记录到最低需要滚动的位)
            for (int8_t i = Digital_Pos_IndexLeft ; i >= Digital_Pos_IndexRight; i--) //从高到低更新，这样ripple才能从低到高
                if(val_array[i] != dp->option_array[i].val)
                    OLED_DigitalSetValAnim(dp, (DigitalPosIndex)i, val_array[i], dir);
        }
    }
    else 
    {
        for (int8_t i = Digital_Pos_IndexLeft ; i >= Digital_Pos_IndexRight; i--) //从高到低更新，这样ripple才能从低到高
            dp->option_array[i].val = val_array[i]; //直接更新值没有触发动画
    }
}

#endif

//-------UI相关函数
//将当前操作的UI对象选择为默认UI对象
void OLED_SelectDefaultUI(void)
{
    p_current_ui = &default_ui;
}
//设置当前正在操作的UI对象
void OLED_SetCurrentUI(WouoUI *ui)
{
    p_current_ui = ui;
}
//初始化一个新的UI对象，需要传入这个UI的设置参数
void OLED_NewUI(WouoUI *ui, UiPara *ui_para)
{
    *ui = default_ui; //将默认UI的值复制到新UI
    ui->upara = ui_para; //将参数设置为新的UI的参数
}
static void OLED_UILayerInProc(void)
{
    if(p_current_ui->current_page != NULL)//检查该页面地址存在
    {
        Page* p = (Page*)(p_current_ui->current_page); //把当前页面转为page读取
        p->ani_init(p_current_ui->current_page);
    }
}


static void OLED_UIPageProc(void) //页面处理任务
{
    if(p_current_ui->current_page != NULL) //该页面地址存在
    {
        Option *op = NULL;
        Page * p = NULL;
        TitlePage * cur_tile = NULL;
        ListPage * cur_list = NULL;
        PageType pt = OLED_CheckPageType(p_current_ui->current_page);
        switch (pt)
        {
            case type_title: 
                cur_tile = (TitlePage *)(p_current_ui->current_page);
                op = &(cur_tile->option_array[cur_tile->select_item]);
                break;
            case type_list:
                cur_list = (ListPage *)(p_current_ui->current_page);
                op = &(cur_list->option_array[cur_list->select_item]);
                break;
            default:break;
        }
        p = (Page *)(p_current_ui->current_page);//把当前页面转为page读取
        p->show(p_current_ui->current_page);
        if(pt == type_list || pt == type_title)  
        {//valwin和conwin不会同时出现
#if UI_CONWIN_ENABLE
            if(p_current_ui->vwin.state == 0 && p_current_ui->cwin.state == 0)p->react(p_current_ui->current_page);
            OLED_WinFSM(&(p_current_ui->cwin), p_current_ui->current_page, op);
#else 
            if(p_current_ui->vwin.state == 0)p->react(p_current_ui->current_page);
#endif
            OLED_WinFSM(&(p_current_ui->vwin), p_current_ui->current_page, op);
        }
        else p->react(p_current_ui->current_page);         
    }
} 

void OLED_UIProc(void) //UI总任务(FSM)
{
    OLED_RefreshBuff();
    switch (p_current_ui->state) //ui状态机轮询
    {
        case ui_layer_in://主要是对页面动画在切换时做一次参数的赋值
            OLED_UILayerInProc(); 
            p_current_ui->state = ui_page_proc;
            break;
        case ui_page_proc://因为主要的绘制图像都在这儿完成，所以进入前需清空一次buff
            OLED_ClearBuff(); 
            OLED_UIPageProc(); //在处理过程中读取输入队列，确定状态转换
            break;
    default:
        break;
    }
}


/**
 * @brief 跳转到指定页面(跳转是会将当前页面id输入，用于关联确认上下级页面关系)
 * 
 * @param self_page 当前页面对象的地址
 * @param terminate_page 目标页面地址
 */
void OLED_UIJumpToPage(PageAddr self_page_addr,PageAddr terminate_page)
{
    // 关联上级页面并跳转页面
    if(terminate_page != NULL)
    {
        Page * p = (Page*)terminate_page; 
        p->last_page = self_page_addr;
        p_current_ui->current_page = terminate_page;
        if(p_current_ui->vwin.state != 0)p_current_ui->vwin.state = 0; //跳转页面时，如果当前页面有弹窗，将其关闭
#if UI_CONWIN_ENABLE
        if(p_current_ui->cwin.state != 0)p_current_ui->cwin.state = 0; //跳转页面时，如果当前页面有弹窗，将其关闭
#endif
        p_current_ui->init_finish_flag = False; // 开启过度动画
        p_current_ui->state = ui_layer_in; // 开始层级渲染
    }
}

/**
 * @brief 改变当前页面(不会关联上下级页面关系，只是单纯将当前页面更改为这个页面)
 * 
 * @param terminate_page The address of the page to change to.
 */
void OLED_UIChangeCurrentPage(PageAddr terminate_page)
{
    if(terminate_page != NULL)
    {
        p_current_ui->current_page = terminate_page;; // Set the current page ID to the page ID of the specified page.
        if(p_current_ui->vwin.state != 0)p_current_ui->vwin.state = 0; //跳转页面时，如果当前页面有弹窗，将其关闭
#if UI_CONWIN_ENABLE
        if(p_current_ui->cwin.state != 0)p_current_ui->cwin.state = 0; //跳转页面时，如果当前页面有弹窗，将其关闭
#endif
        p_current_ui->init_finish_flag = False; // Enable the transition animation.
        p_current_ui->state = ui_layer_in; // Start rendering the layers.
    }
}

/**
 * @brief 得到当前页面的地址
 * 
 * @param terminate_page The address of the page to change to.
 */
Page* OLED_GetCurrentPage(void)
{
    return (Page*)p_current_ui->current_page;
}
