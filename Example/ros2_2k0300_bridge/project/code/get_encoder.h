#ifndef _get_encoder_h_
#define _get_encoder_h_

#include "zf_common_typedef.h"


void encoder_thd_entry(void);

void encoder_callback();

extern int16 g_encoder_left;
extern int16 g_encoder_right;

#endif
