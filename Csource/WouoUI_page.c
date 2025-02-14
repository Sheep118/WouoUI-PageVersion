#include "WouoUI_page.h"
#include "WouoUI.h" //包含自己的顶层文件其实不好(但需要调用到p_cur_ui,后期再改吧)
#include "math.h"
//**********************通用页面处理方法
/**
 * @brief 通用的页面返回上一个页面函数
 *
 * @param page_addr 当前页面地址
 */
void WouoUI_PageReturn(PageAddr page_addr) {
    Page *p = (Page *)page_addr;
    p_cur_ui->in_page = p->last_page; // 将UI页面设置为上一级页面
    // 从主页面返回
    if (p == p_cur_ui->home_page) { //主页面返回这个动画很漂亮
        p_cur_ui->indicator.x.pos_cur = 0;
        p_cur_ui->indicator.y.pos_cur = 0;
        p_cur_ui->indicator.w.pos_cur = WOUOUI_BUFF_WIDTH;
        p_cur_ui->indicator.h.pos_cur = WOUOUI_BUFF_HEIGHT;
    }
}

/**
 * @brief
 *
 * @param page_addr 最基本的页面成员初始化，并加入到页面队列中
 * @param call_back 页面成员点击回调函数
 * @attention 没有进行页面类型初始化，因为这个由各自初始化函数执行
 */
void WouoUI_PageInit(PageAddr page_addr, CallBackFunc call_back) {
    Page *p = (Page *)page_addr;
    if (p_cur_ui->home_page == NULL) // 第一个页面初始化
    {
        p_cur_ui->home_page = page_addr;    // 将其初始化为主页面
        p_cur_ui->current_page = page_addr; // 同时作为当前页面
        p_cur_ui->in_page = page_addr;  //in页面也初始化为主页面
    }
    p->auto_deal_with_msg = true; 
    //默认页面会自动处理msg输入,回调函数可以进行user的其他处理,如果使用者有一个只读的页面,或者自己有处理输入消息和页面活动,可以在回调函数中调用接口自己处理
    p->last_page = p_cur_ui->home_page;
    // 页面初始化时没有上一级页面,只有在页面跳转时才确定页面的上下级关系,默认将主页面(第一个初始化的页面)作为上一级页面
    p->cb = call_back;
}

void WouoUI_SetPageAutoDealWithMsg(Page* page, bool open)
{
    if(WouoUI_CheckPageIsInit(page)){
        page->auto_deal_with_msg = open;
    }else WOUOUI_LOG_E("this page is not inited!!,it can't disable/enable atip dealwth msg");
}


