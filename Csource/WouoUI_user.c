#include "WouoUI_user.h"
#include "WouoUI.h"
#include "math.h"
#include "string.h"

//--------定义页面对象
TitlePage main_page;
    ListPage setting_page;
    WavePage wave_page;
        MsgWin common_msg_page; //共用的消息弹窗
        ValWin common_val_page; //共用的数值弹窗
        ConfWin common_conf_page; //共用的确认弹窗
        SpinWin common_spin_page; //共用的spin弹窗
        ListWin bg_blur_sel_page; //背景模糊的选项
            ValWin test_val_win_page; //测试的数值弹窗
    ValWin volumn_page; //音量调节页面
    ConfWin volumn_conf_page; //音量确认界面
    ListPage about_page;
        ListPage about_origin_page;
            ListPage about_wououi_page;
            ListPage about_version_page;
//--------定义每个页面需要的一些参数
// 部分页面内选项个数
#define MAIN_PAGE_NUM         6 //因为有两个数组所以做宏定义
// 定义部分页面选项
//********main页面的选项
Option mian_option_array[MAIN_PAGE_NUM] =
{
        {.text = (char *)"! WouoUI"},
        {.text = (char *)"+ Setting"},
        {.text = (char *)"+ Wave"},
        {.text = (char *)"~ This is a volumn page for test win in win",},
        {.text = (char *)"% Spin", .val = 123456, .decimalNum = DecimalNum_2},
        {.text = (char *)"- About"}
};
// main页面的图标
Icon main_icon_array[MAIN_PAGE_NUM] =
    {
        [0] = {0xFC, 0xFE, 0xFF, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
               0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x07, 0x07, 0x0F, 0x1F, 0x3F, 0xFF, 0xFE, 0xFC, 0xFF, 0x01,
               0x00, 0x00, 0x00, 0x00, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFC, 0xFC,
               0x00, 0x00, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0xF0, 0xC0, 0x00,
               0x00, 0x00, 0x03, 0x07, 0x0F, 0x1F, 0x3E, 0x3C, 0x3C, 0x3C, 0x1E, 0x1F, 0x0F, 0x03, 0x00, 0x00,
               0x1F, 0x3F, 0x3F, 0x1F, 0x00, 0x00, 0x00, 0xC0, 0xF0, 0xFF, 0xCF, 0xDF, 0xFF, 0xFF, 0xFE, 0xFC,
               0xF8, 0xF8, 0xF0, 0xF0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xF0, 0xF0,
               0xF8, 0xF8, 0xFC, 0xFE, 0xFF, 0xFF, 0xDF, 0xCF}, // logo
        [1] = {0xFC, 0xFE, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x83, 0x81, 0x01, 0x01, 0x81, 0xE1, 0xE1, 0xE1,
               0xE1, 0x81, 0x01, 0x81, 0x81, 0x83, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFE, 0xFC, 0xFF, 0x01,
               0x00, 0x00, 0x00, 0xE0, 0xE0, 0xF3, 0xFF, 0xFF, 0x3F, 0x0F, 0x07, 0x07, 0x03, 0x03, 0x07, 0x07,
               0x0F, 0x3F, 0xFF, 0xFF, 0xF7, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xE0, 0x80, 0x00,
               0x00, 0x01, 0x01, 0x3B, 0x7F, 0x7F, 0x7F, 0x3C, 0x78, 0xF8, 0xF0, 0xF0, 0xF8, 0x78, 0x3C, 0x3F,
               0x7F, 0x7F, 0x33, 0x01, 0x01, 0x00, 0x00, 0x80, 0xE0, 0xFF, 0xCF, 0xDF, 0xFF, 0xFF, 0xFE, 0xFC,
               0xF8, 0xF0, 0xF0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE1, 0xE1, 0xE1, 0xE1, 0xE0, 0xE0, 0xE0, 0xE0, 0xF0,
               0xF0, 0xF8, 0xFC, 0xFC, 0xFF, 0xFF, 0xDF, 0xCF}, // Setting
        [2] = {0xFC, 0xFE, 0x07, 0xF3, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0x03, 0x03,
               0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xF3, 0x07, 0xFE, 0xFC, 0xFF, 0xFF,
               0x00, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0xBF, 0x3F, 0x7F, 0x20, 0x08, 0x8F, 0x07,
               0x61, 0x30, 0x36, 0x38, 0x39, 0x39, 0x39, 0x39, 0x39, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xE7,
               0xE7, 0xE7, 0xE7, 0xE7, 0xC7, 0xDB, 0xC3, 0xE1, 0xF8, 0xFC, 0x04, 0x01, 0xFF, 0xFF, 0xFF, 0xFF,
               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xCF, 0xDF, 0xF8, 0xF3, 0xF7, 0xF7,
               0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF0, 0xF0, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7,
               0xF7, 0xF7, 0xF7, 0xF7, 0xF3, 0xF8, 0xDF, 0xCF}, // Curve
        [3] = {0x00, 0xFE, 0xFE, 0x1E, 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x86, 0x06,
               0x06, 0xC6, 0x86, 0x86, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x1E, 0xFE, 0xFE, 0x00, 0x00, 0xFF,
               0xFF, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xFC, 0xFC, 0xFE, 0xFF, 0xFF, 0x00, 0x00, 0xF1,
               0xE1, 0xE1, 0x03, 0x0F, 0xFC, 0xE0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00,
               0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x0F, 0x0F, 0x1F, 0x3F, 0x7F, 0x00, 0x00, 0xC3, 0x61, 0x61,
               0x70, 0x3C, 0x1F, 0x03, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x1F, 0x1F, 0x1E, 0x1E, 0x18,
               0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
               0x18, 0x18, 0x18, 0x1E, 0x1E, 0x1F, 0x1F, 0x00}, //模拟调节音量，测试弹窗嵌套
        [4] = {0xFC, 0xFE, 0x1F, 0x07, 0x07, 0xC3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0x63, 0x63, 0x63,
               0x63, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xC3, 0x07, 0x07, 0x1F, 0xFE, 0xFC, 0xFF, 0xFF,
               0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x3F, 0x3F, 0x3F, 0x01, 0x00, 0x00, 0x3C, 0x3C, 0x00, 0x00,
               0x01, 0x3F, 0x3F, 0x3F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
               0x00, 0xFF, 0xFF, 0xFF, 0xC0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
               0x80, 0xC0, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xCF, 0xDF, 0xFE, 0xF8, 0xF8, 0xF0,
               0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1,
               0xF1, 0xF1, 0xF0, 0xF8, 0xF8, 0xFE, 0xDF, 0xCF}, // DigitalLock
        [5] = {0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x3F, 0x1F, 0x0F, 0x0F, 0x0F, 0x07, 0x07, 0x07,
               0x07, 0x0F, 0x0F, 0x0F, 0x1F, 0x3F, 0x3F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFC, 0xFF, 0xFF,
               0xFF, 0x3F, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xEE, 0xE6, 0x66, 0x00,
               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
               0xF0, 0xE0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x1F, 0x1F, 0x08, 0x00, 0x00, 0x00,
               0x00, 0x00, 0x00, 0x00, 0x60, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xCF, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF,
               0xFF, 0xFF, 0xFF, 0xFE, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
               0xF8, 0xF8, 0xF8, 0xF8, 0xFF, 0xFF, 0xDF, 0xCF} // about
};

