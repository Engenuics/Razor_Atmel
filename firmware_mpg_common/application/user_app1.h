/**********************************************************************************************************************
File: user_app1.h                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
1. Follow the instructions at the top of user_app1.c
2. Use ctrl-h to find and replace all instances of "user_app1" with "yournewtaskname"
3. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
4. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
5. Add #include yournewtaskname.h" to configuration.h
6. Add/update any special configurations required in configuration.h (e.g. peripheral assignment and setup values)
7. Delete this text (between the dashed lines)
----------------------------------------------------------------------------------------------------------------------

Description:
Header file for user_app1.c

**********************************************************************************************************************/

#ifndef __USER_APP1_H
#define __USER_APP1_H

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
#define PART1
//#define PART2

/* Values for calculating the voltage displayed 
Voltage(mV) = Counts x mV/Count = Counts x VREF / ADC_RESOLUTION
Note maximum accumulator value is 2^32 / 2^12 = 2^20.  To minimize error, this value will be multiplied
first by VREF so could be as high as 2^20 x 3270.  Therefore maximum samples is 320.
*/
#define VREF              (u32)3270     /* Reference value in mV */
#define ADC_RESOLUTION    (u32)4096     /* ADC full scale counts (12-bits) */
#define SCALE_FACTOR      (u32)1        /* External scale factor (e.g. a resistor divider on the channel */

/* Values for managing the averaging rate and display rate */
#define NUM_AVERAGES      (u16)16        /* MUST BE POWER OF 2!!!  Max 256.  Number of samples averaged per displayed reading; sample rate increases with this number */
#define DISPLAY_RATE      (u16)512       /* MUST BE POWER OF 2!!!  LCD refresh rate for readings */
#define SAMPLE_RATE       (DISPLAY_RATE / NUM_AVERAGES)



/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
void UserApp1Initialize(void);
void UserApp1RunActiveState(void);

void AdcCallback(u16 u16Result_);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void UserApp1SM_Idle(void);    

static void UserApp1SM_Error(void);         
static void UserApp1SM_FailedInit(void);        


#endif /* __USER_APP1_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
