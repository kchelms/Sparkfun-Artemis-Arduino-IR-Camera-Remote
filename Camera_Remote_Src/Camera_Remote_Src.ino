#include "Apollo_PWM.h"
#include "RGB_Indicator.h"


//***********
//Button Pin Definitions
//***********
#define   REC_SW            13
#define   REC_SW_PWR        12
#define   SHT_SW            2
#define   SHT_SW_PWR        0


//***********
//IR Remote Training Pin Definitions:
//***********
#define   IR_RECV_IN        7     //Infared Light Sensor
#define   IR_RECV_PWR       9


//***********
//Intervalometer Definitions:
//***********
#define   seconds()         (millis()/1000)
#define   minutes()         (seconds()/60)



//***********
//IR Remote Training Variables:
//***********
const unsigned int raw_code_size = 34;
const unsigned int key_code_size = 34;
volatile unsigned int rpt_code_size = false;

const unsigned int raw_code_split = round(raw_code_size / 2);

volatile int raw_code[raw_code_size][2];      //Unparsed Infared Sensor Information
volatile int key_code[key_code_size][2];      //Initial Infared Burst
volatile int rpt_code[20][2];                 //Repeating Infared Burst

volatile bool key_code_full = false;          
volatile bool rpt_code_full = false;          


//***********
//Button Functions:
//***********
namespace button
{
  void setup();
  bool debounce(int);
  void wakeup_isr(){}                         //Empty ISR to wake up from deep sleep
  
} //button


//***********
//IR Remote Training Functions / Variables:
//***********
namespace recv 
{
  volatile bool on = false;

  bool setup();
  void shutdown();
  void hi();
  void lo();
  void fill_key();
  void fill_rpt();
  void fill_rpt_fail();

} // recv


//***********
//Shutter Functions / Variables:
//***********
namespace sht 
{   
  volatile bool on = false;

  void setup();
  void shutdown();
  void key();
  void rpt();
  void delay(int);

} //sht


//***********
//Intervalometer Functions / Variables:
//***********
namespace interval
{
  volatile bool on = false;
  volatile int shots = false;
  volatile bool count_shots_complete = false;
  volatile int timer = false;
  volatile int interval;
  volatile bool setup_complete = false;
  volatile bool shutter = false;

  bool setup();
  void shutdown();
  void count_shots();
  void count_timer();
  void start();
  void shoot();
}

void setup () {   

  rgb::setup();
  button::setup();

  Serial.begin(115200);
}

void loop() {
  bool all_off = !interval::on && !sht::on && !recv::on;
  
  bool rec_sw_on = !button::debounce(SHT_SW) && button::debounce(REC_SW);
  bool sht_sw_on = !button::debounce(REC_SW) && button::debounce(SHT_SW);
  bool both_sw_on = button::debounce(SHT_SW) && button::debounce(REC_SW); 
  
  if(all_off && both_sw_on)                     //REC_SW + SHT_SW Tap 
    interval::on = interval::setup();           //  -> Start Intervalometer Setup

  else if(interval::setup_complete)             //Intervalometer Setup Complete
    interval::shoot();                          //  -> Fire Intervalometer

  else if(all_off && rec_sw_on)                 //REC_SW Tap -> Begin IR Recording
    recv::on = recv::setup();

  else if (recv::on)                            //IR Recording Active -> Record LED Blink
    rgb::recv_wait_blink();

  else if(all_off && sht_sw_on)                 //SHT_SW Tap -> Fire IR LED
    sht::setup();

  else if (all_off){                            //No Active Functions -> Sleep

    attachInterrupt(digitalPinToInterrupt(REC_SW), button::wakeup_isr, RISING);
    attachInterrupt(digitalPinToInterrupt(SHT_SW), button::wakeup_isr, RISING);
    
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
  }
}

//***********
//Button Functions
//***********

