/**
 * PI.C
 * Physical Inteface Definitions
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
#include <pi.h>

/*------------------------------**
**  Serial port state variables **
**------------------------------*/
static uint8_t RXBUF0;
static uint8_t TXBUF0;
static bit txflag0;
static bit rxflag0;
static bit txdone0;
static void SendByte0(uint8_t c);

/*------------------**
**  Timer variables **
**------------------*/
static uint16_t interruptcnt;
HTIME hdw_secs=0;
#define TIMER0L    4608

/*--------------------**
**  Rx State Machine  **
**--------------------*/
static uint8_t RXBUF1;

/*--------------------**
**  Tx State Machine  **
**--------------------*/
static uint8_t TXBUF1;
bit txflag1;
bit rxflag1;
bit txdone1;

/**
 * Send debug string messages to stderr
 * @param  messg - Sends a debug message
 */
#ifdef DEBUG
#include <hayli.h>
void  OutputDebugString( uint8_t *messg )
{
  SendString1(messg);
  SendString1("\r\n");
}
#endif


/**
 * Serial port 0 Interrupt rutine
 */
void serial0(void)  INT(4, 1)
{
  ES0=0;
  if(TI){
   TI=0;
	if(txflag0){
	  txflag0=0;
	  SBUF = TXBUF0;
	  txdone0=0;
	} else txdone0=1;
   } else {
	RI=0;
    rxflag0=1;
	RXBUF0 = SBUF;
  }
  ES0=1;
}

/**
 * Initialize Physical Layer
 */
void PI_Initialise(void)
{
	extern uint8_t _B;
	/* ************************************************************************** */
	SET(ES0,0);
	TR2=0;
	switch(_B) {
	case '0':
		TL1  = 0xE7;
		TH1  = 0xE7;
		break;
	case '1':
		TL1  = 0xF4;
		TH1  = 0xF4;
		break;
	case '2':
		TL1  = 0xFD;
		TH1  = 0xFD;
		break;
	default:
		TL1  = 0xFD;
		TH1  = 0xFD;
	}
	txflag0=0;
	rxflag0=0;
	txdone0=1;
	ES0=1;
	TR2=1;
	/* ************************************************************************** */
}

/**
 * Clear Buffers and reset port state variables
 */
void PI_Clear(void)
{
 txflag0=0;
 rxflag0=0;
 txdone0=1;
}

/**
 * Returns true if a pending bytes is on port 0
 */
bool PI_IsDataReady(void)
{
return rxflag0;
}

/**
 * returns the next byte pending on port.
 */
uint8_t PI_Receive(void)
{			  
  while (!rxflag0);
  rxflag0 = 0;
  return RXBUF0;
}

/**
 * Sends bLen bytes from outpkt to Port
 * @param outpkt - Buffer to send
 * @param bLen   - number of bytes to send
 * @return  always true.
 */
uint8_t PI_Send(uint8_t *outpkt, uint8_t bLen )
{
    uint8_t i;
    for (i = 0; i<bLen; i++) SendByte0(outpkt[i]);
  return true;
}

/**
 * Send a byte to Port 0
 * @param c - Byte to send.
 */
static void SendByte0(uint8_t c)
{
   while (txflag0);            
   LON(LTX);
   TXBUF0 = c;
   txflag0=1;
   if(txdone0) TI=1;
   LOFF(LTX);
}

/**
 * Interrupt timer 1
 */
void timer0(void) INT(1,1)
{
  /* Refresh watchdog */
/*   TA = 0xAA;  */
/*   TA = 0x55;  */
/*   RWT = 1;    */
    if( ++interruptcnt == TIMER0L )  
    {                              
      hdw_secs++;                    
/*    	  if(hdw_secs>60) hdw_secs=0;         */
      interruptcnt=0;                
    }                                
} 

//**************************************************************************
//**************************************************************************
//**************************************************************************
/**
 * Serial Port 1 interrupt rutine
 */
void serial1(void) INT(7,1)
{
  ES1=0;
  if(TI_1){
   TI_1=0;
	if(txflag1){
	  txflag1=0;
	  SBUF1 = TXBUF1;
	  txdone1=0;
	} else txdone1=1;
  } else {
	RI_1=0;
    rxflag1=1;
	RXBUF1=SBUF1;
  }
  ES1=1;
}


/**
 * Gets the next pending byte at port 1
 * @return Byte read
 */
uint8_t GetByte1(void)
{
  while(!rxflag1);
  rxflag1 = 0;
  return RXBUF1;
}

/**
 * Send a byte to port 1
 * @param c - byte to send.
 */
void SendByte1(uint8_t c)
{
   while (txflag1);
   TXBUF1 = c;
   txflag1=1;
   if(txdone1) TI_1=1;
}

/**
 * Send string to port 1
 * @param s - NULL terminated string to send.
 */
void SendString1(const uint8_t *s)
{
    uint8_t i;
    for (i = 0; s[i]; i++) SendByte1(s[i]);
}

/**
 * Initialize DS8XC520 peripherals
 */
void hl_init(void)
{
	   //DME0 = 1; /*  use internal 1K */
	   //DME1 = 0;
	   PMR |= DME0;
	   PMR &= ~DME1;

	/*    WD0  = 1;  Set WDT to 6.03 s  */
	/*    WD1  = 1;                     */
	/* ************************************************************************** */
	  IE=0;

	  TMOD = 0x22;  /* #00100010B     ;TIMER 1 = CONT. 8AUTO BAUD */
	  TH0  = 0x00;
	  TL0  = 0x00;
	  TL1  = 0x00;

	  TH1  = 0xFD;  /* Serial BAUD RATE */
	  SCON = 0x5A;  /* con REN */
	  PCON = 0x00;  /*   #00000000B     ;SMOD=0 f/Serial speed */
	  IP   = 0x02;  /* #00000010B */

	  /* 2nd UART */
	  SCON1  = 0x5A;
	  T2CON  = 0x30; /* Serial 0  */
	  WDCON &= 0x7F;
	  RCAP2L = 0xDC;
	  RCAP2H = 0xFF;
	  TL2    = 0xDC;
	  TH2    = 0xFF;

	  IE   = 0xD2;   /* Enable ints w/Serial ports */

	  TCON = 0x50;	/* 0101 0000 start T1 y T0 */
	  TR2    = 1;
	/* ************************************************************************** */
	/*   TA = 0xAA;  */
	/*   TA = 0x55;  */
	/*   EWT = 1;    */
}


/**********************************************************[END]*****/