//**********************每个以Page为基类的类都有对应的AnimInit、show、React方法
//--------Title相关基本方法
void WouoUI_TitlePageInParaInit(PageAddr page_addr)
{
    p_cur_ui->tp_var.iconX.pos_cur = 0;
    p_cur_ui->tp_var.iconX.pos_tgt = TILE_ICON_S;
    p_cur_ui->tp_var.iconY.pos_cur = -(TILE_ICON_H/2);
    p_cur_ui->tp_var.iconY.pos_tgt = TILE_ICON_U;
    p_cur_ui->tp_var.barX.pos_cur = -TILE_BAR_W;
    p_cur_ui->tp_var.barX.pos_tgt = 0;
    p_cur_ui->tp_var.titleY.pos_cur = WOUOUI_BUFF_HEIGHT;
    //以下这部分变量在IN中也不会改变，可以放到Init初始化中去
    p_cur_ui->tp_var.titleY.pos_tgt = TILE_BAR_U+(TILE_BAR_H-TILE_B_TITLE_FONT.Height)/2; //字体在磁贴中自动居中
    p_cur_ui->tp_var.title_ss.canvas.h = GET_FNOT_H(TILE_B_TITLE_FONT);
    p_cur_ui->tp_var.title_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[TILE_SSS];
    p_cur_ui->tp_var.title_ss.step = p_cur_ui->upara->slidestrstep_param[TILE_SSS];
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->tp_var.title_ss));
}
bool WouoUI_TitlePageIn(PageAddr page_addr)
{
    bool ret = false; //表示动画是否结束，返回True UI会切换状态
    int16_t temp = 0; // 用于存放临时的icon的x坐标
    TitlePage *tp = (TitlePage *)page_addr;
    // 计算动画参数
    WouoUI_Animation(&p_cur_ui->tp_var.iconX, p_cur_ui->upara->ani_param[TILE_ANI], p_cur_ui->time,&(p_cur_ui->anim_is_finish));   // 图标x坐标
    WouoUI_Animation(&p_cur_ui->tp_var.iconY, p_cur_ui->upara->ani_param[TILE_ANI], p_cur_ui->time,&(p_cur_ui->anim_is_finish));   // 图标y坐标
    WouoUI_Animation(&p_cur_ui->tp_var.barX, p_cur_ui->upara->ani_param[TILE_ANI], p_cur_ui->time,&(p_cur_ui->anim_is_finish));   // 装饰条x坐标
    WouoUI_Animation(&p_cur_ui->tp_var.titleY, p_cur_ui->upara->ani_param[TAG_ANI], p_cur_ui->time,&(p_cur_ui->anim_is_finish)); // 文字y坐标
    // 绘制title
    String show_str = tp->option_array[tp->select_item].text;
    int16_t start_x = MAX(WOUOUI_MIDDLE_H - WouoUI_GetStrWidth(&(show_str[2]), TILE_B_TITLE_FONT)/2, TILE_BAR_W); //字符串起始位置最左侧只能是BAR的位置
    WouoUI_CanvasDrawStr(&(p_cur_ui->w_all), start_x, p_cur_ui->tp_var.titleY.pos_cur, TILE_B_TITLE_FONT, (uint8_t *)&(show_str[2]));
    // 绘制装饰条
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), p_cur_ui->tp_var.barX.pos_cur, TILE_BAR_U,TILE_BAR_W, TILE_BAR_H, 0);
    for (uint8_t i = 0; i < tp->item_num; i++) {
        if (p_cur_ui->upara->ufd_param[TILE_UFD]) // 从第一个选项开始展开，最终保持选中在中间
            temp = ((WOUOUI_BUFF_WIDTH - TILE_ICON_W) >> 1) + i * p_cur_ui->tp_var.iconX.pos_cur - TILE_ICON_S * tp->select_item;
        else // 保证选中的选项在中间，向两侧展开
            temp = ((WOUOUI_BUFF_WIDTH - TILE_ICON_W) >> 1) + (i - tp->select_item) * p_cur_ui->tp_var.iconX.pos_cur;
        WouoUI_CanvasDrawBMP(&(p_cur_ui->w_all), temp, (int16_t)(p_cur_ui->tp_var.iconY.pos_cur), TILE_ICON_W, TILE_ICON_H, tp->icon_array[i], 1);
    }
    if (p_cur_ui->tp_var.iconX.pos_cur == p_cur_ui->tp_var.iconX.pos_tgt) {
        ret = true;
        p_cur_ui->tp_var.iconX.pos_cur = p_cur_ui->tp_var.iconX.pos_tgt = -1 * tp->select_item * TILE_ICON_S;
    }
    return ret;
}
void WouoUI_TitlePageShow(PageAddr page_addr)
{
    TitlePage *tp = (TitlePage *)page_addr;
    // 计算动画参数
    WouoUI_Animation(&p_cur_ui->tp_var.iconX, p_cur_ui->upara->ani_param[TILE_ANI], p_cur_ui->time,&(p_cur_ui->anim_is_finish));   // 图标x坐标
    WouoUI_Animation(&p_cur_ui->tp_var.barX, p_cur_ui->upara->ani_param[TILE_ANI], p_cur_ui->time,&(p_cur_ui->anim_is_finish));   // 装饰条x坐标
    WouoUI_Animation(&p_cur_ui->tp_var.titleY, p_cur_ui->upara->ani_param[TAG_ANI], p_cur_ui->time,&(p_cur_ui->anim_is_finish)); // 文字y坐标
    // 绘制title
    String show_str = tp->option_array[tp->select_item].text;
    int16_t start_x = MAX(WOUOUI_MIDDLE_H - WouoUI_GetStrWidth(&(show_str[2]), TILE_B_TITLE_FONT)/2, TILE_BAR_W); //字符串起始位置最左侧只能是BAR的位置
    p_cur_ui->tp_var.title_ss.str = &(show_str[2]);
    p_cur_ui->tp_var.title_ss.canvas.start_x = start_x;
    p_cur_ui->tp_var.title_ss.canvas.start_y = p_cur_ui->tp_var.titleY.pos_cur;
    WouoUI_CanvasDrawSlideStr(&(p_cur_ui->tp_var.title_ss),0,TILE_B_TITLE_FONT);
    if(p_cur_ui->anim_is_finish && !(p_cur_ui->tp_var.title_ss.slide_is_finish)){ //anim结束，且单次滚动没有完成就使能滚动
        p_cur_ui->tp_var.title_ss.slide_enable = true;
    }
    // 绘制装饰条
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), p_cur_ui->tp_var.barX.pos_cur, TILE_BAR_U,TILE_BAR_W, TILE_BAR_H, 0);
    for (uint8_t i = 0; i < tp->item_num; i++) // 过度动画完成后一般选择时的切换动画
    WouoUI_CanvasDrawBMP(&(p_cur_ui->w_all), (WOUOUI_BUFF_WIDTH - TILE_ICON_W) / 2 + (int16_t)(p_cur_ui->tp_var.iconX.pos_cur) + i * TILE_ICON_S,
                    TILE_ICON_U, TILE_ICON_W, TILE_ICON_H, tp->icon_array[i], 1);
    p_cur_ui->slide_is_finish = !(p_cur_ui->tp_var.title_ss.slide_enable);
    //使用enable而不是is_finish是因为enable时一直在滚动，滚动结束才会置false，而且每次reset enable都是false 
    //记录这个页面中所有slide动画的结果&&,这个必须放在每个show页面最后才能记录下show中所有动画的结果
}
void WouoUI_TitlePageIndicatorCtrl(PageAddr page_addr)
{
    // indicator
    p_cur_ui->indicator.x.pos_tgt = WOUOUI_MIDDLE_H - TILE_ICON_W/2 - TILE_ICON_IND_L;
    p_cur_ui->indicator.y.pos_tgt = TILE_ICON_U - TILE_ICON_IND_U;
    p_cur_ui->indicator.w.pos_tgt = TILE_ICON_IND_W;
    p_cur_ui->indicator.h.pos_tgt = TILE_ICON_IND_H; //目标值的这部分赋值应该放在InParaInti中或者ShowInit中更合适
    WouoUI_GraphSetPenColor(2); // 反色绘制
    WouoUI_CanvasDrawBoxRightAngle(&(p_cur_ui->w_all), p_cur_ui->indicator.x.pos_cur, p_cur_ui->indicator.y.pos_cur,
                                      p_cur_ui->indicator.w.pos_cur, p_cur_ui->indicator.h.pos_cur, TILE_ICON_IND_SL);
    WouoUI_GraphSetPenColor(1); // 恢复实色绘制
}
bool WouoUI_TitlePageReact(PageAddr page_addr)
{
    bool ret = false; //返回true时会触发UI切换状态
    TitlePage *tp = (TitlePage *)page_addr;
    InputMsg msg = WouoUI_MsgQueRead(&(p_cur_ui->msg_queue));
    //只要title页面有输入msg，滚动就重置(防止破坏动画)(可以只判断up/down,click和return在INparaInit中重置就可以)
    if(tp->page.auto_deal_with_msg){ //使能自动处理msg的话
        switch (msg) { 
            case msg_up:
            case msg_left: //上和左兼容
                WouoUI_TitlePageLastItem(tp);
            break;
            case msg_down:
            case msg_right: //下和右兼容
                WouoUI_TItlePageNextItem(tp);
            break;
            case msg_click: break;
            case msg_return:
                WouoUI_PageReturn(page_addr);
                ret = true; //通知UI切换状态
            break;
            default :break;
        }
        if(msg != msg_none && NULL != tp->page.cb) //任何输入的有效信息都会调用回调函数
            tp->page.cb(&(tp->page), msg); //自动处理时,会忽略回调函数返回值(title内只有msg_return输入时才会返回)
    }else {
        if(msg != msg_none && NULL != tp->page.cb){
            ret = tp->page.cb(&(tp->page), msg); //使用者在回调函数返回true,表示退出这个页面
            if(ret == true)WouoUI_PageReturn(page_addr);
        }
    }
    return ret;
}
void WouoUI_TitlePageInit(
    TitlePage *title_page,  // 磁贴页面对象
    uint8_t item_num,       // 选项个数，需与title数组大小，icon数组大小一致
    Option *option_array,   // 整个页面的选项数组(数组大小需与item_num一致)
    Icon *icon_array,       // 整个页面的icon数组(数组大小需与item_num一致)
    CallBackFunc call_back) // 回调函数，参数为确认选中项index（1-256）0表示未确认哪个选项
{
    if(TILE_MACRO_ASSERT)WOUOUI_LOG_W("the size of fnot large than the hight of bar in TitlePage");
    title_page->page.page_type = type_title;
    WouoUI_PageInit((PageAddr)title_page, call_back);
    title_page->page.methods = &(p_cur_ui->tp_mth);// 关联处理函数(方法)
    title_page->select_item = 0;
    title_page->item_num = item_num;
    title_page->option_array = option_array;
    title_page->icon_array = icon_array;
    for (uint8_t i = 0; i < title_page->item_num; i++){
        title_page->option_array[i].order = i; // 选项序号标号
        if(NULL == title_page->option_array[i].text){
            //text不能没有设置，因为在darwstr之前没有检查text文本是否为NULL，这里LOG_E提示
            title_page->option_array[i].text = (char*)WOUOUI_WIN_TXT_DEFAULT; //使用默认文本提示noset
            WOUOUI_LOG_E("The text of %d th item in TitlePgae NO set!!!",i);    
        }
    }
    WouoUI_TitlePageInParaInit((PageAddr)title_page); //做一次进入参数初始化，因为默认UI中没有设置类变量的值
}
//Title提供的接口函数
void WouoUI_TitlePageLastItem(TitlePage *tp)
{
    //动画参数重置
    p_cur_ui->tp_var.barX.pos_cur = -TILE_BAR_W;
    p_cur_ui->tp_var.titleY.pos_cur = WOUOUI_BUFF_HEIGHT;
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->tp_var.title_ss));
    if (tp->select_item > 0) { // 不是第一个选项
        tp->select_item--;
        p_cur_ui->tp_var.iconX.pos_tgt += TILE_ICON_S;
        p_cur_ui->indicator.x.pos_cur += (TILE_ICON_W >> 1); //图标移动时从上一个中心移动到这一次的边角
    } else {                                        // 是第一个选项
        if (p_cur_ui->upara->loop_param[TILE_LOOP]) // 开启循环的话
        {
            tp->select_item = tp->item_num - 1;
            p_cur_ui->tp_var.iconX.pos_tgt = -1 * TILE_ICON_S * (tp->item_num - 1);
            p_cur_ui->indicator.x.pos_cur -= (TILE_ICON_W >> 1);
        }
    }
}
void WouoUI_TItlePageNextItem(TitlePage *tp)
{
    //动画参数重置
    p_cur_ui->tp_var.barX.pos_cur = -TILE_BAR_W;
    p_cur_ui->tp_var.titleY.pos_cur = WOUOUI_BUFF_HEIGHT;
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->tp_var.title_ss));
    if (tp->select_item < (tp->item_num - 1)) { // 不是最后一个选项
        tp->select_item++;
        p_cur_ui->tp_var.iconX.pos_tgt -= TILE_ICON_S;
        p_cur_ui->indicator.x.pos_cur -= (TILE_ICON_W >> 1);
    } else { // 是最后一个选项
        if (p_cur_ui->upara->loop_param[TILE_LOOP]) {
            tp->select_item = 0;
            p_cur_ui->tp_var.iconX.pos_tgt = 0;
            p_cur_ui->indicator.x.pos_cur += (TILE_ICON_W >> 1);
        }
    }
}


