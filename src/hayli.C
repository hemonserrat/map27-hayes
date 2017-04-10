/**
 * HAYLI.C
 * HAYES Commands interpreter 
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
#include <hdw.h>
#include <ustdlib.h>
#include <PI.h>
#include <DLL.h>
#include <dtenl.h>
#include <at24cxx.h>
#include <delays.h>
#include <hayli.h>

static void process_packet(void);
static uint8_t *GetID(uint8_t *buf, uint8_t *prefix, uint16_t *ident);

static CODE uint8_t *psFab   = "(C)2005 Hernan M.";
static CODE uint8_t *psVer   = "rev.010426";

static CODE uint8_t *psAt    = "AT";
static CODE uint8_t *psOk    = "OK";
static CODE uint8_t *psNocar = "NO CARRIER";
static CODE uint8_t *psError = "ERROR";
static CODE uint8_t *psBusy  = "BUSY";

static CODE uint8_t *psERM   = "RM ";

static CODE uint8_t *psStatus = "STATUS";
static CODE uint8_t *psData   = "DATA";
static CODE uint8_t *psFeed   = "\r\n";
static CODE uint8_t *psCfg0   = "CFG0:";
static CODE uint8_t *psCfg1   = "CFG1:";

static uint8_t prefix;
static uint16_t ident;
static bit linkstate=false;
static bit ontrans=false;

/*----------------**
**   current cfg  **
**----------------*/
static uint8_t  _Y=0;
static bit  _E=1;
static bit  _Q=1;
static bit  _X=1;
static uint8_t _K= '3';
static uint8_t _SQ='2';
uint8_t _B= '2';
uint8_t _C='6';

/*-----------------------**
**  HAYLI State Machine  **
**-----------------------*/
static uint8_t bSTATE=RESET_WAIT;  /* HAYLI State */

/*--------------------**
**  Rx State Machine  **
**--------------------*/
static XDATA uint8_t bRX_BUFFER[MSG_BUFF];
static uint8_t bRxIndex=0;
static uint8_t bRxState=IDLE;
extern bit txflag1;
extern bit rxflag1;
extern bit txdone1;


//**************************************************************************
//**************************************************************************
//**************************************************************************

/**
 * HAYLI Idle state. Process Port pending bytes.
 */
void HLI_Idle(void)
{
	uint8_t inBYTE;

	if(rxflag1)
	{
		inBYTE = GetByte1();
		switch(inBYTE)
		{
			case TERM: {
				bRxState = PREADY;
				bRX_BUFFER[bRxIndex]=0x00;
			} break;
			case 8: {
				bRxIndex--;
			} break;
			default: {
				if( (bRxIndex+1) >= MSG_BUFF )
				{
					bRxIndex=0;
				}
				bRX_BUFFER[bRxIndex++]=inBYTE;
			}
		}
		if(_E)
		{
			SendByte1(inBYTE);
		}
	}

	/* if packet ready */
	if( bRxState == PREADY )
	{
		process_packet();
		bRxState=IDLE;
		bRxIndex = 0;
	}

}/* HLI_Idle */

/**
 * process a received packed from port 1 (DTE)
 */
