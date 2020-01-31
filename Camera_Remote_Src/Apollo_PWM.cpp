#include "Apollo_PWM.h"


//***********
//Apollo PWM Timer Functions
//***********

//
//  < Timer_setup >
//    Initializes timer and sets interrupts for PWM
//
void pwm::timer_setup () 
{
  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

  am_bsp_low_power_init();

  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_XTAL_START, 0);
  
  //
  // Configure the output pin.
  //
  am_hal_ctimer_output_config(IR_PWM_TIMER,
                              IR_PWM_TIMER_SEG,
                              IR_LED,
                              AM_HAL_CTIMER_OUTPUT_NORMAL,
                              AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA);  
  //
  //Configure a timer to drive the LED.
  //  
  am_hal_ctimer_config_single(IR_PWM_TIMER,                     
                              IR_PWM_TIMER_SEG,                 
                             (AM_HAL_CTIMER_FN_PWM_REPEAT    |  
                              PWM_CLK                        |
                              AM_HAL_CTIMER_INT_ENABLE) ); 
  
  //
  // Set up initial timer periods.
  //  
  am_hal_ctimer_period_set(IR_PWM_TIMER, 
                           IR_PWM_TIMER_SEG,
                           PERIOD, ON_TIME);
  
  //
  // Enable interrupts for the Timer we are using on this board.
  //  
  am_hal_ctimer_int_enable(IR_PWM_TIMER_INT);
  NVIC_EnableIRQ(CTIMER_IRQn);
  am_hal_interrupt_master_enable();
}

//
//  < Timer_shutdown >
//    Stops timer if on currently, and disables PWM timer interrupts
//
void pwm::timer_shutdown ()
{
  pwm::stop();
  
  am_hal_ctimer_int_disable(IR_PWM_TIMER_INT);
  NVIC_DisableIRQ(CTIMER_IRQn);
}

//
//  < Start >
//    Starts PWM timer
//
void pwm::start ()
{
  am_hal_ctimer_start(IR_PWM_TIMER, IR_PWM_TIMER_SEG);
}

//
// < Stop >
//    Stops and clears the timer, and clears the timer interrupt
//
void pwm::stop () 
{
  am_hal_ctimer_clear(IR_PWM_TIMER, IR_PWM_TIMER_SEG);
  am_hal_ctimer_int_clear(IR_PWM_TIMER_INT);   
}

//
//  < Am_ctimer_isr > ~Interrupt Service Routine~
//    Called by PWM timer interrupt
//
extern "C" void am_ctimer_isr (void)
{  
  am_hal_ctimer_clear(IR_PWM_TIMER, IR_PWM_TIMER_SEG);
  am_hal_ctimer_int_clear(IR_PWM_TIMER_INT);
}

//***********
//END Apollo PWM Timer Functions
//***********