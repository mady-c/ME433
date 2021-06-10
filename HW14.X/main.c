#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <math.h>
//#include "font.h"
//#include "spi.h"
//#include "ST7789.h"
//#include <stdio.h>
//#include <string.h>


#define HALFSEC 12000000

#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
//#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FNOSC = FRCPLL //use internal oscillator -- might fluctuate in 
                              //speed <1% as temp changes but we don't care!

#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
//#pragma config POSCMOD = HS // high speed crystal mode
#pragma config POSCMOD = OFF //for internal resonator (RC?)
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz USE NORMALLY


// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations


void PosToDC(int pos);
void genSin(int * posArray );

int main(void) {
  // call initializations
  __builtin_disable_interrupts();
  int posArray[25];
  genSin(posArray);
  // set the pin you want to use to OC(5)
  RPB6Rbits.RPB6R = 0b0110; //OC5
  
  T2CONbits.TCKPS = 0b100;     // (N=16) set the timer prescaler so that you can use the largest PR2 value as possible without going over 65535 and the frequency is 50Hz
  // possible values for TCKPS are 0 for 1:1, 1 for 1:2, 2 for 1:4, 3 for 1:8, 4 for 1:16, 5 for 1:32, ...
  PR2 = 59999;              // max value for PR2 is 65535
  TMR2 = 0;                // initial TMR2 count is 0
  OC5CONbits.OCM = 0b110;  // PWM mode without fault pin; other OCxCON bits are defaults
  OC5RS = 30000;             // duty cycle = OCxRS/(PR2+1)
  OC5R = 30000;              // initialize before turning OCx on; afterward it is read-only
  OC5CONbits.OCTSEL = 0; //T2 = source
  T2CONbits.ON = 1;        // turn on Timer2
  OC5CONbits.ON = 1;       // turn on OCx
  int i=0;

  __builtin_enable_interrupts();
  // the rest of your code
  while(1){
      _CP0_SET_COUNT(0);
      while(_CP0_GET_COUNT()<480000){} //updates with 50Hz
      PosToDC(posArray[i]); //update position duty cycle with appropriate value for position in sine wave
      i++;
      if(i>25){
          i=0;
      }

  }
  // set OCxRS to get between a 0.5ms and 2.5ms pulse out of the possible 20ms (50Hz)
}

//this "function" converts angular position in degrees to a duty cycle input for the OC pin
void PosToDC(int pos){
    //50Hz = 20ms
    //0 deg = 0.5ms
    //180 deg = 2.5ms
//    float dutyms =0; //duty cycle in ms
    if(pos<10){
        pos = 10;
    } else if (pos>170){
        pos = 170;
    }
    
//    dutyms = (float)(2/180)*pos + 0.5; //equation of a line
//    float dutypercent = 0;
//    dutypercent = (float) dutyms/(float)20;
//    OC5RS = (int) (dutypercent*60000); // duty cycle = OC5RS/(PR2+1)  
    
    OC5RS = 60000*(pos*2/180+ 0.5)/20; //see above for math, this works better and doesn't involve type casting
}

//generates a sin wave with 25 points 
void genSin(int * posArray ){
    int i=0;
    for(i=0; i<=25; i++){
        posArray[i] = 90+80*sin(2*3.14159*i/25); //sin takes in radians. cnovert! 
    }
}