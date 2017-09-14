/* 
 * File:   main.c
 * Author: A19356
 *
 * Created on August 22, 2017, 7:42 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include "main.h"

#define  uint8  unsigned char
#define lock_RTCC()      RTCWREN = 0;
#define on_RTCC()        RTCEN = 1;
#define off_RTCC()       RTCEN = 0;

/*
 * 
 */

void initialize_System(void);
void initialize_OSC(void);
void start_RTCC(void);
void unlock_RTCC(void);

uint8 BCDtoDEC(uint8 BCDByte); 
uint8 DECtoBCD(uint8 DECByte);


/*RTCC Attributes*/
typedef struct{
    uint8 YEAR;
    uint8 MONTH;
    uint8 DAY;
    uint8 WEEKDAY;
    uint8 HOURS;
    uint8 MINUTES;
    uint8 SECONDS;
    uint8 unknown;
} RTCC_VAL;

/*RTCC Operations*/
void ctor_RTCC(RTCC_VAL * const me, uint8 year, uint8 month, uint8 day, uint8 weekday, uint8 hours, uint8 minutes, uint8 seconds);
void read_RTCC(RTCC_VAL * const me);
void write_RTCC(RTCC_VAL * const me);

/*==============================================================================
 ===== MAIN ====================================================================
 =============================================================================*/
int main(int argc, char** argv) {
    initialize_System();
    RTCC_VAL Today;
    ctor_RTCC(&Today, 99, 12, 31, 2, 23,59,56);
    write_RTCC(&Today);    
    
    while(1){
        read_RTCC(&Today);
    }
    

    return (EXIT_SUCCESS);
}

void initialize_System(void){
    initialize_OSC();
    INTERRUPT_Initialize();
    PIN_MANAGER_Initialize();
}

void initialize_OSC(void){
    //OSC Freq = 31kHz, SCS = primary clock source
    OSCCON = 0b00000000;
    PADCFG1 = 0b00000100;
}

/*RTCC Constructor*/
void ctor_RTCC(RTCC_VAL * const me, uint8 year, uint8 month, uint8 day, uint8 weekday, uint8 hours, uint8 minutes, uint8 seconds){
    me->YEAR = DECtoBCD(year);
    me->MONTH = DECtoBCD(month);
    me->DAY = DECtoBCD(day);
    me->WEEKDAY = DECtoBCD(weekday);
    me->HOURS = DECtoBCD(hours);
    me->MINUTES = DECtoBCD(minutes);
    me->SECONDS = DECtoBCD(seconds);
}

/*Write RTCC to RTCPTR*/
void write_RTCC(RTCC_VAL * const me){
    
    unlock_RTCC();
    off_RTCC();
    
    RTCOE = 1;
    
    RTCCFGbits.RTCPTR1 = 1;     //Point to Year
    RTCCFGbits.RTCPTR0 = 1;
    
    RTCVALL = me->YEAR;
    RTCVALH = me->unknown;
    
    RTCVALL = me->DAY;
    RTCVALH = me->MONTH;
    
    RTCVALL = me->HOURS;
    RTCVALH = me->WEEKDAY; 
    
    RTCVALL = me->SECONDS;
    RTCVALH = me->MINUTES;
    
    on_RTCC();
    lock_RTCC();
}

/*Read RTCC from RTCPTR*/
void read_RTCC(RTCC_VAL * const me){
    while(RTCSYNC);     //check if safe to access registers wait 'till zero
    unlock_RTCC();
    
    RTCCFGbits.RTCPTR1 = 1;     //Point to Year
    RTCCFGbits.RTCPTR0 = 1;
    
    me->YEAR = RTCVALL;
    me->unknown = RTCVALH;
    
    me->DAY = RTCVALL;
    me->MONTH = RTCVALH;
    
    me->HOURS = RTCVALL;
    me->WEEKDAY = RTCVALH;
    
    me->SECONDS = RTCVALL;
    me->MINUTES = RTCVALH;
    
    lock_RTCC();
}

void unlock_RTCC(void){   
#asm
    MOVLB 0x0F          //sequence followed for unlocking RTCC
    MOVLW 0x55
    MOVWF EECON2
    MOVLW 0xAA
    MOVWF EECON2
    BSF RTCCFG, 5, 1
#endasm
}

uint8 BCDtoDEC (uint8 BCDByte){
    return(((BCDByte & 0xF0) >> 4) * 10) + (BCDByte &0x0F);
}

uint8 DECtoBCD (uint8 DECByte){
    return(((DECByte / 10) << 4) | (DECByte %10));
}