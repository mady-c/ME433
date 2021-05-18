#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <math.h>
#include "spi.h"

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
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations


void voltageSend(unsigned short V, unsigned char C);
void delay(int counts);
void genWaveforms(unsigned short *voltA, unsigned short *voltB);

int main() {
    unsigned short voltA[100], voltB[100]; //initialize A and B waveforms
    //voltA: 2Hz sine wave
    //voltB: 1 Hz triangle wave 
    
    
    __builtin_disable_interrupts();
    genWaveforms(voltA, voltB);
    //next few lines from uart code
    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG (debugger) to get pins back
    DDPCONbits.JTAGEN = 0;
    
    CS = 1; //initialize hi
    initSPI();
    
    __builtin_enable_interrupts();
    
    char curcount = 0;
    while(1){
        voltageSend(voltA[curcount], 0); //sends voltageA waveform via channel A
        voltageSend(voltB[curcount], 1); //sends voltageB 
        curcount++;
        if(curcount==100){
            curcount = 0; //rolls over count for infinity waves
        }
    }
}

void voltageSend(unsigned short V, unsigned char C){
    // V = 12 bit voltage, 0-4096
    // C = a or b channel
    unsigned short p; //printed value
    p = C<<15; //shifts to the left by 15, adds 15 zeros behind it
    p = p|(0b111<<12); // pastes 111 bit-wise into p
    p = p|(V); //pastes in V

    //Send voltage via SPI 
    CS = 0;
    spi_io(p>>8); //spi-io can only send 1 byte at a time
    spi_io(p); //should truncate automatically? check in debug
    CS = 1;
    delay(82000); //guess and check hehehe
    

}

void delay(int counts){
    _CP0_SET_COUNT(0);
    while (_CP0_GET_COUNT()<= counts){;} //do nothing
}

void genWaveforms(unsigned short *voltA, unsigned short *voltB){
    char i = 0;
    for(i=0; i<100; ++i){
        voltA[i] = (unsigned short) 2047 + 2048*sin(i/7.96); //7.96 = 100/4pi
        if(i<50){
            voltB[i] = (unsigned short) i*(82); //4095/100.0
        } if(i>=50){
            voltB[i] = (unsigned short)4095- i*(82);
        }
    }
}