/** \file antdefines.h
*
* Conatins definitions for ANT config parameters and ANT event codes
*
* $Name:  $
*/
/*
 * Copyright (C) 2006 Dynastream Innovations Inc. All rights reserved. 
 *
 * The intellectual property contained within this file is sole property of Dynastream Innovations Inc. 
 * Any distribution without the prior written consent of Dynastream Innovations Inc. is strictly prohibited.
 *
 * Dynastream Innovations Inc. does not guarantee error free operation of the intellectual property contained 
 * herein. 
 * 
 * Please forward all questions regarding this code to ANT Technical Support.
 * 
 * Dynastream Innovations Inc.
 * 228 River Avenue
 * Cochrane, Alberta, Canada
 * T4C 2C1
 * 
 * (P) (403) 932-9292
 * (F) (403) 932-6521
 * (TF) 1-866-932-9292 in North America
 * (E) support@thisisant.com
 * 
 * www.thisisant.com
 */ 

#ifndef ANTDEFINES_H
#define ANTDEFINES_H

//////////////////////////////////////////////
// ANT Clock Definition
//////////////////////////////////////////////
#define ANT_CLOCK_FREQUENCY                        ((ULONG)32768)          ///< ANT system clock frequency

//////////////////////////////////////////////
// Radio TX Power Definitions
//////////////////////////////////////////////
#define RADIO_TX_POWER_MASK                        ((UCHAR)0x03)
#define RADIO_TX_POWER_MINUS20DBM                  ((UCHAR)0x00)
#define RADIO_TX_POWER_MINUS10DBM                  ((UCHAR)0x01)
#define RADIO_TX_POWER_MINUS5DBM                   ((UCHAR)0x02)
#define RADIO_TX_POWER_0DBM                        ((UCHAR)0x03)
#define RADIO_TX_POWER_4DBM                        ((UCHAR)0x04)

//////////////////////////////////////////////
// Default System Definitions
//////////////////////////////////////////////
#define DEFAULT_CHANNEL_MESSAGE_FREQUENCY          ((ULONG)4)
#define DEFAULT_CHANNEL_MESSAGE_PERIOD             ((USHORT)( ANT_CLOCK_FREQUENCY / DEFAULT_CHANNEL_MESSAGE_FREQUENCY )) ///< 8192 (4Hz)
#define DEFAULT_RADIO_TX_POWER                     RADIO_TX_POWER_0DB      ///< ANT default RF power
#define DEFAULT_RADIO_CHANNEL                      ((UCHAR)66)             ///< 2400MHz + 1MHz * Channel Number = 2466MHz
#define DEFAULT_RX_SEARCH_TIMEOUT                  ((UCHAR)12)             ///< 12 * 2.5 seconds = 30 seconds

//////////////////////////////////////////////
// ID Definitions
//////////////////////////////////////////////
#define ID_MANUFACTURER_OFFSET                     ((UCHAR)3)
#define ID_DEVICE_TYPE_OFFSET                      ((UCHAR)2)
#define ID_DEVICE_NUMBER_HIGH_OFFSET               ((UCHAR)1)
#define ID_DEVICE_NUMBER_LOW_OFFSET                ((UCHAR)0)
#define ID_DEVICE_TYPE_PAIRING_FLAG                ((UCHAR)0x80)

//////////////////////////////////////////////
// Assign Channel Parameters
//////////////////////////////////////////////
#define PARAMETER_TX_NOT_RX                        ((UCHAR)0x10)
#define PARAMETER_MULTIPLE_ACCESS_CHANNEL          ((UCHAR)0x20)  
#define PARAMETER_NO_TX_GUARD_BAND                 ((UCHAR)0x40)   
#define PARAMETER_ALWAYS_RX_WILD_CARD_SEARCH_ID    ((UCHAR)0x40)

//////////////////////////////////////////////
// Assign Channel Types
//////////////////////////////////////////////
#define CHANNEL_TYPE_SLAVE                         ((UCHAR) 0x00)
#define CHANNEL_TYPE_MASTER                        ((UCHAR) 0x10)
#define CHANNEL_TYPE_MASTER_TX_ONLY                ((UCHAR) 0x50)
#define CHANNEL_TYPE_SHARED_SLAVE                  ((UCHAR) 0x20)
#define CHANNEL_TYPE_SHARED_MASTER                 ((UCHAR) 0x30)

//////////////////////////////////////////////
// Channel Status
//////////////////////////////////////////////
#define STATUS_UNASSIGNED_CHANNEL                  ((UCHAR)0x00)
#define STATUS_ASSIGNED_CHANNEL                    ((UCHAR)0x01)
#define STATUS_SEARCHING_CHANNEL                   ((UCHAR)0x02)
#define STATUS_TRACKING_CHANNEL                    ((UCHAR)0x03)
#define STATUS_OVERRUN                             ((UCHAR)0x40)
#define STATUS_UNDERRUN                            ((UCHAR)0x80)

