#ifndef APOLLO_PWM_H
#define APOLLO_PWM_H

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

//***********
//Apollo PWM Timer Definitions:
//***********

#define   PWM_CLK           AM_HAL_CTIMER_HFRC_187_5KHZ
#define   IR_PWM_TIMER      3
#define   IR_PWM_TIMER_SEG  AM_HAL_CTIMER_TIMERA
#define   IR_PWM_TIMER_INT  AM_HAL_CTIMER_INT_TIMERA3

#define   PERIOD            4
#define   ON_TIME           2
#define   IR_LED            22    //Pin 5


//***********
//Apollo PWM Timer Functions:
//***********
namespace pwm
{

void timer_setup ();
void timer_shutdown ();
void start ();
void stop ();

} // pwm

extern "C" void am_ctimer_isr (void);

#endif