static void process_packet(void)
{
	uint8_t len;
	uint8_t *ptr;
	uint8_t tmp;
	len = 0;

	if( bRxIndex > 1 )
	{
		if(!umemicmp(&bRX_BUFFER[0], psAt, 2) )
		{
			len=2;
			while( bRX_BUFFER[len]==' ' ) len++;
			if( bRxIndex<=len )
				Response(psOk);
			while(bRxIndex>len)
			{
				switch(utoupr(bRX_BUFFER[len++]))
				{
					case CMD_ECO: {
						OutputDebugString("\n\r>ECO\n\r");
						if(bRX_BUFFER[len++]=='0')
							_E=0;
						else
							_E=1;
						Response(psOk);
					}	break;
					case CMD_INFO: {
						OutputDebugString("\n\r>INFO\n\r");
						if(bRX_BUFFER[len]=='0')
							Response(psFab);
						if(bRX_BUFFER[len]=='3')
							Response(psVer);
						Response(psOk);
						len++;
					}	break;
					case CMD_RESP: {
						OutputDebugString("\n\r>RESP\n\r");
						if(bRX_BUFFER[len++]=='0')
							_Q=0;
						else
							_Q=1;
						Response(psOk);
					}	break;
					case CMD_CFG: {
						OutputDebugString("\n\r>ATZ\n\r");
						if(ATZ(bRX_BUFFER[len]))
							len++;
						Response(psOk);
					}	break;
					case CMD_PER: {
						switch(utoupr(bRX_BUFFER[len++]))
						{
							case CMD_FR: {
								OutputDebugString("\n\r>F\n\r");
								F();
								Response(psOk);
							}	break;
							case CMD_FC: {
								OutputDebugString("\n\r>&K\n\r");
								if( (bRX_BUFFER[len]>='3' &&
										bRX_BUFFER[len]<='5') ||
										bRX_BUFFER[len]=='0' ){
									_K = bRX_BUFFER[len++];
									Response(psOk);
								}	else
									Response(psError);
							}	break;
							case CMD_SI: {
								OutputDebugString("\n\r>CFG\n\r");
								ShowCfgs();
							}	break;
							case CMD_SAVE: {
								OutputDebugString("\n\r>SAVE\n\r");
								if(Save(bRX_BUFFER[len++]))
									Response(psOk);
								else
									Response(psError);
							}	break;
							case CMD_LC: {
								OutputDebugString("\n\r>Y\n\r");
								if(bRX_BUFFER[len]=='0')
									_Y=0;
								if(bRX_BUFFER[len]=='1')
									_Y=1;
								Response(psOk);
								len++;
							}	break;
							default:
								Response(psError);
						}
					} break;
					case CMD_SLA: {
						switch(utoupr(bRX_BUFFER[len++]))
						{
						case CMD_FC2: {
							OutputDebugString("\n\r>\\Q\n\r");
							if( bRX_BUFFER[len]>='0' &&
									bRX_BUFFER[len]<='2' ){
								_SQ = bRX_BUFFER[len++];
								Response(psOk);
							}	else
								Response(psError);
						}	break;
						case CMD_XON: {
							OutputDebugString("\n\r>\\X\n\r");
							if(bRX_BUFFER[len++]=='0')
								_X=0;
							else
								_X=1;
							Response(psOk);
						}	break;
						default:
							Response(psError);
						}
					} break;
					default: {
							OutputDebugString("\n\r>1ERR\n\r");
							Response(psError);
					}
				} /* switch */
			} /* while */
		} /* xmem */
		else if(!umemicmp(&bRX_BUFFER[0], psERM, 2) )
		{
				len=2;
				while( bRX_BUFFER[len]==' ' ) len++;
				if( bRxIndex<=len){
					if(linkstate)
						Response(psOk);
					else
						Response(psError);
					return;
				}
				switch(utoupr(bRX_BUFFER[len++]))
				{
					case CMD_STATUS: {
						OutputDebugString("\n\r>STATUS\n\r");

						ptr = GetID(&bRX_BUFFER[len], &prefix, &ident);
						if( ptr ){
							if(!DTENL_Status(prefix, ident, uatoi(ptr)))
								Response(psError);
							else
								ontrans=true;
						} else
							Response(psError);
					}	break;
					case CMD_MST: {
						OutputDebugString("\n\r>MST\n\r");
						ptr = GetID(&bRX_BUFFER[len], &prefix, &ident);

						if( ptr )
						{
							tmp=ustrlen(ptr);
							if(tmp>22)
							{
								if(!DTENL_PostMSTMessage(prefix, ident, ptr, tmp))
									Response(psError);
								else
									ontrans=true;

							} else	{
								if(!DTENL_PostSSTMessage(prefix, ident, ptr, tmp))
									Response(psError);
								else
									ontrans=true;
							}
						} else
							Response(psError);
					}	break;
					case CMD_BAUD: {
						OutputDebugString("\n\r>BAUD\n\r");
						if(bRX_BUFFER[len]>='0' &&
								bRX_BUFFER[len]<='2' )
						{
							_B = bRX_BUFFER[len++];
							Response(psOk);
						}	else
							Response(psError);
					}	break;
					case CMD_CODING: {
						OutputDebugString("\n\r>CODING\n\r");
						if(bRX_BUFFER[len]>='0' &&
								bRX_BUFFER[len]<='6' )
						{
							_C = bRX_BUFFER[len++];
							Response(psOk);
						}	else
							Response(psError);
					}	break;
					default:
						Response(psError);
				} /* switch */
			} /* xmem */
			else
			{
				Response(psError);
			}
	}

} /* process_packet */

