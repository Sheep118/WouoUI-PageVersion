#include "WouoUI_anim.h"

/**
 * @brief 非线性运动函数
 *
 * @param animStr[in/out] 动画结构体
 * @param aniTime[in] 动画时间参数
 * @param inrtime[in] 轮序间隔时间
 * @param ret[out] 动画是否结束的结果指针(用于统计所有动画是否结束,true表示结束)
 */
void WouoUI_Animation(AnimPos *animStr, uint16_t aniTime, uint16_t inrTime, uint8_t* ret) {
    uint8_t temp = false; //默认动画没有结束
    if (animStr->pos_cur != animStr->pos_tgt) {
        animStr->pos_err += (animStr->pos_tgt - animStr->pos_cur);
        animStr->pos_cur += animStr->pos_err / (aniTime / inrTime);
        animStr->pos_err %= (aniTime / inrTime);
    } else {
        animStr->pos_err = 0;
        temp = true;
    }
    (*ret) = temp && (*ret);
}

