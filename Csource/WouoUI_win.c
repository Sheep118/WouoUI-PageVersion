#include "WouoUI_win.h"
#include "WouoUI.h" //包含自己的顶层文件其实不好(但需要调用到p_cur_ui,后期再改吧)

//-----弹窗通用的函数，用于获取背景页面的选中项参数(背景为选项页面才有选中项，背景为其他页面的，自己在回调函数中使用共享内存传递数据吧)
static Option* _WouoUI_WinGetBGSelectItem(PageAddr bg) {
    Option* ret = NULL;
    switch (WouoUI_CheckPageType(bg)) {
    case type_title:
        ret = &((TitlePage*)bg)->option_array[((TitlePage*)bg)->select_item];
        break;
    case type_list:
        ret = &((ListPage*)bg)->option_array[((ListPage*)bg)->select_item];
        break;
    default:
        WOUOUI_LOG_I("The BackGround of Win is not a menu");
        break;
    }
    return ret;
}

//-----MsgWin相关方法和函数
bool WouoUI_MsgWinPageIn(
    PageAddr
        page_addr) // 进入页面的过度动画函数(返回值为True表示这个状态已经可以结束了，切换下一个状态)
{
    bool ret = false;
    MsgWin* mw = (MsgWin*)page_addr;
    WouoUI_GraphSetPenColor(PEN_COLOR_BLACK);
    if (p_cur_ui->state == ui_page_in)
        WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), p_cur_ui->indicator.x.pos_cur,
                              p_cur_ui->indicator.y.pos_cur, p_cur_ui->indicator.w.pos_cur,
                              p_cur_ui->indicator.h.pos_cur, MSG_WIN_R); // 清空出白色背景
    WouoUI_GraphSetPenColor(PEN_COLOR_WHITE);
    // 绘制部分和show基本重合，因为语句不多，就不单独抽成函数了
    p_cur_ui->mw_var.canvas.start_x = p_cur_ui->indicator.x.pos_cur + MSG_WIN_FONT_MARGIN;
    p_cur_ui->mw_var.canvas.start_y = p_cur_ui->indicator.y.pos_cur + MSG_WIN_FONT_MARGIN;
    p_cur_ui->mw_var.canvas.h = p_cur_ui->indicator.h.pos_cur - 2 * MSG_WIN_FONT_MARGIN;
    p_cur_ui->mw_var.canvas.w = p_cur_ui->indicator.w.pos_cur - 2 * MSG_WIN_FONT_MARGIN;
    if (NULL != mw->content && p_cur_ui->state == ui_page_in)
        WouoUI_CanvasDrawStrAutoNewlineEx(&(p_cur_ui->mw_var.canvas), 0, mw->str_start_y,
                                          MSG_WIN_FONT, MSG_WIN_CFONT, (uint8_t*)mw->content);
    // msgwin的文字动画是随着指示器运动的，所以这里直接判断指示器是否完成与否即可
    if (p_cur_ui->indicator.x.pos_cur == p_cur_ui->indicator.x.pos_tgt &&
        p_cur_ui->indicator.h.pos_cur == p_cur_ui->indicator.h.pos_tgt &&
        p_cur_ui->indicator.w.pos_cur == p_cur_ui->indicator.w.pos_tgt &&
        p_cur_ui->indicator.y.pos_cur == p_cur_ui->indicator.y.pos_tgt)
        ret = true;
    return ret; // 通知上层需要切换状态了
}
void WouoUI_MsgWinPageInParaInit(PageAddr page_addr) // 进入页面过度动画参数初始化函数
{
    MsgWin* mw = (MsgWin*)page_addr;
    if (mw->auto_get_bg_opt) {
        mw->bg_opt = _WouoUI_WinGetBGSelectItem(mw->page.last_page);
        if (NULL != mw->bg_opt) {
            if (NULL != mw->bg_opt->content)
                mw->content = mw->bg_opt->content; // 取出选中项的内容
            else if (NULL != mw->bg_opt->text)
                mw->content = mw->bg_opt->text; // 如果选中项没有内容文本，显示标题文本
        }
    } else { // 如果没有使能自动获取，且自己还没有设置的话
        if (NULL == mw->content)
            mw->content = (char*)WOUOUI_WIN_TXT_DEFAULT;
    }
    p_cur_ui->mw_var.autonewline_need_h = WouoUI_GetStrHeightAutoNewLineEx(
        MSG_WIN_W - 2 * MSG_WIN_FONT_MARGIN, mw->content, MSG_WIN_FONT, MSG_WIN_CFONT);
    p_cur_ui->mw_var.indicator_h_temp =
        MIN(p_cur_ui->mw_var.autonewline_need_h + 2 * MSG_WIN_FONT_MARGIN, MSG_WIN_H_MAX);
    mw->str_start_y = 0;                                               // 显示进度清零
    p_cur_ui->win_bg_blur = p_cur_ui->upara->winbgblur_param[MGS_WBB]; // 设置该弹窗背景的虚化程度
}
void WouoUI_MsgWinPageShow(PageAddr page_addr) // 页面的展示函数
{
    MsgWin* mw = (MsgWin*)page_addr;
    Page* bg = (Page*)(((Page*)page_addr)->last_page);
    bg->methods->show(bg); // 先绘制背景
    WouoUI_GraphSetPenColor(PEN_COLOR_BLACK);
    WouoUI_BuffAllBlur(p_cur_ui->win_bg_blur); // 背景虚化
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), p_cur_ui->indicator.x.pos_cur,
                          p_cur_ui->indicator.y.pos_cur, p_cur_ui->indicator.w.pos_cur,
                          p_cur_ui->indicator.h.pos_cur, MSG_WIN_R); // 清空出白色背景
    WouoUI_GraphSetPenColor(PEN_COLOR_WHITE);
    p_cur_ui->mw_var.canvas.start_x = p_cur_ui->indicator.x.pos_cur + MSG_WIN_FONT_MARGIN;
    p_cur_ui->mw_var.canvas.start_y = p_cur_ui->indicator.y.pos_cur + MSG_WIN_FONT_MARGIN;
    p_cur_ui->mw_var.canvas.h = p_cur_ui->indicator.h.pos_cur - 2 * MSG_WIN_FONT_MARGIN;
    p_cur_ui->mw_var.canvas.w = p_cur_ui->indicator.w.pos_cur - 2 * MSG_WIN_FONT_MARGIN;
    if (NULL != mw->content)
        WouoUI_CanvasDrawStrAutoNewlineEx(&(p_cur_ui->mw_var.canvas), 0, mw->str_start_y,
                                          MSG_WIN_FONT, MSG_WIN_CFONT, (uint8_t*)mw->content);
}
bool WouoUI_MsgWinPageReact(
    PageAddr page_addr) // 页面的响应函数(返回值为True表示这个状态已经可以结束了，切换下一个状态)
{
    bool ret = false; // 返回true时会触发UI切换状态
    InputMsg msg = WouoUI_MsgQueRead(&(p_cur_ui->msg_queue));
    MsgWin* mw = (MsgWin*)page_addr;
    if (mw->page.auto_deal_with_msg) {
        switch (msg) {
        case msg_right:
        case msg_down:
            WouoUI_MsgWinPageSlideUpTxt(mw);
            break;
        case msg_left:
        case msg_up:
            WouoUI_MsgWinPageSlideDownTxt(mw);
            break;
        case msg_return:
        case msg_click:
            WouoUI_PageReturn(page_addr);
            ret = true;
            break;
        default:
            break;
        }
        if (msg_none != msg && NULL != mw->page.cb)
            mw->page.cb(&(mw->page), msg); // 任何输入的有效信息都会调用回调函数
    } else {
        if (msg_none != msg && NULL != mw->page.cb) { // 任何输入的有效信息都会调用回调函数
            ret = mw->page.cb(&(mw->page), msg);      // 返回ture表示退出这个页面
            if (ret)
                WouoUI_PageReturn(page_addr);
        }
    }
    return ret;
}
void WouoUI_MsgWinPageIndicatorCtrl(PageAddr page_addr) // 页面与UI指示器交互的控制函数
{
    MsgWin* mw = (MsgWin*)page_addr;
    p_cur_ui->indicator.w.pos_tgt = MSG_WIN_W;
    if (NULL == mw->content)
        p_cur_ui->indicator.h.pos_tgt = MSG_WIN_FONT_H; // 文本为空的话，弹窗高度为单字符
    else {
        p_cur_ui->indicator.h.pos_tgt = p_cur_ui->mw_var.indicator_h_temp;
    }
    p_cur_ui->indicator.x.pos_tgt = (WOUOUI_BUFF_WIDTH - MSG_WIN_W) / 2;
    p_cur_ui->indicator.y.pos_tgt = (WOUOUI_BUFF_HEIGHT - p_cur_ui->indicator.h.pos_tgt) / 2;
    WouoUI_CanvasDrawRBoxEmpty(&(p_cur_ui->w_all), p_cur_ui->indicator.x.pos_cur,
                               p_cur_ui->indicator.y.pos_cur, p_cur_ui->indicator.w.pos_cur,
                               p_cur_ui->indicator.h.pos_cur, MSG_WIN_R);
}

