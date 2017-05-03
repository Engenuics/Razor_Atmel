/** \file antmessage.h
*
*  Contains ANT message IDs and other definiations useful for ANT messaging
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

#ifndef ANTMESSAGE_H
#define ANTMESSAGE_H

/////////////////////////////////////////////////////////////////////////////
// Message Format
// Messages are in the format:
//
// AX XX YY -------- CK
//
// where: AX    is the 1 byte sync byte either transmit or recieve
//        XX    is the 1 byte size of the message (0-249) NOTE: THIS WILL BE LIMITED BY THE EMBEDDED RECEIVE BUFFER SIZE
//        YY    is the 1 byte ID of the message (1-255, 0 is invalid)
//        ----- is the data of the message (0-249 bytes of data)
//        CK    is the 1 byte Checksum of the message
/////////////////////////////////////////////////////////////////////////////
#define MESG_TX_SYNC                      ((UCHAR)0xA4)   /* ANT > HOST */
#define MESG_RX_SYNC                      ((UCHAR)0xA5)   /* HOST > ANT */
#define MESG_SIZE_OFFSET                  ((UCHAR)1)    
#define MESG_ID_OFFSET                    ((UCHAR)2)     
#define MESG_DATA_OFFSET                  ((UCHAR)4)     
#define MESG_RESPONSE_MESG_ID_OFFSET      ((UCHAR)4)
#define MESG_RESPONSE_CODE_OFFSET         ((UCHAR)5)
#define MESG_SYNC_SIZE                    ((UCHAR)1)
#define MESG_SIZE_SIZE                    ((UCHAR)1)
#define MESG_ID_SIZE                      ((UCHAR)1)
#define MESG_CHECKSUM_SIZE                ((UCHAR)1)
#define MESG_MAX_DATA_SIZE                ((UCHAR)9)
#define MESG_EXT_DATA_SIZE                ((UCHAR)13)
#define MESG_HEADER_SIZE                  (MESG_SYNC_SIZE + MESG_SIZE_SIZE + MESG_ID_SIZE)
#define MESG_FRAME_SIZE                   (MESG_HEADER_SIZE + MESG_CHECKSUM_SIZE)
#define MESG_SAVED_FRAME_SIZE             (MESG_SIZE_SIZE + MESG_ID_SIZE)
#define MESG_MAX_SIZE                     (MESG_EXT_DATA_SIZE + MESG_FRAME_SIZE)
#define MESG_BUFFER_SIZE                  (MESG_MAX_DATA_SIZE + MESG_SAVED_FRAME_SIZE)

//////////////////////////////////////////////
// Buffer Indices - use these indexes when the buffer does NOT include the SYNC byte
//////////////////////////////////////////////
#define BUFFER_INDEX_MESG_SIZE            ((UCHAR)0x00)
#define BUFFER_INDEX_MESG_ID              ((UCHAR)0x01)
#define BUFFER_INDEX_CHANNEL_NUM          ((UCHAR)0x02)
#define BUFFER_INDEX_VERSION_BYTE0        ((UCHAR)0x02)
#define BUFFER_INDEX_MESG_DATA            ((UCHAR)0x03)
#define BUFFER_INDEX_RESPONSE_MESG_ID     ((UCHAR)0x03)
#define BUFFER_INDEX_RESPONSE_CODE        ((UCHAR)0x04)
#define BUFFER_INDEX_SHARED_ADDRESS_LSB   ((UCHAR)0x03)
#define BUFFER_INDEX_SHARED_ADDRESS_MSB   ((UCHAR)0x04)
#define BUFFER_INDEX_SHARED_DATA_TYPE     ((UCHAR)0x05)
#define BUFFER_INDEX_EXT_DATA_FLAGS       ((UCHAR)(BUFFER_INDEX_MESG_DATA + 8))
#define BUFFER_INDEX_EXT_DATA             ((UCHAR)(BUFFER_INDEX_EXT_DATA_FLAGS + 1))

   
//////////////////////////////////////////////
// Message ID's
//////////////////////////////////////////////
#define MESG_INVALID_ID                   ((UCHAR)0x00)
#define MESG_EVENT_ID                     ((UCHAR)0x01)

#define MESG_APPVERSION_ID                ((UCHAR)0x3D)  ///< application interface version
#define MESG_VERSION_ID                   ((UCHAR)0x3E)  ///< protocol library version
#define MESG_RESPONSE_EVENT_ID            ((UCHAR)0x40)
#define MESG_RESTART_ID                   ((UCHAR)0x6F)
#define MESG_ENABLE_EXTENDED_ID           ((UCHAR)0x66)
#define MESG_LIB_CONFIG_ID                ((UCHAR)0x6E)
#define LIB_CONFIG_CHANNEL_ID_FLAG        ((UCHAR)0x80)
#define LIB_CONFIG_RSSI_FLAG              ((UCHAR)0x40)
#define LIB_CONFIG_RX_TIMESTAMP_FLAG      ((UCHAR)0x20)
#define LIB_CONFIG_DISABLE                ((UCHAR)0x00)
   
