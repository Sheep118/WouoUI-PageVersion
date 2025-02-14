#ifndef __WOUOUI_MSG_H__
#define __WOUOUI_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "WouoUI_common.h"

//---------输入消息枚举类型
typedef enum {
    msg_up = 0x00,   // 上，或者last消息，表上一个
    msg_down,        // 下，或者next消息，表下一个
    msg_left,        // 左，混合模式时表示上一个
    msg_right,       // 右，混合模式时表示下一个
    msg_click,       // 点击消息，表确认，确认某一选项，回调用一次回调
    msg_return,      // 返回消息，表示返回，从一个页面退出
    msg_home,        // home消息，表回主界面(尚未设计，目前还没有设计对应的功能，默认以page_id为0的页面为主页面)
    msg_none = 0xFF, // none表示没有操作
} InputMsg;          // 输入消息类型，UI设计只供输入5种消息
// 输入消息缓冲队列的大小(至少为2)
#define INPUT_MSG_QUNEE_SIZE    WOUOUI_INPUT_MSG_QUNEE_SIZE
typedef struct 
{
    InputMsg queue[INPUT_MSG_QUNEE_SIZE]; // 输入消息队列存放数组
    uint8_t font; // 队列头指针
    uint8_t rear; // 队列尾指针
}InputMsgQue;

//--------msg_que处理相关函数                         
/**
 * @brief 判断消息是否为方向消息
 * 
 * @param msg 输入的消息
 * @return 如果消息为方向消息，返回true，否则返回false
 */
#define WouoUI_MsgIsDirectMsg(msg) ((uint8_t)msg < (uint8_t)msg_click)

/**
 * @brief 判断消息队列是否为空
 * 
 * @param msg_que 输入的消息队列指针
 * @return 如果消息队列为空，返回true，否则返回false
 */
bool WouoUI_MsgQueIsEmpty(InputMsgQue* msg_que);

/**
 * @brief 判断消息队列是否已满
 * 
 * @param msg_que 输入的消息队列指针
 * @return 如果消息队列已满，返回true，否则返回false
 */
bool WouoUI_MsgQueIsFull(InputMsgQue* msg_que);

/**
 * @brief 发送消息到消息队列
 * 
 * @param msg_que 输入的消息队列指针
 * @param msg 要发送的消息
 * @attention 如果消息队列已满，消息将不会被发送
 */
void WouoUI_MsgQueSend(InputMsgQue* msg_que, InputMsg msg);

/**
 * @brief 从消息队列读取消息
 * 
 * @param msg_que 输入的消息队列指针
 * @return 读取到的消息
 * @attention 如果消息队列为空，返回的消息可能无效
 */
InputMsg WouoUI_MsgQueRead(InputMsgQue* msg_que);

/**
 * @brief 清空消息队列
 * 
 * @param msg_que 输入的消息队列指针
 * @return 如果清空成功，返回true，否则返回false
 */
bool WouoUI_MsgQueClear(InputMsgQue * msg_que);

#ifdef __cplusplus
}
#endif

#endif // WOUOUI_MSG_H