// 设置的列表选项数组
Option setting_option_array[] ={
    {.text = (char *)"- Setting"}, // 第一个做说明标签，没有功能
    {.text = (char *)"~ Title Ani"},
    {.text = (char *)"% List Ani", .decimalNum = DecimalNum_0,},
    {.text = (char *)"~ IND Ani"},
    {.text = (char *)"@ Title UFD"},
    {.text = (char *)"@ List UFD"},
    {.text = (char *)"@ Title Loop"},
    {.text = (char *)"# List Loop"},
    {.text = (char *)"> Msg win gb blur"},
    {.text = (char *)"> Conf win gb blur"},
    {.text = (char *)"> Val win gb blur"},
    {.text = (char *)"> Spin win gb blur"},
    {.text = (char *)"> List win gb blur"},
};

String bg_blur_sel_str_array[] = {
    (char *)"BLUR_0_4",
    (char *)"BLUR_1_4",
    (char *)"BLUR_2_4",
    (char *)"BLUR_3_4",
    (char *)"BLUR_4_4",
    (char *)"This itme is invalid, it use to test slide str"
};

// about页面的选项数组
Option about_option_array[] ={
        {.text = (char *)"+ Radio box"},    // 原wououi的说明
        {.text = (char *)"+ About wouo"},   // 第一项只是说明
        {.text = (char *)"+ Page version"}, // Page version的说明
};

