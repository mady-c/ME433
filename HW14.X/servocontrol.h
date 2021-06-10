#ifndef SERVOCONTROL__H__
#define SERVOCONTROL__H__

//#define IMAX 100

volatile int Duty_cycle;
volatile int CurrentWaveform[IMAX];



void servocontrol_setup(void);
//void ITEST_waveform_gen(void);

#endif