/**
 * Parse Ident and prefix information from last received
 * packet.
 * @param buf - Packet to extract ident and prefix
 * @param prefix - buffer to store prefix number
 * @param ident - buffer to store ident number
 * @return pointer to next byte after ID info.
 */
uint8_t *GetID(uint8_t *buf, uint8_t *prefix, uint16_t *ident)
{
XDATA uint8_t  tmp[80];
uint8_t *ptr;

  while( *buf==' ' ) buf++;

   ptr = tmp;
   while(*buf!=' ') *ptr++ = *buf++;
   *ptr=0;
   *prefix = uatoi(tmp);
	if( *prefix > 128 )
		return NULL;
  while( *buf==' ' ) buf++;
  ptr = tmp;
   while(*buf!=',') *ptr++ = *buf++;
   *ptr=0;
   *ident = uatoi(tmp);
	 if( *ident > 8191 )
		 return NULL;
return ++buf;
} /* GetID */

void Response(const uint8_t *str)
{
 if(_Q)
 {
   SendString1(psFeed);
   SendString1(str);
   SendString1(psFeed);
 }
} /* Response */

#ifdef _UNUSED
uint8_t PreProcess(uint8_t *Buff, uint8_t Len )
{
uint8_t x;
	   for( x=0; x<Len; x++)
				if( Buff[x]==0 )
							    return x;
return x;
}
#endif

/**
 * DTENL callback
 * @param type - message type
 * @param Buff - aditional data
 * @param Len  - number of bytes in Len
 */
void DTENL_UserApp(uint8_t type, uint8_t *Buff, uint8_t Len)
{
	uint16_t x;
	NLHEADER hdr;
	XDATA uint8_t tmp[20];
	umemcpy( (uint8_t*)&hdr, Buff, 5);
	switch(type)
	{
		case M_ACTIVE: {
			/* LINK READY */
			OutputDebugString("\n\r>M_ACTIVE\n\r");
			if(!Len)
			{
				/* 		   Response(psNocar); */
				OutputDebugString("\n\rFAIL\n\r");
				linkstate=false;
				LOFF(LCA);
			} else {
				linkstate=true;
				LON(LCA);
			}
			/* wait */
		}	break;
		case RECEIVE_STATUS: {
			SendString1(psERM);
			SendString1(psStatus);
			uitoa(hdr.PFIX, tmp, 3);
			SendString1(tmp);
			uitoa(GET_IDENT(hdr.Ident), tmp, 5);
			SendString1(tmp);
			SendByte1(',');
			uitoa(Buff[5], tmp, 2);
			SendString1(tmp);
			SendString1(psFeed);
		}	break;
		case M_PCKPEND: {
			OutputDebugString("\n\r>M_PCKPEND\n\r");
			LON(LCA);
			linkstate=true;
			if(!Len)
			{
				OutputDebugString("\n\rFAIL\n\r");
				Response(psNocar);
				LOFF(LCA);
				linkstate=false;
			}
		}	break;
		case STACK_POS: {
			/* MODEM OK */
			Response(psOk);
			ontrans=(ontrans)?false:true;
		}	break;
		case STACK_QUE: {
			Response(psOk);	  /* SEARCH */
		}	break;
		case STACK_NEG: {
			/* MODEM ERROR */
			Response(psError);
		}	break;
		case RECEIVE_SST:
		case RECEIVE_MST: {
			SendString1(psERM);
			SendString1(psData);
			uitoa(hdr.PFIX, tmp, 3);
			SendString1(tmp);
			uitoa(GET_IDENT(hdr.Ident), tmp, 5);
			SendString1(tmp);
			SendByte1(',');
			//		   Len = PreProcess(&Buff[6], Len-6 );
			for(x=0; x<(Len-6); x++)
				SendByte1(Buff[x+6]);

			SendString1(psFeed);
		}	break;
		case CLEAR_NOR:
		case CLEAR_ABN: {
			Response(psError);
		}	break;
			/* Not implemented messagges */
		case SETUP_POS:
		case SETUP_QUE:
		case SETUP_NEG:
		case INCOM_VMC:
		case INCOM_EVMC:
		case RECEIVE_POS:
		case RECEIVE_WAR:
		case RECEIVE_CNC:
		case RECEIVE_MD:
		case DIVACK_POS:
		case DIVACK_NEG:
		case RADIO_PER:
		case NUM_INFO:
		case OPER_COND:
		case NET_INFO:
		case RADIO_SET:
		case RADIO_TSTRES: {
			LON(LXX);
			delay(500);
			LOFF(LXX);
		} break;
		case RPROTO_INFO:
		default:
			/* MODEM ERROR */
			Response(psError);
	}
} /* DTENL_UserApp */