// about wououi页面的数组
Option about_origin_array[] ={
        {.text = (char *)"- Radio box"},
        {.text = (char *)"# test1", .val = 1,},
        {.text = (char *)"# test2", .val = 0,},
        {.text = (char *)"@ test3", },
        {.text = (char *)"@ test4", },
        {.text = (char *)"@ test5", },
};
Option about_wououi_array[] ={
        {.text = (char *)"- From WouoUI1.2"},
        {.text = (char *)"- Thanks for the "},
        {.text = (char *)"  original author"},
        {.text = (char *)"! Bilibili UID:"},
        {.text = (char *)"@ 9182439"},
        {.text = (char *)"- Github URL: "},
        {.text = (char *)"@ https://"},
        {.text = (char *)"  github.com/"},
        {.text = (char *)"  RQNG/WouoUI"},
};
Option about_version_array[] ={
        {.text = (char *)"- Page Version"},
        {.text = (char *)"- MyBilili UID:"},
        {.text = (char *)"@ 679703519"},
        {.text = (char *)"- Github URL:"},
        {.text = (char *)"@ https://"},
        {.text = (char *)"  github.com/"},
        {.text = (char *)"  Sheep118/WouoUI"},
        {.text = (char *)"  -PageVersion"},
};
//波形数据
WaveData wave_data_array[] = {
    [0] = {
        .text = (char *)"sin wave",
        .waveType = WaveType_Solid,
        .data = {0},
        .decimal_num = DecimalNum_2,
    },
    [1] = {
        .text = (char *)"cos wave",
        .waveType = WaveType_Dash,
        .data = {0},
        .decimal_num = DecimalNum_1,
    },
};


// //--------定义每个页面的回调函数
bool MainPage_CallBack(const Page *cur_page_addr, InputMsg msg) {
    Option* select_item = WouoUI_ListTitlePageGetSelectOpt(cur_page_addr);
    if(msg_click == msg){ //可以依靠text识别选项
        if (!strcmp(select_item->text, "! WouoUI")) {//设置消息弹窗内容并跳转
            WouoUI_MsgWinPageSetContent(&common_msg_page, (char*)"WouoUI Page\nStay tuned for more page developments");
            WouoUI_JumpToPage((PageAddr)cur_page_addr, &common_msg_page);
        } else if (!strcmp(select_item->text, "+ Setting")) {
            WouoUI_JumpToPage((PageAddr)cur_page_addr, &setting_page);
        } else if(!strcmp(select_item->text, "+ Wave")){
            WouoUI_JumpToPage((PageAddr)cur_page_addr, &wave_page);
        } else if(!strcmp(select_item->text, "~ This is a volumn page for test win in win")){
            WouoUI_JumpToPage((PageAddr)cur_page_addr, &volumn_page);
        } else if(!strcmp(select_item->text, "% Spin")){//设置spin弹窗的设置范围和小数点数并跳转
            WouoUI_SpinWinPageSetMinMaxDecimalnum(&common_spin_page,-500000,500000,select_item->decimalNum);
            WouoUI_JumpToPage((PageAddr)cur_page_addr, &common_spin_page);
        } else if(!strcmp(select_item->text, "- About")){
            WouoUI_JumpToPage((PageAddr)cur_page_addr, &about_page);
        }
    }
    return false;
}