//
//  << Setup >>
//    Initializes all button switch pins
//
void button::setup () 
{
  pinMode(REC_SW, INPUT);
  pinMode(REC_SW_PWR, OUTPUT);

  pinMode(SHT_SW, INPUT);
  pinMode(SHT_SW_PWR, OUTPUT);


  //Power buttons
  digitalWrite(REC_SW_PWR, HIGH);
  digitalWrite(SHT_SW_PWR, HIGH);
}

//
//  << Debounce >>
//    Verifies button press on @pin. Returns true or false
//
//    True: Button was held for a period of at least 75 ms
//
//    False: Button was released before 75 ms
//
bool button::debounce (int pin)
{
  int wait_start = millis();
  int wait_current = 0;

  while((wait_current = millis() - wait_start) < 75) 
    if (digitalRead(pin) == LOW) return false;

  return true;
}

//***********
//END Button Functions
//***********


//***********
//IR Remote Training Functions:
//***********

//
//  recv::hi & recv::lo variables
//
volatile int recv_count;
volatile int recv_start;
volatile int recv_stop;


//
//  << Setup >>
//    Sets initial state for IR training variables. Initializes IR sensor pins.
//    Assigns rising interrupt for IR sensor.
//    Returns true when complete.
//
bool recv::setup()
{
  recv_count = 0;

  key_code_full = false;

  pinMode(IR_RECV_IN, INPUT);
  pinMode(IR_RECV_PWR, OUTPUT);

  digitalWrite(IR_RECV_PWR, HIGH); //Power IR sensor

  delay(25); //Debounce delay

  attachInterrupt(digitalPinToInterrupt(IR_RECV_IN), recv::hi, RISING);
  
  attachInterrupt(digitalPinToInterrupt(SHT_SW), recv::shutdown, RISING);
  attachInterrupt(digitalPinToInterrupt(REC_SW), recv::shutdown, RISING);

  return true;
}

//
//  << Shutdown >>
//    Sets IR training functionality to off state
//
void recv::shutdown()
{ 
  if(!recv::on) return;

  delay(750);

  rgb::shutdown();
  
  recv::on = false;
  digitalWrite(IR_RECV_PWR, LOW); //Power off IR sensor
}

//
//  << Hi >> ~Interrupt Service Routine~
//    Called by IR sensor rising interrupt.
//
//    Records time since previous IR sensor falling interrupt into current 
//    index of the @raw_code array
//
//    Assigns Falling interrupt to IR sensor.
//
//    Deassigns interrupts from IR sensor, and calls @fill_key or @fill_rpt 
//    when @raw_code is full.
//
void recv::hi() 
{ 
  attachInterrupt(digitalPinToInterrupt(IR_RECV_IN), recv::lo, FALLING);
 
  recv_stop = micros() - recv_start;
  recv_start = micros();
  
  if(recv_count < raw_code_size) {
    if(recv_stop > 0){
      raw_code[recv_count][0] = recv_stop;
      raw_code[recv_count][1] = 0;

      recv_count++;
    }
  }
    
  else {
    detachInterrupt(digitalPinToInterrupt(IR_RECV_IN));
    
    if(!key_code_full) recv::fill_key(); //Calls @fill_key if key is empty

    else recv::fill_rpt();               //Calls @fill_rpt if key is full
  }
}

//
//  << Lo >> ~Interrupt Service Routine~
//     Called by IR sensor falling interrupt
//
//      Identical to @hi
// 
void recv::lo() 
{
  attachInterrupt(digitalPinToInterrupt(IR_RECV_IN), recv::hi, RISING);

   
  recv_stop = micros() - recv_start;
  recv_start = micros();
  
  if(recv_count < raw_code_size) {
    if(recv_stop > 0){
      raw_code[recv_count][0] = recv_stop;
      raw_code[recv_count][1] = 1;

      recv_count++;
    }
  }
    
  else {
    detachInterrupt(digitalPinToInterrupt(IR_RECV_IN));
    
    if(!key_code_full) recv::fill_key();
    
    else recv::fill_rpt();
  }
}

