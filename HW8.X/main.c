#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
//#include <math.h>
#include "i2c_master_noint.h"

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

//Addresses for I2C (bank = 0))
unsigned char writeTo = 0b01000000; //address to write to io expander
unsigned char readFrom = 0b01000001; //address to read from io expander
unsigned char addrIODIRA = 0x00; //addr for IODIRA
unsigned char addrIODIRB = 0x01; //addr for IODIRB
unsigned char addrOLATA = 0x14; //address of OLATA 
unsigned char addrGPIOB = 0x13;//address of GPIOB

void initpins(void);
unsigned char ToggleGPA7(unsigned char OLATAstate); //deprecated
void setPin(unsigned char addr, unsigned char reg, unsigned char val);
unsigned char readPin(unsigned char addr, unsigned char reg);
void delay(void);


int main(){
   i2c_master_setup();
   initpins();
   unsigned char OLATAstate = 0x00;
   unsigned char lastButtonVal = 1;
   while(1){
       //First part of assignment:
//       OLATAstate ^= (1 << 7); //toggles value for GPA7
//       setPin(writeTo, addrOLATA, OLATAstate);
//       delay();
       //second part:
        unsigned char buttonVal = (readPin(readFrom, addrGPIOB)) & 1; //returns 0th bit
        if(buttonVal != lastButtonVal){
            OLATAstate ^= (1 << 7); //toggles value for GPA7
            setPin(writeTo, addrOLATA, OLATAstate);
        }
        lastButtonVal = buttonVal;
   }
}

void initpins(void){
    //initialize all A port to output
    i2c_master_start();
    i2c_master_send(writeTo);
    i2c_master_send(addrIODIRA);
    i2c_master_send(0x00);
    i2c_master_stop();
    
    //initialize GPB0 port to input
    i2c_master_start();
    i2c_master_send(writeTo);
    i2c_master_send(addrIODIRB);
    i2c_master_send(0b1);
    i2c_master_stop();    
}
unsigned char ToggleGPA7(unsigned char OLATAstate){ //deprecated
    OLATAstate ^= 1 << 7; //toggles bit 7 
    i2c_master_start();
    i2c_master_send(writeTo);
    i2c_master_send(addrOLATA);
    i2c_master_send(OLATAstate);
    i2c_master_stop(); 
    return OLATAstate;
}

void setPin(unsigned char addr, unsigned char reg, unsigned char val){
    i2c_master_start();
    i2c_master_send(addr); //should be writeTo
    i2c_master_send(reg);
    i2c_master_send(val);
    i2c_master_stop(); 
}

unsigned char readPin(unsigned char addr, unsigned char reg){
    i2c_master_start();
    i2c_master_send(writeTo); //(addr & (~1<<0)); //clears first bit so it becomes write to
    i2c_master_send(reg);
    i2c_master_restart();
    i2c_master_send(addr); //should be readFrom
    unsigned char msg = i2c_master_recv();
    i2c_master_ack(1); //no more messages requested
    i2c_master_stop(); 
    return(msg);
}


void delay(void){
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<=10000000){;} //do nothing for a bit
}