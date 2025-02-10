#ifndef __WOUOUI_ANIM__
#define __WOUOUI_ANIM__

#ifdef __cplusplus
extern "C" {
#endif

#include "WouoUI_common.h"

typedef struct _AnimPos {
    int16_t pos_cur;
    int16_t pos_tgt;
    int16_t pos_err;
} AnimPos;

void WouoUI_Animation(AnimPos *animStr, uint16_t aniTime, uint16_t inrTime, uint8_t* ret);

#ifdef __cplusplus
}
#endif

#endif
