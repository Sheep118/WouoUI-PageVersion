#include "WouoUI_msg.h"

bool WouoUI_MsgQueIsEmpty(InputMsgQue* msg_que) {// 队列空
    return msg_que->font == msg_que->rear;
}

bool WouoUI_MsgQueIsFull(InputMsgQue* msg_que) {// 队列满
    return (msg_que->rear + 1) % INPUT_MSG_QUNEE_SIZE == msg_que->font;
}

/**
 * @brief 向msg队列发送消息
 *
 * @param msg 消息
 */
void WouoUI_MsgQueSend(InputMsgQue* msg_que, InputMsg msg) {
    if (!WouoUI_MsgQueIsFull(msg_que)) {
        msg_que->queue[msg_que->rear] = msg;
        msg_que->rear++;
        if (msg_que->rear == INPUT_MSG_QUNEE_SIZE)msg_que->rear = 0;
    }
}

/**
 * @brief 读msg队列消息
 *
 * @return InputMsg 消息
 */
InputMsg WouoUI_MsgQueRead(InputMsgQue* msg_que) {
    InputMsg msg = msg_none;
    if (!WouoUI_MsgQueIsEmpty(msg_que)) {
        msg = msg_que->queue[msg_que->font];
        msg_que->font++;
        if (msg_que->font == INPUT_MSG_QUNEE_SIZE)msg_que->font = 0;
    }
    return msg;
}

/**
 * @brief 清空msg队列消息
 */
bool WouoUI_MsgQueClear(InputMsgQue * msg_que) {
    msg_que->font = 0;
    msg_que->rear = 0;
    return WouoUI_MsgQueIsEmpty(msg_que);
}