/**
 * HAYLI Initialization rutine
 */
void HLI_Initialize(void)
{

	LONALL();
	/* Autobaud detection on port 1 */
	_Y=AT24CXXRead(0x02);
	if(_Y!=0x56)
	{
		OutputDebugString(">INIT EE\n\r");
		AT24CXXWrite(0x02, 0x56);
		Save(0);
		Save(1);
	}

	_Y=AT24CXXRead(0x01);
	if(_Y)
	{
		_E=AT24CXXRead(0x11);
		_Q=AT24CXXRead(0x12);
		_X=AT24CXXRead(0x13);
		_K=AT24CXXRead(0x14);
		_SQ=AT24CXXRead(0x15);
		_B=AT24CXXRead(0x16);
		_C=AT24CXXRead(0x17);
	}
	  else
	{
		_E=AT24CXXRead(0x03);
		_Q=AT24CXXRead(0x04);
		_X=AT24CXXRead(0x05);
		_K=AT24CXXRead(0x06);
		_SQ=AT24CXXRead(0x07);
		_B=AT24CXXRead(0x08);
		_C=AT24CXXRead(0x09);
	}
	Reset();
} /* HLI_Initialize */

/**
 * Reset command
 */
void Reset(void)
{
	/* init */
	LOFFALL();
	SET(ES1,0);
	SET(TR1,0);
	umemset(bRX_BUFFER, 0x00, MSG_BUFF );
	txflag1=0;
	rxflag1=0;
	txdone1=1;
	SET(ES1,1);
	SET(TR1,1);
	/* MAP27 */
	DTENL_PowerOn();
}  /* Reset */

/**
 * ATZ command
 * @param n - configuration to use
 * @return true/false
 */
uint8_t ATZ(uint8_t n)
{
uint8_t res;
  res = true;

  switch(n)
  {
   case '0': {
	 _E=AT24CXXRead(0x03);
	 _Q=AT24CXXRead(0x04);
	 _X=AT24CXXRead(0x05);
	 _K=AT24CXXRead(0x06);
	 _SQ=AT24CXXRead(0x07);
	 _B=AT24CXXRead(0x08);
	 _C=AT24CXXRead(0x09);
   } break;
   case '1': {
	 _E=AT24CXXRead(0x11);
	 _Q=AT24CXXRead(0x12);
	 _X=AT24CXXRead(0x13);
	 _K=AT24CXXRead(0x14);
	 _SQ=AT24CXXRead(0x15);
	 _B=AT24CXXRead(0x16);
	 _C=AT24CXXRead(0x17);
   } break;
   default:
     res = false;
  }
	 Reset();
 return res;
} /* ATZ */

