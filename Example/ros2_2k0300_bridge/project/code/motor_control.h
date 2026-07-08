
#ifndef _motor_control_h_
#define _motor_control_h_

#include "zf_common_typedef.h"

void pwm_control(int16 left_duty, int16 right_duty);
void servo_control(int16 servo_duty);


#endif
