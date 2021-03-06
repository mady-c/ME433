#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
//#include <math.h>
//#include <stdio.h>
//#include <string.h>
#include "ws2812b.h"

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

int main(){
    __builtin_disable_interrupts();
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    ws2812b_setup();
    __builtin_enable_interrupts();
    
    wsColor colorin[4];
//    wsColor *coloradd;
//    coloradd = &colorin;
    int colornum = 0;
    while(1){
//        _CP0_SET_COUNT(0);
//        while (_CP0_GET_COUNT()<HALFSEC){;}
//        LATAINV = 0b10000;
        colorin[0] = HSBtoRGB(colornum, 1.0, 1.0);
        colorin[1] = HSBtoRGB((colornum+30)%360, 1.0, 1.0);
        colorin[2] = HSBtoRGB((colornum+60)%360, 1.0, 1.0);
        colorin[3] = HSBtoRGB((colornum+90)%360, 1.0, 1.0);
        ws2812b_setColor(colorin, 1);
        colornum+= 30;
        if(colornum >360){
            colornum = 0;
        }
        _CP0_SET_COUNT(0);
       while(_CP0_GET_COUNT() <= 24000000*0.5){ }// 1/100 of a second delay
    }
}