void WouoUI_MsgWinPageInit(MsgWin* mw,           // 消息弹窗对象指针
                           char* content,        // 显示的文本内容
                           bool auto_get_bg_opt, // 是否自动获取背景菜单页面中的文本
                           uint16_t move_step,   // 文本移动时的调整步长
                           CallBackFunc cb)      // 回调函数
{
    mw->page.page_type = type_msgwin;
    WouoUI_PageInit(mw, cb);
    mw->auto_get_bg_opt = auto_get_bg_opt;
    mw->content = content;
    mw->str_start_y = 0;
    mw->str_move_step = move_step;
    mw->bg_opt = NULL;
    mw->page.methods = &(p_cur_ui->mw_mth);
}
// MsgWin的接口函数
bool WouoUI_MsgWinPageSetContent(MsgWin* mw, char* content) {
    bool ret = false;
    if (WouoUI_CheckPageIsInit(mw)) {
        mw->content = content;
        ret = true;
    }
    return ret;
}
bool WouoUI_MsgWinPageSlideUpTxt(MsgWin* mw) // 返回值表示能否成功滑动
{
    bool ret = false;
    if (p_cur_ui->mw_var.autonewline_need_h + mw->str_start_y >
        MSG_WIN_H_MAX - 2 * MSG_WIN_FONT_MARGIN) {
        mw->str_start_y -= mw->str_move_step;
        ret = true;
    }
    return ret;
}
bool WouoUI_MsgWinPageSlideDownTxt(MsgWin* mw) {
    bool ret = false;
    if (mw->str_start_y < 0) {
        mw->str_start_y += mw->str_move_step;
        ret = true;
    }
    return ret;
}

//-----ConfWin相关方法和函数

// 抽象出这个函数主要因为在IN和show中有大部分重合的绘制语句
static void _WouoUI_ConfWinPageDraw(ConfWin* cw) {
    Canvas temp_canvas;
    // 绘制外框
    WouoUI_CanvasDrawRBoxEmpty(
        &(p_cur_ui->w_all), CONF_WIN_HORIZON_S, p_cur_ui->cw_var.win_y.pos_cur, CONF_WIN_W,
        p_cur_ui->cw_var.canvas.h + CONF_WIN_BTN_H + 2 * CONF_WIN_FONT_MARGIN + CONF_TEXT_BTN_S,
        CONF_WIN_R);
    if (NULL != cw->content) // 绘制提示文本
        WouoUI_CanvasDrawStrAutoNewlineEx(&(p_cur_ui->cw_var.canvas), 0, cw->str_start_y,
                                          CONF_WIN_FONT, CONF_WIN_CFONT, (uint8_t*)cw->content);
    p_cur_ui->cw_var.btn_ss.canvas.start_y =
        p_cur_ui->cw_var.canvas.start_y + p_cur_ui->cw_var.canvas.h + CONF_TEXT_BTN_S;
    WouoUI_CanvasDrawSlideStrEx(&(p_cur_ui->cw_var.btn_ss), CONF_WIN_IND_BTN_S, CONF_WIN_FONT,
                                CONF_WIN_CFONT); // 绘制选中项文本
    // 绘制另一个选项
    temp_canvas = p_cur_ui->cw_var.btn_ss.canvas;
    if (cw->conf_ret) { // 选中左边，绘制右边
        temp_canvas.w =
            MIN(WouoUI_GetStrWidthEx(cw->str_right, CONF_WIN_FONT, CONF_WIN_CFONT), CONF_BTN_MAX_W);
        temp_canvas.start_x = WOUOUI_MIDDLE_H + (CONF_WIN_W >> 2) - (temp_canvas.w >> 1);
        WouoUI_CanvasDrawTextEx(&temp_canvas, 0, CONF_WIN_IND_BTN_S, CONF_WIN_FONT, CONF_WIN_CFONT,
                                (uint8_t*)(cw->str_right));
    } else {
        temp_canvas.w =
            MIN(WouoUI_GetStrWidthEx(cw->str_left, CONF_WIN_FONT, CONF_WIN_CFONT), CONF_BTN_MAX_W);
        temp_canvas.start_x = CONF_WIN_HORIZON_S + (CONF_WIN_W >> 2) - (temp_canvas.w >> 1);
        WouoUI_CanvasDrawTextEx(&temp_canvas, 0, CONF_WIN_IND_BTN_S, CONF_WIN_FONT, CONF_WIN_CFONT,
                                (uint8_t*)(cw->str_left));
    }
}

bool WouoUI_ConfWinPageIn(PageAddr page_addr) {
    bool ret = false;
    ConfWin* cw = (ConfWin*)page_addr;
    WouoUI_Animation(&(p_cur_ui->cw_var.win_y), p_cur_ui->upara->ani_param[WIN_ANI], p_cur_ui->time,
                     &(p_cur_ui->anim_is_finish));
    // 绘制白色背景
    p_cur_ui->cw_var.canvas.start_y = p_cur_ui->cw_var.win_y.pos_cur + CONF_WIN_FONT_MARGIN;
    WouoUI_GraphSetPenColor(PEN_COLOR_BLACK);
    WouoUI_CanvasDrawRBox(
        &(p_cur_ui->w_all), CONF_WIN_HORIZON_S, p_cur_ui->cw_var.win_y.pos_cur, CONF_WIN_W,
        p_cur_ui->cw_var.canvas.h + CONF_WIN_BTN_H + 2 * CONF_WIN_FONT_MARGIN + CONF_TEXT_BTN_S,
        CONF_WIN_R); // 清空出白色背景
    WouoUI_GraphSetPenColor(PEN_COLOR_WHITE);
    _WouoUI_ConfWinPageDraw(cw); // 绘制弹窗页面内部的元素
    if (p_cur_ui->cw_var.win_y.pos_cur == p_cur_ui->cw_var.win_y.pos_tgt)
        ret = true;
    return ret;
}
void WouoUI_ConfWinPageInParaInit(PageAddr page_addr) {
    ConfWin* cw = (ConfWin*)page_addr;
    if (cw->auto_get_bg_opt) {
        cw->bg_opt = _WouoUI_WinGetBGSelectItem(cw->page.last_page);
        if (NULL != cw->bg_opt) {
            if (NULL != cw->bg_opt->content)
                cw->content = cw->bg_opt->content; // 取出选中项的内容
            else if (NULL != cw->bg_opt->text)
                cw->content = cw->bg_opt->text; // 如果选中项没有内容文本，显示标题文本
            cw->conf_ret = cw->bg_opt->val;     // 获取背景页面选中项的选中与否
        }
    } else { // 如果没有使能自动获取，且自己还没有设置的话
        if (NULL == cw->content)
            cw->content = (char*)WOUOUI_WIN_TXT_DEFAULT;
    }
    cw->str_start_y = 0; // 显示进度清零
    // 计算文本框高度
    if (NULL != cw->content)
        p_cur_ui->cw_var.autonewline_need_h = WouoUI_GetStrHeightAutoNewLineEx(
            CONF_WIN_W - 2 * CONF_WIN_FONT_MARGIN, cw->content, CONF_WIN_FONT, CONF_WIN_CFONT);
    else
        p_cur_ui->cw_var.autonewline_need_h =
            CONF_WIN_FONT_H; // 如果是NULL的话，文本框高度为单字符高度
    p_cur_ui->cw_var.canvas.h =
        MIN(p_cur_ui->cw_var.autonewline_need_h,
            CONF_WIN_H_MAX - 2 * CONF_WIN_FONT_MARGIN - CONF_WIN_BTN_H - CONF_TEXT_BTN_S);
    p_cur_ui->cw_var.canvas.w = CONF_WIN_W - 2 * CONF_WIN_FONT_MARGIN;
    p_cur_ui->cw_var.canvas.start_x = CONF_WIN_HORIZON_S + CONF_WIN_FONT_MARGIN;
    p_cur_ui->cw_var.btn_ss.canvas.h = CONF_WIN_BTN_H;
    if (cw->conf_ret) {
        p_cur_ui->cw_var.btn_ss.str = cw->str_left;
        p_cur_ui->cw_var.btn_ss.canvas.w =
            MIN(WouoUI_GetStrWidthEx(cw->str_left, CONF_WIN_FONT, CONF_WIN_CFONT), CONF_BTN_MAX_W);
        p_cur_ui->cw_var.btn_ss.canvas.start_x =
            CONF_WIN_HORIZON_S + (CONF_WIN_W >> 2) - (p_cur_ui->cw_var.btn_ss.canvas.w >> 1);
    } else {
        p_cur_ui->cw_var.btn_ss.str = cw->str_right;
        p_cur_ui->cw_var.btn_ss.canvas.w =
            MIN(WouoUI_GetStrWidthEx(cw->str_right, CONF_WIN_FONT, CONF_WIN_CFONT), CONF_BTN_MAX_W);
        p_cur_ui->cw_var.btn_ss.canvas.start_x =
            WOUOUI_MIDDLE_H + (CONF_WIN_W >> 2) - (p_cur_ui->cw_var.btn_ss.canvas.w >> 1);
    }
    p_cur_ui->cw_var.win_y.pos_tgt =
        ((WOUOUI_BUFF_HEIGHT - p_cur_ui->cw_var.canvas.h - p_cur_ui->cw_var.btn_ss.canvas.h -
          2 * CONF_WIN_FONT_MARGIN - CONF_TEXT_BTN_S) >>
         1);
    p_cur_ui->cw_var.win_y.pos_cur = -p_cur_ui->cw_var.win_y.pos_tgt;
    p_cur_ui->cw_var.btn_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WIN_TXT_SSS];
    p_cur_ui->cw_var.btn_ss.step = p_cur_ui->upara->slidestrstep_param[WIN_TXT_SSS];
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->cw_var.btn_ss));
    p_cur_ui->win_bg_blur = p_cur_ui->upara->winbgblur_param[CONF_WBB];
}
void WouoUI_ConfWinPageShow(PageAddr page_addr) {
    ConfWin* cw = (ConfWin*)page_addr;
    Page* bg = (Page*)(((Page*)page_addr)->last_page);
    bg->methods->show(bg); // 先绘制背景
    WouoUI_GraphSetPenColor(PEN_COLOR_BLACK);
    WouoUI_BuffAllBlur(p_cur_ui->win_bg_blur); // 背景虚化
    WouoUI_CanvasDrawRBox(
        &(p_cur_ui->w_all), CONF_WIN_HORIZON_S, p_cur_ui->cw_var.win_y.pos_cur, CONF_WIN_W,
        p_cur_ui->cw_var.canvas.h + CONF_WIN_BTN_H + 2 * CONF_WIN_FONT_MARGIN + CONF_TEXT_BTN_S,
        CONF_WIN_R); // 清空出白色背景
    WouoUI_GraphSetPenColor(PEN_COLOR_WHITE);
    _WouoUI_ConfWinPageDraw(cw);
    if (p_cur_ui->anim_is_finish &&
        !(p_cur_ui->cw_var.btn_ss.slide_is_finish)) { // anim结束，且单次滚动没有完成就使能滚动
        p_cur_ui->cw_var.btn_ss.slide_enable = true;
    }
    // p_cur_ui->slide_is_finish = !(p_cur_ui->cw_var.btn_ss.slide_enable);
    // //这个统计必须放show函数最后
    p_cur_ui->slide_is_finish =
        p_cur_ui->slide_is_finish && !(p_cur_ui->cw_var.btn_ss.slide_enable);
    // 在弹窗中&&上自身是因为自身存着背景中slide动画是否完成的标志
    // 和anim_is_finish的标志一样在show阶段最开始置true，在show最后验收给motionless的标志位
}
bool WouoUI_ConfWinPageReact(PageAddr page_addr) {
    bool ret = false; // 返回true时会触发UI切换状态
    InputMsg msg = WouoUI_MsgQueRead(&(p_cur_ui->msg_queue));
    ConfWin* cw = (ConfWin*)page_addr;
    if (cw->page.auto_deal_with_msg) {
        switch (msg) {
        case msg_down:
            WouoUI_ConfWinPageSlideUpTxt(cw);
            break;
        case msg_up:
            WouoUI_ConfWinPageSlideDownTxt(cw);
            break;
        case msg_left:
        case msg_right:
            WouoUI_ConfWinPageToggleBtn(cw);
            break;
        case msg_click:
            if (cw->auto_set_bg_opt) { // 如果使能自动赋值bg页面选中项值的话
                Option* opt = _WouoUI_WinGetBGSelectItem(cw->page.last_page);
                if (NULL != opt)
                    opt->val = cw->conf_ret;
            }
        case msg_return:
            WouoUI_PageReturn(page_addr);
            p_cur_ui->cw_var.win_y.pos_tgt = -CONF_WIN_H_MAX;
            ret = true;
            break;
        default:
            break;
        }
        if (msg_none != msg && NULL != cw->page.cb)
            cw->page.cb(&(cw->page), msg); // 任何输入的有效信息都会调用回调函数
    } else {
        if (msg_none != msg && NULL != cw->page.cb) { // 任何输入的有效信息都会调用回调函数
            ret = cw->page.cb(&(cw->page), msg);
            if (ret) { // 回调函数返回true表示退出页面
                WouoUI_PageReturn(page_addr);
                p_cur_ui->cw_var.win_y.pos_tgt = -CONF_WIN_H_MAX;
            }
        }
    }
    return ret;
}
void WouoUI_ConfWinPageIndicatorCtrl(PageAddr page_addr) {
    UNUSED_PARAMETER(page_addr);
    p_cur_ui->indicator.x.pos_tgt = p_cur_ui->cw_var.btn_ss.canvas.start_x - CONF_WIN_IND_BTN_S;
    p_cur_ui->indicator.w.pos_tgt = p_cur_ui->cw_var.btn_ss.canvas.w + 2 * CONF_WIN_IND_BTN_S;
    p_cur_ui->indicator.h.pos_tgt = p_cur_ui->cw_var.btn_ss.canvas.h;
    // 因为btnss的start_y是会随win_y运动的
    p_cur_ui->indicator.y.pos_tgt = p_cur_ui->cw_var.win_y.pos_tgt + CONF_WIN_FONT_MARGIN +
                                    p_cur_ui->cw_var.canvas.h + CONF_TEXT_BTN_S;
    WouoUI_GraphSetPenColor(2); // 反色绘制
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), p_cur_ui->indicator.x.pos_cur,
                          p_cur_ui->indicator.y.pos_cur, p_cur_ui->indicator.w.pos_cur,
                          p_cur_ui->indicator.h.pos_cur, CONF_WIN_BTN_R);
    WouoUI_GraphSetPenColor(1); // 实色绘制
}