//--------List相关函数
static void WouoUI_ListAuotCanvasDrawLineTailValTxt(Option *item, Canvas* p_canvas_txt, Canvas* p_canvas_val, char* val_buff, uint8_t select_order)
{   //这个函数纯粹因为用的次数多，分离出来的，没有什么特殊作用
    int16_t val_str_len = (int16_t)WouoUI_GetStrWidth(val_buff, LIST_TEXT_FONT);
    int16_t txt_str_len = (int16_t)WouoUI_GetStrWidth(item->text, LIST_TEXT_FONT);
    if(txt_str_len + LIST_TEXT_L_S*2 + LIST_TEXT_R_S + val_str_len + LIST_IND_VAL_S < WOUOUI_BUFF_WIDTH){
        //两个字符串的宽度之和不超过屏幕宽度，则按长短调整
        p_canvas_txt->w = txt_str_len;
        p_canvas_val->start_x = WOUOUI_BUFF_WIDTH - val_str_len - LIST_TEXT_R_S; //放得下的时候值靠右侧放
        p_canvas_val->w = val_str_len;
    }else if(val_str_len < LIST_VAL_MAX_LEN){
        //如果长度之和超过屏幕宽度，但值放得下，则值优先靠右侧放
        p_canvas_val->start_x = WOUOUI_BUFF_WIDTH - val_str_len - LIST_TEXT_R_S; //放得下的时候值靠右侧放
        p_canvas_txt->w = p_canvas_val->start_x - 2*LIST_TEXT_L_S - LIST_IND_VAL_S;
        p_canvas_val->w = val_str_len;
    }
    if(item->order == select_order){
        p_cur_ui->lp_var.opt_val_ss.canvas = *p_canvas_val;
        p_cur_ui->lp_var.opt_val_ss.str = val_buff;
        WouoUI_CanvasDrawSlideStr(&(p_cur_ui->lp_var.opt_val_ss), 0, LIST_TEXT_FONT);
        if(p_cur_ui->anim_is_finish && !(p_cur_ui->lp_var.opt_val_ss.slide_is_finish)){ 
            p_cur_ui->lp_var.opt_val_ss.slide_enable = true;//anim结束，且单次滚动没有完成就使能滚动
        }
    }
    else WouoUI_CanvasDrawStr(p_canvas_val, 0, 0, LIST_TEXT_FONT, (uint8_t *)val_buff);
}

static void WouoUI_ListDrawText_CheckBox(int16_t start_y, Option *item, uint8_t select_order)
{
    Canvas canvas_txt={.start_x = LIST_TEXT_L_S,
                       .start_y = (int16_t)(start_y + LIST_TEXT_U_S),
                       .w = LIST_TEXT_MAX_LEN,
                       .h = (int16_t)(LIST_TEXT_FONT.Height)};
    Canvas canvas_val={.start_x = LIST_TEXT_L_S*2 + LIST_TEXT_MAX_LEN + LIST_IND_VAL_S,
                       .start_y = (int16_t)(start_y + LIST_TEXT_U_S),
                       .w = LIST_VAL_MAX_LEN,
                       .h = (int16_t)(LIST_TEXT_FONT.Height)};
    // 绘制表尾
    char val_buff[LIST_VAL_BUFF_SIZE];
    if(NULL != strchr(LIST_LINETAIL_VAL_PREFIX, item->text[0])){
        ui_itoa_str(item->val, val_buff); //这儿应该加上长度警告！！！
        WouoUI_ListAuotCanvasDrawLineTailValTxt(item, &canvas_txt, &canvas_val,val_buff,select_order);
    }else if(NULL != strchr(LIST_LINETAIL_SPIN_PREFIX, item->text[0])){
        ui_ftoa_f_str(item->val, item->decimalNum, val_buff); //这儿应该加上长度警告！！！
        WouoUI_ListAuotCanvasDrawLineTailValTxt(item, &canvas_txt, &canvas_val,val_buff,select_order);
    }else if(NULL != strchr(LIST_LINETAIL_CONF_PREFIX, item->text[0])){// 如果是二值选框// 如果是二值确认弹窗
        canvas_txt.w = MIN(WouoUI_GetStrWidth(item->text,LIST_TEXT_FONT),\
                           WOUOUI_BUFF_WIDTH-CHECK_BOX_R_S-CHECK_BOX_F_W-LIST_TEXT_L_S*2-LIST_IND_VAL_S); //除确认框外的宽度，其他都算在文字宽度中
        WouoUI_CanvasDrawRBoxEmpty(&(p_cur_ui->w_all), WOUOUI_BUFF_WIDTH - CHECK_BOX_R_S - CHECK_BOX_F_W,
                              start_y + CHECK_BOX_U_S, CHECK_BOX_F_W, CHECK_BOX_F_H, CHECK_BOX_R);
        if (item->val != 0) // 非0即是选中，画框内的点(这里加的常量1是外框的线条宽)
            WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), WOUOUI_BUFF_WIDTH - CHECK_BOX_R_S - CHECK_BOX_F_W + CHECK_BOX_D_S +1,
                             start_y + CHECK_BOX_U_S + CHECK_BOX_D_S+1, CHECK_BOX_D_W, CHECK_BOX_D_H, CHECK_BOX_D_R);
    }else if(NULL != strchr(LIST_LINETAIL_TXT_PREFIX, item->text[0])){// 如果是列表弹窗
        if(NULL != item->content){
            WouoUI_ListAuotCanvasDrawLineTailValTxt(item, &canvas_txt, &canvas_val,item->content,select_order);
        }
    }else 
        canvas_txt.w = MIN(WouoUI_GetStrWidth(item->text,LIST_TEXT_FONT), WOUOUI_BUFF_WIDTH-LIST_TEXT_R_S-LIST_TEXT_L_S*2);
    // 绘制文字
    if(item->order == select_order){
        char temp[3] = {item->text[0],item->text[1],0}; //前面的标识符
        p_cur_ui->lp_var.indicator_w_temp = LIST_TEXT_L_S*2 + canvas_txt.w; //计算出每一项的宽度后，顺便给inductor的宽度赋值
        WouoUI_CanvasDrawStr(&(canvas_txt), 0, 0, LIST_TEXT_FONT, (uint8_t *)temp);
        canvas_txt.start_x += LIST_TEXT_FONT.Width*2;
        canvas_txt.w -= LIST_TEXT_FONT.Width*2; //去掉前面标识符的长度
        p_cur_ui->lp_var.opt_text_ss.canvas = canvas_txt; 
        p_cur_ui->lp_var.opt_text_ss.str = &(item->text[2]); 
        WouoUI_CanvasDrawSlideStr(&(p_cur_ui->lp_var.opt_text_ss), 0, LIST_TEXT_FONT);
        if(p_cur_ui->anim_is_finish && !(p_cur_ui->lp_var.opt_text_ss.slide_is_finish)){ 
            p_cur_ui->lp_var.opt_text_ss.slide_enable = true;//anim结束，且单次滚动没有完成就使能滚动
        }
    }
    else WouoUI_CanvasDrawStr(&(canvas_txt), 0, 0, LIST_TEXT_FONT, (uint8_t *)(item->text));
}

void WouoUI_ListPageInParaInit(PageAddr page_addr)
{
    UNUSED_PARAMETER(page_addr);
    p_cur_ui->lp_var.optInt.pos_cur = 0;
    p_cur_ui->lp_var.optInt.pos_tgt = LIST_LINE_H;
    p_cur_ui->scrollBar.display = true;
    //step其实可以移到init中，因为这个值在整个状态机中基本是是不变的
    p_cur_ui->lp_var.opt_text_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[LIST_TEXT_SSS];
    p_cur_ui->lp_var.opt_val_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[LIST_TEXT_SSS];
    p_cur_ui->lp_var.opt_text_ss.step = p_cur_ui->upara->slidestrstep_param[LIST_TEXT_SSS];
    p_cur_ui->lp_var.opt_val_ss.step = p_cur_ui->upara->slidestrstep_param[LIST_VAL_SSS];
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->lp_var.opt_text_ss));
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->lp_var.opt_val_ss));
}