#define MESG_UNASSIGN_CHANNEL_ID          ((UCHAR)0x41)
#define MESG_ASSIGN_CHANNEL_ID            ((UCHAR)0x42)
#define MESG_CHANNEL_MESG_PERIOD_ID       ((UCHAR)0x43)
#define MESG_CHANNEL_SEARCH_TIMEOUT_ID    ((UCHAR)0x44)
#define MESG_CHANNEL_RADIO_FREQ_ID        ((UCHAR)0x45)
#define MESG_NETWORK_KEY_ID               ((UCHAR)0x46)
#define MESG_RADIO_TX_POWER_ID            ((UCHAR)0x47)
#define MESG_RADIO_CW_MODE_ID             ((UCHAR)0x48)
#define MESG_SEARCH_WAVEFORM_ID           ((UCHAR)0x49)

#define MESG_SYSTEM_RESET_ID              ((UCHAR)0x4A)
#define MESG_OPEN_CHANNEL_ID              ((UCHAR)0x4B)
#define MESG_CLOSE_CHANNEL_ID             ((UCHAR)0x4C)
#define MESG_REQUEST_ID                   ((UCHAR)0x4D)
#define MESG_OPEN_SCAN_CHANNEL_ID         ((UCHAR)0x5B)

#define MESG_BROADCAST_DATA_ID            ((UCHAR)0x4E)
#define MESG_ACKNOWLEDGED_DATA_ID         ((UCHAR)0x4F)
#define MESG_BURST_DATA_ID                ((UCHAR)0x50)

#define MESG_CHANNEL_ID_ID                ((UCHAR)0x51)
#define MESG_CHANNEL_STATUS_ID            ((UCHAR)0x52)
#define MESG_RADIO_CW_INIT_ID             ((UCHAR)0x53)
#define MESG_CAPABILITIES_ID              ((UCHAR)0x54)
#define MESG_SENSRCORE_DIGITAL_DATA       ((UCHAR)0x40)

//////////////////////////////////////////////
// Message Sizes
//////////////////////////////////////////////
#define MESG_INVALID_SIZE                 ((UCHAR)0)

#define MESG_RESPONSE_EVENT_SIZE          ((UCHAR)3)
#define MESG_CHANNEL_STATUS_SIZE          ((UCHAR)2)
#define MESG_VERSION_SIZE                 ((UCHAR)10)
#define MESG_ENABLE_EXTENDED_SIZE         ((UCHAR)2)
#define MESG_LIB_CONFIG_SIZE              ((UCHAR)2)

#define MESG_UNASSIGN_CHANNEL_SIZE        ((UCHAR)1)
#define MESG_ASSIGN_CHANNEL_SIZE          ((UCHAR)3)
#define MESG_CHANNEL_ID_SIZE              ((UCHAR)5)
#define MESG_CHANNEL_MESG_PERIOD_SIZE     ((UCHAR)3)
#define MESG_CHANNEL_SEARCH_TIMEOUT_SIZE  ((UCHAR)2)
#define MESG_CHANNEL_RADIO_FREQ_SIZE      ((UCHAR)2)
#define MESG_NETWORK_KEY_SIZE             ((UCHAR)9)
#define MESG_RADIO_TX_POWER_SIZE          ((UCHAR)2)
#define MESG_RADIO_CW_MODE_SIZE           ((UCHAR)3)
#define MESG_RADIO_CW_INIT_SIZE           ((UCHAR)1)
#define MESG_SEARCH_WAVEFORM_SIZE         ((UCHAR)3)

#define MESG_SYSTEM_RESET_SIZE            ((UCHAR)1)
#define MESG_OPEN_CHANNEL_SIZE            ((UCHAR)1)
#define MESG_CLOSE_CHANNEL_SIZE           ((UCHAR)1)
#define MESG_REQUEST_SIZE                 ((UCHAR)2)
#define MESG_CAPABILITIES_SIZE            ((UCHAR)4)

#define MESG_DATA_SIZE                    ((UCHAR)9)
#define EXT_DATA_CHANNEL_ID_SIZE          ((UCHAR)4)
#define EXT_DATA_RSSI_SIZE                ((UCHAR)3)
#define EXT_DATA_TIMESTAMP_SIZE           ((UCHAR)2)
   
   
#endif // !ANTMESSAGE_H