void WouoUI_ConfWinPageInit(ConfWin* cw,     // 确认弹窗对象指针
                            char* content,   // 显示的文本内容
                            char* str_left,  // 左按键的文本
                            char* str_right, // 右按键的文本
                            bool conf_ret,   // 确认弹窗结果的初始值(如果下面两个使能，这个无效)
                            bool auto_get_bg_opt, // 是否自动获取背景菜单页面中的文本
                            bool auto_set_bg_opt, // 是否自动赋值背景菜单页面中的文本
                            uint16_t move_step,   // 文本移动时的调整步长
                            CallBackFunc cb)      // 回调函数
{
    cw->page.page_type = type_confwin;
    WouoUI_PageInit(cw, cb);
    cw->auto_get_bg_opt = auto_get_bg_opt;
    cw->auto_set_bg_opt = auto_set_bg_opt;
    cw->content = content;
    cw->conf_ret = conf_ret;
    if (NULL == str_left)
        cw->str_left = (char*)"Yes";
    else
        cw->str_left = str_left;
    if (NULL == str_right)
        cw->str_right = (char*)"No";
    else
        cw->str_right = str_right;
    cw->str_start_y = 0;
    cw->str_move_step = move_step;
    cw->bg_opt = NULL;
    cw->page.methods = &(p_cur_ui->cw_mth);
}
// COnfwin相关接口函数
bool WouoUI_ConfWinPageSlideUpTxt(ConfWin* cw) {
    bool ret = false;
    if (p_cur_ui->cw_var.autonewline_need_h + cw->str_start_y >
        CONF_WIN_H_MAX - 2 * CONF_WIN_FONT_MARGIN - CONF_WIN_BTN_H - CONF_TEXT_BTN_S) {
        cw->str_start_y -= cw->str_move_step;
        ret = true;
    }
    return ret;
}
bool WouoUI_ConfWinPageSlideDownTxt(ConfWin* cw) {
    bool ret = false;
    if (cw->str_start_y < 0) {
        cw->str_start_y += cw->str_move_step;
        ret = true;
    }
    return ret;
}
void WouoUI_ConfWinPageToggleBtn(ConfWin* cw) {
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->cw_var.btn_ss));
    cw->conf_ret = !cw->conf_ret;
    if (cw->conf_ret) {
        p_cur_ui->cw_var.btn_ss.str = cw->str_left;
        p_cur_ui->cw_var.btn_ss.canvas.w =
            MIN(WouoUI_GetStrWidthEx(cw->str_left, CONF_WIN_FONT, CONF_WIN_CFONT), CONF_BTN_MAX_W);
        p_cur_ui->cw_var.btn_ss.canvas.start_x =
            CONF_WIN_HORIZON_S + (CONF_WIN_W >> 2) - (p_cur_ui->cw_var.btn_ss.canvas.w >> 1);
    } else {
        p_cur_ui->cw_var.btn_ss.str = cw->str_right;
        p_cur_ui->cw_var.btn_ss.canvas.w =
            MIN(WouoUI_GetStrWidthEx(cw->str_right, CONF_WIN_FONT, CONF_WIN_CFONT), CONF_BTN_MAX_W);
        p_cur_ui->cw_var.btn_ss.canvas.start_x =
            WOUOUI_MIDDLE_H + (CONF_WIN_W >> 2) - (p_cur_ui->cw_var.btn_ss.canvas.w >> 1);
    }
}

//-------数值弹窗的相关方法和函数