bool WouoUI_ListPageIn(PageAddr page_addr)
{
    bool ret = false; //默认动画没有完成
    ListPage *lp = (ListPage *)page_addr;
    int16_t item_y = 0; // 用于临时存放列表每一项的y坐标
    const uint8_t list_line_h = LIST_LINE_H; //因为行高是用宏计算的，这里用变量保存不用每次都计算
    WouoUI_Animation(&p_cur_ui->lp_var.optInt, p_cur_ui->upara->ani_param[LIST_ANI], p_cur_ui->time, &(p_cur_ui->anim_is_finish)); // 文字y坐标
    for (uint8_t i = 0; i < (lp->item_num); i++) {
        if (p_cur_ui->upara->ufd_param[LIST_UFD]) // 从头展开
            item_y = i * (p_cur_ui->lp_var.optInt.pos_cur) - list_line_h * (lp->select_item) + lp->ind_y_tgt;
        else // 选中项展开
            item_y = (i - (lp->select_item)) * (p_cur_ui->lp_var.optInt.pos_cur) + lp->ind_y_tgt;
        WouoUI_ListDrawText_CheckBox(item_y, &(lp->option_array[i]),lp->select_item);
        }
    if (p_cur_ui->lp_var.optInt.pos_cur == p_cur_ui->lp_var.optInt.pos_tgt) { // 动画抵达目标位置
        ret = true;
        p_cur_ui->lp_var.optInt.pos_cur = p_cur_ui->lp_var.optInt.pos_tgt = -list_line_h * (lp->select_item) + lp->ind_y_tgt;
        // 过度动画完成后optInt不再表示行间距了，而是表示第一个选项顶部的位置与屏幕顶部的距离(而且是个负数)
    }
    return ret;
}

void WouoUI_ListPageShow(PageAddr page_addr)
{
    ListPage *lp = (ListPage *)page_addr;
    int16_t item_y = 0; // 用于临时存放列表每一项的y坐标
    const uint8_t list_line_h = LIST_LINE_H; //因为行高是用宏计算的，这里用变量保存不用每次都计算
    // 计算动画过渡值
    WouoUI_Animation(&p_cur_ui->lp_var.optInt, p_cur_ui->upara->ani_param[LIST_ANI], p_cur_ui->time, &(p_cur_ui->anim_is_finish)); // 文字y坐标
    for (uint8_t i = 0; i < (lp->item_num); i++) {
        item_y = list_line_h * i + (p_cur_ui->lp_var.optInt.pos_cur);
        WouoUI_ListDrawText_CheckBox(item_y, &(lp->option_array[i]),lp->select_item);
    }
    p_cur_ui->slide_is_finish = !(p_cur_ui->lp_var.opt_text_ss.slide_enable) && \
                                !(p_cur_ui->lp_var.opt_val_ss.slide_enable); //记录这个页面中所有slide动画的结果&&
}

void WouoUI_ListPageIndicatorCtrl(PageAddr page_addr)
{
    ListPage *lp = (ListPage *)page_addr;
    // indicator
    p_cur_ui->indicator.x.pos_tgt = 0;
    p_cur_ui->indicator.y.pos_tgt = lp->ind_y_tgt;
    p_cur_ui->indicator.w.pos_tgt = p_cur_ui->lp_var.indicator_w_temp;
    p_cur_ui->indicator.h.pos_tgt = LIST_LINE_H;
    WouoUI_GraphSetPenColor(2); // 反色绘制
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), p_cur_ui->indicator.x.pos_cur, p_cur_ui->indicator.y.pos_cur,
                             p_cur_ui->indicator.w.pos_cur, p_cur_ui->indicator.h.pos_cur, LIST_IND_BOX_R);
    WouoUI_GraphSetPenColor(1); // 恢复实色绘制
}

void WouoUI_ListPageScrollBarCtrl(PageAddr page_addr)
{
    ListPage *lp = (ListPage *)page_addr;
    // scrollBar
    p_cur_ui->scrollBar.y.pos_tgt = (int16_t)(lp->select_item * WOUOUI_BUFF_HEIGHT / (lp->item_num - 1)) + 1;
    WouoUI_CanvasDrawLine_H(&(p_cur_ui->w_all), WOUOUI_BUFF_WIDTH - LIST_BAR_W, WOUOUI_BUFF_WIDTH, 0);
    WouoUI_CanvasDrawLine_H(&(p_cur_ui->w_all), WOUOUI_BUFF_WIDTH - LIST_BAR_W, WOUOUI_BUFF_WIDTH, WOUOUI_BUFF_HEIGHT - 1);
    WouoUI_CanvasDrawLine_V(&(p_cur_ui->w_all), WOUOUI_BUFF_WIDTH - ((LIST_BAR_W / 2) + 1), 0, WOUOUI_BUFF_HEIGHT);
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), WOUOUI_BUFF_WIDTH - LIST_BAR_W, 0, LIST_BAR_W, p_cur_ui->scrollBar.y.pos_cur, 0);
}

bool WouoUI_ListPageReact(PageAddr page_addr)
{
    bool ret = false;
    ListPage *lp = (ListPage *)page_addr;
    String selcet_string = NULL;
    InputMsg msg = WouoUI_MsgQueRead(&(p_cur_ui->msg_queue));
    if(lp->page.auto_deal_with_msg){
        switch (msg){
            case msg_up:
            case msg_left: //左和上兼容
                WouoUI_ListPageLastItem(lp);
                break;
            case msg_down:
            case msg_right: //下和右兼容
                WouoUI_ListPageNextItem(lp);
                break;
            case msg_click : //如果没有使用自动处理msg的话,二值框的值和是否是单选项肯定也是用户自己处理嘞
                selcet_string = lp->option_array[lp->select_item].text;
                if (selcet_string[0] == ((char*)LIST_LINETAIL_CONF_PREFIX)[0]) { // 二值选框
                        lp->option_array[lp->select_item].val = !lp->option_array[lp->select_item].val; // 将对应值取反
                } 
                // 单选框页面
                if (lp->page_setting == Setting_radio) { //如果列表页面是单页面
                    for (uint8_t i = 0; i < lp->item_num; i++) {
                        if (NULL != strchr(LIST_LINETAIL_CONF_PREFIX, lp->option_array[i].text[0])) {
                            if (i != lp->select_item)
                                lp->option_array[i].val = false;
                            else
                                lp->option_array[i].val = true; // 确保单选
                        }
                    }
                }
            break;
            case msg_return:
                WouoUI_PageReturn(page_addr);
                ret = true; //通知UI切换状态
                break;
            default: break;
        }
        if (msg_none != msg && NULL != lp->page.cb) //任何输入的有效信息都会调用回调函数
            lp->page.cb(&(lp->page), msg);
    }else {
        if (msg_none != msg && NULL != lp->page.cb){ //任何输入的有效信息都会调用回调函数
            ret = lp->page.cb(&(lp->page), msg); //使用者在回调函数返回true,表示退出这个页面
            if(ret == true)WouoUI_PageReturn(page_addr);
        }
    }
    return ret;
}

