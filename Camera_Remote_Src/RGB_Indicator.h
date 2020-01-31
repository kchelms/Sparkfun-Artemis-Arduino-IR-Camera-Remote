#ifndef INDICATOR_H
#define INDICATOR_H

#include <Arduino.h>


//***********
//RGB Pin Definitions
//***********

#define   RED               A3
#define   GRN               A1
#define   BLU               A0


//***********
//RGB Functions
//***********

namespace rgb
{

void setup();
void shutdown();

void recv_wait_blink();
void recv_success();
void sht_setup();
void interval_setup();
void interval_shutdown();
void interval_count_shots();
void interval_count_shots_complete();
void interval_count_timer();
void interval_count_timer_complete();

} // rgb

#endif