static void _WouoUI_ValWinPageDraw(ValWin* vw) {
    char str_buff[VAL_WIN_STR_BUFF_SIZE] = {0};
    // 弹窗外框
    WouoUI_CanvasDrawRBoxEmpty(&(p_cur_ui->w_all), VAL_WIN_X, p_cur_ui->vw_var.win_y.pos_cur,
                               VAL_WIN_W, VAL_WIN_H, VAL_WIN_R);
    // 进度条外框
    WouoUI_CanvasDrawRBoxEmpty(&(p_cur_ui->w_all), VAL_WIN_BAR_X,
                               p_cur_ui->vw_var.win_y.pos_cur + VAL_WIN_BAR_Y_OFS, VAL_WIN_BAR_W,
                               VAL_WIN_BAR_H, VAL_WIN_BAR_R);
    // 绘制提示文本
    p_cur_ui->vw_var.text_ss.canvas.start_x = VAL_WIN_TXTVAL_X;
    p_cur_ui->vw_var.text_ss.canvas.start_y = p_cur_ui->vw_var.win_y.pos_cur + VAL_WIN_TXTVAL_Y_OFS;
    p_cur_ui->vw_var.text_ss.canvas.h = VAL_WIN_TXTVAL_H;
    p_cur_ui->vw_var.text_ss.str = vw->text;
    p_cur_ui->vw_var.text_ss.canvas.w = WouoUI_GetStrWidthEx(vw->text, VAL_WIN_FONT, VAL_WIN_CFONT);
    // 绘制数值
    p_cur_ui->vw_var.val_ss.canvas = p_cur_ui->vw_var.text_ss.canvas;
    ui_itoa_str(vw->val, str_buff);
    p_cur_ui->vw_var.val_ss.str = str_buff;
    p_cur_ui->vw_var.val_ss.canvas.w = WouoUI_GetStrWidthEx(str_buff, VAL_WIN_FONT, VAL_WIN_CFONT);
    if (p_cur_ui->vw_var.val_ss.canvas.w + p_cur_ui->vw_var.text_ss.canvas.w + VAL_WIN_TXTVAL_S >
        VAL_WIN_W - 2 * VAL_WIN_FONT_MARGIN) {
        // 放不下的话按设置允许的最大长度来
        if (p_cur_ui->vw_var.val_ss.canvas.w < VAL_WIN_VAL_W_MAX) { // 如果值放得下的话，优先值靠右
            p_cur_ui->vw_var.text_ss.canvas.w = VAL_WIN_W - 2 * VAL_WIN_FONT_MARGIN -
                                                p_cur_ui->vw_var.val_ss.canvas.w - VAL_WIN_TXTVAL_S;
        } else {
            p_cur_ui->vw_var.text_ss.canvas.w = VAL_WIN_TXT_W_MAX;
            p_cur_ui->vw_var.val_ss.canvas.w = VAL_WIN_VAL_W_MAX;
            p_cur_ui->vw_var.val_ss.canvas.start_x =
                VAL_WIN_TXTVAL_X + VAL_WIN_TXT_W_MAX + VAL_WIN_TXTVAL_S;
        }
    }
    // 值靠右放
    p_cur_ui->vw_var.val_ss.canvas.start_x =
        VAL_WIN_X + VAL_WIN_W - VAL_WIN_FONT_MARGIN - p_cur_ui->vw_var.val_ss.canvas.w;
    WouoUI_CanvasDrawSlideStrEx(&(p_cur_ui->vw_var.text_ss), 0, VAL_WIN_FONT, VAL_WIN_CFONT);
    WouoUI_CanvasDrawSlideStrEx(&(p_cur_ui->vw_var.val_ss), 0, VAL_WIN_FONT, VAL_WIN_CFONT);
    // 绘制最大最小值
    p_cur_ui->vw_var.min_ss.canvas.start_y = p_cur_ui->vw_var.win_y.pos_cur + VAL_WIN_MMVAL_Y_OFS;
    p_cur_ui->vw_var.min_ss.canvas.h = VAL_WIN_MMVAL_H;
    ui_itoa_str(vw->min, str_buff);
    p_cur_ui->vw_var.min_ss.str = str_buff;
    p_cur_ui->vw_var.min_ss.canvas.w =
        MIN(WouoUI_GetStrWidthEx(str_buff, VAL_WIN_FONT, VAL_WIN_CFONT), VAL_WIN_MMVAL_W_MAX);
    p_cur_ui->vw_var.min_ss.canvas.start_x =
        VAL_WIN_TXTVAL_X + (VAL_WIN_MMVAL_W_MAX >> 1) - (p_cur_ui->vw_var.min_ss.canvas.w >> 1);
    WouoUI_CanvasDrawSlideStrEx(&(p_cur_ui->vw_var.min_ss), 0, VAL_WIN_FONT, VAL_WIN_CFONT);
    p_cur_ui->vw_var.max_ss.canvas = p_cur_ui->vw_var.min_ss.canvas;
    ui_itoa_str(vw->max, str_buff);
    p_cur_ui->vw_var.max_ss.str = str_buff;
    p_cur_ui->vw_var.max_ss.canvas.w =
        MIN(WouoUI_GetStrWidthEx(str_buff, VAL_WIN_FONT, VAL_WIN_CFONT), VAL_WIN_MMVAL_W_MAX);
    p_cur_ui->vw_var.max_ss.canvas.start_x = VAL_WIN_X + VAL_WIN_W - VAL_WIN_FONT_MARGIN -
                                             (VAL_WIN_MMVAL_W_MAX >> 1) -
                                             (p_cur_ui->vw_var.max_ss.canvas.w >> 1);
    WouoUI_CanvasDrawSlideStrEx(&(p_cur_ui->vw_var.max_ss), 0, VAL_WIN_FONT, VAL_WIN_CFONT);
}

bool WouoUI_ValWinPageIn(PageAddr page_addr) {
    ValWin* vw = (ValWin*)page_addr;
    bool ret = false;
    // 窗口y坐标运动
    WouoUI_Animation(&p_cur_ui->vw_var.win_y, p_cur_ui->upara->ani_param[WIN_ANI], p_cur_ui->time,
                     &(p_cur_ui->anim_is_finish));
    // 绘制白色背景
    WouoUI_GraphSetPenColor(PEN_COLOR_BLACK);
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), VAL_WIN_X, p_cur_ui->vw_var.win_y.pos_cur, VAL_WIN_W,
                          VAL_WIN_H, VAL_WIN_R); // 清空出白色背景
    WouoUI_GraphSetPenColor(PEN_COLOR_WHITE);
    _WouoUI_ValWinPageDraw(vw);
    if (p_cur_ui->vw_var.win_y.pos_cur == p_cur_ui->vw_var.win_y.pos_tgt)
        ret = true;

    return ret;
}
void WouoUI_ValWinPageInParaInit(PageAddr page_addr) {
    ValWin* vw = (ValWin*)page_addr;
    p_cur_ui->vw_var.win_y.pos_tgt = VAL_WIN_Y;
    p_cur_ui->vw_var.win_y.pos_cur = -p_cur_ui->vw_var.win_y.pos_tgt;
    if (vw->auto_get_bg_opt) {
        vw->bg_opt = _WouoUI_WinGetBGSelectItem(vw->page.last_page);
        if (NULL != vw->bg_opt) {
            if (NULL != vw->bg_opt->text)
                vw->text = &(vw->bg_opt->text[2]); // 取出选中项文本，同时跳过提示文本
            vw->val = vw->bg_opt->val;             // 获取背景页面选中项的值
            if (vw->val > vw->max) {
                vw->val = vw->max;
                WOUOUI_LOG_I("auto get val gt max");
            }
            if (vw->val < vw->min) {
                vw->val = vw->min;
                WOUOUI_LOG_I("auto get val lt min");
            }
        }
    } else { // 如果没有使能自动获取，且自己还没有设置的话
        if (NULL == vw->text)
            vw->text = (char*)WOUOUI_WIN_TXT_DEFAULT;
    }
    p_cur_ui->vw_var.text_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WIN_TXT_SSS];
    p_cur_ui->vw_var.val_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WIN_VAL_SSS];
    p_cur_ui->vw_var.min_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WIN_VAL_SSS];
    p_cur_ui->vw_var.max_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WIN_VAL_SSS];
    p_cur_ui->vw_var.text_ss.step = p_cur_ui->upara->slidestrstep_param[WIN_TXT_SSS];
    p_cur_ui->vw_var.val_ss.step = p_cur_ui->upara->slidestrstep_param[WIN_VAL_SSS];
    p_cur_ui->vw_var.min_ss.step = p_cur_ui->upara->slidestrstep_param[WIN_VAL_SSS];
    p_cur_ui->vw_var.max_ss.step = p_cur_ui->upara->slidestrstep_param[WIN_VAL_SSS];
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->vw_var.text_ss));
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->vw_var.val_ss));
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->vw_var.min_ss));
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->vw_var.max_ss));
    p_cur_ui->win_bg_blur = p_cur_ui->upara->winbgblur_param[VAL_WBB];
}
void WouoUI_ValWinPageShow(PageAddr page_addr) {
    ValWin* vw = (ValWin*)page_addr;
    Page* bg = (Page*)(((Page*)page_addr)->last_page);
    bg->methods->show(bg); // 先绘制背景
    WouoUI_GraphSetPenColor(PEN_COLOR_BLACK);
    WouoUI_BuffAllBlur(p_cur_ui->win_bg_blur); // 背景虚化
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), VAL_WIN_X, p_cur_ui->vw_var.win_y.pos_cur, VAL_WIN_W,
                          VAL_WIN_H, VAL_WIN_R); // 清空出白色背景
    WouoUI_GraphSetPenColor(PEN_COLOR_WHITE);
    _WouoUI_ValWinPageDraw(vw);
    if (p_cur_ui->anim_is_finish) { // anim结束，且单次滚动没有完成就使能滚动
        if (!(p_cur_ui->vw_var.text_ss.slide_is_finish))
            p_cur_ui->vw_var.text_ss.slide_enable = true;
        if (!(p_cur_ui->vw_var.val_ss.slide_is_finish))
            p_cur_ui->vw_var.val_ss.slide_enable = true;
        if (!(p_cur_ui->vw_var.max_ss.slide_is_finish))
            p_cur_ui->vw_var.max_ss.slide_enable = true;
        if (!(p_cur_ui->vw_var.min_ss.slide_is_finish))
            p_cur_ui->vw_var.min_ss.slide_enable = true;
    }
    // 记录这个页面中所有slide动画的结果&&(同时处理背景中的slide动画)
    p_cur_ui->slide_is_finish =
        p_cur_ui->slide_is_finish && !(p_cur_ui->vw_var.text_ss.slide_enable) &&
        !(p_cur_ui->vw_var.val_ss.slide_enable) && !(p_cur_ui->vw_var.max_ss.slide_enable) &&
        !(p_cur_ui->vw_var.min_ss.slide_enable);
}
bool WouoUI_ValWinPageReact(PageAddr page_addr) {
    bool ret = false;
    InputMsg msg = WouoUI_MsgQueRead(&(p_cur_ui->msg_queue));
    ValWin* vw = (ValWin*)page_addr;
    if (vw->page.auto_deal_with_msg) {
        switch (msg) {
        case msg_left:
        case msg_up:
            WouoUI_ValWinPageValIncrease(vw);
            break;
        case msg_right:
        case msg_down:
            WouoUI_ValWinPageValDecrease(vw);
            break;
        case msg_click:
            if (vw->auto_set_bg_opt) { // 如果使能自动赋值bg页面选中项值的话
                Option* opt = _WouoUI_WinGetBGSelectItem(vw->page.last_page);
                if (NULL != opt)
                    opt->val = vw->val;
            }
        case msg_return:
            WouoUI_PageReturn(page_addr);
            p_cur_ui->vw_var.win_y.pos_tgt = -1 * VAL_WIN_H;
            ret = true;
            break;
        default:
            break;
        }
        if (msg_none != msg && NULL != vw->page.cb)
            vw->page.cb(&(vw->page), msg); // 任何输入的有效信息都会调用回调函数
    } else {
        if (msg_none != msg && NULL != vw->page.cb) { // 任何输入的有效信息都会调用回调函数
            ret = vw->page.cb(&(vw->page), msg);
            if (ret) { // 回调函数返回true表示退出
                WouoUI_PageReturn(page_addr);
                p_cur_ui->vw_var.win_y.pos_tgt = -1 * VAL_WIN_H;
            }
        }
    }
    return ret;
}
void WouoUI_ValWinPageIndicatorCtrl(PageAddr page_addr) {
    ValWin* vw = (ValWin*)page_addr;
    // indicator 进度条长度由指示器绘制
    p_cur_ui->indicator.x.pos_tgt = VAL_WIN_BAR_X + 2;                 // 外框宽占1
    p_cur_ui->indicator.y.pos_tgt = VAL_WIN_Y + VAL_WIN_BAR_Y_OFS + 2; // 外框宽占1
    p_cur_ui->indicator.w.pos_tgt = (vw->val - vw->min) * (VAL_WIN_BAR_W - 4) / (vw->max - vw->min);
    p_cur_ui->indicator.h.pos_tgt = VAL_WIN_BAR_H - 4;
    WouoUI_GraphSetPenColor(2); // 反色绘制
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), p_cur_ui->indicator.x.pos_cur,
                          p_cur_ui->indicator.y.pos_cur, p_cur_ui->indicator.w.pos_cur,
                          p_cur_ui->indicator.h.pos_cur, VAL_WIN_BAR_R);
    WouoUI_GraphSetPenColor(1); // 实色绘制
}