void WouoUI_ListPageInit(    
    ListPage *lp,             // 列表页面对象
    uint8_t item_num,         // 选项个数，
    Option *option_array,     // 整个页面的选项数组(数组大小需与item_num一致)
    PageSetting page_setting, // 页面设置
    CallBackFunc call_back    // 回调函数，参数为确认选中项index（1-256）0表示未确认哪个选项)
){
    if(LIST_MACRO_ASSERT)WOUOUI_LOG_W("The chamfer of the check box is too large, exceeding its width.");
    lp->page.page_type = type_list;
    lp->page.methods = &(p_cur_ui->lp_mth);
    WouoUI_PageInit((PageAddr)lp, call_back);
    lp->page_setting = page_setting;
    lp->item_num = item_num;
    lp->select_item = 0;
    lp->option_array = option_array;
    lp->line_n = WOUOUI_BUFF_HEIGHT / LIST_LINE_H;
    for (uint8_t i = 0; i < lp->item_num; i++)
    {
        lp->option_array[i].order = i; // 选项序号标号
        if(NULL == lp->option_array[i].text){
            //text不能没有设置，因为在darwstr之前没有检查text文本是否为NULL，这里LOG_E提示
            lp->option_array[i].text = (char*)WOUOUI_WIN_TXT_DEFAULT; //使用默认文本提示noset
            WOUOUI_LOG_E("The text of %d th item in ListPgae NO set!!!", i);        
        }
    }
}
//list的接口函数
Option* WouoUI_ListTitlePageGetSelectOpt(const Page* cur_page_addr)
{
    PageType pt = WouoUI_CheckPageType(cur_page_addr); 
    Option* opt = NULL;
    switch (pt)
    {
        case type_title: //如果是title页面
            opt = &(((TitlePage*)cur_page_addr)->option_array[((TitlePage*)cur_page_addr)->select_item]);
            break;
        case type_list: //如果是list页面
            opt = &(((ListPage*)cur_page_addr)->option_array[((ListPage*)cur_page_addr)->select_item]);
            break;
        default:
            WOUOUI_LOG_E("The Page type is not title or list, please check the input page");
            break;
    }
    return opt;
}
void WouoUI_ListPageLastItem(ListPage *lp)
{   //重置动画参数
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->lp_var.opt_text_ss));
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->lp_var.opt_val_ss));
    const uint8_t list_line_h = LIST_LINE_H; //因为行高是用宏计算的，这里用变量保存不用每次都计算
    if (lp->select_item == 0) {                                                               // 选中第一个的话
        if (p_cur_ui->upara->loop_param[LIST_LOOP]) {                                         // 同时loop参数开的话，从顶部滑动到底部
            lp->select_item = lp->item_num - 1;                                               // 选中最后一个
            if (lp->item_num > lp->line_n) {                                                  // 数目超出一页的最大数目
                lp->ind_y_tgt = WOUOUI_BUFF_HEIGHT - list_line_h;                             // 更改box到最底
                p_cur_ui->lp_var.optInt.pos_tgt = WOUOUI_BUFF_HEIGHT - (lp->item_num) * list_line_h; // 更改文字到最底
            } else                                                                            // 没有超出数目则是到最后一个
                lp->ind_y_tgt = (lp->item_num - 1) * list_line_h;
        }
    } else {                                                                              // 没有选中第一个
        lp->select_item--;                                                                // 选中减1
        if ((lp->select_item - 1) < -((p_cur_ui->lp_var.optInt.pos_tgt) / list_line_h)) { // 光标盒子到页面顶了
            if (!(WOUOUI_BUFF_HEIGHT % list_line_h))                                      // 上面剩余完整的行
                p_cur_ui->lp_var.optInt.pos_tgt += list_line_h;                           // 文字上移
            else {                                                                        // 上面的行不完整(list_line_h该项用于页面和行高非整除时)
                if (lp->ind_y_tgt == WOUOUI_BUFF_HEIGHT - list_line_h * lp->line_n) {            // 文字往下走一行，且光标盒子置于0处，把上面的非整行去掉
                    p_cur_ui->lp_var.optInt.pos_tgt += (lp->line_n + 1) * list_line_h - WOUOUI_BUFF_HEIGHT;
                    lp->ind_y_tgt = 0;
                } else if (lp->ind_y_tgt == list_line_h)
                    lp->ind_y_tgt = 0; // 上面整行直接移动光标盒子
                else
                    p_cur_ui->lp_var.optInt.pos_tgt += list_line_h; // 顶页整行，文字直接往下走
            }
        } else                            // 光标盒子没到页面顶
            lp->ind_y_tgt -= list_line_h; // 直接光标盒子往上走即可。
    }
}
void WouoUI_ListPageNextItem(ListPage* lp)
{   //重置动画参数
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->lp_var.opt_text_ss));
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->lp_var.opt_val_ss));
    const uint8_t list_line_h = LIST_LINE_H; //因为行高是用宏计算的，这里用变量保存不用每次都计算
    if (lp->select_item == (lp->item_num) - 1) {                   // 到最后一个选项了
        if (p_cur_ui->upara->loop_param[LIST_LOOP]) {              // loop开关开,全部回到顶部
            p_cur_ui->lp_var.optInt.pos_tgt = 0;
            lp->select_item = 0;
            lp->ind_y_tgt = 0;
        }
    } else { // 不是最后一个选项
        lp->select_item++;
        if ((lp->select_item + 1) > ((lp->line_n) - (p_cur_ui->lp_var.optInt.pos_tgt) / list_line_h)) { // 光标到页面底
            if (!(WOUOUI_BUFF_HEIGHT % list_line_h))                                                           // 上面剩余完整的行
                p_cur_ui->lp_var.optInt.pos_tgt -= list_line_h;                                         // 文字下移
            else {                                                                                      // 非整行的情况
                if (lp->ind_y_tgt == list_line_h * (lp->line_n - 1)) {
                    p_cur_ui->lp_var.optInt.pos_tgt -= (lp->line_n + 1) * list_line_h - WOUOUI_BUFF_HEIGHT;
                    // 取出要向下移动的距离，对y_tgt来说就是向上移动的距离
                    lp->ind_y_tgt = WOUOUI_BUFF_HEIGHT - list_line_h;
                } else if (lp->ind_y_tgt == WOUOUI_BUFF_HEIGHT - list_line_h * 2) // 这种情况什么时候会出现呢嗯?
                    lp->ind_y_tgt = WOUOUI_BUFF_HEIGHT - list_line_h;
                else
                    p_cur_ui->lp_var.optInt.pos_tgt -= list_line_h;
            }
        } else
            lp->ind_y_tgt += list_line_h;
    }
}

//--------WavePage相关方法和函数
static int16_t _roundToNearestTen(int16_t value) {
    int16_t abs_val = abs(value);
    int16_t result;
    if (abs_val % 10 <= 5)
        result = (abs_val / 10) * 10;
    else
        result = ((abs_val / 10) + 1) * 10;
    return value < 0 ? -result : result;
}

static bool _WouoUI_indexIsInShowRange(uint16_t idx, uint16_t idx_show_head, uint16_t idx_show_tail) {
    if (idx_show_head < idx_show_tail) // 队列没有回绕
        return (idx >= idx_show_head && idx < idx_show_tail);
    else if (idx_show_head > idx_show_tail) // 队列回绕
        return (idx >= idx_show_head || idx < idx_show_tail);
    else
        return false; //队列空的时候，也不在范围内
}

static void _WouoUI_WaveUpdateRange(WaveData *wd) {
    bool maxFlag = false;          // 最大值改变标志
    bool minFlag = false;          // 最小值改变标志
    uint16_t idx_show_head = wd->idx_show_head, idx_show_tail = wd->idx_show_tail;
    if (!_WouoUI_indexIsInShowRange(wd->idx_show_max,idx_show_head,idx_show_tail)) { //如果最大值已经移出展示范围
        int16_t max = -32768; //从新计算一次最大值(用最小值作为比较初值，省去了第一次判断，还是很巧妙的)
        for (uint16_t i = idx_show_head; i != idx_show_tail ; i++,i %= WAVE_DEPTH)
            if (wd->data[i] >= max) {
                max = wd->data[i];
                 wd->idx_show_max = i;
            }
        maxFlag = true;
    }
    if (!_WouoUI_indexIsInShowRange(wd->idx_show_min,idx_show_head,idx_show_tail)) { //如果最小值已经移出展示范围
        int16_t min = 32767;   //从新计算一次最小值
        for (uint16_t i = idx_show_head; i != idx_show_tail ; i++,i %= WAVE_DEPTH)
            if (wd->data[i] <= min) {
                min = wd->data[i];
                wd->idx_show_min = i;
            }
        minFlag = true;
    }
    //这个函数只能用于更新左右移动一步的情况，因为下面只有判断队列两边进入的一个值
    if (wd->data[idx_show_head] >= wd->data[wd->idx_show_max]) { // 新加入的数据大于最大值，更新最大值
        wd->idx_show_max = idx_show_head;// 新加入的数据大于最大值，更新最大值
        maxFlag = true;
    }
    if (wd->data[idx_show_head] <= wd->data[wd->idx_show_min]) { // 新加入的数据小于最大值，更新最大值
        wd->idx_show_min = idx_show_head;
        minFlag = true;
    }
    if (wd->data[(idx_show_tail+WAVE_DEPTH-1)%WAVE_DEPTH] >= wd->data[wd->idx_show_max]) { // 新加入的数据大于最大值，更新最大值
        wd->idx_show_max = (idx_show_tail+WAVE_DEPTH-1)%WAVE_DEPTH;
        maxFlag = true;
    }
    if (wd->data[(idx_show_tail+WAVE_DEPTH-1)%WAVE_DEPTH] <= wd->data[wd->idx_show_min]) { // 新加入的数据小于最大值，更新最大值
        wd->idx_show_min = (idx_show_tail+WAVE_DEPTH-1)%WAVE_DEPTH;
        minFlag = true;
    }
    if (maxFlag) // 最大值改变
        wd->rangeMax = _roundToNearestTen(wd->data[wd->idx_show_max] + 10);
    if (minFlag) // 最小值改变
        wd->rangeMin = _roundToNearestTen(wd->data[wd->idx_show_min] - 10);
}

