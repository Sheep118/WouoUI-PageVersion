#ifndef __WOUOUI_CONF_H__
#define __WOUOUI_CONF_H__

//---------------------与调试相关的参数

#define WOUOUI_LOG_ENABLE   //是否使能WouoUI_Log的打印，使能的话需要实现下面的LOG打印函数
#ifdef  WOUOUI_LOG_ENABLE
#define WOUOUI_LOG(fmt,...)     printf(fmt,##__VA_ARGS__)
#else
#define WOUOUI_LOG(fmt,...)     
#endif

//---------------------与UI相关的参数
#define WOUOUI_BUFF_WIDTH           128 // 屏幕宽
#define WOUOUI_BUFF_HEIGHT          64  // 屏幕高
#define WOUOUI_INPUT_MSG_QUNEE_SIZE 4 // ui内部消息对列的大小(至少需要是2)
#define WOUOUI_WIN_TXT_DEFAULT      "NO SET TEXT!!!" //当弹窗没有设置自动获取且没有设置文本时的默认文本

#define SOFTWARE_DYNAMIC_REFRESH    1   //使能软件动态刷新
#define HARDWARE_DYNAMIC_REFRESH    0   //使能硬件动态刷新

#define WOUOUI_STR_LINE_SPACING     1   // 绘制字符串时行间距
#define WOUOUI_SLIDESTR_START_DELAY  20  //slidestr开始滑动的延迟大小(最大255)


//------------------与title页面相关的默认参数
#define DEFAULT_TILE_B_TITLE_FNOT           Font_12_24  // 磁贴大标题字体
#define DEFAULT_TILE_ICON_W                 30         // 磁贴图标宽度
#define DEFAULT_TILE_ICON_H                 30         // 磁贴图标高度
#define DEFAULT_TILE_ICON_IND_U             3          //磁贴指示器与磁贴的上边距
#define DEFAULT_TILE_ICON_IND_D             3         //磁贴指示器与磁贴的下边距
#define DEFAULT_TILE_ICON_IND_L             3         //磁贴指示器与磁贴的左边距
#define DEFAULT_TILE_ICON_IND_R             3        //磁贴指示器与磁贴的右边距
#define DEFAULT_TILE_ICON_IND_SL            5        //磁贴指示器边长SideLength
#define DEFAULT_TILE_ICON_S                 6       //磁贴图标间距(图标边和边的距离)
#define DEFAULT_TILE_BAR_D                  2       //磁贴装饰条下边距
#define DEFAULT_TILE_BAR_W                  8       //磁贴装饰条宽度
#define DEFAULT_TILE_BAR_H                  24      //磁贴装饰条高度
#define DEFAULT_TILE_SLIDESTR_MODE          2       //磁贴标题文本的滚动模式
//------------------与list页面相关的默认参数
#define DEFAULT_LIST_TEXT_FONT              Font_6_8   //列表文字的字体
#define DEFAULT_LIST_TEXT_U_S               3          //列表文字的上边距
#define DEFAULT_LIST_TEXT_D_S               2          //列表文字的下边距
#define DEFAULT_LIST_TEXT_L_S               3          //列表文字的左边距
#define DEFAULT_LIST_TEXT_R_S               3          //列表文字的右边距(到侧边进度条的距离)
#define DEFAULT_LIST_IND_VAL_S              1          //指示器和val文本的间距
#define DEFAULT_LIST_BAR_W                  3          //列表进度条宽度，需要是奇数，因为正中间有1像素宽度的线
#define DEFAULT_LIST_IND_BOX_R              2          //列表指示器选择框倒角
#define DEFAULT_LIST_CHECK_BOX_R            2          //列表确认选择框倒角
#define DEFAULT_LIST_CHECK_BOX_D_R          2          //列表确认选择框内点的倒角
#define DEFAULT_LIST_CHECK_BOX_D_S          1          //列表确认选择框内点到宽的边距
#define DEFAULT_LIST_TEXT_LEN_PER_MAX       62         //这个是百分占比即列表文字最大长度占宽度的比例
#define DEFAULT_LIST_VAL_BUFF_SIZE          16         //列表值文本缓冲区的大小
#define DEFAULT_LIST_TXT_SLIDESTR_MODE      2          //list文本的滚动模式
#define DEFAULT_LIST_VAL_SLIDESTR_MODE      2          //list数值文本的滚动模式
    //以下四个字符串不能有重复，重复会按单选框显示，
    //个人习惯是'-'一般选项；'~'数值弹窗；'='数值显示;'>'列表弹窗；'@'二值选项；'#'确认弹窗；'$'Spin弹窗