void WouoUI_ValWinPageSetMinStepMax(ValWin* vw, int32_t min, int32_t step, int32_t max) {
    if (type_slidevalwin != WouoUI_CheckPageType(vw)) {
        WOUOUI_LOG_W("Don't set other type page with min step max!");
        return;
    }
    vw->min = MIN(min, max);
    vw->max = MAX(min, max);
    vw->step = ABS(step);
}
void WouoUI_ValWinPageInit(ValWin* vw,           // 数值弹窗对象指针
                           char* text,           // 显示的文本内容(如)
                           int32_t init_val,     // 数值弹窗结果的初始值(如果下面两个使能，这个无效)
                           int32_t min,          // 弹窗页面的最小值限制
                           int32_t max,          // 弹窗页面的最大值限制
                           int32_t step,         // 弹窗页面滑动条的增减步长
                           bool auto_get_bg_opt, // 是否自动获取背景菜单页面中的文本
                           bool auto_set_bg_opt, // 是否自动赋值背景菜单页面中的文本
                           CallBackFunc cb) {
    if (VAL_WIN_MACRO_ASSERT)
        WOUOUI_LOG_W("the font /the margin of val_win is too large");
    vw->page.page_type = type_slidevalwin;
    WouoUI_PageInit(vw, cb);
    vw->auto_get_bg_opt = auto_get_bg_opt;
    vw->auto_set_bg_opt = auto_set_bg_opt;
    vw->text = text;
    WouoUI_ValWinPageSetMinStepMax(vw, min, step, max);
    vw->val = init_val;
    vw->bg_opt = NULL;
    vw->page.methods = &(p_cur_ui->vw_mth);
}
// valwin相关接口函数
bool WouoUI_ValWinPageValIncrease(ValWin* vw) {
    bool ret = false;
    if (vw->val + vw->step <= vw->max) {
        vw->val += vw->step;
        ret = true;
    }
    return ret;
}
bool WouoUI_ValWinPageValDecrease(ValWin* vw) {
    bool ret = false;
    if (vw->val - vw->step >= vw->min) {
        vw->val -= vw->step;
        ret = true;
    }
    return ret;
}

//-------微调弹窗的相关方法和函数
static void _WouoUI_SpinWinPageDraw(SpinWin* spw) {
    char temp_str_buff[SPIN_WIN_STR_BUFF_SIZE] = {0};
    int16_t num_x = 0;
    // 弹窗外框
    WouoUI_CanvasDrawRBoxEmpty(&(p_cur_ui->w_all), SPIN_WIN_X, p_cur_ui->spw_var.win_y.pos_cur,
                               SPIN_WIN_W, SPIN_WIN_H, SPIN_WIN_R);
    // 提示文本跳过“% ”
    p_cur_ui->spw_var.text_ss.canvas.h = SPIN_WIN_FONT_H;
    p_cur_ui->spw_var.text_ss.canvas.start_y =
        p_cur_ui->spw_var.win_y.pos_cur + SPIN_WIN_FONT_MARGIN;
    p_cur_ui->spw_var.text_ss.canvas.w =
        MIN(WouoUI_GetStrWidthEx(spw->text, SPIN_WIN_FONT, SPIN_WIN_CFONT),
            SPIN_WIN_W - 2 * SPIN_WIN_FONT_MARGIN);
    p_cur_ui->spw_var.text_ss.canvas.start_x =
        WOUOUI_MIDDLE_H - (p_cur_ui->spw_var.text_ss.canvas.w >> 1);
    p_cur_ui->spw_var.text_ss.str = spw->text;
    WouoUI_CanvasDrawSlideStrEx(&(p_cur_ui->spw_var.text_ss), 0, SPIN_WIN_FONT, SPIN_WIN_CFONT);
    // 绘制value range
    p_cur_ui->spw_var.min_ss.canvas.h = SPIN_WIN_NUM_FONT_H;
    p_cur_ui->spw_var.min_ss.canvas.start_y =
        p_cur_ui->spw_var.win_y.pos_cur + SPIN_WIN_MMVAL_Y_OFS;
    ui_ftoa_g_str(spw->min, spw->dec_num, temp_str_buff);
    p_cur_ui->spw_var.min_ss.str = temp_str_buff;
    p_cur_ui->spw_var.min_ss.canvas.w =
        MIN(WouoUI_GetStrWidthEx(temp_str_buff, SPIN_WIN_NUM_FONT, SPIN_WIN_NUM_CFONT),
            SPIN_WIN_MIN_W_MAX);
    p_cur_ui->spw_var.min_ss.canvas.start_x = SPIN_WIN_MMVAL_MID_LX - SPIN_WIN_MMVAL_S -
                                              p_cur_ui->spw_var.min_ss.canvas.w; // 靠近中间放置
    WouoUI_CanvasDrawASCII(&(p_cur_ui->w_all), SPIN_WIN_MMVAL_MID_LX,
                           p_cur_ui->spw_var.min_ss.canvas.start_y, SPIN_WIN_NUM_FONT,
                           '~'); // 绘制最大最小值间的字符(居中)
    WouoUI_CanvasDrawSlideStrEx(&(p_cur_ui->spw_var.min_ss), 0, SPIN_WIN_NUM_FONT,
                                SPIN_WIN_NUM_CFONT);
    p_cur_ui->spw_var.max_ss.canvas = p_cur_ui->spw_var.min_ss.canvas;
    ui_ftoa_g_str(spw->max, spw->dec_num, temp_str_buff);
    p_cur_ui->spw_var.max_ss.str = temp_str_buff;
    p_cur_ui->spw_var.max_ss.canvas.w =
        MIN(WouoUI_GetStrWidthEx(temp_str_buff, SPIN_WIN_NUM_FONT, SPIN_WIN_NUM_CFONT),
            SPIN_WIN_MAX_W_MAX);
    p_cur_ui->spw_var.max_ss.canvas.start_x =
        SPIN_WIN_MMVAL_MID_RX + SPIN_WIN_MMVAL_S; // 靠近中间放置
    WouoUI_CanvasDrawSlideStrEx(&(p_cur_ui->spw_var.max_ss), 0, SPIN_WIN_NUM_FONT,
                                SPIN_WIN_NUM_CFONT);
    // decimal point
    num_x = (WOUOUI_BUFF_WIDTH - p_cur_ui->spw_var.num_w_temp) >> 1;
    if (DecimalNum_0 != spw->dec_num) // 需要显示小数点
    {
        int16_t dec_point_x_ofs =
            num_x + (8 - (int16_t)(spw->dec_num)) * GET_FNOT_W(SPIN_WIN_NUM_FONT) +
            (7 - (int16_t)(spw->dec_num)) * SPIN_WIN_NUM_S; // 减去小数点自身的宽度
        WouoUI_CanvasDrawASCII(&(p_cur_ui->w_all), dec_point_x_ofs,
                               p_cur_ui->spw_var.win_y.pos_cur + SPIN_WIN_NUM_Y_OFS,
                               SPIN_WIN_NUM_FONT, '.');
    }
    sprintf(temp_str_buff, "%+08d", spw->val);
    // show numDigit
    for (uint8_t i = 0; i < 8; i++) {
        WouoUI_CanvasDrawASCII(&(p_cur_ui->w_all), num_x,
                               p_cur_ui->spw_var.win_y.pos_cur + SPIN_WIN_NUM_Y_OFS,
                               SPIN_WIN_NUM_FONT, temp_str_buff[i]);
        if (7 - i == (int16_t)spw->dec_num)
            num_x += (GET_FNOT_W(SPIN_WIN_NUM_FONT) << 1); // 加上小数点的宽度
        else
            num_x += (GET_FNOT_W(SPIN_WIN_NUM_FONT) + SPIN_WIN_NUM_S);
    }
}