static void _WouoUI_WaveAnimParaReaset(void){ //这个函数抽象出来仅仅是因为用得多
    p_cur_ui->wt_var.text_y.pos_cur = WOUOUI_BUFF_HEIGHT;
    p_cur_ui->wt_var.y_axis_val_x.pos_cur = WOUOUI_BUFF_WIDTH; //下面两个可以放全局变量里初始化其实
    p_cur_ui->wt_var.text_y.pos_tgt = WAVE_BOX_U_S + WAVE_BOX_H + WAVE_BOX_D_S;
    p_cur_ui->wt_var.y_axis_val_x.pos_tgt = WAVE_BOX_L_S + WAVE_BOX_W + 1;
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->wt_var.title_ss));
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->wt_var.val_ss));
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->wt_var.max_val_ss));
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->wt_var.mid_val_ss));
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->wt_var.min_val_ss));
}


void WouoUI_WavePageInParaInit(PageAddr page_addr)
{
    UNUSED_PARAMETER(page_addr);
    p_cur_ui->wt_var.title_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WAVE_TEXT_SSS];
    p_cur_ui->wt_var.val_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WAVE_TEXT_SSS];
    p_cur_ui->wt_var.max_val_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WAVE_VAL_SSS];
    p_cur_ui->wt_var.mid_val_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WAVE_VAL_SSS];
    p_cur_ui->wt_var.min_val_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WAVE_VAL_SSS];
    p_cur_ui->wt_var.title_ss.step = p_cur_ui->upara->slidestrstep_param[WAVE_TEXT_SSS];
    p_cur_ui->wt_var.val_ss.step = p_cur_ui->upara->slidestrstep_param[WAVE_TEXT_SSS];
    p_cur_ui->wt_var.max_val_ss.step = p_cur_ui->upara->slidestrstep_param[WAVE_VAL_SSS];
    p_cur_ui->wt_var.mid_val_ss.step = p_cur_ui->upara->slidestrstep_param[WAVE_VAL_SSS];
    p_cur_ui->wt_var.min_val_ss.step = p_cur_ui->upara->slidestrstep_param[WAVE_VAL_SSS];
    _WouoUI_WaveAnimParaReaset();
}

