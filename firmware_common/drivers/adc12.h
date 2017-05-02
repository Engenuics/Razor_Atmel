/**********************************************************************************************************************
File: adc12.h                                                                

Description:
Header file for adc12.c

**********************************************************************************************************************/

#ifndef __ADC12_APP_H
#define __ADC12_APP_H

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/
typedef enum {ADC12_CH0 = 0, ADC12_CH1 = 1, ADC12_CH2 = 2, ADC12_CH3 = 3, 
              ADC12_CH4 = 4, ADC12_CH5 = 5, ADC12_CH6 = 6, ADC12_CH7 = 7} Adc12ChannelType;


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
#define ADC12B_MR_INIT (u32)0x0F041700
/*
    31 [0] Reserved
    30 [0] "
    29 [0] "
    28 [0] "

    27 [1] SHTIM set for maximum time to allow for maximum source impedance
    26 [1] "
    25 [1] "
    24 [1] "

    23 [0] STARTUP set for maximum startup time at 1MHz ADC clock
    22 [0] "
    21 [0] "
    20 [0] "

    19 [0] "
    18 [1] "
    17 [0] "
    16 [0] "

    15 [0] PRESCAL is 23 (gives 1MHz ADC clock)
    14 [0] "
    13 [0] "
    12 [1] "

    11 [0] "
    10 [1] "
    09 [1] "
    08 [1] "

    07 [0] Reserved
    06 [0] "
    05 [0] SLEEP Normal mode
    04 [0] LOWRES 12-bit resolution

    03 [0] TRGSEL not applicable
    02 [0] "
    01 [0] "
    00 [0] TRGEN Hardware triggers disabled
*/

#define ADC12B_CHDR_INIT (u32)0x000000FF
/*
    31-08 [0] Reserved

    07 [1] CH7 disabled
    06 [1] CH6 disabled
    05 [1] CH5 disabled
    04 [1] CH4 disabled

    03 [1] CH3 disabled
    02 [1] CH2 disabled
    01 [1] CH1 disabled
    00 [1] CH0 disabled
*/

#define ADC12B_ACR_INIT (u32)0x00000101
/*
    31 [0] Reserved 
    30 [0] "
    29 [0] "
    28 [0] "

    27 [0] "
    26 [0] "
    25 [0] "
    24 [0] "

    23 [0] "
    22 [0] "
    21 [0] "
    20 [0] "

    19 [0] "
    18 [0] "
    17 [0] OFFSET Vrefin/2G
    16 [0] DIFF Single ended mode

    15 [0] Reserved
    14 [0] "
    13 [0] "
    12 [0] "

    11 [0] "
    10 [0] "
    09 [0] IBCTL Bias current control typical
    08 [1] 

    07 [0] Reserved
    06 [0] "
    05 [0] "
    04 [0] "

    03 [0] "
    02 [0] "
    01 [0] GAIN 1
    00 [1] "
*/


#define ADC12B_EMR_INIT (u32)0x00040001
/*
    31-24 [0] Reserved

    23 [0] OFF_MODE_STARTUP_TIME target 40us
    22 [0] (4 + 1)*8/1MHz = 40us
    21 [0] "
    20 [0] "

    19 [0] "
    18 [1] "
    17 [0] "
    16 [0] "

    15-4 [0] Reserved

    03 [0] Reserved
    02 [0] "
    01 [0] "
    00 [1] OFFMODES Use off mode
*/

#define ADC12B_IDR_INIT (u32)0x000FFFFF
/*
    31 - 20 [0] Reserved

    19 [1] RXBUFF: Receive Buffer Full Interrupt Disabled
    18 [1] ENDRX: End of Receive Buffer Interrupt Disabled
    17 [1] GOVRE: General Overrun Error Interrupt Disabled
    16 [1] DRDY: Data Ready Interrupt Disabled

    15 [1] OVRE7: Overrun Error Interrupt Disabled
    14 [1] OVRE6: Overrun Error Interrupt Disabled
    13 [1] OVRE5: Overrun Error Interrupt Disabled
    12 [1] OVRE4: Overrun Error Interrupt Disabled

    11 [1] OVRE3: Overrun Error Interrupt Disabled
    10 [1] OVRE2: Overrun Error Interrupt Disabled
    09 [1] OVRE1: Overrun Error Interrupt Disabled
    08 [1] OVRE0: Overrun Error Interrupt Disabled

    07 [1] EOC7: End of Conversion Interrupt Disabled
    06 [1] EOC6: End of Conversion Interrupt Disabled
    05 [1] EOC5: End of Conversion Interrupt Disabled
    04 [1] EOC4: End of Conversion Interrupt Disabled

    03 [1] EOC3: End of Conversion Interrupt Disabled
    02 [1] EOC2: End of Conversion Interrupt Disabled
    01 [1] EOC1: End of Conversion Interrupt Disabled
    00 [1] EOC0: End of Conversion Interrupt Disabled
*/

/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/
#if 0
void Adc12EnableChannel(Adc12ChannelType eAdcChannel_);
void Adc12DisableChannel(Adc12ChannelType eAdcChannel_);
#endif

bool Adc12StartConversion(Adc12ChannelType eAdcChannel_);
void Adc12AssignCallback(Adc12ChannelType eAdcChannel_, fnCode_u16_type fpUserCallback_);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
void Adc12Initialize(void);
void Adc12RunActiveState(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/
void Adc12DefaultCallback(u16 u16Result_);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void Adc12SM_Idle(void);    

static void Adc12SM_Error(void);         
static void Adc12SM_FailedInit(void);        


#endif /* __ADC12_APP_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
