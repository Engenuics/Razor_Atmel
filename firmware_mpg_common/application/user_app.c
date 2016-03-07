/**********************************************************************************************************************
File: user_app.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app as a template:
 1. Copy both user_app.c and user_app.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserAppInitialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserAppRunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserAppFlags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

#ifdef MPG2
extern PixelBlockType G_sLcdClearLine7;                /* From lcd_NHD-C12864LZ.c */
#endif /* MPG2 */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp_StateMachine;            /* The state machine function pointer */
static u32 UserApp_u32Timeout;                      /* Timeout counter used across states */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserAppInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserAppInitialize(void)
{
  u8 au8SongTitle[] = "Heart and Soul";
  
#ifdef MPGL1
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR, au8SongTitle);
#endif /* MPGL1 */

#ifdef MPG2
  PixelAddressType sStringLocation; 
  sStringLocation.u16PixelColumnAddress = LCD_CENTER_COLUMN - ( strlen((char const*)au8SongTitle) * (LCD_SMALL_FONT_COLUMNS + LCD_SMALL_FONT_SPACE) / 2 );
  sStringLocation.u16PixelRowAddress = LCD_SMALL_FONT_LINE7;
  
  LcdClearPixels(&G_sLcdClearLine7);
  LcdLoadString(au8SongTitle, LCD_FONT_SMALL, &sStringLocation);
