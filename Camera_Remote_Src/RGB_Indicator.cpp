#include "RGB_Indicator.h" 

//***********
//RGB LED Indicator Functions
//***********

//
//  < Setup >
//    Initializes RGB LED pins
//
void rgb::setup () 
{
  pinMode(RED, OUTPUT);
  pinMode(GRN, OUTPUT);
  pinMode(BLU, OUTPUT);
}

//
//  < Shutdown >
//    Sets all RGB LED pins to low
//
void rgb::shutdown () 
{
  digitalWrite(RED, LOW);
  digitalWrite(GRN, LOW);
  digitalWrite(BLU, LOW);
}

//
//  < Recv_wait_blink >
//    Sets RED pin to low or high depending on current state,
//    and delays for 750 ms
//
void rgb::recv_wait_blink ()
{
  digitalWrite(RED, digitalRead(RED) ? LOW : HIGH);

  delay(750);
}

//
//  < Recv_success >
//    Sets RED pin to low, and GRN pin to high
//
void rgb::recv_success ()
{
  digitalWrite(RED, LOW);
  digitalWrite(GRN, HIGH);
}

//
//  < Sht_setup >
//    Blinks BLU twice with 50 ms delays between
//
void rgb::sht_setup()
{
  digitalWrite(BLU, HIGH);
  delay(50);
  digitalWrite(BLU, LOW);
  delay(50);
  digitalWrite(BLU, HIGH);
  delay(50);
  digitalWrite(BLU, LOW);
}

//
//  < Interval_setup >
//    Sets RED pin to high
//
void rgb::interval_setup()
{
  digitalWrite(RED, HIGH);
}

//
//  < Interval_shutdown >
//    Blinks RED three times with 50 ms delays between
//
void rgb::interval_shutdown()
{
  digitalWrite(RED, LOW);
  delay(50);
  digitalWrite(RED, HIGH);
  delay(50);
  digitalWrite(RED, LOW);
  delay(50);
  digitalWrite(RED, HIGH);
  delay(50);
  digitalWrite(RED, LOW);
  delay(50);
  digitalWrite(RED, HIGH);
  delay(50);
  digitalWrite(RED, LOW);
} 

//
//  < Interval_count_shots >
//    Calls @shutdown
//
//    Blinks BLU once for 50 ms
//
void rgb::interval_count_shots()
{
  rgb::shutdown();
  
  digitalWrite(BLU, HIGH);
  delay(50);
  digitalWrite(BLU, LOW);
}

//
//  < Interval_count_shots_complete >
//    Blinks BLU three times with 50 ms delays between
//
void rgb::interval_count_shots_complete()
{
  digitalWrite(BLU, LOW);
  delay(50);
  digitalWrite(BLU, HIGH);
  delay(50);
  digitalWrite(BLU, LOW);
  delay(50);
  digitalWrite(BLU, HIGH);
  delay(50);
  digitalWrite(BLU, LOW);
  delay(50);
  digitalWrite(BLU, HIGH);
  delay(50);
  digitalWrite(BLU, LOW);
}

//
//  < Interval_count_timer >
//    Blinks GRN once for 50 ms
//
void rgb::interval_count_timer()
{
  digitalWrite(GRN, HIGH);
  delay(50);
  digitalWrite(GRN, LOW);
}

//
//  < Interval_count_timer_complete >
//    Blinks GRN three times with 50 ms delays between
//
void rgb::interval_count_timer_complete()
{
  digitalWrite(GRN, LOW);
  delay(50);
  digitalWrite(GRN, HIGH);
  delay(50);
  digitalWrite(GRN, LOW);
  delay(50);
  digitalWrite(GRN, HIGH);
  delay(50);
  digitalWrite(GRN, LOW);
  delay(50);
  digitalWrite(GRN, HIGH);
  delay(50);
  digitalWrite(GRN, LOW);
}

//***********
//END RGB Indicator Functions
//***********
