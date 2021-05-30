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


// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

void drawChar(unsigned short x, unsigned short y, unsigned short color, unsigned char letter);
void drawString(unsigned short x, unsigned short y, unsigned short color, unsigned char *msg);
void progressInit(unsigned short x, unsigned short y, unsigned short len); //draws a rectangle
void drawProgress(unsigned short x, unsigned short y, unsigned char place); //updates rectangle
void clearFPS(unsigned short x, unsigned short y, unsigned short color, unsigned short len);
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
     sprintf(msg, "Hello World!!!"); //thing to be shown later
     
     while(1){ //infinite loop
//         drawChar(28, 32, WHITE, "H"); //writes just an H
         drawString(28,32, WHITE, msg);  //writes "hello world" at x = 28, y=32
         delay(8000000); //delay to show msg 
         LCD_clearScreen(MAGENTA); //clear screen 
         delay(20000); //wait a little bit before starting over
     }
    
}

//drawChar goes pixed by pixel to draw a letter
void drawChar(unsigned short x, unsigned short y, unsigned short color, unsigned char letter){
    int i = 0, j = 0; //i = col, j = row
//    const unsigned char* lettarray = ASCII[letter-32];
    for(i=0; i<=4; i++){
        for(j=0; j<=7; j++){
            if((((ASCII[letter-32][i])>>j) & 1) == 1){ //gives value of jth bit of ith column for the letter's corresponding row in the matrix
                LCD_drawPixel(x+i,y+j, color); //updates the pixel to the new color
            }           //if  !=1, do nothing (leave pixel as it was)
        }
    }
}

//drawString goes letter by letter to draw a string
void drawString(unsigned short x, unsigned short y, unsigned short color, unsigned char *msg){
    int i = 0;
    unsigned short xinit=x; //for progress bar and FPS use
    progressInit(x, y+15, 2*strlen(msg)); //initializes progress bar to be 2x the length of the string
    while(msg[i] != 0){ //0 = the exit character for a string
        _CP0_SET_COUNT(0); //for timing purposes
        drawChar(x,y,color, msg[i]); //current letter
        x+=6; //since each letter is 5 pixels wide
        delay(10000); //so FPS can keep up
//        dispTime(xinit+2*strlen(msg)+5, y+15); //initially I had the time between letters displayed next to progress bar
        clearFPS(xinit+6*strlen(msg)+3, y, MAGENTA, 25);
//        delay(100000);
        dispFPS(xinit+6*strlen(msg)+3, y); //displays current fps in ns
        
        drawProgress(xinit, y+15, i); //updates progress bar
        
        i+=1;
    }
}

//progressInit draws a rectangle with sides x = len+2, y = 5
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

//drawProgress updates the progress bar with 2 hashes for each letter 
void drawProgress(unsigned short x, unsigned short y, unsigned char place){
    int j = 0;
    while(j<5){
        LCD_drawPixel(x+2*place+1,y+j, GREEN);
        LCD_drawPixel(x+(2*place)+2,y+j, GREEN);
        j+=1;
    }
}

//clearFPS clears the printed FPS so it can be rewritten
void clearFPS(unsigned short x, unsigned short y, unsigned short color, unsigned short len){
    int i=0, j=0;
    for(i=0;i<len; i++){
        for(j=0; j<=7; j++){
            LCD_drawPixel(x+i,y+j, color);
        }
    }
}

//dispTime prints the time in nanoseconds between letters- next to progress bar
void dispTime(unsigned short x, unsigned short y){
    int outns = (int) _CP0_GET_COUNT()/(24000000/1000000); 
    int i=0;
    unsigned char m[10];
    sprintf(m, "%d", outns); //prints the number to a string
    while(m[i] != 0){ //goes thru same process as drawString, but gets called within drawString and I didn't want to deal with recursion
        drawChar(x,y,WHITE,m[i]);
        x+=6;
        i+=1;
    }
    drawChar(x+1,y,WHITE,' ');
    drawChar(x+7,y,WHITE,'n');
    drawChar(x+13,y,WHITE,'s');
    
    //crashes PIC
//    for(i=0; i<(floor(log10(abs(outms))) + 1); i++){ //number of digits in the number
//        sprintf(*m, "%d", (short int)(outms/pow(10, i))%10); //i-th digit of the number
//        drawChar(x+i,y,WHITE, &m);
//    }
}

//dispFPS displays frequency of letter writes - next to string
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


void delay(int counts){ //so PIC can catch up with itself and so things are visible
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<counts){;}
}