bool WouoUI_SpinWinPageIn(PageAddr page_addr) {
    SpinWin* spw = (SpinWin*)page_addr;
    bool ret = false;
    // 窗口y坐标运动
    WouoUI_Animation(&p_cur_ui->spw_var.win_y, p_cur_ui->upara->ani_param[WIN_ANI], p_cur_ui->time,
                     &(p_cur_ui->anim_is_finish));
    // 绘制白色背景
    WouoUI_GraphSetPenColor(PEN_COLOR_BLACK);
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), SPIN_WIN_X, p_cur_ui->spw_var.win_y.pos_cur,
                          SPIN_WIN_W, SPIN_WIN_H, SPIN_WIN_R); // 清空出白色背景
    WouoUI_GraphSetPenColor(PEN_COLOR_WHITE);
    _WouoUI_SpinWinPageDraw(spw);
    if (p_cur_ui->spw_var.win_y.pos_cur == p_cur_ui->spw_var.win_y.pos_tgt)
        ret = true;
    return ret;
}
void WouoUI_SpinWinPageInParaInit(PageAddr page_addr) {
    SpinWin* spw = (SpinWin*)page_addr;
    p_cur_ui->spw_var.win_y.pos_tgt = SPIN_WIN_Y;
    p_cur_ui->spw_var.win_y.pos_cur = -1 * p_cur_ui->spw_var.win_y.pos_tgt;
    // p_cur_ui->spw_var.win_y.pos_tgt = SPIN_WIN_Y;
    // p_cur_ui->spw_var.win_y.pos_cur = -1*(WOUOUI_BUFF_HEIGHT>>1);
    // 因为spin移动幅度比较小，下降动画不明显，换成一个远一点的起始位置，还是不明显其实🤣
    if (spw->auto_get_bg_opt) {
        spw->bg_opt = _WouoUI_WinGetBGSelectItem(spw->page.last_page);
        if (NULL != spw->bg_opt) {
            if (NULL != spw->bg_opt->text)
                spw->text = &(spw->bg_opt->text[2]); // 取出选中项文本，同时跳过提示文本
            spw->val = spw->bg_opt->val;             // 获取背景页面选中项的值
            spw->dec_num = spw->bg_opt->decimalNum;  // 获取选中项的定点位数
            if (spw->val > spw->max) {
                spw->val = spw->max;
                WOUOUI_LOG_I("auto get val gt max");
            }
            if (spw->val < spw->min) {
                spw->val = spw->min;
                WOUOUI_LOG_I("auto get val lt min");
            }
        }
    } else { // 如果没有使能自动获取，且自己还没有设置的话
        if (NULL == spw->text)
            spw->text = (char*)WOUOUI_WIN_TXT_DEFAULT;
    }
    if (spw->dec_num == DecimalNum_0)
        p_cur_ui->spw_var.num_w_temp = 8 * GET_FNOT_W(SPIN_WIN_NUM_FONT) + 7 * (SPIN_WIN_NUM_S);
    else
        p_cur_ui->spw_var.num_w_temp =
            9 * GET_FNOT_W(SPIN_WIN_NUM_FONT) + 6 * (SPIN_WIN_NUM_S); //.和数字之间不需要边距
    spw->sel_flag = false; // 一开始进入默认什么都没有选中
    p_cur_ui->spw_var.text_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WIN_TXT_SSS];
    p_cur_ui->spw_var.min_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WIN_VAL_SSS];
    p_cur_ui->spw_var.max_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WIN_VAL_SSS];
    p_cur_ui->spw_var.text_ss.step = p_cur_ui->upara->slidestrstep_param[WIN_TXT_SSS];
    p_cur_ui->spw_var.min_ss.step = p_cur_ui->upara->slidestrstep_param[WIN_VAL_SSS];
    p_cur_ui->spw_var.max_ss.step = p_cur_ui->upara->slidestrstep_param[WIN_VAL_SSS];
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->spw_var.text_ss));
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->spw_var.min_ss));
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->spw_var.max_ss));
    p_cur_ui->win_bg_blur = p_cur_ui->upara->winbgblur_param[SPIN_WBB];
}
void WouoUI_SpinWinPageShow(PageAddr page_addr) {
    SpinWin* spw = (SpinWin*)page_addr;
    Page* bg = (Page*)(((Page*)page_addr)->last_page);
    bg->methods->show(bg); // 先绘制背景
    WouoUI_GraphSetPenColor(PEN_COLOR_BLACK);
    WouoUI_BuffAllBlur(p_cur_ui->win_bg_blur); // 背景虚化
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), SPIN_WIN_X, p_cur_ui->spw_var.win_y.pos_cur,
                          SPIN_WIN_W, SPIN_WIN_H, SPIN_WIN_R); // 清空出白色背景
    WouoUI_GraphSetPenColor(PEN_COLOR_WHITE);
    _WouoUI_SpinWinPageDraw(spw);
    if (p_cur_ui->anim_is_finish) { // anim结束，且单次滚动没有完成就使能滚动
        if (!p_cur_ui->spw_var.text_ss.slide_is_finish)
            p_cur_ui->spw_var.text_ss.slide_enable = true;
        if (!p_cur_ui->spw_var.min_ss.slide_is_finish)
            p_cur_ui->spw_var.min_ss.slide_enable = true;
        if (!p_cur_ui->spw_var.max_ss.slide_is_finish)
            p_cur_ui->spw_var.max_ss.slide_enable = true;
    }
    // 记录这个页面中所有slide动画的结果&&(同时处理背景中的slide动画)
    p_cur_ui->slide_is_finish =
        p_cur_ui->slide_is_finish && !(p_cur_ui->spw_var.text_ss.slide_enable) &&
        !(p_cur_ui->spw_var.max_ss.slide_enable) && !(p_cur_ui->spw_var.min_ss.slide_enable);
}
bool WouoUI_SpinWinPageReact(PageAddr page_addr) {
    InputMsg msg = WouoUI_MsgQueRead(&(p_cur_ui->msg_queue));
    SpinWin* spw = (SpinWin*)page_addr;
    bool ret = false;
    if (spw->page.auto_deal_with_msg) {
        switch (msg) {
        case msg_up:
        case msg_left:
            if (spw->sel_flag)                          // 选中状态
                WouoUI_SpinWinPageChangeSelbit(spw, 1); // 增加
            else                                        // 未选中状态
                WouoUI_SpinWinPageShiftSelbit(spw, 0);
            break;
        case msg_down:
        case msg_right:
            if (spw->sel_flag)                           // 选中状态
                WouoUI_SpinWinPageChangeSelbit(spw, -1); // 减少
            else                                         // 未选中状态
                WouoUI_SpinWinPageShiftSelbit(spw, 1);
            break;
        case msg_click:
            WouoUI_SpinWinPageToggleSelState(spw);
            break;
        case msg_return:
            if (spw->auto_set_bg_opt) {
                Option* opt = _WouoUI_WinGetBGSelectItem(spw->page.last_page);
                if (NULL != opt)
                    opt->val = spw->val;
            }
            WouoUI_PageReturn(page_addr);
            p_cur_ui->spw_var.win_y.pos_tgt = -SPIN_WIN_H;
            ret = true;
            break;
        default:
            break;
        }
        if (msg_none != msg && NULL != spw->page.cb)
            spw->page.cb(&(spw->page), msg); // 任何输入的有效信息都会调用回调函数
    } else {
        if (msg_none != msg && NULL != spw->page.cb) { // 任何输入的有效信息都会调用回调函数
            ret = spw->page.cb(&(spw->page), msg);
            if (ret) { // 如果回调函数返回true的话,表示退出这个页面
                WouoUI_PageReturn(page_addr);
                p_cur_ui->spw_var.win_y.pos_tgt = -SPIN_WIN_H;
            }
        }
    }
    return ret;
}
void WouoUI_SpinWinPageIndicatorCtrl(PageAddr page_addr) {
    SpinWin* spw = (SpinWin*)page_addr;
    p_cur_ui->indicator.w.pos_tgt = GET_FNOT_W(SPIN_WIN_NUM_FONT) + (SPIN_WIN_NUM_S);
    if (spw->sel_bit < (int16_t)spw->dec_num)
        p_cur_ui->indicator.x.pos_tgt = ((WOUOUI_BUFF_WIDTH - p_cur_ui->spw_var.num_w_temp) >> 1) +
                                        p_cur_ui->indicator.w.pos_tgt * (7 - spw->sel_bit) +
                                        GET_FNOT_W(SPIN_WIN_NUM_FONT) - SPIN_WIN_NUM_S -
                                        (SPIN_WIN_NUM_S >> 1);
    else
        p_cur_ui->indicator.x.pos_tgt = ((WOUOUI_BUFF_WIDTH - p_cur_ui->spw_var.num_w_temp) >> 1) +
                                        p_cur_ui->indicator.w.pos_tgt * (7 - spw->sel_bit) -
                                        (SPIN_WIN_NUM_S >> 1);
    if (spw->sel_flag) {
        p_cur_ui->indicator.y.pos_tgt = SPIN_WIN_Y + SPIN_WIN_NUM_Y_OFS - (SPIN_WIN_V_S >> 1);
        p_cur_ui->indicator.h.pos_tgt = SPIN_WIN_NUM_FONT_H + SPIN_WIN_BOX_H + (SPIN_WIN_V_S >> 1);
    } else {
        p_cur_ui->indicator.y.pos_tgt = SPIN_WIN_Y + SPIN_WIN_NUM_Y_OFS + SPIN_WIN_NUM_FONT_H;
        p_cur_ui->indicator.h.pos_tgt = SPIN_WIN_BOX_H;
    }
    WouoUI_GraphSetPenColor(2); // 反色绘制
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), p_cur_ui->indicator.x.pos_cur,
                          p_cur_ui->indicator.y.pos_cur, p_cur_ui->indicator.w.pos_cur,
                          p_cur_ui->indicator.h.pos_cur, SPIN_WIN_BOX_R);
    WouoUI_GraphSetPenColor(1); // 实色绘制
}