//////////////////////////////////////////////
// Standard capabilities defines
//////////////////////////////////////////////
#define CAPABILITIES_NO_RX_CHANNELS                ((UCHAR)0x01)
#define CAPABILITIES_NO_TX_CHANNELS                ((UCHAR)0x02)
#define CAPABILITIES_NO_RX_MESSAGES                ((UCHAR)0x04)
#define CAPABILITIES_NO_TX_MESSAGES                ((UCHAR)0x08)
#define CAPABILITIES_NO_ACKD_MESSAGES              ((UCHAR)0x10)
#define CAPABILITIES_NO_BURST_TRANSFER             ((UCHAR)0x20)

//////////////////////////////////////////////
// Advanced capabilities defines
//////////////////////////////////////////////
#define CAPABILITIES_DETECT_OVERRUN_UNDERRUN       ((UCHAR)0x01)
#define CAPABILITIES_NETWORK_ENABLED               ((UCHAR)0x02)

//////////////////////////////////////////////
// Burst Message Sequence 
//////////////////////////////////////////////
#define CHANNEL_NUMBER_MASK                        ((UCHAR)0x1F)
#define SEQUENCE_NUMBER_MASK                       ((UCHAR)0xE0)
#define SEQUENCE_NUMBER_INC                        ((UCHAR)0x20)
#define SEQUENCE_NUMBER_ROLLOVER                   ((UCHAR)0x60)
#define SEQUENCE_LAST_MESSAGE                      ((UCHAR)0x80)

//////////////////////////////////////////////
// Shared Channel Commands / Datatypes
//////////////////////////////////////////////
#define SHARED_CMD_SLOT_AVALIBLE                   ((UCHAR)0xFF)
#define SHARED_CMD_BUSY_ACQUIRING                  ((UCHAR)0xFE)
#define SHARED_CMD_COMMAND_REQUEST_TO_ACQUIRE      ((UCHAR)0xFD)
#define SHARED_CMD_CONFIRM_ACQUIRED                ((UCHAR)0xFC)
#define SHARED_CMD_NO_SLOTS_AVAILABLE              ((UCHAR)0xFB)
//...
#define SHARED_TYPE_RELAY                          ((UCHAR)0x43)           
#define SHARED_TYPE_COUNTER                        ((UCHAR)0x42)
#define SHARED_TYPE_A_TO_D                         ((UCHAR)0x41)
#define SHARED_TYPE_DIGITAL                        ((UCHAR)0x40)           
#define SHARED_TYPE_UNDEFINED                      ((UCHAR)0x00)

///////////////////////////////////////////////////////////////////////
// AtoD SubTypes
///////////////////////////////////////////////////////////////////////
#define TEMPERATURE                                ((UCHAR)0xFE)
#define BATT_VOLTAGE                               ((UCHAR)0xFF)

//////////////////////////////////////////////
// Response / Event Codes
//////////////////////////////////////////////
#define RESPONSE_NO_ERROR                          ((UCHAR)0x00)             

#define EVENT_RX_SEARCH_TIMEOUT                    ((UCHAR)0x01)             
#define EVENT_RX_FAIL                              ((UCHAR)0x02)             
#define EVENT_TX                                   ((UCHAR)0x03)             
#define EVENT_TRANSFER_RX_FAILED                   ((UCHAR)0x04)             
#define EVENT_TRANSFER_TX_COMPLETED                ((UCHAR)0x05)             
#define EVENT_TRANSFER_TX_FAILED                   ((UCHAR)0x06)             
#define EVENT_CHANNEL_CLOSED                       ((UCHAR)0x07)
#define EVENT_RX_FAIL_GO_TO_SEARCH                 ((UCHAR)0x08)

#define CHANNEL_IN_WRONG_STATE                     ((UCHAR)0x15)           ///< returned on attempt to perform an action from the wrong channel state
#define CHANNEL_NOT_OPENED                         ((UCHAR)0x16)           ///< returned on attempt to communicate on a channel that is not open
#define CHANNEL_ID_NOT_SET                         ((UCHAR)0x18)           ///< returned on attempt to open a channel without setting the channel ID

#define TRANSFER_IN_PROGRESS                       ((UCHAR)0x1F)           ///< returned on attempt to communicate on a channel with a TX transfer in progress
#define TRANSFER_SEQUENCE_NUMBER_ERROR             ((UCHAR)0x20)           ///< returned when sequence number is out of order on a Burst transfer

#define INVALID_MESSAGE                            ((UCHAR)0x28)           ///< returned when the message has an invalid parameter
#define INVALID_NETWORK_NUMBER                     ((UCHAR)0x29)           ///< returned when an invalid network number is provided

#endif // !ANTDEFINES_H                                                    


