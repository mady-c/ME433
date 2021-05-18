#include "spi.h"
#include<sys/attribs.h>  // __ISR macro
#include <xc.h>
//#define CS LATBbits.LATB8
// initialize SPI1
void initSPI() {
    // Pin B14 has to be SCK1

    // Turn off analog pins
    ANSELB = 0;
    ANSELA = 0;
    // Make an output pin for CS
    TRISBbits.TRISB8 = 0; //sets as output
    
//    CS = 1;
    
    // Set SDO1
    RPB13Rbits.RPB13R = 0b0011; //SDO1 (pin 24)
    // Set SDI1
    SDI1R = 0b0000; //A1 (pin 3) (not plugged in for hw6!)

    // setup SPI1
    SPI1CON = 0; // turn off the spi module and reset it
    SPI1BUF; // clear the rx buffer by reading from it
    SPI1BRG = 2399; // 1000 for 24kHz, 1 for 12MHz; // baud rate to 10 MHz [SPI1BRG = (48000000/(2*desired))-1]
    SPI1CON2bits.AUDEN = 0;
    SPI1CONbits.MODE32 = 0;
    SPI1CONbits.MODE16 = 0; //8 bit mode
    SPI1STATbits.SPIROV = 0; // clear the overflow bit
    SPI1CONbits.CKE = 1; // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1; // master operation
    SPI1CONbits.ON = 1; // turn on spi 
}


// send a byte via spi and return the response
//note to self: change name to spi1 for clarity in future use? 
unsigned char spi_io(unsigned short writebyte) {
  SPI1BUF = writebyte;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}

//put spidac in main.c instead (called voltageSend)
// void SPIDAC(unsigned short V, unsigned char C){
//     // unsigned short V = 0; //12 bit voltage, 0-4096
//     // unsigned char c = 1; //a or b channel
//     unsigned short p; //printed value
//     p = c<<15; //shifts to the left by 15, adds 15 zeros behind it
//     p = p|(0b111<<12); // pastes 111 bit-wise into p
//     p = p|V; //pastes in V


// }