bool SettingPage_CallBack(const Page *cur_page_addr, InputMsg msg) {
    Option* select_item = WouoUI_ListTitlePageGetSelectOpt(cur_page_addr);
    if(msg_click == msg){ //也可以靠选项的order顺序来识别item(以下顺序为其在数组中的下标)
        switch (select_item->order)
        {
            case 0:
                WouoUI_MsgWinPageSetContent(&common_msg_page, (char*)"Setting Page\nYou can change the setting of this UI");
                WouoUI_JumpToPage((PageAddr)cur_page_addr, &common_msg_page);
            break;
            case 1: //设置数组弹窗调整的min step max 并跳转
                WouoUI_ValWinPageSetMinStepMax(&common_val_page, 0, 1, 500);
                WouoUI_JumpToPage((PageAddr)cur_page_addr, &common_val_page);
            break; 
            case 2:
                WouoUI_SpinWinPageSetMinMaxDecimalnum(&common_spin_page, 0, 50000, select_item->decimalNum);
                WouoUI_JumpToPage((PageAddr)cur_page_addr, &common_spin_page);
            break;
            case 3:
                WouoUI_ValWinPageSetMinStepMax(&common_val_page, 0, 1, 500);
                WouoUI_JumpToPage((PageAddr)cur_page_addr, &common_val_page);
            break;
            //@二值选项框的值直接取出赋给我们想要的变量即可
            case 4: g_default_ui_para.ufd_param[TILE_UFD] = !!(select_item->val); break;
            case 5: g_default_ui_para.ufd_param[LIST_UFD] = !!(select_item->val); break;
            case 6: g_default_ui_para.loop_param[LIST_LOOP] = !!(select_item->val); break;
            case 7:    
                WouoUI_JumpToPage((PageAddr)cur_page_addr,&common_conf_page);
            break;
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
                WouoUI_JumpToPage((PageAddr)cur_page_addr,&bg_blur_sel_page);
            break;
            default:
                break;
        }
    } 
    return false;
}

bool WavePage_CallBack(const Page *cur_page_addr, InputMsg msg)
{
    if(WouoUI_WaveDataIsStop(&wave_page.wave_data_array[wave_page.wave_data_select])){
        //当前波形暂停的时候
        switch (msg)
        {
            case msg_up:
            case msg_left://如果当前波形不能移动了,消息弹窗提示
            if(!WouoUI_WavePageCanShiftWave(&wave_page, wave_page.wave_data_select, 0)){
                WouoUI_MsgWinPageSetContent(&common_msg_page, (char*)"arrive the head of buff,\nCan't left shift wave.");
                WouoUI_JumpToPage((PageAddr)cur_page_addr, &common_msg_page);
            }
            break;
            case msg_down:
            case msg_right:
            if(!WouoUI_WavePageCanShiftWave(&wave_page, wave_page.wave_data_select, 1)){
                WouoUI_MsgWinPageSetContent(&common_msg_page, (char*)"arrive the tail of buff,\nCan't right shift wave.");
                WouoUI_JumpToPage((PageAddr)cur_page_addr, &common_msg_page);
            }
            break;
            default: break;
        }
    }
    return false;
}

bool VolumePage_CallBack(const Page *cur_page_addr, InputMsg msg)
{
    ValWin* vw = (ValWin*)cur_page_addr;
    if(vw->val == 80 && (msg_up == msg || msg_left == msg)) //如果调整到80的时候，弹出弹窗警告，音量过高，是否继续
    {
        volumn_conf_page.content = (char*)"The volumn is too high, do you want to increase it continue?";
        WouoUI_JumpToPage((PageAddr)cur_page_addr ,&volumn_conf_page);
    }
    return false;
}

bool VolumnConf_CallBack(const Page *cur_page_addr, InputMsg msg)
{
    if(msg_return == msg) //如果直接返回的话，音量会保持在80
        volumn_page.val = 79;
    if(msg_click == msg)
    {
        if(volumn_conf_page.conf_ret == false)volumn_page.val = 79; //如果选择是no的话，音量保持80
    }
    return false;
}