#define DEFAULT_LIST_LINETAIL_VAL_PREFIX    "~"       //listtext中使用这些字符其中一个，行尾会把val显示出来
#define DEFAULT_LIST_LINETAIL_TXT_PREFIX    "><"        //listtext中使用这些字符其中一个，行尾会把content显示出来
#define DEFAULT_LIST_LINETAIL_SPIN_PREFIX   "%$="        //listtext中使用这些字符其中一个，行尾会把val按照decimal显示为定点数
#define DEFAULT_LIST_LINETAIL_CONF_PREFIX   "@#"        //listtext中使用这些字符其中一个，行尾会显示一个单选框(其中第一个作为二值选项框会自动处理,如果开启自动处理的话)
//------------------与Wave页面相关的默认参数
#define DEFAULT_WAVE_FONT                   Font_6_8   // 波形字体
#define DEFAULT_WAVE_Y_AXIS_LABEL_WIDTH     20      //y轴标签的宽度
#define DEFAULT_WAVE_D_CUR_VAL_WIDTH        20  //底部数值的显示宽度
#define DEFAULT_WAVE_BOX_L_S                0   // 波形边框左边距
#define DEFAULT_WAVE_BOX_R_S                0   // 波形边框右边距
#define DEFAULT_WAVE_BOX_U_S                0   // 波形边框上边距
#define DEFAULT_WAVE_BOX_D_S                1   // 波形边框下边距
#define DEFAULT_WAVE_D_TXT_VAL_S            3   //底部文本和数值和显示标志的间隔
#define DEFAULT_WAVE_TEXT_D_S               1   //文本行到底边的边距
#define DEFAULT_WAVE_TEXT_W                 60  //文本行的宽度
#define DEFAULT_WAVE_DEPTH                  (WOUOUI_BUFF_WIDTH-DEFAULT_WAVE_Y_AXIS_LABEL_WIDTH+100)   //默认波形的储存深度
#define DEFAULT_WAVE_TXT_SLIDESTR_MODE      2   //WAVE文本的滚动模式
#define DEFAULT_WAVE_VAL_SLIDESTR_MODE      2   //WAVE数值文本的滚动模式

//---------------与MsgWin页面相关的默认参数
#define DEFAULT_MSG_WIN_FONT                Font_6_8 // Msg弹窗字体
#define DEFAULT_MSG_WIN_W                   100  //MSG弹窗的宽度
#define DEFAULT_MSG_WIN_V_S_MIN             12  //MSG弹窗到屏幕两侧的上下边距的最小值
#define DEFAULT_MSG_WIN_R                   2   //MSG弹窗的倒角大小
#define DEFAULT_MSG_WIN_FONT_MARGIN         4   //MSG弹窗文本到边框来的边距

//---------------与ConfWin页面相关的默认参数
#define DEFAULT_CONF_WIN_FONT               Font_6_8  // CONF弹窗字体
#define DEFAULT_CONF_WIN_W                  100  //CONF弹窗的宽度
#define DEFAULT_CONF_WIN_V_S_MIN            12  //CONF弹窗到屏幕两侧的上下边距的最小值
#define DEFAULT_CONF_WIN_R                  2   //CONF弹窗的倒角大小
#define DEFAULT_CONF_WIN_FONT_MARGIN        4   //CONF弹窗文本到边框来的边距
#define DEFAULT_CONF_WIN_IND_BTN_S          2   //按键内文本和指示器的竖直间距
#define DEFAULT_CONF_WIN_BTN_R              2   // 按键倒角
#define DEFAULT_CONF_WIN_TEXT_BTN_S         2   //文本和按键的竖直间距
#define DEFAULT_CONF_BTN_SLIDESTR_MODE      2   //WAVE文本的滚动模式