//
//  << Fill_key >>
//    Called by @hi or @lo when @raw_code is full
//
//    Fills @key_code directly from @raw_code, calls @fill_rpt when complete
//
void recv::fill_key()
{  
  for(int i = 0; i < key_code_size; i ++) {  
    key_code[i][0] = raw_code[i][0];
    key_code[i][1] = raw_code[i][1];
  }  
       
  key_code_full = true;

  recv::fill_rpt();
}

//
//  << Fill_rpt >>
//    Called by @hi, @lo, or @fill_key
//
//    Finds a repeating sequence within raw_code, and then fills @rpt_code with
//    that sequence
//    
//    If the process fails (no repeating sequence to be found), @fill_rpt_fail
//    is called
//
void recv::fill_rpt() 
{  
  rpt_code_size = false;
  rpt_code_full = false;  
  int compare_size = 5;

  for(int i = raw_code_split + compare_size;                      //i iterates from the
    i < raw_code_size - compare_size && !rpt_code_size; i++) {    //middle of @raw_code + 5
    for(int j = 0; j < compare_size; j++) {                       //j iterates 1 -> 5

      //if @raw_code at middle + j == @raw_code at i + j do not match (+- 15), break loop
      if(!(abs(raw_code[j + raw_code_split][0] - raw_code[i + j][0]) <= 15)){
        break;
      }

      //if 5 values match, set @rpt_code_size to i - index at middle of @raw_code
      else if(j + 1 == compare_size) {
        rpt_code_size = i - raw_code_split;
      }
    }
  }

  if(rpt_code_size) {                       //Success case (if @rpt_code_size has been set)
    for(int i = 0; i < 20; i++) {
      if(i < rpt_code_size) {               //Write @raw_code up to index @rpt_code_size to 
                                            // @rpt_code

        rpt_code[i][0] = raw_code[i + raw_code_split][0];
        rpt_code[i][1] = raw_code[i + raw_code_split][1];
      }
  
      else {                                //Write 0 to @rpt_code after index @rpt_code_size
        rpt_code[i][0] = 0;
        rpt_code[i][1] = 0;
      }
    }  

    rgb::recv_success();
    recv::shutdown();
    rpt_code_full = true;
  }  

  else recv::fill_rpt_fail();                 //Fail case
}

//
//  < Fill_rpt_fail > 
//    Called by @fill_rpt
//
//    Resets @raw_code index (@recv_count), and re-assigns rising interrupt
//    to the IR sensor
//
//    This sets up to fill @raw_code again and attempt @fill_rpt again
//
void recv::fill_rpt_fail() 
{
  recv_count = 0;

  attachInterrupt(digitalPinToInterrupt(IR_RECV_IN), recv::hi, RISING);
}

//***********
//END IR Remote Training Functions
//***********


//***********
//Shutter Functions
//***********

//
//  < Setup >
//    Only if IR training has succeeded:
//
//    Calls @pwm::timer_setup to prepare 38 khz IR led pulse
//
//    Calls @key
//
void sht::setup ()
{
  if(rpt_code_full){  
    pwm::timer_setup();
    
    rgb::sht_setup();
    
    sht::key();
  }
}

//
//  < Shutdown >
//    Calls @pwm::timer_shutdown()
//
//    Resets Shutter state to off
//
void sht::shutdown ()
{
  pwm::timer_shutdown();

  sht::on = false;
}

//
//  < Key >
//    Calls @pwm::start, @pwm::stop and @delay in order to output 
//    the code stored in @key_code through the IR LED
//
void sht::key () 
{  
  for (int i = 1; i < key_code_size; i++){
    !key_code[i][1] ? pwm::start() : pwm::stop();

    sht::delay(key_code[i][0]);
  }

  interval::shutter = true;
  sht::rpt();
}

//
//  < Rpt >
//    Calls @pwm::start, @pwm::stop and @delay in order to output 
//    the code stored in @rpt_code through the IR LED
//
//    Calls itself recursively in order to repeat 
//
void sht::rpt ()
{
  if(digitalRead(SHT_SW) == HIGH || interval::shutter) {
    
    for(int i = 0; i < 20 && rpt_code[i][0] > 0; i++) {
      !rpt_code[i][1] ? pwm::start() : pwm::stop();

      sht::delay(rpt_code[i][0]);
    }

    interval::shutter = false;
    sht::rpt();
  }

  else sht::shutdown();
}

