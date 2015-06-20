#if 0
/*-----------------------------------------------------------------------------
Function: ReadByte

Description:
Reads a byte from the ANT-connected SPI peripheral.
Adapted from Dynastream Innovations Inc reference design, BitSyncSerial.c.

Requires:
  - ANT_SPI points to the configured SPI peripheral attached to the ANT device
  - All communication setup has been performed and ANT is ready to send the byte
    as soon as the SRDY bit is pulsed.
  - u8Byte is in R0

Promises:
  - Returns the byte received.
*/

u8 ReadByte(void)
{
  u32 u32Byte = 0;
#if 0  
  SYNC_SRDY_ASSERT();
  SYNC_SRDY_DEASSERT();
  while( !(ANT_SPI->SR & SSP_SR_RNE) ); /* #PCUR# Should add timeout */
  
  /* Read the byte which clears RNE; return the byte */
  u32Byte = ANT_SPI->DR;

  /* If ANT has de-asserted (raised) the chip select line, then this was the last byte in the
  message so the P0_22_ANT_SPI_CS_IN should also be de-asserted. */
  if(GPIO0->FIOPIN & P0_22_ANT_SPI_CS_IN)
  {
    GPIO1->FIOSET = P1_15_ANT_SPI_CS_OUT;
  }
  
  /* The bits are backwards because the SSP module only works MSB first, so use
   inline assembly to flip the bits with a single instruction. */
  u32Byte = __RBIT(u32Byte)>>24;
#endif  
  return ((u8)u32Byte);
 
} /* end ReadByte() */


#if 0
/*-----------------------------------------------------------------------------
Function: WriteByte

Description:
Writes a byte to the ANT-connected device using the SPI peripheral.
Adapted from Dynastream Innovations Inc reference design, BitSyncSerial.c.

Requires:
  - ANT_SPI points to the configured SPI peripheral attached to the ANT device
  - All communication setup has been performed and ANT is ready to receive the byte
    as soon as the SRDY bit is pulsed.
  - u8Byte_ is the byte to be sent

Promises:
  - Returns the byte received.
*/
void WriteByte(u8 u8Byte_)
{
  volatile u8 u8Dummy;
  u32 u32Byte;

  /* Flip the bits in the data byte */
  u32Byte = __RBIT((u32)u8Byte_)>>24;

  /* Load the SPI data register then pulse SRDY to tell ANT to get the data */
  ANT_SPI->DR = (u8)u32Byte;
  SYNC_SRDY_ASSERT();
  SYNC_SRDY_DEASSERT();

  /* Wait for the data to clock out */
  while( !(ANT_SPI->SR & SSP_SR_TFE) ); /* #PCUR# Should add timeout */
  
  /* Since a receive always happens with a write but Host <-> ANT communication is
  always simplex, wait for SR_RNE then read the dummy byte to clear the receive buffer */
  while( !(ANT_SPI->SR & SSP_SR_RNE) ); /* #PCUR# Should add timeout */
  u8Dummy = (u8)(ANT_SPI->DR);

} /* end WriteByte() */

#endif  

void AntRxMessage(void)
/* Change this to look for received bytes and just validate them as they are put in.
SSP interrupt could fully handle this based on incoming bytes.  A successfully received
message could flag a call to process it.  How coupled should this be?  Can it be?  If 
the length of incoming bytes must be known, then the SSP interrupt has to have some
knowledge of the ANT protocol.  */ 
{
  u8 u8Byte, u8Checksum, u8Length;
  u8 *pu8AntRxBufferBookmark = Ant_pu8AntRxBufferNextChar; 

  /* If the Global _ANT_FLAGS_TX_INTERRUPTED flag has been set, then we have already read the TX_SYNC byte */
  if(G_u32AntFlags & _ANT_FLAGS_TX_INTERRUPTED)
  {
    /* Clear flag and load the value we know was already received to allow the function to proceed*/
    G_u32AntFlags &= ~_ANT_FLAGS_TX_INTERRUPTED;
    u8Byte = MESG_TX_SYNC;
  }
  else
  {
    /* Read the byte expected */
    u8Byte = ReadByte();                       
  }

  /* If the received byte is MESG_TX_SYNC, then we have an incoming ANT message */
  if (u8Byte == MESG_TX_SYNC)                     
  {
    u8Checksum = u8Byte;                         
    *Ant_pu8AntRxBufferNextChar = u8Byte;
    AdvanceAntRxBufferNextCharPointer();

    /* Read the second byte which is the length of the message and add two for the
    message ID and checksum that the following loop will receive and process */
    u8Byte = ReadByte();
    u8Length = u8Byte + 2;  
 
    /* Loop through remaining bytes and store them in the receive buffer */
    do
    {
      *Ant_pu8AntRxBufferNextChar = u8Byte;
      AdvanceAntRxBufferNextCharPointer();
      u8Checksum ^= u8Byte;                     
      u8Byte      = ReadByte();                 
    } while (--u8Length);
    
    /* Validate that a good message has been received */
    if (u8Checksum == u8Byte)      
    {
      Ant_u8AntNewRxMessages++;
      *Ant_pu8AntRxBufferNextChar = u8Byte;
      AdvanceAntRxBufferNextCharPointer();
    }
    /* If the message was not good, then reset Ant_pu8AntRxBufferNextChar back to where it started */
    else
    {
      Ant_pu8AntRxBufferNextChar = pu8AntRxBufferBookmark;
    }
  } /* end if (u8Byte == MESG_TX_SYNC) */
  
} /* end AntRxMessage() */