void WouoUI_SpinWinPageSetMinMaxDecimalnum(SpinWin* spw, int32_t min, int32_t max,
                                           DecimalNum decnum) {
    if (type_spinwin != WouoUI_CheckPageType(spw)) {
        WOUOUI_LOG_W("Don't set other type page with min max decimalNum!");
        return;
    }
    spw->min = MIN(min, max);
    spw->max = MAX(min, max);
    spw->dec_num = decnum;
}
void WouoUI_SpinWinPageInit(
    SpinWin* spw,         // 微调数值弹窗对象指针
    char* text,           // 显示的文本内容(如果下面两个使能，这个无效)
    int32_t init_val,     // 微调数值弹窗结果的初始值(如果下面两个使能，这个无效)
    DecimalNum dec_num,   // 微调数值弹窗的定点位数(如果下面两个使能，这个无效)
    int32_t min,          // 弹窗页面的最小值限制
    int32_t max,          // 弹窗页面的最大值限制
    bool auto_get_bg_opt, // 是否自动获取背景菜单页面中的文本
    bool auto_set_bg_opt, // 是否自动赋值背景菜单页面中的文本
    CallBackFunc cb) {
    if (SPIN_WIN_MACRO_ASSERT)
        WOUOUI_LOG_W(
            "The Auto height gt the height of buff,Or The width of num gt the width of SpinWin");
    spw->page.page_type = type_spinwin;
    WouoUI_PageInit(spw, cb);
    spw->auto_get_bg_opt = auto_get_bg_opt;
    spw->auto_set_bg_opt = auto_set_bg_opt;
    spw->text = text;
    spw->val = init_val;
    WouoUI_SpinWinPageSetMinMaxDecimalnum(spw, min, max, dec_num);
    spw->bg_opt = NULL;
    spw->page.methods = &(p_cur_ui->spw_mth);
}

void WouoUI_SpinWinPageShiftSelbit(SpinWin* spw, bool left0ORright1) {
    if (left0ORright1) { // 右移
        if (spw->sel_bit > 0)
            spw->sel_bit--;
    } else { // 左移
        if (spw->sel_bit < 7)
            spw->sel_bit++;
    }
}
void WouoUI_SpinWinPageToggleSelState(SpinWin* spw) {
    spw->sel_flag = !spw->sel_flag;
}

bool WouoUI_SpinWinPageChangeSelbit(SpinWin* spw, int32_t Inc1OrDec_1) {
    bool ret = false;
    int32_t val_new = spw->val; // 保存上一次的值，防止超出范围
    int32_t delta_val = Inc1OrDec_1;
    if (spw->sel_flag) {         // 选中状态才能增加选中位
        if (spw->sel_bit != 7) { // 选中的不是符号位
            for (uint8_t i = 0; i < spw->sel_bit; i++)
                delta_val *= 10;
            val_new += delta_val;
        } else
            val_new = -val_new; // 选中符号位
        if (val_new >= spw->min && val_new <= spw->max) {
            spw->val = val_new;
            ret = true; // 修改成功
        }
    }
    return ret;
}

//----------ListWinPage相关函数
static void _WouoUI_listWinPageDraw(ListWin* lw) {
    Canvas canvas_temp;
    // 弹窗外框
    WouoUI_CanvasDrawRBoxEmpty(&(p_cur_ui->w_all), p_cur_ui->lw_var.win_x.pos_cur, -1,
                               LIST_WIN_W + LIST_WIN_R, WOUOUI_BUFF_HEIGHT + 2, LIST_WIN_R);
    canvas_temp.h = LIST_WIN_LINE_H;
    canvas_temp.w = LIST_WIN_W - 2 * LIST_WIN_L_S - LIST_WIN_R_S;
    canvas_temp.start_x = p_cur_ui->lw_var.win_x.pos_cur + LIST_WIN_L_S;
    p_cur_ui->lw_var.str_ss.canvas = canvas_temp;
    // 绘制选项
    for (uint8_t i = 0; i < lw->array_num; i++) {
        if (i != lw->sel_str_index) {
            canvas_temp.start_y = LIST_WIN_LINE_H * i + (p_cur_ui->lw_var.list_y.pos_cur);
            WouoUI_CanvasDrawTextEx(&canvas_temp, 0, LIST_WIN_TEXT_U_S, LIST_WIN_FONT,
                                    LIST_WIN_CFONT, (uint8_t*)(lw->str_array[i]));
        } else {
            p_cur_ui->lw_var.str_ss.str = lw->str_array[i];
            p_cur_ui->lw_var.str_ss.canvas.start_y =
                LIST_WIN_LINE_H * i + (p_cur_ui->lw_var.list_y.pos_cur);
            WouoUI_CanvasDrawSlideStrEx(&(p_cur_ui->lw_var.str_ss), LIST_WIN_TEXT_U_S,
                                        LIST_WIN_FONT, LIST_WIN_CFONT);
        }
    }
}

bool WouoUI_ListWinPageIn(PageAddr page_addr) {
    ListWin* lw = (ListWin*)page_addr;
    bool ret = false;
    // 窗口x坐标运动
    WouoUI_Animation(&p_cur_ui->lw_var.win_x, p_cur_ui->upara->ani_param[WIN_ANI], p_cur_ui->time,
                     &(p_cur_ui->anim_is_finish));
    // 弹窗填充
    WouoUI_GraphSetPenColor(0);
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), p_cur_ui->lw_var.win_x.pos_cur, -1,
                          LIST_WIN_W + LIST_WIN_R, WOUOUI_BUFF_HEIGHT + 2, LIST_WIN_R);
    WouoUI_GraphSetPenColor(1);
    _WouoUI_listWinPageDraw(lw);
    if (p_cur_ui->lw_var.win_x.pos_cur == p_cur_ui->lw_var.win_x.pos_tgt)
        ret = true;
    return ret;
}
void WouoUI_ListWinPageInParaInit(PageAddr page_addr) {
    ListWin* lw = (ListWin*)page_addr;
    lw->bg_opt = _WouoUI_WinGetBGSelectItem(lw->page.last_page);
    p_cur_ui->lw_var.win_x.pos_cur = WOUOUI_BUFF_WIDTH;
    p_cur_ui->lw_var.win_x.pos_tgt = WOUOUI_BUFF_WIDTH - LIST_WIN_W;
    p_cur_ui->lw_var.line_n = WOUOUI_BUFF_HEIGHT / LIST_WIN_LINE_H;
    p_cur_ui->lw_var.str_ss.slide_mode = p_cur_ui->upara->slidestrmode_param[WIN_TXT_SSS];
    p_cur_ui->lw_var.str_ss.step = p_cur_ui->upara->slidestrstep_param[WIN_TXT_SSS];
    WouoUI_CanvasSlideStrReset(&(p_cur_ui->lw_var.str_ss));
    p_cur_ui->win_bg_blur = p_cur_ui->upara->winbgblur_param[LIST_WBB];
}
void WouoUI_ListWinPageShow(PageAddr page_addr) {
    ListWin* lw = (ListWin*)page_addr;
    Page* bg = (Page*)(((Page*)page_addr)->last_page);
    bg->methods->show(bg); // 先绘制背景
    WouoUI_GraphSetPenColor(PEN_COLOR_BLACK);
    WouoUI_BuffAllBlur(p_cur_ui->win_bg_blur); // 背景虚化
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), p_cur_ui->lw_var.win_x.pos_cur, -1,
                          LIST_WIN_W + LIST_WIN_R, WOUOUI_BUFF_HEIGHT + 2, LIST_WIN_R);
    WouoUI_GraphSetPenColor(PEN_COLOR_WHITE);
    // 选项移动动画
    WouoUI_Animation(&p_cur_ui->lw_var.list_y, p_cur_ui->upara->ani_param[LIST_ANI], p_cur_ui->time,
                     &(p_cur_ui->anim_is_finish));
    _WouoUI_listWinPageDraw(lw);
    if (p_cur_ui->anim_is_finish) { // anim结束，且单次滚动没有完成就使能滚动
        if (!p_cur_ui->lw_var.str_ss.slide_is_finish)
            p_cur_ui->lw_var.str_ss.slide_enable = true;
    }
    // 记录这个页面中所有slide动画的结果&&(同时处理背景中的slide动画)
    p_cur_ui->slide_is_finish =
        p_cur_ui->slide_is_finish && !(p_cur_ui->lw_var.str_ss.slide_enable);
}
bool WouoUI_ListWinPageReact(PageAddr page_addr) {
    bool ret = false;
    InputMsg msg = WouoUI_MsgQueRead(&(p_cur_ui->msg_queue));
    ListWin* lw = (ListWin*)page_addr;
    if (WouoUI_MsgIsDirectMsg(msg))
        WouoUI_CanvasSlideStrReset(&(p_cur_ui->lw_var.str_ss));
    if (lw->page.auto_deal_with_msg) {
        switch (msg) {
        case msg_up:
        case msg_left:
            WouoUI_ListWinPageLastItem(lw);
            break;
        case msg_right:
        case msg_down:
            WouoUI_ListWinPageNextItem(lw);
            break;
        case msg_click:
            if (lw->auto_set_bg_opt) { // 如果使能自动赋值bg页面选中项值的话
                Option* opt = _WouoUI_WinGetBGSelectItem(lw->page.last_page);
                if (NULL != opt)
                    opt->content = lw->str_array[lw->sel_str_index];
            }
        case msg_return:
            WouoUI_PageReturn(page_addr);
            p_cur_ui->lw_var.win_x.pos_tgt = WOUOUI_BUFF_WIDTH; // 弹窗滑动退出动画
            ret = true;
            break;
        default:
            break;
        }
        if (msg_none != msg && NULL != lw->page.cb)
            lw->page.cb(&(lw->page), msg); // 任何输入的有效信息都会调用回调函数
    } else {
        if (msg_none != msg && NULL != lw->page.cb) { // 任何输入的有效信息都会调用回调函数
            ret = lw->page.cb(&(lw->page), msg);
            if (ret) { // 如果回调函数返回true的话退出该页面
                WouoUI_PageReturn(page_addr);
                p_cur_ui->lw_var.win_x.pos_tgt = WOUOUI_BUFF_WIDTH; // 弹窗滑动退出动画
            }
        }
    }
    return ret;
}
void WouoUI_ListWinPageIndicatorCtrl(PageAddr page_addr) {
    ListWin* lw = (ListWin*)page_addr;
    p_cur_ui->indicator.x.pos_tgt = WOUOUI_BUFF_WIDTH - LIST_WIN_W + (LIST_WIN_L_S >> 1);
    p_cur_ui->indicator.y.pos_tgt = p_cur_ui->lw_var.ind_y_temp;
    p_cur_ui->indicator.w.pos_tgt =
        MIN(WouoUI_GetStrWidthEx(lw->str_array[lw->sel_str_index], LIST_WIN_FONT, LIST_WIN_CFONT) +
                LIST_WIN_L_S,
            LIST_WIN_W - LIST_WIN_L_S - LIST_WIN_R_S);
    p_cur_ui->indicator.h.pos_tgt = LIST_WIN_LINE_H;
    WouoUI_GraphSetPenColor(2); // 反色绘制
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), p_cur_ui->indicator.x.pos_cur,
                          p_cur_ui->indicator.y.pos_cur, p_cur_ui->indicator.w.pos_cur,
                          p_cur_ui->indicator.h.pos_cur, LIST_WIN_BOX_R);
    WouoUI_GraphSetPenColor(1); // 实色绘制
}
void WouoUI_ListWinPageScrollBarCtrl(PageAddr page_addr) {
    ListWin* lw = (ListWin*)page_addr;
    p_cur_ui->scrollBar.y.pos_tgt =
        (int16_t)(lw->sel_str_index * WOUOUI_BUFF_HEIGHT / (lw->array_num - 1)) + 1;
    WouoUI_CanvasDrawLine_H(&(p_cur_ui->w_all), WOUOUI_BUFF_WIDTH - LIST_WIN_BAR_W,
                            WOUOUI_BUFF_WIDTH, 0);
    WouoUI_CanvasDrawLine_H(&(p_cur_ui->w_all), WOUOUI_BUFF_WIDTH - LIST_WIN_BAR_W,
                            WOUOUI_BUFF_WIDTH, WOUOUI_BUFF_HEIGHT - 1);
    WouoUI_CanvasDrawLine_V(&(p_cur_ui->w_all), WOUOUI_BUFF_WIDTH - ((LIST_WIN_BAR_W / 2) + 1), 0,
                            WOUOUI_BUFF_HEIGHT);
    WouoUI_CanvasDrawRBox(&(p_cur_ui->w_all), WOUOUI_BUFF_WIDTH - LIST_WIN_BAR_W, 0, LIST_WIN_BAR_W,
                          p_cur_ui->scrollBar.y.pos_cur, 0);
}

