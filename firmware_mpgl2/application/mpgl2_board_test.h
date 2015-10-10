/***********************************************************************************************************************
File: mpgl2_board_test.h                                                                

Description:
Header file for mpgl2_board_test.c.

QUICK FIND OF ANT CHANNEL SETTINGS:
DEVICE NUMBER: 4660 (0x1234)
DEVICE TYPE:   96 (0x60)
TRANS TYPE:    1 (0x01)
NO PAIRING BIT
CHANNEL PERIOD 8192 (0x2000)
FREQUENCY: 50
***********************************************************************************************************************/

#ifndef __BOARD_TEST_H
#define __BOARD_TEST_H

/**********************************************************************************************************************
Type Definitions
***********************************************************************************************************************/


/***********************************************************************************************************************
Constants / Definitions
***********************************************************************************************************************/
/* BoardTest_u32Flags */
#define _LEDS_ON                              (u32)0x000000001  /* Set when audio and ANT test is active */
#define _AUDIO_ANT_ON                         (u32)0x000000008  /* Set when audio and ANT test is active */


#define   BOARDTEST_INIT_MSG_TIMEOUT      (u32)1000   /* Timeout for init display message to send */
#define   LOGO_UPDATE_PERIOD              (u32)100    /* Time in ms between LCD data refreshes */


/* BoardTest ANT radio settings */
#define	ANT_SERIAL_LO_BOARDTEST			          (u8)0x34
#define ANT_SERIAL_HI_BOARDTEST			          (u8)0x12
#define	ANT_DEVICE_TYPE_BOARDTEST					    DEVICE_TYPE_BOARDTEST
#define	ANT_TRANSMISSION_TYPE_BOARDTEST		    (u8)0x01

/* Default channel configuration parameters */
#define ANT_CHANNEL_BOARDTEST                 (u8)0
#define ANT_CHANNEL_TYPE_BOARDTEST            CHANNEL_TYPE_MASTER
#define ANT_NETWORK_BOARDTEST                 (u8)0

#define ANT_CHANNEL_PERIOD_DEC_BOARDTEST      (u16)8192
#define ANT_CHANNEL_PERIOD_HEX_BOARDTEST      (u16)0x2000
#define ANT_CHANNEL_PERIOD_HI_BOARDTEST		    (u8)0x20
#define ANT_CHANNEL_PERIOD_LO_BOARDTEST		    (u8)0x00

#define ANT_FREQUENCY_BOARDTEST						    (50)
#define ANT_TX_POWER_BOARDTEST					  	  RADIO_TX_POWER_0DBM

/* MPG Level 2 specific Device Types */
#define	DEVICE_TYPE_BOARDTEST		              (u8)0x60


/***********************************************************************************************************************
Function Declarations
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
void BoardTestInitialize(void);
void BoardTestRunActiveState(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/
bool BoardTestUpdateLogoPosition(void);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void BoardTestSM_Idle(void);             
static void BoartTestSM_Error(void);         
static void BoardTestSM_FailedInit(void);         


#endif /* __BOARD_TEST_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