/**
 * F command
 */
void F(void)
{
 _E=1;
 _Q=1;
 _K='3';
 _Y=0;
 _SQ='2';
 _X=1;
 _B='2';
 _C='6';
	 Reset();
} /* F */

/**
 * Show active and store configurations
 */
void ShowCfgs(void)
{

bit  Y=_Y;
bit  E=_E;
bit  Q=_Q;
bit  X=_X;
uint8_t K=_K;
uint8_t SQ=_SQ;
uint8_t B=_B;
uint8_t C=_C;
uint8_t count=2;

 SendString1(psFeed);
 SendString1(psFeed);
 SendByte1(CMD_PER);
 SendByte1(CMD_LC);
 if(Y)
	 SendByte1('1');
 else
	 SendByte1('0');

 SendByte1(' ');

do {
 SendByte1(CMD_ECO);
 if(E)
	 SendByte1('1');
 else
	 SendByte1('0');

 SendByte1(' ');

 SendByte1(CMD_RESP);
 if(Q)
	 SendByte1('1');
 else
	 SendByte1('0');

 SendByte1(' ');

 SendByte1(CMD_PER);
 SendByte1(CMD_FC);
 SendByte1(K);

 SendByte1(' ');

 SendByte1(CMD_SLA);
 SendByte1(CMD_FC2);
 SendByte1(SQ);

 SendByte1(' ');

 SendByte1(CMD_SLA);
 SendByte1(CMD_XON);
 if(X)
	 SendByte1('1');
 else
	 SendByte1('0');

 SendByte1(' ');

 SendByte1(CMD_BAUD);
 SendByte1(B);

 SendByte1(' ');

 SendByte1(CMD_CODING);
 SendByte1(C);


	 SendString1(psFeed);
	 SendString1(psFeed);

 switch(count)
 {
	case 2: {
	 E=AT24CXXRead(0x03);
	 Q=AT24CXXRead(0x04);
	 X=AT24CXXRead(0x05);
	 K=AT24CXXRead(0x06);
	 SQ=AT24CXXRead(0x07);
	 B=AT24CXXRead(0x08);
	 C=AT24CXXRead(0x09);

	 SendString1(psCfg0);
	 SendString1(psFeed);
	} break;
	case 1: {
	 E=AT24CXXRead(0x11);
	 Q=AT24CXXRead(0x12);
	 X=AT24CXXRead(0x13);
	 K=AT24CXXRead(0x14);
	 SQ=AT24CXXRead(0x15);
	 B=AT24CXXRead(0x16);
	 C=AT24CXXRead(0x17);

	 SendString1(psCfg1);
	 SendString1(psFeed);
	} break;
 }

} while( count-- );

 SendString1(psFeed);
 SendString1(psFeed);

} /* ShowCfgs */

/**
 * Store configurations on Flash
 * @param n - address to store
 * @return true/false
 */
uint8_t Save(uint8_t n)
{
 uint8_t res;

  res=true;
  switch(n)
  {
   case '0': {
		AT24CXXWrite(0x01, _Y);

		AT24CXXWrite(0x03, _E);
		AT24CXXWrite(0x04, _Q);
		AT24CXXWrite(0x05, _X);
		AT24CXXWrite(0x06, _K);
		AT24CXXWrite(0x07, _SQ);
		AT24CXXWrite(0x08, _B);
		AT24CXXWrite(0x09, _C);
   } break;
   case '1': {
		AT24CXXWrite(0x01, _Y);

		AT24CXXWrite(0x11, _E);
		AT24CXXWrite(0x12, _Q);
		AT24CXXWrite(0x13, _X);
		AT24CXXWrite(0x14, _K);
		AT24CXXWrite(0x15, _SQ);
		AT24CXXWrite(0x16, _B);
		AT24CXXWrite(0x17, _C);
   } break;
   default:
    res = false;
  }
return res;
} /* Save */

/****************************************************[END]******/