void WouoUI_ListWinPageInit(ListWin* lw,          // 列表弹窗对象
                            uint8_t array_num,    // 列表弹窗字符串数组的大小
                            String* str_array,    // 字符串数组
                            bool auto_set_bg_opt, // 是否自动设置背景中的选中项
                            CallBackFunc cb)      // 页面回调函数
{
    if (NULL == lw || NULL == str_array) {
        WOUOUI_LOG_E("lw is NULL or str_array is NULL!");
        return;
    }
    lw->page.page_type = type_listwin;
    WouoUI_PageInit((PageAddr)lw, cb);
    lw->array_num = array_num;
    lw->str_array = str_array;
    lw->auto_set_bg_opt = auto_set_bg_opt;
    lw->sel_str_index = 0;
    lw->page.methods = &(p_cur_ui->lw_mth);
    lw->bg_opt = NULL;
    for (uint8_t i = 0; i < array_num; i++) {
        if (NULL == str_array[i]) {
            WOUOUI_LOG_E("The %d th string of str_array in ListWin is NULL!!!", i);
            str_array[i] = (char*)WOUOUI_WIN_TXT_DEFAULT; // 使用默认文本做提示
        }
    }
}

void WouoUI_ListWinPageLastItem(ListWin* lw) {
    int16_t list_line_h = LIST_WIN_LINE_H;
    if (lw->sel_str_index == 0) {                          // 选中第一个的话
        if (p_cur_ui->upara->loop_param[LIST_WIN_LOOP]) {  // 同时loop参数开的话
            lw->sel_str_index = lw->array_num - 1;         // 选中最后一个
            if (lw->array_num > p_cur_ui->lw_var.line_n) { // 数目超出一页的最大数目
                p_cur_ui->lw_var.ind_y_temp = WOUOUI_BUFF_HEIGHT - list_line_h; // 更改box到最底
                p_cur_ui->lw_var.list_y.pos_tgt =
                    WOUOUI_BUFF_HEIGHT - (lw->array_num) * list_line_h; // 更改文字到最底
            } else // 没有超出数目则是到最后一个
                p_cur_ui->lw_var.ind_y_temp = (lw->array_num - 1) * list_line_h;
        }
    } else {                 // 没有选中第一个
        lw->sel_str_index--; // 选中减1
        if ((lw->sel_str_index - 1) <
            -((p_cur_ui->lw_var.list_y.pos_tgt) / list_line_h)) { // 光标盒子到页面顶了????
            if (!(WOUOUI_BUFF_HEIGHT % list_line_h))              // 上面剩余完整的行
                p_cur_ui->lw_var.list_y.pos_tgt += list_line_h;   // 文字下移
            else { // 上面的行不完整(list_line_h该项用于页面和行高非整除时)
                if (p_cur_ui->lw_var.ind_y_temp ==
                    WOUOUI_BUFF_HEIGHT -
                        list_line_h *
                            p_cur_ui->lw_var
                                .line_n) { // 文字往下走一行，且光标盒子置于0处，把上面的非整行去掉
                    p_cur_ui->lw_var.list_y.pos_tgt +=
                        (p_cur_ui->lw_var.line_n + 1) * list_line_h - WOUOUI_BUFF_HEIGHT;
                    p_cur_ui->lw_var.ind_y_temp = 0;
                } else if (p_cur_ui->lw_var.ind_y_temp == list_line_h)
                    p_cur_ui->lw_var.ind_y_temp = 0; // 上面整行直接移动光标盒子
                else
                    p_cur_ui->lw_var.list_y.pos_tgt += list_line_h; // 顶页整行，文字直接往下走
            }
        } else                                          // 光标盒子没到页面顶
            p_cur_ui->lw_var.ind_y_temp -= list_line_h; // 直接光标盒子往上走即可。
    }
}

void WouoUI_ListWinPageNextItem(ListWin* lw) {
    int16_t list_line_h = LIST_WIN_LINE_H;
    if (lw->sel_str_index == (lw->array_num) - 1) {       // 到最后一个选项了
        if (p_cur_ui->upara->loop_param[LIST_WIN_LOOP]) { // loop开关开,全部回到顶部
            p_cur_ui->lw_var.list_y.pos_tgt = 0;
            lw->sel_str_index = 0;
            p_cur_ui->lw_var.ind_y_temp = 0;
        }
    } else { // 不是最后一个选项
        lw->sel_str_index++;
        if ((lw->sel_str_index + 1) >
            ((p_cur_ui->lw_var.line_n) -
             (p_cur_ui->lw_var.list_y.pos_tgt) / list_line_h)) { // 光标到页面底
            if (!(WOUOUI_BUFF_HEIGHT % list_line_h))             // 上面剩余完整的行
                p_cur_ui->lw_var.list_y.pos_tgt -= list_line_h;  // 文字上移
            else {                                               // 非整行的情况
                if (p_cur_ui->lw_var.ind_y_temp == list_line_h * (p_cur_ui->lw_var.line_n - 1)) {
                    p_cur_ui->lw_var.list_y.pos_tgt -=
                        (p_cur_ui->lw_var.line_n + 1) * list_line_h - WOUOUI_BUFF_HEIGHT;
                    // 取出要向下移动的距离，对y_tgt来说就是向上移动的距离
                    p_cur_ui->lw_var.ind_y_temp = WOUOUI_BUFF_HEIGHT - list_line_h;
                } else if (p_cur_ui->lw_var.ind_y_temp ==
                           WOUOUI_BUFF_HEIGHT - list_line_h * 2) // 这种情况什么时候会出现呢嗯?
                    p_cur_ui->lw_var.ind_y_temp = WOUOUI_BUFF_HEIGHT - list_line_h;
                else
                    p_cur_ui->lw_var.list_y.pos_tgt -= list_line_h;
            }
        } else
            p_cur_ui->lw_var.ind_y_temp += list_line_h;
    }
}

// WinPageTemplate
//  bool WouoUI_ValWinPageIn(PageAddr page_addr)
//  {
//      ValWin* vw = (ValWin*)page_addr;
//      bool ret = false;

//     return ret;
// }
// void WouoUI_ValWinPageInParaInit(PageAddr page_addr)
// {
//     ValWin* vw = (ValWin*)page_addr;

// }
// void WouoUI_ValWinPageShow(PageAddr page_addr)
// {
//     ValWin* vw = (ValWin*)page_addr;

// }
// bool WouoUI_ValWinPageReact(PageAddr page_addr)
// {
//     bool ret = false;
//     InputMsg msg = WouoUI_MsgQueRead(&(p_cur_ui->msg_queue));
//     ValWin* vw = (ValWin*)page_addr;
//     switch (msg)
//     {
//         case msg_return:
//             WouoUI_PageReturn(page_addr);
//             ret = true;
//             break;

//         default:break;
//     }
//     return ret;
// }
// void WouoUI_ValWinPageIndicatorCtrl(PageAddr page_addr)
// {
//     ValWin* vw = (ValWin*)page_addr;

// }
// void WouoUI_ValWinPageScrollBarCtrl(PageAddr page_addr)
// {
//     ValWin* vw = (ValWin*)page_addr;

// }
