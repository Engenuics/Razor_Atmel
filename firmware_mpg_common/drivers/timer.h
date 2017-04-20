/**********************************************************************************************************************
File: timer.h                                                                

Description:
Header file for timer.c

**********************************************************************************************************************/

#ifndef __TIMER_H
#define __TIMER_H

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/
typedef enum {TIMER_CHANNEL0 = 0, TIMER_CHANNEL1 = 0x40, TIMER_CHANNEL2 = 0x80} TimerChannelType;


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------------------
Generic Timer Setup
Internal timer clocks sources are based on MCK (48MHz)
TIMER_CLOCK1 = MCK/2 (41.7ns / tick)
TIMER_CLOCK2 = MCK/8 (167ns / tick)
TIMER_CLOCK3 = MCK/32 (667ns / tick)
TIMER_CLOCK4 = MCK/128 (2.67us / tick)
TIMER_CLOCK5(1) SLCK
*/

/* TC Block Mode Register */
#define TCB_BMR_INIT (u32)0x00100800
/*
    31 [0] Reserved
    30 [0] "
    29 [0] "
    28 [0] "

    27 [0] "
    26 [0] "
    25 [0] MAXFILT Filter period is 1 (not used)
    24 [0] "

    23 [0] "
    22 [0] "
    21 [0] "
    20 [1] "

    19 [0] FILTER IDX, PHA, PHB not filtered
    18 [0] Reserved
    17 [0] IDXPHB IDX pin drives TIOA1
    16 [0] SWAP No swap between PHA and PHB

    15 [0] INVIDX IDX directly drives quadrature logic
    14 [0] INVB PHB directly drive quadrature decoder logic
    13 [0] INVA PHA directly drive quadrature decoder logic
    12 [0] EDGPHA Edges detected on PHA and PHB

    11 [1] QDTRANS Quadrature decoding logic is inactive
    10 [0] SPEEDEN Speed measure disabled
    09 [0] POSEN Position measure disabled
    08 [0] QDEN Quadrature decoder logic disabled

    07 [0] Reserved
    06 [0] "
    05 [0] TC2XC2S TCLK2 connected to XC2
    04 [0] "

    03 [0] TC1XC1S TCLK1 connected to XC1
    02 [0] "
    01 [0] TC0XC0S TCLK0 connected to XC0
    00 [0] "
*/
      

/* Timer Channel 1 Setup
Note:
PA26 is an open pin avaialble as external clock input TCLK2 if set for Peripheral B function 
PB5 is an open pin available for TIOA1 I/O function if set for Peripheral A 
PB6 is an open pin available for TIOB1 I/O function if set for Peripheral A 
*/

/* Default Timer 1 interrupt period of just about 100us (1 tick = 2.67us); max 65535 */
#define TC1_RC_INIT (u32)37

#define TC1_CCR_INIT (u32)0x00000002
/*
    31-04 [0] Reserved

    03 [0] Reserved
    02 [0] SWTRG no software trigger
    01 [1] CLKDIS Clock disabled to start
    00 [0] CLKEN Clock not enabled 
*/

#define TC1_CMR_INIT (u32)0x000CC403
/*
    31 [0] BSWTRG no software trigger effect on TIOB
    30 [0] "
    29 [0] BEEVT no external event effect on TIOB
    28 [0] "

    27 [0] BCPC no RC compare effect on TIOB
    26 [0] "
    25 [0] BCPB no RB compare effect on TIOB
    24 [0] "

    23 [0] ASWTRG no TIOA software trigger effect
    22 [0] "
    21 [0] AEEVT no TIOA effect on external compare
    20 [0] "

    19 [1] ACPC Toggle TIOA on RC compare
    18 [1] "
    17 [0] ACPA No RA compare effect on TIOA
    16 [0] "

    15 [1] WAVE Waveform Mode is enabled
    14 [1] WAVSEL Up to RC mode
    13 [0] "
    12 [0] ENETRG external event has no effect

    11 [0] EEVT external event assigned to XC0
    10 [1] "
    09 [0] EEVTEDG no external event trigger
    08 [0] "

    07 [0] CPCDIS clock is NOT disabled when reaches RC
    06 [0] CPCSTOP clock is NOT stopped when reaches RC
    05 [0] BURST not gated
    04 [0] "

    03 [0] CLKI Counter incremented on rising edge
    02 [0] TCCLKS TIMER_CLOCK4 (MCK/128 = 2.67us / tick)
    01 [1] "
    00 [1] "
*/

#define TC1_IER_INIT (u32)0x00000010
/*
    31 -08 [0] Reserved 

    07 [0] ETRGS RC Load interrupt not enabled
    06 [0] LDRBS RB Load interrupt not enabled
    05 [0] LDRAS RA Load interrupt not enabled
    04 [1] CPCS RC compare interrupt is enabled

    03 [0] CPBS RB compare interrupt not enabled
    02 [0] CPAS RA Compare Interrupt enabled
    01 [0] LOVRS Lover's bit? Load Overrun interrupt not enabled 
    00 [0] COVFS Counter Overflow interrupt not enabled
*/

#define TC1_IDR_INIT (u32)0x000000EF
/*
    31-08 [0] Reserved 

    07 [1] ETRGS RC Load interrupt disabled
    06 [1] LDRBS RB Load interrupt disabled
    05 [1] LDRAS RA Load interrupt disabled
    04 [0] CPCS RC compare interrupt not disabled

    03 [1] CPBS RB compare interrupt disabled
    02 [1] CPAS RA Compare Interrupt not disabled
    01 [1] LOVRS Lover's bit?!? Load Overrun interrupt disabled 
    00 [1] COVFS Counter Overflow interrupt disabled
*/


/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/
void TimerSet(TimerChannelType eTimerChannel_, u16 u16TimerValue_);
void TimerStart(TimerChannelType eTimerChannel_);
void TimerStop(TimerChannelType eTimerChannel_);
u16 TimerGetTime(TimerChannelType eTimerChannel_);
void TimerAssignCallback(TimerChannelType eTimerChannel_, fnCode_type fpUserCallBack_);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
void TimerInitialize(void);
void TimerRunActiveState(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/
inline void TimerDefaultCallback(void);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void TimerSM_Idle(void);    

static void TimerSM_Error(void);         
static void TimerSM_FailedInit(void);        


#endif /* __TIMER_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
