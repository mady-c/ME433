#include <sys/attribs.h>  // __ISR macro
#include <xc.h>
#define CS LATBbits.LATB8


#ifndef SPI__H__
#define SPI__H__


void initSPI();
unsigned char spi_io(unsigned short writebyte);

#endif // SPI__H__