/**********************************************************************************************************************
File: user_app2.h                                                                

Description:
Header file for user_app2.c

**********************************************************************************************************************/

#ifndef __USER_APP2_H
#define __USER_APP2_H

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/
typedef struct
{
  LedNumberType eLED;       /* LED to operate on */
  u32 u32Time;              /* Time of action */
  bool bOn;                 /* TRUE if this is an ON event */
  LedRateType eCurrentRate; /* Current rate for this LED */
} LedCommandType;

typedef struct
{
  LedCommandType eCommand;  /* Command information */
  void* psNextNode;         /* Pointer to next command node */
} LedDisplayListNodeType;

typedef struct
{
  u8 u8ListSize;                             /* Total size of the list */
  LedDisplayListNodeType* psFirstCommand;    /* Pointer to the first command in the list */
} LedDisplayListHeadType;


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
//#define USER_LIST_EXAMPLE         /* Define to load a default User LED command list */

#define LED_FADE_TIME             (u32)20                       /* On time in ms for each level of PWM during fade */
#define LED_TOTAL_FADE_TIME       (u32)(10 x LED_FADE_TIME)     /* Total fading time in ms for an LED turning on or off */

#define LCD_DMO_ON_CHAR_ADDRESS   (u8)0x43                      /* Address to show '<' when DMO code is running */
#define LCD_USR_ON_CHAR_ADDRESS   (u8)0x49                      /* Address to show '<' when USR code is running */
#define LCD_PAUSE_CHAR_ADDRESS    (u8)0x4D                      /* Starting address for the '||' symbol on the LCD */
#define LCD_DARK_CHAR_ADDRESS     (u8)0x53                      /* LCD address for the 'X' (dark) symbol */


/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/
void LedDisplayStartList(void);
bool LedDisplayAddCommand(LedDisplayListHeadType* psDisplayList_, LedCommandType* pCommandInfo_);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
void UserApp2Initialize(void);
void UserApp2RunActiveState(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/
void AllLedsOff(void);
void LoadLcdScreen(void);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void UserApp2SM_Idle(void);    
static void UserApp2SM_RunCommandList(void);

static void UserApp2SM_Error(void);         
static void UserApp2SM_FailedInit(void);        


#endif /* __USER_APP2_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