//--------------与ValWin页面相关的默认参数
#define DEFAULT_VAL_WIN_FONT                Font_6_8  // Val弹窗字体
#define DEFAULT_VAL_WIN_STR_BUFF_SIZE       16        //临时buff的大小
#define DEFAULT_VAL_WIN_W                   120  //Val弹窗的宽度
#define DEFAULT_VAL_WIN_R                   2    //Val弹窗的倒角大小
#define DEFAULT_VAL_WIN_TXT_W_MAX           78   //val弹窗内文本可显示宽度的最大值
#define DEFAULT_VAL_WIN_TXTVAL_S            8    //弹窗内文本和val间的间距
#define DEFAULT_VAL_WIN_BAR_W               66   // 进度条宽度
#define DEFAULT_VAL_WIN_BAR_H               7    // 进度条高度
#define DEFAULT_VAL_WIN_BAR_R               1    // 进度条倒角
#define DEFAULT_VAL_MMVAL_BAR_S             2    // 进度条与MIN MAX VAL间的边距
#define DEFAULT_VAL_TEXT_BAR_S              8    // 下方进度条和上方文本的间距
#define DEFAULT_VAL_WIN_FONT_MARGIN         4    // 弹窗内部元素到边框的边距
#define DEFAULT_VAL_WIN_TXT_SLISTRMODE      3    // Val弹窗文本的滚动模式
#define DEFAULT_VAL_WIN_VAL_SLISTRMODE      3    // Val弹窗数值文本的滚动模式

//--------------与SpinWin页面相关的默认参数
#define DEFAULT_SPIN_WIN_FONT               Font_7_12    // 弹窗字体
#define DEFAULT_SPIN_WIN_NUM_FONT           Font_7_12   // 数字字体
#define DEFAULT_VAL_WIN_STR_BUFF_SIZE       16   //临时buff的大小
#define DEFAULT_SPIN_WIN_W                  100  // Spin弹窗的宽度
#define DEFAULT_SPIN_WIN_NUM_S              2    // spin内数字之间横向间隔(必须是偶数，为了显示好看)
#define DEFAULT_SPIN_WIN_FONT_MARGIN        4    // Spin弹窗内元素到边框的边距
#define DEFAULT_SPIN_WIN_V_S                2    // Spin弹窗内元素间上下边距
#define DEFAULT_SPIN_WIN_BOX_R              1    // Spin弹窗的指示器倒角大小
#define DEFAULT_SPIN_WIN_BOX_H              3    // Spin弹窗的指示器高度
#define DEFAULT_SPIN_WIN_R                  2    // Spin弹窗的倒角大小
#define DEFAULT_SPIN_WIN_MMVAL_S            1    // Spin弹窗的最小最大值间距
#define DEFAULT_SPIN_WIN_SLI_TXT_MODE       2    // Spin弹窗文本的滚动模式
#define DEFAULT_SPIN_WIN_SLI_VAL_MODE       2    // Spin弹窗数值文本的滚动模式

//---------------与ListWin页面相关的默认参数
#define DEFAULT_LIST_WIN_FONT               Font_6_8   // 弹窗字体
#define DEFAULT_LIST_WIN_W                  50         // 弹窗宽度
#define DEFAULT_LIST_WIN_R                  3          // 弹窗倒角
#define DEFAULT_LIST_WIN_BOX_R              2          // 列表选择框倒角
#define DEFAULT_LIST_WIN_L_S                6          // 列表文字左边距
#define DEFAULT_LIST_WIN_R_S                0          // 列表文字的右边距(到侧边进度条的距离)
#define DEFAULT_LIST_WIN_BAR_W              3          // 必须是单数，因为需要中间为1的进度条
#define DEFAULT_LIST_WIN_TEXT_U_S           3          // 列表每行文字的上边距
#define DEFAULT_LIST_WIN_TEXT_D_S           3          // 列表每行文字的上边距

#endif
