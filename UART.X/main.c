#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>

#define HALFSEC 12000000
// DEVCFG0
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

void LEDcycle(void);
void readUART1(char * string, int maxLength);
void writeUART1(const char * string);

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG (debugger) to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0; //output
    LATAbits.LATA4 = 0; //off
    TRISBbits.TRISB4 = 1;
    
    //configure UART
    U1RXRbits.U1RXR = 0b0001; //U1RX is B6 , i.e. pin 15
    RPB7Rbits.RPB7R = 0b0001; //U1TX is B7, i.e. pin 16
    
    //still UART, copied from NU32.c
    U1MODEbits.BRGH = 0; 
    U1BRG = ((48000000 / 115200) / 16) - 1; //equation from datasheet
    U1MODEbits.PDSEL = 0; //no parity bit
    U1MODEbits.STSEL = 0; //1 stop bit
    U1STAbits.UTXEN = 1; //use tx 
    U1STAbits.URXEN = 1; //use rx
//    U1MODEbits.UEN = 2; //not use float control here! used on nu32
    U1MODEbits.ON = 1;
    
    
    __builtin_enable_interrupts();
    char entered = 0;
    
    while (1) {
        if (!PORTBbits.RB4){
            entered = 1;  
        }
        if(entered == 1){
            LEDcycle();
            entered = 0;
        }
        // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
        // remember the core timer runs at half the sysclk
            
    }
}

void LEDcycle(void){
    char m[100]; 
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<HALFSEC){
        LATAbits.LATA4 = 1;
    }
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<HALFSEC){
        LATAbits.LATA4 = 0;
    }
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<HALFSEC){
        LATAbits.LATA4 = 1;
    }
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<HALFSEC){
        LATAbits.LATA4 = 0;
    }
    sprintf(m, "Hello!\r\n");
    writeUART1(m);
    }
        
void readUART1(char * message, int maxLength){
    char data = 0;
    int complete = 0, num_bytes = 0;
    //loop until you get a '\r' or '\n'
    while (!complete){
        if(U1STAbits.URXDA){//if data is available
            data = U1RXREG;
            if ((data == '\n') || (data == '\r')){
                complete = 1;
            } else{
                message[num_bytes] = data;
                ++num_bytes;
                //rollover if array is too small
                if (num_bytes >= maxLength){
                    num_bytes = 0;
                }
                }
            }
        }
    message[num_bytes] = '\0';
    }

void writeUART1(const char * string){
    while (*string != '\0'){ //as long as it's not the null character
        while (U1STAbits.UTXBF){
            ; //wait until tx buffer is empty
        }
        U1TXREG = *string;
        ++string;
    }
}
    
    
 