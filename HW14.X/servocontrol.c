#include "currentcontrol.h"
#include <xc.h>
#include <sys/attribs.h>

//5kHz ISR
static void T2setup(void){
    //sets up timer 2
    T2CONbits.TCKPS = 1; //N=1 for prescaler
    PR2 = 15999;        //frequency = 80M/(N*(PR2+1)) = 5kHz
    TMR2 = 0;           //initialize T2 to 0
    T2CONbits.ON = 1;   //turns T2 on
    //interrupt setup
    IPC2bits.T2IP = 5; //priority 5
    IPC2bits.T2IS = 0; //subpriority 0
    IFS0bits.T2IF = 0; //clear flag
    IEC0bits.T2IE = 1; //enable
}

//timer and output compare to generate 20 kHz PWM
static void PWMsetup(void){
    T3CONbits.TCKPS = 0;        //N=0 for prescaler
    PR3 = 3999;                 //frequency = 80M/(PR3+1) = 20 KHz
    TMR3 = 0;
    OC1CONbits.OCTSEL = 1;      //T3 for OC
    OC1CONbits.OCM = 0b110;     //PWM mode w/o fault pin
    OC1R = 3000;                //initializes duty cycle = OC1R/(PR3+1) = 25% (read only after this)
    // OC1RS = 3000;               //changes duty cycle w/equation above
    T3CONbits.ON = 1;           //T3 on
    OC1CONbits.ON = 1;          //OC1 on

}

void servocontrol_setup(void){
    T2setup();
    PWMsetup();
    Duty_cycle = 1000;      //25%
    TRISFbits.TRISF0 = 0;   //F0 = digital output
    LATFbits.LATF0 = 0;     //initialize F0 low
    E_current = 0;          //initialize current error to zero 
    Eint_current = 0;       //integral error zero
    ITEST_counter = 0;
}

//void ITEST_waveform_gen(void){
//    //pretty simple. waveform centered at center with amplitude A
//    int i=0, center =0, A=200;
//    for (i=0; i<IMAX; i++){
//        if (i< 3*IMAX/4){
//            CurrentWaveform[i] = center + A;
//            if (i < IMAX/2 && i >IMAX/4){
//                CurrentWaveform[i] = center - A;
//            }
//        } else{
//            CurrentWaveform[i] = center - A;
//        }
//    }
//}