#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <math.h>
#include "font.h"
#include "spi.h"
#include "ST7789.h"
#include <stdio.h>
#include <string.h>


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
//#pragma config FPLLODIV = DIV_4

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

void drawChar(unsigned short x, unsigned short y, unsigned short color, unsigned char letter);
void drawString(unsigned short x, unsigned short y, unsigned short color, unsigned char *msg);
void progressInit(unsigned short x, unsigned short y, unsigned short len); //draws a rectangle
void drawProgress(unsigned short x, unsigned short y, unsigned char place); //updates rectangle
void clearProgress(unsigned short x, unsigned short y, unsigned short color, unsigned short len);
void dispTime(unsigned short x, unsigned short y);
void dispFPS(unsigned short x, unsigned short y);
void delay(int counts);


int main(){
    __builtin_disable_interrupts();
    initSPI();
    LCD_init();
    __builtin_enable_interrupts();
    
     LCD_clearScreen(MAGENTA);
     char msg[100];
     sprintf(msg, "Hello World!!!");
     
     while(1){
//         drawChar(28, 32, WHITE, "H");
         drawString(28,32, WHITE, msg);
         delay(8000000);
         LCD_clearScreen(MAGENTA);
         delay(20000);
     }
    
}

void drawChar(unsigned short x, unsigned short y, unsigned short color, unsigned char letter){
    int i = 0, j = 0; //i = col, j = row
//    const unsigned char* lettarray = ASCII[letter-32];
    for(i=0; i<=4; i++){
        for(j=0; j<=7; j++){
            if((((ASCII[letter-32][i])>>j) & 1) == 1){
//                LCD_clearScreen(BLUE);
                LCD_drawPixel(x+i,y+j, color);
            }           
        }
    }
}

void drawString(unsigned short x, unsigned short y, unsigned short color, unsigned char *msg){
    int i = 0;
    unsigned short xinit=x; 
    progressInit(x, y+15, 2*strlen(msg));
    while(msg[i] != 0){
        _CP0_SET_COUNT(0);
        drawChar(x,y,color, msg[i]);
        x+=6;
        delay(10000);
//        dispTime(xinit+2*strlen(msg)+5, y+15);
        dispFPS(xinit+6*strlen(msg)+3, y);
        drawProgress(xinit, y+15, i);
        
        i+=1;
    }
}

void progressInit(unsigned short x, unsigned short y, unsigned short len){
    int i = 0, j = 0;
    for(i =0; i<(len+2); i++){
        LCD_drawPixel(x+i,y, WHITE);
        LCD_drawPixel(x+i,y+5, WHITE);
    }
    for(j=0; j<5;j++){
        LCD_drawPixel(x,y+j, WHITE);
        LCD_drawPixel(x+len+1,y+j, WHITE);
    }
}

void drawProgress(unsigned short x, unsigned short y, unsigned char place){
    int j = 0;
    while(j<5){
        LCD_drawPixel(x+2*place+1,y+j, GREEN);
        LCD_drawPixel(x+(2*place)+2,y+j, GREEN);
        j+=1;
    }
}

void clearProgress(unsigned short x, unsigned short y, unsigned short color, unsigned short len){
    int i=0, j=0;
    for(i=0;i<len; i++){
        for(j=0; j<5; j++){
            LCD_drawPixel(x+i,y+j, color);
        }
    }
}

void dispTime(unsigned short x, unsigned short y){
    int outms = (int) _CP0_GET_COUNT()/(24000000/1000000);
    int i=0;
    unsigned char m[10];
    sprintf(m, "%d", outms);
    while(m[i] != 0){
        drawChar(x,y,WHITE,m[i]);
        x+=6;
        i+=1;
    }
    drawChar(x+1,y,WHITE,' ');
    drawChar(x+7,y,WHITE,'n');
    drawChar(x+13,y,WHITE,'s');
    
    //crashes PIC
//    for(i=0; i<(floor(log10(abs(outms))) + 1); i++){
//        sprintf(*m, "%d", (short int)(outms/pow(10, i))%10);
//        drawChar(x+i,y,WHITE, &m);
//    }
}
void dispFPS(unsigned short x, unsigned short y){
    int outHz = (int) 24000000/_CP0_GET_COUNT();
    int i=0;
    unsigned char m[15];
    sprintf(m, "%d", outHz);
    while(m[i] != 0){
        drawChar(x,y,WHITE,m[i]);
        x+=6;
        i+=1;
    }
    drawChar(x+1,y,WHITE,' ');
    drawChar(x+7,y,WHITE,'f');
    drawChar(x+13,y,WHITE,'p');
    drawChar(x+19,y,WHITE,'s');
}


void delay(int counts){
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<counts){;}
}
