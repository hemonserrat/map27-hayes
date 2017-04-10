/**
 * DTENL.C
 * Data Terminal Equipment Network Layer Definitions.
 *
 * This file is part of MAP27-HAYES Commands Bridge.
 *
 * Copyright (C) 2005,  Hernan Monserrat hemonserrat<at>gmail<dot>com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <hdw.h>   /* hardware specific interface */
#include <ustdlib.h>
#include <pi.h>	   /* Physical Interface	Layer */
#include <dll.h>   /* Data Link Layer  */
#include <dtenl.h> /* Data Terminal Equipment Network Layer */

#define STRANGE 31
#define MAXSST  22
#define MAXMST  88

uint8_t LastTrans=T_NS;
extern uint8_t _C;


static CODE uint8_t gCodes[]={
		MPT1343_BCD_RP,
		MPT1343_CCITT_RP,
		MPT1343_BINARY,
		MPT1343_BCD,
		MPT1343_CCITT,
		MPT1343_CCITT_5,
		MPT1343_PC8
};


/**
 *   Call the DTENL Idle action
 */
void DTENL_GetMessage(void)
{
	if( LastTrans==T_NS )
	{
		DTENL_PowerOn();
		OutputDebugString("PWR ON\n");
	}
	DLL_Idle();
}

/**
 * DTENL main entry point
 * @param type - Packet type
 * @param Buff - Buffer to process
 * @param bLen - Number of bytes in buffer
 */
void DTENL_input(uint8_t type, uint8_t *Buff, uint8_t bLen )
{

	switch( type )
	{
	 case I_LTR: {
			  LastTrans=T_PCKPEND;
				 umemmove( Buff, &Buff[4], bLen-4 );
				 DTENL_UserApp(Buff[0], Buff, bLen-4);
			  LastTrans=T_IDLE;
	 } break;
	 case I_NLP: {
			   DTENL_UserApp(M_PCKPEND, NULL, bLen);
			   LastTrans=T_IDLE;
	 } break;
	 case I_LRQ: {
			  if( bLen==1 ){
				/* link ready */
				LastTrans=T_IDLE;
				OutputDebugString( "LINK READY\n");
			  }	 
					DTENL_UserApp(M_ACTIVE, NULL, bLen);
	 } break;
	 case I_RTO:
	 case I_PON:
	 case I_LAK:
	 case I_LFD: { /* link failure */
				LastTrans=T_NS;
	 } break;
	}
}

/**
 * Send SST MAP27 message
 *
 * @param prefix - called party prefix
 * @param ident  - called party ident
 * @param messg  - buffer to send
 * @param len    - number of bytes to send
 * @return  RESULT - true/false
 */
bool DTENL_PostSSTMessage(uint8_t prefix, uint16_t ident, uint8_t *messg, uint8_t len)
{
 XDATA uint8_t Buffer[30];
 NLHEADER hdr;
 uint8_t coding;
			   if( LastTrans!=T_IDLE )
								return false;
		if( len>MAXSST)
		   return false;
		   coding=_C-'0';
  hdr.Msgtyp=SEND_SST;
  hdr.PFIX=prefix;
  hdr.Ident=SET_IDENT(ident);
  hdr.AdescLength=NOADD_ADDR;
  umemcpy( Buffer, (uint8_t*)&hdr, sizeof(uint8_t)*5);
  Buffer[5]= gCodes[coding];
  umemcpy( &Buffer[6], messg, len );
  OutputDebugString(">SST\n");
  DLL_input(I_NLP, Buffer, len+6 );
  LastTrans=T_STW;
  return true;
}

/**
 * Send MST MAP27  message
 *
 * @param prefix - called party prefix
 * @param ident  - called party ident
 * @param messg  - buffer to send
 * @param len    - number of bytes to send
 * @return  RESULT - true/false
 */
bool DTENL_PostMSTMessage(uint8_t prefix, uint16_t ident, uint8_t *messg, uint8_t len)
{
 XDATA uint8_t Buffer[110];
 NLHEADER hdr;
 uint8_t coding;
			   if( LastTrans!=T_IDLE )
								return false;
			   if( len>MAXMST )
			     return false;
		   coding=_C-'0';
  hdr.Msgtyp=SEND_MST;
  hdr.PFIX=prefix;
  hdr.Ident=SET_IDENT(ident);
  hdr.AdescLength=NOADD_ADDR;
  umemcpy( Buffer, (uint8_t*)&hdr, sizeof(uint8_t)*5);
  Buffer[5]=gCodes[coding];
  umemcpy( &Buffer[6], messg, len );
  OutputDebugString(">MST\n");
  DLL_input(I_NLP, Buffer, len+6 );
  LastTrans=T_STW;
  return true;
}

/**
 * Send STATUS MAP27 message
 *
 * @param prefix - called party prefix
 * @param ident  - called party ident
 * @param St     - Status number
 * @return   RESULT - true/false
 */
bool DTENL_Status(uint8_t prefix, uint16_t ident, uint8_t St)
{
 XDATA uint8_t Buffer[7];

			   if( LastTrans!=T_IDLE )
								return false;
			   if( St>STRANGE ) 
			            return false;

  Buffer[0]=SEND_STATUS;
  Buffer[1]=prefix;
  Buffer[2]=HIBYTE(ident);
  Buffer[3]=LOBYTE(ident);
  Buffer[4]=NOADD_ADDR;
  Buffer[5]=St; 
  OutputDebugString(">ST\n");
   DLL_input(I_NLP, Buffer,  6); 
  LastTrans=T_STW;
  return true;
}

#ifdef _EXTENDED_
/**
 * Normal Disconnect
 *
 * @param prefix - called party prefix
 * @param ident  - called party ident
 * @param cause  - reason for disconnect
 * @return   RESULT - true/false
 */
bool DTENL_NorDisconnect(uint8_t prefix, uint16_t ident, uint8_t cause)
{
 uint8_t Buffer[6];
 NLHEADER hdr;

  hdr.Msgtyp=DISC_NOR;
  hdr.PFIX=prefix;
  hdr.Ident=SET_IDENT(ident);
  hdr.AdescLength=cause;
  umemcpy( Buffer, (uint8_t*)&hdr, sizeof(uint8_t)*5);
  OutputDebugString(">DISC\n");
  DLL_input(I_NLP, Buffer,  5);
  LastTrans=T_IDLE;
  return true;
}

/**
 * Abnormal Disconnect
 *
 * @param prefix - called party prefix
 * @param ident  - called party ident
 * @param cause  - reason for disconnect
 * @return   RESULT - true/false
 */
bool DTENL_AbnorDisconnect(uint8_t prefix, uint16_t ident, uint8_t cause)
{
 uint8_t Buffer[6];
 NLHEADER hdr;

  hdr.Msgtyp=DISC_CAN;
  hdr.PFIX=prefix;
  hdr.Ident=SET_IDENT(ident);
  hdr.AdescLength=cause;
  umemcpy( Buffer, (uint8_t*)&hdr, sizeof(uint8_t)*5);
  OutputDebugString(">DISC\n");
  DLL_input(I_NLP, Buffer,  5);
  LastTrans=T_IDLE;
  return true;
}
#endif
/****************************************************[END]******/