// bool WouoUI_WavePageIn(PageAddr page_addr)
// { //文字和y轴的动画可以在过度动画做，把具体绘制部分代码分离成函数调用即可，那部分代码有点多，因为那段动画挺短的，In动画不做也行
//   //不做In动画(这个函数没啥用)直接使用默认返回true函数初始化类方法就可以了。
//     WavePage *wp = (WavePage*)page_addr;
//     bool ret = true; // 默认动画完成，返回true进入下一个状态
//     return ret;
// }
void WouoUI_WavePageShow(PageAddr page_addr)
{
    WavePage *wp = (WavePage*)page_addr;
    uint16_t wave_x = 0; // 波形点的x坐标,从1开始，因为0是边框
    int16_t data_val = 0; // 波形点的数据
    WaveData *wd = &(wp->wave_data_array[wp->wave_data_select]);
    //绘制波形(绘制选中波形数据)
    for(uint16_t j = wd->idx_show_head; j != wd->idx_show_tail; j++, j %= WAVE_DEPTH){
        wave_x++; //取点和像素计算
        data_val = wd->data[j];
        //这儿有必要做这个限制嘛？？？？因为每次更新进去，我们不应该保证wp保存的最大最小值是所有波形的最大最小值
        data_val = data_val < wd->rangeMin ? wd->rangeMin : data_val; // 限制波形数据范围
        data_val = data_val > wd->rangeMax ? wd->rangeMax : data_val; 
        p_cur_ui->wt_var.cur_y = WAVE_BOX_H + WAVE_BOX_U_S - (data_val - wd->rangeMin) * WAVE_BOX_H / (wd->rangeMax - wd->rangeMin) - 1;
        if(j == wd->idx_show_head)p_cur_ui->wt_var.last_y = p_cur_ui->wt_var.cur_y;
        //绘制波形
        switch (wd->waveType){
            case WaveType_Solid: // 实线
                if (p_cur_ui->wt_var.cur_y > (p_cur_ui->wt_var.last_y + 1))
                    WouoUI_CanvasDrawLine_V(&(p_cur_ui->w_all), WAVE_BOX_L_S + wave_x, p_cur_ui->wt_var.last_y + 1, p_cur_ui->wt_var.cur_y);
                else if (p_cur_ui->wt_var.cur_y < (p_cur_ui->wt_var.last_y - 1))
                    WouoUI_CanvasDrawLine_V(&(p_cur_ui->w_all), WAVE_BOX_L_S + wave_x, p_cur_ui->wt_var.last_y - 1, p_cur_ui->wt_var.cur_y);
                else
                    WouoUI_CanvasDrawPoint(&(p_cur_ui->w_all), WAVE_BOX_L_S + wave_x, p_cur_ui->wt_var.cur_y);
            break;
            case WaveType_Dash: // 虚线
                if (wave_x % 2 == 0){ // 间隔一个点画一个点
                    if (p_cur_ui->wt_var.cur_y > (p_cur_ui->wt_var.last_y + 1)) // data2
                        WouoUI_CanvasDrawLine_V(&(p_cur_ui->w_all), WAVE_BOX_L_S + wave_x, p_cur_ui->wt_var.last_y + 1, p_cur_ui->wt_var.cur_y);
                    else if (p_cur_ui->wt_var.cur_y < (p_cur_ui->wt_var.last_y - 1))
                        WouoUI_CanvasDrawLine_V(&(p_cur_ui->w_all), WAVE_BOX_L_S + wave_x, p_cur_ui->wt_var.last_y - 1, p_cur_ui->wt_var.cur_y);
                    else
                        WouoUI_CanvasDrawPoint(&(p_cur_ui->w_all), WAVE_BOX_L_S + wave_x, p_cur_ui->wt_var.cur_y);
                }
            break;
            case WaveType_Fill: //填充波形
                WouoUI_CanvasDrawLine_V(&(p_cur_ui->w_all), WAVE_BOX_L_S + wave_x, WAVE_BOX_U_S + WAVE_BOX_H - 1, p_cur_ui->wt_var.cur_y);
            break;
            default: break;
        }
        p_cur_ui->wt_var.last_y = p_cur_ui->wt_var.cur_y;
    }
    //绘制start/stop标志
    if(false == wd->stop_flag)WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), 1, WAVE_BOX_U_S + WAVE_BOX_H + WAVE_BOX_D_S,\
                            WAVE_STOP_SIG_HW, WAVE_STOP_SIG_HW, UINT_DIVISION_CELL(WAVE_STOP_SIG_HW,3));
    else WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), 1, WAVE_BOX_U_S + WAVE_BOX_H + WAVE_BOX_D_S,\
                            WAVE_STOP_SIG_HW, WAVE_STOP_SIG_HW, 0);
    WouoUI_Animation(&p_cur_ui->wt_var.text_y, p_cur_ui->upara->ani_param[WAVE_ANI], p_cur_ui->time, &(p_cur_ui->anim_is_finish)); // 文字y坐标
    WouoUI_Animation(&p_cur_ui->wt_var.y_axis_val_x, p_cur_ui->upara->ani_param[WAVE_ANI], p_cur_ui->time, &(p_cur_ui->anim_is_finish)); // 文字x坐标
    //绘制Title字符串
    p_cur_ui->wt_var.title_ss.canvas.start_x = WAVE_STOP_SIG_HW + 2 + WAVE_TEXT_L_S;
    p_cur_ui->wt_var.title_ss.canvas.start_y = p_cur_ui->wt_var.text_y.pos_cur;
    p_cur_ui->wt_var.title_ss.canvas.h = GET_FNOT_H(WAVE_FONT);
    p_cur_ui->wt_var.title_ss.canvas.w = WAVE_TEXT_W;
    p_cur_ui->wt_var.title_ss.str = wd->text;
    WouoUI_CanvasDrawSlideStr(&(p_cur_ui->wt_var.title_ss), 0, WAVE_FONT);
    //绘制下面的cur值
    p_cur_ui->wt_var.val_ss.str = ui_ftoa_g(wd->data[(wd->idx_show_tail+WAVE_DEPTH-1)%WAVE_DEPTH], wd->decimal_num);
    p_cur_ui->wt_var.val_ss.canvas.w = WOUOUI_BUFF_WIDTH - WAVE_TEXT_L_S - WAVE_TEXT_R_S - WAVE_TEXT_W - WAVE_STOP_SIG_HW - 2;
    p_cur_ui->wt_var.val_ss.canvas.h = GET_FNOT_H(WAVE_FONT);
    p_cur_ui->wt_var.val_ss.canvas.start_x = WAVE_TEXT_W + WAVE_STOP_SIG_HW + 2 + WAVE_TEXT_L_S + WAVE_TEXT_R_S;
    p_cur_ui->wt_var.val_ss.canvas.start_y = p_cur_ui->wt_var.text_y.pos_cur;
    WouoUI_CanvasDrawSlideStr(&(p_cur_ui->wt_var.val_ss), 0, WAVE_FONT);
    //绘制侧边的最大值和最小值和中间值
    p_cur_ui->wt_var.max_val_ss.canvas.start_x = p_cur_ui->wt_var.y_axis_val_x.pos_cur;
    p_cur_ui->wt_var.max_val_ss.canvas.start_y = WAVE_BOX_U_S;
    p_cur_ui->wt_var.max_val_ss.canvas.h = GET_FNOT_H(WAVE_FONT);
    p_cur_ui->wt_var.max_val_ss.canvas.w = WAVE_Y_AXIS_LABEL_WIDTH;
    p_cur_ui->wt_var.max_val_ss.str = ui_ftoa_g(wd->rangeMax, wd->decimal_num);
    WouoUI_CanvasDrawSlideStr(&(p_cur_ui->wt_var.max_val_ss), 0, WAVE_FONT);
    p_cur_ui->wt_var.mid_val_ss.canvas = p_cur_ui->wt_var.max_val_ss.canvas;
    p_cur_ui->wt_var.mid_val_ss.canvas.start_y = WAVE_BOX_U_S + (WAVE_BOX_H >> 1) - (WAVE_FONT.Height >> 1);
    p_cur_ui->wt_var.mid_val_ss.str = ui_ftoa_g((wd->rangeMax + wd->rangeMin)/2, wd->decimal_num);
    WouoUI_CanvasDrawSlideStr(&(p_cur_ui->wt_var.mid_val_ss), 0, WAVE_FONT);
    p_cur_ui->wt_var.min_val_ss.canvas = p_cur_ui->wt_var.max_val_ss.canvas;
    p_cur_ui->wt_var.min_val_ss.canvas.start_y = WAVE_BOX_U_S + WAVE_BOX_H - WAVE_FONT.Height;
    p_cur_ui->wt_var.min_val_ss.str = ui_ftoa_g(wd->rangeMin, wd->decimal_num);
    WouoUI_CanvasDrawSlideStr(&(p_cur_ui->wt_var.min_val_ss), 0 ,WAVE_FONT);
    if(p_cur_ui->anim_is_finish){//anim结束，且单次滚动没有完成就使能滚动
        if(!(p_cur_ui->wt_var.title_ss.slide_is_finish))p_cur_ui->wt_var.title_ss.slide_enable = true;
        if(!(p_cur_ui->wt_var.val_ss.slide_is_finish))p_cur_ui->wt_var.val_ss.slide_enable = true;
        if(!(p_cur_ui->wt_var.max_val_ss.slide_is_finish))p_cur_ui->wt_var.max_val_ss.slide_enable = true;
        if(!(p_cur_ui->wt_var.mid_val_ss.slide_is_finish))p_cur_ui->wt_var.mid_val_ss.slide_enable = true;
        if(!(p_cur_ui->wt_var.min_val_ss.slide_is_finish))p_cur_ui->wt_var.min_val_ss.slide_enable = true;
    }
    // 纵轴刻度
    uint16_t axis_tick_x = p_cur_ui->indicator.x.pos_cur + p_cur_ui->indicator.w.pos_cur;
    WouoUI_CanvasDrawPoint(&(p_cur_ui->w_all), axis_tick_x, p_cur_ui->indicator.y.pos_cur);
    WouoUI_CanvasDrawPoint(&(p_cur_ui->w_all), axis_tick_x, p_cur_ui->indicator.y.pos_cur + UINT_DIVISION_ROUND(p_cur_ui->indicator.h.pos_cur,2)-1);
    WouoUI_CanvasDrawPoint(&(p_cur_ui->w_all), axis_tick_x, p_cur_ui->indicator.y.pos_cur + p_cur_ui->indicator.h.pos_cur - 1);
    p_cur_ui->slide_is_finish = !(p_cur_ui->wt_var.title_ss.slide_enable) && \
                                !(p_cur_ui->wt_var.val_ss.slide_enable) && \
                                !(p_cur_ui->wt_var.max_val_ss.slide_enable) && \
                                !(p_cur_ui->wt_var.mid_val_ss.slide_enable) && \
                                !(p_cur_ui->wt_var.min_val_ss.slide_enable) && \
                                wd->stop_flag; //记录这个页面中所有slide动画的结果&&(同时页面必须是停止状态，不然还是通过slide_is_finish这个标志位控制is_motion_less这个标志位)
}
bool WouoUI_WavePageReact(PageAddr page_addr)
{
    bool ret = false;
    InputMsg msg = WouoUI_MsgQueRead(&(p_cur_ui->msg_queue));
    WavePage *wp = (WavePage*)page_addr;
    WaveData *wd = &(wp->wave_data_array[wp->wave_data_select]);
    if(wp->page.auto_deal_with_msg){ //自动处理msg的话
        switch (msg){
            case msg_up:
            case msg_left: //左和上兼容
                if(false == wd->stop_flag) //运动状态下,上左切换波形
                    WouoUI_WavePageShowLastWaveData(wp);
                else //停止状态下移动波形查看
                    WouoUI_WavePageLeftShiftWave(wp, wp->wave_data_select);
                break;
            case msg_down:
            case msg_right: //下和右兼容
                if(false == wd->stop_flag)
                    WouoUI_WavePageShowNextWaveData(wp);
                else 
                    WouoUI_WavePageRightShiftWave(wp, wp->wave_data_select);
                break;
            case msg_click:
                WouoUI_WavePageStopRestartWave(wp, wp->wave_data_select, !wd->stop_flag);
                break;
            case msg_return:
                WouoUI_PageReturn(page_addr);
                ret = true; //通知UI切换状态
                break;
            default: break;
        }
        if (msg_none != msg && NULL != wp->page.cb)wp->page.cb(&(wp->page), msg); //任何输入的有效信息都会调用回调函数
    } else {
        if (msg_none != msg && NULL != wp->page.cb){ //任何输入的有效信息都会调用回调函数
            ret = wp->page.cb(&(wp->page), msg); //使用者在回调函数返回true,表示退出这个页面
            if(ret == true)WouoUI_PageReturn(page_addr);
        }
    }
    return ret;
}
void WouoUI_WavePageIndicatorCtrl(PageAddr page_addr)
{
    UNUSED_PARAMETER(page_addr);
    // indicator也是波形外框
    p_cur_ui->indicator.x.pos_tgt = WAVE_BOX_L_S;
    p_cur_ui->indicator.y.pos_tgt = WAVE_BOX_U_S;
    p_cur_ui->indicator.w.pos_tgt = WAVE_BOX_W;
    p_cur_ui->indicator.h.pos_tgt = WAVE_BOX_H;
    WouoUI_CanvasDrawRBoxEmpty(&(p_cur_ui->w_all), p_cur_ui->indicator.x.pos_cur, p_cur_ui->indicator.y.pos_cur,
                                  p_cur_ui->indicator.w.pos_cur, p_cur_ui->indicator.h.pos_cur, 0);
}