//
//  < Delay >
//    Called by @key and @rpt to space pulses of the IR output
//
void sht::delay (int delay_time)
{  
  int delay_start = micros();
  int delay_current = 0;

  while (delay_current < delay_time) {
    delay_current = micros() - delay_start;
  }
}

//***********
//END Shutter Functions
//***********


//***********
//Intervalometer Functions
//***********

//
//  < Setup >
//    Sets initial state for intervalometer variables. Initializes PWM timer.
//    Assigns rising interrupts for both buttons. Returns true when complete.
//
bool interval::setup ()
{
  interval::shots = 0;
  interval::timer = 0;


  delay(150);
    
  attachInterrupt(digitalPinToInterrupt(SHT_SW), interval::count_shots, RISING);
  attachInterrupt(digitalPinToInterrupt(REC_SW), interval::count_timer, RISING);

  pwm::timer_setup();

  rgb::interval_setup();

  return true;
}

//
//  < Shutdown >
//    Sets intervalometer functionality to off state
//
void interval::shutdown()
{
  if(!interval::on) return;
  
  rgb::interval_shutdown();

  pwm::timer_shutdown();
  
  interval::on = false;
  interval::shots = false;
  interval::count_shots_complete = false;
  interval::timer = false;
  interval::interval = false;
  interval::setup_complete = false;
  interval::shutter = false;

  delay(150);
}

//
//  < Count_shots > ~Interrupt Service Routine~
//    Called by SHT_SW rising interrupt
//
//    Increments @shots by 1
//
void interval::count_shots()
{
  rgb::shutdown();

  if(!interval::on) return;
  
  if(!rpt_code_full) interval::shutdown();

  else {
    rgb::interval_count_shots();
    interval::shots++;
  }
}

//
//  < Count_timer > ~Interrupt Service Routine~
//    Called by REC_SW rising interrupt
//
//    Assigns rising interrupt to SHT_SW on first call
//
//    Increments @timer by 15
//
void interval::count_timer()
{  
  if(shots){ 
    if(!count_shots_complete) {
      count_shots_complete = true;
      rgb::interval_count_shots_complete();
      attachInterrupt(digitalPinToInterrupt(SHT_SW), interval::start, RISING);      
    }

    else{
      rgb::interval_count_timer();
      timer += 15;
    }
  }

  else interval::shutdown();  
}

//
//  < Start > ~Interrupt Service Routine~
//    Called by SHT_SW rising interrupt
//
//    Determines interval length (@shots / @timer)
//
//    Calls @sht::key, and decrements @shots by 1
//
//    Assigns rising interrupts to both buttons
//
void interval::start() 
{
   if(timer == 0) interval::shutdown();
  
   interval::setup_complete = true;

   interval::interval = round(timer / shots);

   rgb::interval_count_timer_complete();
   interval::shutter = true;

   shots--;
   sht::key();
   
   attachInterrupt(digitalPinToInterrupt(SHT_SW), interval::shutdown, RISING);  
   attachInterrupt(digitalPinToInterrupt(SHT_SW), interval::shutdown, RISING);  
}

//
//  < Shoot >
//    If @shots is greater than 0:
//      waits length of @interval in seconds, then calls @sht::key and decrements 
//      @shots.
//
//      Calls itself recursively
//
//    Else:
//      Calls @shutdown
//
void interval::shoot(){
  if(shots){
    int wait_start = seconds();
    int wait_current = 0;

    while((wait_current = seconds() - wait_start) < interval);

    rgb::interval_count_shots();

    if(shots){
      interval::shots--;    
      interval::shutter = true;
      sht::key();
    }

    Serial.print("Yep \n");

    interval::shoot();
  }

  else interval::shutdown();
}

//***********
//END Intervalometer Functions
//***********