#endif /* MPG 2 */

  /* If good initialization, set state to Idle */
  if( 1 /* Add condition for good init */)
  {
    UserApp_StateMachine = UserAppSM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp_StateMachine = UserAppSM_FailedInit;
  }

} /* end UserAppInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function UserAppRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserAppRunActiveState(void)
{
  UserApp_StateMachine();

} /* end UserAppRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/
/* LED mapping for right hand:
D5  WHITE
E5  PURPLE
F5  BLUE
G5  CYAN
A5  GREEN
A5S YELLOW
C6  ORANGE
D6  RED

LED mapping for left: 
A3S  PURPLE  
C4   BLUE
D4   CYAN
E4   GREEN
F4   YELLOW
A4   RED
OFF  WHITE
*/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void UserAppSM_Idle(void)
{
  /* Sound - related variable definitions */
  static u16 au16NotesRight[]    = {F5, F5, F5, F5, F5, E5, D5, E5, F5, G5, A5, A5, A5, A5, A5, G5, F5, G5, A5, A5S, C6, F5, F5, D6, C6, A5S, A5, G5, F5, NO, NO};
  static u16 au16DurationRight[] = {QN, QN, HN, EN, EN, EN, EN, EN, EN, QN, QN, QN, HN, EN, EN, EN, EN, EN, EN, QN,  HN, HN, EN, EN, EN, EN, QN,  QN, HN, HN, FN};
  static u16 au16NoteTypeRight[] = {RT, RT, HT, RT, RT, RT, RT, RT, RT, RT, RT, RT, HT, RT, RT, RT, RT, RT, RT, RT,  RT, HT, RT, RT, RT, RT, RT,  RT, RT, HT, HT};
  static u32 u32RightTimer = 0;
  static u16 u16CurrentDurationRight = 0;
  static u8 u8IndexRight = 0;
  static bool bNoteActiveNextRight = TRUE;
  static u16 u16NoteSilentDurationRight = 0;
  
  static u16 au16NotesLeft[]     = {F4, F4, A4, A4, D4, D4, F4, F4, A3S, A3S, D4, D4, C4, C4, E4, E4};
  static u16 au16DurationLeft[]  = {EN, EN, EN, EN, EN, EN, EN, EN, EN,  EN,  EN, EN, EN, EN, EN, EN};
  static u16 au16NoteTypeLeft[]  = {RT, RT, RT, RT, RT, RT, RT, RT, RT,  RT,  RT, RT, RT, RT, RT, RT};
  static u32 u32LeftTimer = 0;
  static u16 u16CurrentDurationLeft = 0;
  static u8 u8IndexLeft = 0;
  static bool bNoteActiveNextLeft = TRUE;
  static u16 u16NoteSilentDurationLeft = 0;
  
  u8 u8CurrentIndex;
  static u32 u32CycleCounter = 0;
  
  /* Light - related variable definitions */

  
  
#ifdef BUZZER1_ON
  /* Right Hand */
  if(IsTimeUp(&u32RightTimer, (u32)u16CurrentDurationRight))
  {
    u32RightTimer = G_u32SystemTime1ms;
    u8CurrentIndex = u8IndexRight;
    
    /* Set up to play current note */
    if(bNoteActiveNextRight)
    {
      if(au16NoteTypeRight[u8CurrentIndex] == RT)
      {
        u16CurrentDurationRight = au16DurationRight[u8CurrentIndex] - REGULAR_NOTE_ADJUSTMENT;
        u16NoteSilentDurationRight = au16NoteTypeRight[u8CurrentIndex];
        bNoteActiveNextRight = FALSE;
      }
    
      else if(au16NoteTypeRight[u8CurrentIndex] == ST)
      {
        u16CurrentDurationRight = STACCATO_NOTE_TIME;
        u16NoteSilentDurationRight = au16DurationRight[u8CurrentIndex] - STACCATO_NOTE_TIME;
        bNoteActiveNextRight = FALSE;
      }

      else if(au16NoteTypeRight[u8CurrentIndex] == HT)
      {
        u16CurrentDurationRight = au16DurationRight[u8CurrentIndex];
        u16NoteSilentDurationRight = 0;
        bNoteActiveNextRight = TRUE;

        u8IndexRight++;
        if(u8IndexRight == sizeof(au16NotesRight) / sizeof(u16) )
        {
          u8IndexRight = 0;
        }
      }

      /* Set the buzzer frequency and LED for the note (handle NO special case) */
      if(au16NotesRight[u8CurrentIndex] != NO)
      {
        PWMAudioSetFrequency(BUZZER1, au16NotesRight[u8CurrentIndex]);
        PWMAudioOn(BUZZER1);
        
        /* LED control */
#ifdef MPG1
        switch(au16NotesRight[u8CurrentIndex])
        {
          case D5:
            LedOn(WHITE);
            break;
            
          case E5:
            LedOn(PURPLE);
            break;
            
          case F5:
            LedOn(BLUE);
            break;
            
          case G5:
            LedOn(CYAN);
            break;
            
          case A5:
            LedOn(GREEN);
            break;
            
          case A5S:
            LedOn(YELLOW);
            break;
            
          case C6:
            LedOn(ORANGE);
            break;
            
          case D6:
            LedOn(RED);
            break;
            
          default:
            break;
            
        } /* end switch */
#endif /* MPG 1 */
      }
      else
      {                
        PWMAudioOff(BUZZER1);
#ifdef MPG1        
        LedOff(WHITE);
        LedOff(PURPLE);
        LedOff(BLUE);
        LedOff(CYAN);
        LedOff(GREEN);
        LedOff(YELLOW);
        LedOff(ORANGE);
        LedOff(RED);
#endif /* MPG1 */
      }
    }
    else
    {
      PWMAudioOff(BUZZER1);
      u32RightTimer = G_u32SystemTime1ms;
      u16CurrentDurationRight = u16NoteSilentDurationRight;
      bNoteActiveNextRight = TRUE;
 
#ifdef MPG1
      LedOff(WHITE);
      LedOff(PURPLE);
      LedOff(BLUE);
      LedOff(CYAN);
      LedOff(GREEN);
      LedOff(YELLOW);
      LedOff(ORANGE);
      LedOff(RED);
#endif /* MPG1 */
      u8IndexRight++;
      if(u8IndexRight == sizeof(au16NotesRight) / sizeof(u16) )
      {
        u8IndexRight = 0;
      }
    }
  }
#endif /* BUZZER1_ON */

#ifdef BUZZER2_ON
  /* Left Hand */
  if(IsTimeUp(&u32LeftTimer, (u32)u16CurrentDurationLeft))
  {
    u32LeftTimer = G_u32SystemTime1ms;
    u8CurrentIndex = u8IndexLeft;
    
    /* Set up to play current note */
    if(bNoteActiveNextLeft)
    {
      if(au16NoteTypeLeft[u8CurrentIndex] == RT)
      {
        u16CurrentDurationLeft = au16DurationLeft[u8CurrentIndex] - REGULAR_NOTE_ADJUSTMENT;
        u16NoteSilentDurationLeft = au16NoteTypeLeft[u8CurrentIndex];
        bNoteActiveNextLeft = FALSE;
      }
    
      else if(au16NoteTypeLeft[u8CurrentIndex] == ST)
      {
        u16CurrentDurationLeft = STACCATO_NOTE_TIME;
        u16NoteSilentDurationLeft = au16DurationLeft[u8CurrentIndex] - STACCATO_NOTE_TIME;
        bNoteActiveNextLeft = FALSE;
      }

      else if(au16NoteTypeLeft[u8CurrentIndex] == HT)
      {
        u16CurrentDurationLeft = au16DurationLeft[u8CurrentIndex];
        u16NoteSilentDurationLeft = 0;
        bNoteActiveNextLeft = TRUE;

        u8IndexLeft++;
        if(u8IndexLeft == sizeof(au16NotesLeft) / sizeof(u16) )
        {
          u8IndexLeft = 0;
          
          LedOff(LCD_RED);
          LedOff(LCD_GREEN);
          LedOff(LCD_BLUE);
          u32CycleCounter++;

          switch (u32CycleCounter % 8)
          {
            case 0:
              LedOn(LCD_RED);
              LedOn(LCD_GREEN);
              LedOn(LCD_BLUE);
              break;
            
            case 1:
              LedPWM(LCD_RED, LED_PWM_50);
              LedOn(LCD_BLUE);
              break;

            case 2:
              LedOn(LCD_BLUE);
              break;

            case 3:
              LedOn(LCD_GREEN);
              LedOn(LCD_BLUE);
              break;

            case 4:
              LedOn(LCD_GREEN);
              break;

            case 5:
              LedOn(LCD_RED);
              LedOn(LCD_GREEN);
              break;

            case 6:
              LedOn(LCD_RED);
              LedPWM(LCD_GREEN, LED_PWM_25);
              break;

            case 7:
              LedOn(LCD_RED);
              break;
              
            default:
              break;
          }
        }
      }

      /* Set the buzzer frequency for the note (handle NO special case) */
      if(au16NotesLeft[u8CurrentIndex] != NO)
      {
        PWMAudioSetFrequency(BUZZER2, au16NotesLeft[u8CurrentIndex]);
        PWMAudioOn(BUZZER2);

      }
      else
      {                
        PWMAudioOff(BUZZER2);
      }
    }
    else
    {
      PWMAudioOff(BUZZER2);
      u32LeftTimer = G_u32SystemTime1ms;
      u16CurrentDurationLeft = u16NoteSilentDurationLeft;
      bNoteActiveNextLeft = TRUE;
      
      u8IndexLeft++;
      if(u8IndexLeft == sizeof(au16NotesLeft) / sizeof(u16) )
      {
        u8IndexLeft = 0;

          LedOff(LCD_RED);
          LedOff(LCD_GREEN);
          LedOff(LCD_BLUE);
          u32CycleCounter++;

          switch (u32CycleCounter % 8)
          {
            case 0:
              LedOn(LCD_RED);
              LedOn(LCD_GREEN);
              LedOn(LCD_BLUE);
              break;
            
            case 1:
              LedPWM(LCD_RED, LED_PWM_50);
              LedOn(LCD_BLUE);
              break;

            case 2:
              LedOn(LCD_BLUE);
              break;

            case 3:
              LedOn(LCD_GREEN);
              LedOn(LCD_BLUE);
              break;

            case 4:
              LedOn(LCD_GREEN);
              break;

            case 5:
              LedOn(LCD_RED);
              LedOn(LCD_GREEN);
              break;

            case 6:
              LedOn(LCD_RED);
              LedPWM(LCD_GREEN, LED_PWM_25);
              break;

            case 7:
              LedOn(LCD_RED);
              break;
              
            default:
              break;
          }
      }
    }
  }
#endif /* BUZZER2_ON */
    
} /* end UserAppSM_Idle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserAppSM_Error(void)          
{
  UserApp_StateMachine = UserAppSM_Idle;
  
} /* end UserAppSM_Error() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserAppSM_FailedInit(void)          
{
    
} /* end UserAppSM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