void WouoUI_WavePageInit(
    WavePage *wp,           // 波形页面对象
    uint8_t wave_num,        // 波形数量
    WaveData * wave_data_array, // 波形数据数组
    CallBackFunc call_back) //回调函数
{
    wp->page.page_type = type_wave;
    WouoUI_PageInit((PageAddr)wp, call_back);
    wp->wave_data_num = wave_num;
    wp->wave_data_array = wave_data_array;
    wp->wave_data_select = 0;
    wp->page.methods = &(p_cur_ui->wt_mth);
    for(uint8_t i = 0; i < wave_num; i++){
        wp->wave_data_array[i].idx_head = 0;
        wp->wave_data_array[i].idx_tail = 0;
        wp->wave_data_array[i].idx_show_head = 0;
        wp->wave_data_array[i].idx_show_tail = 0;
        wp->wave_data_array[i].idx_show_max = 0;
        wp->wave_data_array[i].idx_show_min = 0;
        wp->wave_data_array[i].rangeMax = -32768;
        wp->wave_data_array[i].rangeMin = 32767;
        wp->wave_data_array[i].stop_flag = false;//默认是运动的
    }
}

// wave页面给使用者的接口函数
void WouoUI_WavePageUpdateVal(WavePage* wp, uint8_t wave_num, int16_t new_data) { // 更新波形数据
    if(wave_num >= wp->wave_data_num){WOUOUI_LOG_E("Valid wave_num, it is too large");return;}
    WaveData* wave_data = &(wp->wave_data_array[wave_num]);
    if(false == wave_data->stop_flag) //波形运动时才能更新数据
    {
#if SOFTWARE_DYNAMIC_REFRESH
        if((Page*)wp == (Page*)WouoUI_GetCurrentPage() && wave_num == wp->wave_data_select) //往页面选中项页面存波形数据才有必要刷新屏幕
            p_cur_ui->is_motionless = false; //当有波形数据进入时，启用刷新(如果使用软件刷新的话)     
#endif
        wave_data->data[wave_data->idx_tail] = new_data; //放入新数据(先赋值再移动index,有利于其他从head到tail的数据循环，tail指向数据保持空)
        wave_data->idx_tail++;
        wave_data->idx_tail %= (WAVE_DEPTH); //尾指针后移
        wave_data->idx_show_tail = wave_data->idx_tail; //更新显示范围
        if(((wave_data->idx_show_tail + WAVE_DEPTH - wave_data->idx_show_head) % WAVE_DEPTH) > WAVE_SHOW_WIDTH){ //如果显示范围扩大了
            wave_data->idx_show_head++;wave_data->idx_show_head %= (WAVE_DEPTH); //整体后移
        }
        if(wave_data->idx_tail == wave_data->idx_head){ //队列满了,直接覆盖(注意尾指针已经后移过了)
            wave_data->idx_head++;
            wave_data->idx_head %= (WAVE_DEPTH);
        } 
        _WouoUI_WaveUpdateRange(wave_data); //更新数据范围
    }
}

void WouoUI_WavePageStopRestartWave(WavePage* wp, uint8_t wave_num, bool stop)
{
    if(wave_num >= wp->wave_data_num){WOUOUI_LOG_E("Valid wave_num, it is too large");return;}
    WaveData* wd = &(wp->wave_data_array[wave_num]);
    wd->stop_flag = stop;
    if(false == wd->stop_flag){ //从true->false,停止后从新开始，需要把展示指针移到最新的数据处
        int32_t delta = wd->idx_tail - wd->idx_show_tail;
        if (delta < 0) {
            delta += WAVE_DEPTH;
        }
        wd->idx_show_head += delta;//show tail移动多少，head移动多少
        wd->idx_show_head %= WAVE_DEPTH;
        wd->idx_show_tail = wd->idx_tail;
        wd->idx_show_max = wd->idx_show_tail;
        wd->idx_show_min = wd->idx_show_tail; //把min和max的指针强制置到show的范围外(因为上面向右移动了不止一个点)
        _WouoUI_WaveUpdateRange(wd); //这样调用这个函数可以强制对所有数据进行一次轮询最大最小值
    }
}
void WouoUI_WavePageShowLastWaveData(WavePage* wp)
{
    _WouoUI_WaveAnimParaReaset();
    if (wp->wave_data_select == 0) wp->wave_data_select = wp->wave_data_num - 1; // 选中最后一个
    else wp->wave_data_select--; // 选中减1
}
void WouoUI_WavePageShowNextWaveData(WavePage* wp)
{
    _WouoUI_WaveAnimParaReaset();
    if (wp->wave_data_select == wp->wave_data_num - 1) wp->wave_data_select = 0; // 选中第一个
    else wp->wave_data_select++; // 选中加1
}
bool WouoUI_WavePageLeftShiftWave(WavePage* wp, uint8_t wave_num) //返回值表示是否已经到头了(即有没有成功shift波形)
{
    bool ret = WouoUI_WavePageCanShiftWave(wp, wave_num, 0); //能否左移
    if(ret){
        WaveData* wd = &(wp->wave_data_array[wave_num]);
        if(wd->idx_show_head == 0)wd->idx_show_head = WAVE_DEPTH-1;
        else wd->idx_show_head--;//整体前移
        if(wd->idx_show_tail == 0)wd->idx_show_tail = WAVE_DEPTH-1; 
        else wd->idx_show_tail--;//整体前移
        _WouoUI_WaveUpdateRange(wd); //更新数据范围
        ret = true; //成功移动波形
    }
    return ret; 
}
bool WouoUI_WavePageRightShiftWave(WavePage* wp, uint8_t wave_num)
{
    bool ret = WouoUI_WavePageCanShiftWave(wp, wave_num, 1);//能否右移
    if(ret){
        WaveData* wd = &(wp->wave_data_array[wave_num]);
        wd->idx_show_head++;wd->idx_show_head %= (WAVE_DEPTH); //整体后移
        wd->idx_show_tail++;wd->idx_show_tail %= (WAVE_DEPTH); //整体后移
        _WouoUI_WaveUpdateRange(wd); //更新数据范围
        ret = true; //成功移动波形
    }
    return ret; 
}

bool WouoUI_WavePageCanShiftWave(WavePage * wp, uint8_t wave_num, uint8_t left0ORright1) //返回true,表示能够移动
{
    bool ret = false;
    if(wave_num >= wp->wave_data_num){WOUOUI_LOG_E("Valid wave_num, it is too large");return ret;}
    WaveData* wd = &(wp->wave_data_array[wave_num]);
    if(wd->stop_flag){//只有波形停止的时候才能移动波形
        if(left0ORright1 == 0)ret = ((wd->idx_show_head == wd->idx_head)?false:true); //能够左移
        else ret = ((wd->idx_show_tail == wd->idx_tail)?false:true); //能够右移
    }else {WOUOUI_LOG_I("Wave data is run, Can't shift it");}
    return ret; 
}