bool About_CallBack(const Page *cur_page_addr, InputMsg msg) {
    if(msg_click == msg){
        Option* select_item = WouoUI_ListTitlePageGetSelectOpt(cur_page_addr);
        switch (select_item->order) { // 第0项是说明文字
        case 0:
            WouoUI_JumpToPage((PageAddr)cur_page_addr, &about_origin_page);
            break;
        case 1:
            WouoUI_JumpToPage((PageAddr)cur_page_addr, &about_wououi_page);
            break;
        case 2:
            WouoUI_JumpToPage((PageAddr)cur_page_addr, &about_version_page);
            break;
        default:
            break;
        }
    }
    return false;
}


bool CommonConfPage_CallBack(const Page *cur_page_addr, InputMsg msg) {
    //这个弹窗只在设置setting 的listloop选项中使用,所以可以不用判断bg_opt(参考下面通用val弹窗)
    //不然一般通用确认弹窗可以在初始化设置auto_get_bg_opt中会自动获取bg_opt的content,和选中项的指针
    // 可以靠这个判断是哪个选项进入的, 要是不想省一些内存,每一个确认弹窗单独使用一个confwinpage页面也是可以的
    if(msg_click == msg){
        g_default_ui_para.loop_param[LIST_LOOP] = common_conf_page.conf_ret;
    }
    return false;
}

bool CommonValPage_CallBack(const Page *cur_page_addr, InputMsg msg) {
    if (msg_click == msg) {
        if (!strcmp(common_val_page.bg_opt->text, "~ Title Ani")) {
            g_default_ui_para.ani_param[TILE_ANI] = common_val_page.val;
        } else if (!strcmp(common_val_page.bg_opt->text, "~ IND Ani")) {
            g_default_ui_para.ani_param[IND_ANI] = common_val_page.val;
        }
    }
    return false;
}

bool BgBlurSelPage_CallBack(const Page *cur_page_addr, InputMsg msg){
    BLUR_DEGREE ret = BLUR_2_4;
    bool res = false; //失能自动处理消息后，返回值true表示return，其他false
    switch (msg)
    {
        case msg_click:
            if(bg_blur_sel_page.sel_str_index < 5){ //click时选中的选项有效
                ret = (BLUR_DEGREE)bg_blur_sel_page.sel_str_index;
                //判断是哪个选项跳转到这个弹窗来的,与list中一样,可以靠order/text进行识别都可以
                if(!strcmp(bg_blur_sel_page.bg_opt->text,"> Msg win gb blur")){
                    g_default_ui_para.winbgblur_param[MGS_WBB] = ret;
                } else if(!strcmp(bg_blur_sel_page.bg_opt->text,"> Conf win gb blur")){
                    g_default_ui_para.winbgblur_param[CONF_WBB] = ret;
                } else if(!strcmp(bg_blur_sel_page.bg_opt->text,"> Val win gb blur")){
                    g_default_ui_para.winbgblur_param[VAL_WBB] = ret;
                } else if(!strcmp(bg_blur_sel_page.bg_opt->text,"> Spin win gb blur")){
                    g_default_ui_para.winbgblur_param[SPIN_WBB] = ret;
                } else if(!strcmp(bg_blur_sel_page.bg_opt->text,"> List win gb blur")){
                    g_default_ui_para.winbgblur_param[LIST_WBB] = ret;
                }
            } else if(bg_blur_sel_page.sel_str_index == 5){ //测试跳转嵌套弹窗
                WouoUI_JumpToPage((PageAddr)cur_page_addr, &test_val_win_page);
            }
            break;
        case msg_return:
            res = true; //失能自动处理消息后，返回值true表示return，其他false
            break;
        case msg_up:
        case msg_left: //上翻一页
            WouoUI_ListWinPageLastItem(&bg_blur_sel_page); //上翻一页
            break;
        case msg_down:
        case msg_right: //下翻一页
            WouoUI_ListWinPageNextItem(&bg_blur_sel_page); //下翻一页
            break;
        default: break;
    }
    return res;
}


//--------------给主函数调用的接口函数
void TestUI_Init(void) {
    // OLED_Init();  //硬件的初始化
    WouoUI_SelectDefaultUI(); // 选择默认UI(这里先绑定刷屏函数再选择默认UI也没关系，因为p_cur_ui初始化时就是指向默认UI)
    WouoUI_BuffClear();      // 清空缓存
    WouoUI_BuffSend();       // 刷新屏幕(清空屏幕)
    WouoUI_GraphSetPenColor(1); // 设置绘制颜色
    // 补充列表页面的初值
    setting_option_array[1].val = g_default_ui_para.ani_param[TILE_ANI];
    setting_option_array[2].val = g_default_ui_para.ani_param[LIST_ANI];
    setting_option_array[3].val = g_default_ui_para.ani_param[IND_ANI];
    setting_option_array[4].val = g_default_ui_para.ufd_param[TILE_UFD];
    setting_option_array[5].val = g_default_ui_para.ufd_param[LIST_UFD];
    setting_option_array[6].val = g_default_ui_para.loop_param[TILE_LOOP];
    setting_option_array[7].val = g_default_ui_para.loop_param[LIST_LOOP];
    setting_option_array[8].content = bg_blur_sel_str_array[(uint8_t)g_default_ui_para.winbgblur_param[MGS_WBB]];
    setting_option_array[9].content = bg_blur_sel_str_array[(uint8_t)g_default_ui_para.winbgblur_param[CONF_WBB]];
    setting_option_array[10].content = bg_blur_sel_str_array[(uint8_t)g_default_ui_para.winbgblur_param[VAL_WBB]];
    setting_option_array[11].content = bg_blur_sel_str_array[(uint8_t)g_default_ui_para.winbgblur_param[SPIN_WBB]];
    setting_option_array[12].content = bg_blur_sel_str_array[(uint8_t)g_default_ui_para.winbgblur_param[LIST_WBB]];
    // 设置界面选项
    WouoUI_TitlePageInit(&main_page, MAIN_PAGE_NUM, mian_option_array, main_icon_array, MainPage_CallBack);
    WouoUI_ListPageInit(&setting_page, sizeof(setting_option_array)/sizeof(Option), setting_option_array, Setting_none, SettingPage_CallBack);
    WouoUI_ListPageInit(&about_page, sizeof(about_option_array)/sizeof(Option), about_option_array, Setting_none, About_CallBack);
    WouoUI_ListPageInit(&about_origin_page, sizeof(about_origin_array)/sizeof(Option), about_origin_array, Setting_radio, NULL);
    WouoUI_ListPageInit(&about_wououi_page, sizeof(about_wououi_array)/sizeof(Option), about_wououi_array, Setting_none, NULL);
    WouoUI_ListPageInit(&about_version_page, sizeof(about_version_array)/sizeof(Option), about_version_array, Setting_none, NULL);
    WouoUI_WavePageInit(&wave_page, sizeof(wave_data_array)/sizeof(WaveData), wave_data_array, WavePage_CallBack);

    WouoUI_MsgWinPageInit(&common_msg_page, NULL, false, 2, NULL);
    WouoUI_ConfWinPageInit(&common_conf_page, NULL, NULL, NULL, true, true, true, 2, CommonConfPage_CallBack);
    WouoUI_ValWinPageInit(&common_val_page, NULL, 0, 0, 100, 20, true, true, CommonValPage_CallBack);
    WouoUI_SpinWinPageInit(&common_spin_page, NULL, 0, DecimalNum_0, -500000, 500000, true, true, NULL);
    WouoUI_ListWinPageInit(&bg_blur_sel_page, sizeof(bg_blur_sel_str_array)/sizeof(String), bg_blur_sel_str_array, true, BgBlurSelPage_CallBack);
    WouoUI_SetPageAutoDealWithMsg((Page*)&bg_blur_sel_page, false); //失能自动处理消息，因为想要在弹窗中click跳转其他页面

    WouoUI_ValWinPageInit(&test_val_win_page, (char*)"This is test for Pop-up window nesting", 0, 0, 100, 20, true, true, NULL);
    
    WouoUI_ValWinPageInit(&volumn_page, NULL, 50, 0, 100, 1, false, false, VolumePage_CallBack);
    WouoUI_ConfWinPageInit(&volumn_conf_page, NULL, NULL, NULL, true, true, true, 2, VolumnConf_CallBack);
}
