/**
 * DLL.C
 * Dynamic Link Layer Implementation
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
 *
 * 	@notes
 * 			05/30/2000 DSM
 *			#define CRC_ACTIVE was incorporate to activate the CRC
 *			Check.  By default it was disable because TATE's
 *			message C4 does not verify the CRC.
 */
#include <hdw.h>
#include <stdint.h>
#include <stdbool.h>
#include <ustdlib.h>
#include <delays.h>
#include <pi.h>
#include <dll.h>
#include <dtenl.h>

//****************************************************************
// depends of target platform
#define  k_K 1	//< constant K
#define  k	 1  //< window size
#define  N1	 6  //< Maximum length of octets
#define  CV  1  //< unit version

//****************************************************************
// constants types

#define N2 10    //< Maximum Number of retransmisions
#define N3 10    //< Maximum Number of inactivity timeouts
#define TX_MSG_BUFF (k_K*89)*2 //< by 2 coz DLE Stuff (k*16*(N1+1))  message length
#define U_MAX_LEN  255	//< maximum message length
#define U_MAX_WIN  1	//< maximum window size
#define U_VERSION  1	//< maximum version unit knows

//****************************************************************
//  timers counters in ms

#define T0 2     //< link_establishment_timer 100ms a 15s
#define T1 5     //< retry_timer
#define T2 5     //< acknowledgement_timer
#define T3 15    //< activity_timer
#define T4 N3*T3 //< link_failure_detection_timer

static HTIME sT0, sT1, sT2, sT3, sT4;
static HTIMESTOP tsT0, tsT1, tsT2, tsT3, tsT4;
#define TIMESTART(A)    s##A=HDW_Time(); ts##A=true
#define TIMESTOP(A)     ts##A=false

//****************************************************************
// Variables
static uint8_t AR;  /* LT_acknoledge_request */
static uint8_t C1;  /* retransmision_count */
static uint8_t Nk;  /* rx_credit_number */
static uint8_t NR;  /* rx_sequence_number */
static uint8_t Rk;  /* receive credit */
static uint8_t Sk;  /* send_credit */
static uint8_t SNR; /* stored acknowledged rx sequence number */
static uint8_t SRk; /* stored receive credit */
static uint8_t VS;  /* send state variable */
static uint8_t VR;  /* receive state variable */
static uint8_t txNR;    /* Last transmited LA rx_seq_number */
static uint8_t txNK;    /* Last Transmited LA rx_credit_number */
static uint8_t txLNK;   /* (Last Transmited-1) LA rx_credit_number */
static uint8_t bLRSent; /* Do we sent an LR in this link stablisment?  */
/****************************************************************/
/* DLL State Machine */
static uint8_t bSTATE=RESET_WAIT;  /* DLL State */


/****************************************************************/
/* Rx State Machine */
static XDATA uint8_t bRX_BUFFER[TX_MSG_BUFF+12+1];
static uint8_t bRxIndex=0;
static uint8_t bRxState=IDLE;

/****************************************************************/
/* Tx State Machine */
static XDATA uint8_t bTX_BUFFER[TX_MSG_BUFF+12+1];
static uint8_t bTXLen;

/****************************************************************/
/* HELPER TASKS */
#define decrement_retransmission_counter()  C1= (C1>1)? C1-1:0
#define decrement_send_credit()             Sk= (Sk>1)? Sk-1:0
#define increment_send_state()              VS++
#define set_retransmission_counter()        C1=N2
#define maximise_link_parameters()          /*  CONSTANTS */                       
#define set_activity_counter()              C3=N3
#define store_receive_credit()              SRk=Rk
#define increment_receive_state()           VR++
#define decrement_receive_credit()          Rk--
#define send_credit_available()			    (Sk > 0) 
#define retransmision_count()               (C1==0)
#define all_tx_ack(LastReceivedLA)		    (LastReceivedLA[1]==VS)
#define repeated_link_ack(LastReceivedLA)	(LastReceivedLA[1]==SNR    \
				                             && LastReceivedLA[1]!=VS)
#define pckt_outof_sn(LastReceivedLT)       (LastReceivedLT[1]!=VR)
#define receive_credit_available()          DTENL_IsSpace()
#define inmediate_reply_request(LastReceivedLT)	(LastReceivedLT[2]==AR_AI)
#define ack_inside_win(LastReceivedLA)	 (LastReceivedLA[1]>=SNR   \
	                                     &&  LastReceivedLA[1]<=VS)

#define packet_outside_win()               false
#define record_send_credit(LastReceivedLA) Sk = LastReceivedLA[2]
#define set_retransmision_counter()        C1=N2
#define rewind_packet_number()             VS=SNR
#define delete_ack_pkt()	               DTENL_RemovePackets()
#define store_rx_sn(LastReceivedLA)		   SNR=LastReceivedLA[1]
#define update_receive_credit(bCredit)	   Rk=bCredit
#define adjust_link_parameters(LastReceivedLR)

/*   State Machine Functions				                        */
static void ready_handler( uint8_t type, uint8_t * Buff, uint8_t bLen );
static void link_wait_handler( uint8_t type, uint8_t * Buff, uint8_t bLen );
static void reset_wait_handler( uint8_t type, uint8_t *Buff, uint8_t bLen );

/*   Interface to Physical interface functions                      */
static void process_packet(void);

/*   timers  functions                                              */
#define link_establishment_timeout()   ( (HDW_Time() - sT0) > T0 )
#define retry_timeout()			       ( (HDW_Time() - sT1) > T1 )
#define acknowledge_timeout()		   ( (HDW_Time() - sT2) > T2 )
#define activity_timeout()			   ( (HDW_Time() - sT3) > T3 )
#define lfd_timeout()				   ( (HDW_Time() - sT4) > T4 )


/*   Utility functions                                              */
static uint8_t MakeDLEStuff(uint8_t *bBuff, uint8_t bLen);
static uint8_t CutDLEStuff(uint8_t *bBuff, uint8_t bLen);
static void link_request(void);
static void link_ack(void);
static void link_transfer(void);


/*   System Variables  Exchange                                     */
/* static bool adjust_link_parameters(uint8_t *LastReceivedLR); */
static void initialise_variables(void);
static bool received_parameters_acceptable( uint8_t *Buff );
static void store_packet(uint8_t *Buff, uint8_t length);

/* FCS Calculation Functions */

static uint16_t DLL_Crc( uint8_t *bBuffer, uint8_t bLen);

#define CRC16 0xA001;

CODE const uint16_t wMtab[]= {
	0x0000,0xC1C0,0x81C1,0x4001,0x01C3,0xC003,0x8002,0x41C2,0x01C6,
	0xC006,0x8007,0x41C7,0x0005,0xC1C5,0x81C4,0x4004,0x01CC,0xC00C,
	0x800D,0x41CD,0x000F,0xC1CF,0x81CE,0x400E,0x000A,0xC1CA,0x81CB,
	0x400B,0x01C9,0xC009,0x8008,0x41C8,0x01D8,0xC018,0x8019,0x41D9,
	0x001B,0xC1DB,0x81DA,0x401A,0x001E,0xC1DE,0x81DF,0x401F,0x01DD,
	0xC01D,0x801C,0x41DC,0x0014,0xC1D4,0x81D5,0x4015,0x01D7,0xC017,
	0x8016,0x41D6,0x01D2,0xC012,0x8013,0x41D3,0x0011,0xC1D1,0x81D0,
	0x4010,0x01F0,0xC030,0x8031,0x41F1,0x0033,0xC1F3,0x81F2,0x4032,
	0x0036,0xC1F6,0x81F7,0x4037,0x01F5,0xC035,0x8034,0x41F4,0x003C,
	0xC1FC,0x81FD,0x403D,0x01FF,0xC03F,0x803E,0x41FE,0x01FA,0xC03A,
	0x803B,0x41FB,0x0039,0xC1F9,0x81F8,0x4038,0x0028,0xC1E8,0x81E9,
	0x4029,0x01EB,0xC02B,0x802A,0x41EA,0x01EE,0xC02E,0x802F,0x41EF,
	0x002D,0xC1ED,0x81EC,0x402C,0x01E4,0xC024,0x8025,0x41E5,0x0027,
	0xC1E7,0x81E6,0x4026,0x0022,0xC1E2,0x81E3,0x4023,0x01E1,0xC021,
	0x8020,0x41E0,0x01A0,0xC060,0x8061,0x41A1,0x0063,0xC1A3,0x81A2,
	0x4062,0x0066,0xC1A6,0x81A7,0x4067,0x01A5,0xC065,0x8064,0x41A4,
	0x006C,0xC1AC,0x81AD,0x406D,0x01AF,0xC06F,0x806E,0x41AE,0x01AA,
	0xC06A,0x806B,0x41AB,0x0069,0xC1A9,0x81A8,0x4068,0x0078,0xC1B8,
	0x81B9,0x4079,0x01BB,0xC07B,0x807A,0x41BA,0x01BE,0xC07E,0x807F,
	0x41BF,0x007D,0xC1BD,0x81BC,0x407C,0x01B4,0xC074,0x8075,0x41B5,
	0x0077,0xC1B7,0x81B6,0x4076,0x0072,0xC1B2,0x81B3,0x4073,0x01B1,
	0xC071,0x8070,0x41B0,0x0050,0xC190,0x8191,0x4051,0x0193,0xC053,
	0x8052,0x4192,0x0196,0xC056,0x8057,0x4197,0x0055,0xC195,0x8194,
	0x4054,0x019C,0xC05C,0x805D,0x419D,0x005F,0xC19F,0x819E,0x405E,
	0x005A,0xC19A,0x819B,0x405B,0x0199,0xC059,0x8058,0x4198,0x0188,
	0xC048,0x8049,0x4189,0x004B,0xC18B,0x818A,0x404A,0x004E,0xC18E,
	0x818F,0x404F,0x018D,0xC04D,0x804C,0x418C,0x0044,0xC184,0x8185,
	0x4045,0x0187,0xC047,0x8046,0x4186,0x0182,0xC042,0x8043,0x4183,
	0x0041,0xC181,0x8180,0x4040 };

/**
 *  Checks for pending bytes on receive queue and process it.
 *  Checks for timeouts.
 */
void DLL_Idle(void)
{
uint8_t inBYTE;

  if( PI_IsDataReady() )
  {
	   inBYTE=PI_Receive();
	   if( bRxIndex+1 > TX_MSG_BUFF )
	   {
		bRxIndex = 0;
		bRxState=IDLE;
	   }
	LON(LRX);
		 switch(inBYTE)
		 {
		   case SYN: {
			 if(bRxState==IDLE)
			 {
			 		bRxState= STARTFLAG1;	
					bRxIndex=0;
			 } else if(bRxState==STARTFLAG3) 
			 {
				    bRX_BUFFER[bRxIndex++]=inBYTE;
			 }
		   } break;
		   case DLE: {
				switch( bRxState )
				{
					case STARTFLAG1: {
						bRxState = STARTFLAG2; 
					} break;
					case STARTFLAG3: {
					    bRX_BUFFER[bRxIndex++]=inBYTE;
						bRxState = WAITDLE;
					} break;
					case WAITDLE: {
					    bRX_BUFFER[bRxIndex++]=inBYTE;
						bRxState = STARTFLAG3;
					} break;
				}
		   } break;
		   case STX: {
				if( bRxState==STARTFLAG2 )
				{
						bRxState = STARTFLAG3;
				} else if( bRxState==STARTFLAG3 )
				{
					    bRX_BUFFER[bRxIndex++]=inBYTE;
				}
		   } break;
		   case ETX: {
				if( bRxState==WAITDLE )
				{
					bRX_BUFFER[bRxIndex++]=inBYTE;
					bRxState = WAITFCS1;
				} else if( bRxState==STARTFLAG3 )
				{
					bRX_BUFFER[bRxIndex++]=inBYTE;
				}
		   } break;
		   default: {
			   switch( bRxState )
			   {
				case WAITFCS2: {
					 bRX_BUFFER[bRxIndex++]=inBYTE;
					 bRxState = PREADY;
				} break;
				case WAITFCS1: {
					 bRxState = WAITFCS2;
				case STARTFLAG3:
					 bRX_BUFFER[bRxIndex++]=inBYTE;
				} break;
			   }
		   }
		 }/* switch */
 		LOFF(LRX);
  } /* if */

/* if packet ready */
	if( bRxState == PREADY )
	{
		process_packet();
		bRxState=IDLE;
	}

/* check out timers */
  if( link_establishment_timeout() && tsT0 )
  {
  		  OutputDebugString("\t\tT0\n");

					 DLL_input( I_LET, NULL, 0 );
  }

  if( retry_timeout() && tsT1 )
  {
		  OutputDebugString("\t\tT1\n");
					 DLL_input( I_RTO, NULL, 0 );
  }

  if( acknowledge_timeout() && tsT2 )
  {
		  OutputDebugString("\t\tT2\n");
					 DLL_input( I_ATO, NULL, 0 );
  }

  if( activity_timeout() && tsT3 )
  {
		  OutputDebugString("\t\tT3\n");
					 DLL_input( I_ACO, NULL, 0 );
  }

  if( lfd_timeout() && tsT4 )
  {
		  OutputDebugString("\t\tT4\n");
					 DLL_input( I_LFD, NULL, 0 );
  }

  DTENL_input(I_LRQ, NULL, (bSTATE==READY)?1:0);

}/* DLL_Idle */

/**
 * Dinamic link layer messages entry point
 * @param type - Packet type
 * @param Buff - last received buffer contents
 * @param bLen - number of bytes in buffer
 */
void DLL_input(uint8_t type, uint8_t *Buff, uint8_t bLen )
{
     switch(bSTATE)
	 {
	   case RESET_WAIT: {
	        reset_wait_handler(type, Buff, bLen ); 
	   } break;
	   case LINK_WAIT: {
			link_wait_handler(type, Buff, bLen );
	   } break;
	   case READY: {
			ready_handler(type, Buff, bLen );
	   } break;
	 }
}

/**
 * Ready state dispatcher
 * @param type - message type
 * @param Buff - last received buffer contents
 * @param bLen - number of bytes in buffer
 */
static void ready_handler( uint8_t type, uint8_t *Buff, uint8_t bLen )
{
	bLRSent=false;
	switch( type )
	{
		case I_NLR: { /* network_layer_reset */
			  PI_Initialise();
			  maximise_link_parameters();
			  TIMESTOP(T1);
			  TIMESTOP(T2);
			  TIMESTOP(T3);
			  TIMESTOP(T4);
			  link_request();
			  TIMESTART(T0);
			  bSTATE = RESET_WAIT;
				  OutputDebugString("\tREADY NLR\n");
		} break;
		case I_RTO: { /* Retry timeout */
				decrement_retransmission_counter();
			   if( retransmision_count() )
			   {
					maximise_link_parameters();
					/* OUTPUT link_failure */
						 DTENL_input(I_RTO, NULL, 0);
					  TIMESTOP(T1);
					  TIMESTOP(T2);
					  TIMESTOP(T3);
					  TIMESTOP(T4);
					link_request();
					TIMESTART(T0);
				   bSTATE=RESET_WAIT;
			   } else {
				 rewind_packet_number();
/* 				 OUTPUT link transfer */
				 link_transfer();
				 TIMESTART(T1);
				 bSTATE=READY;
			   }
				  OutputDebugString("\tREADY RTO\n");
		} break;
		case I_PON: { /* power_on  */
			  PI_Initialise();
			  maximise_link_parameters();
			  /* OUTPUT link_failure */
					 DTENL_input(I_PON, NULL, 0);
			  TIMESTOP(T1);
			  TIMESTOP(T2);
			  TIMESTOP(T3);
			  TIMESTOP(T4);
			  link_request();
			  TIMESTART(T0);
			  bSTATE = RESET_WAIT;
				  OutputDebugString("\tREADY PON\n");
		} break;
		case I_LRQ: {  /* link request */
				   adjust_link_parameters(Buff);
				  /* OUTPUT link_failure */
					 DTENL_input(I_LRQ, NULL, 0 );
				  TIMESTOP(T1);
				  TIMESTOP(T2);
				  TIMESTOP(T3);
				  TIMESTOP(T4);
							link_request();
				  TIMESTART(T0);
				   bSTATE=RESET_WAIT;
				  OutputDebugString("\tREADY LRQ\n");
		} break;
		case I_LAK: { /* link acknowledge */
			   if( ack_inside_win(Buff) )
			   {
				 TIMESTART(T4);
				 TIMESTART(T3);
				 record_send_credit(Buff);
				 delete_ack_pkt();
				 bSTATE = READY;
				 if( all_tx_ack(Buff) )
				 {
					TIMESTOP(T1);
					set_retransmision_counter();
					store_rx_sn(Buff);
				   bSTATE = READY;
				 } else {
					if( repeated_link_ack(Buff) )
					{
					   decrement_retransmission_counter();
					   if( retransmision_count() )
					   {
						  maximise_link_parameters();
						 /* OUTPUT link_failure */
								 DTENL_input(I_LAK, NULL, 0);
						  TIMESTOP(T1);
						  TIMESTOP(T2);
						  TIMESTOP(T3);
						  TIMESTOP(T4);
							link_request();
						  TIMESTART(T0);
						   bSTATE=RESET_WAIT;
					   } else {
						 rewind_packet_number();
/* 						 OUTPUT link transfer */
							link_transfer();
						 TIMESTART(T1);
						 bSTATE=READY;
					   } /* retransmision_count */
					} /* repeated_link_ack */
				 } /* all_tx_ack */
			} else { /* ack_inside_win */
			  maximise_link_parameters();
			 /*	 OUTPUT link_failure */
					 DTENL_input(I_LAK, NULL, 0 );
				  TIMESTOP(T1);
				  TIMESTOP(T2);
				  TIMESTOP(T3);
				  TIMESTOP(T4);
							link_request();
				  TIMESTART(T0);
				   bSTATE=RESET_WAIT;
			}
				  OutputDebugString("\tREADY ACK\n");
		} break;
		case I_LTR: {  /* link transfer */
			  TIMESTART(T4);
#if	packet_outside_win() > 0
			  if( packet_outside_win() )
			  {
				  maximise_link_parameters();
				 // OUTPUT link_failure
					 DTENL_input(I_LTR, NULL, 0 );
				  TIMESTOP(T1);
				  TIMESTOP(T2);
				  TIMESTOP(T3);
				  TIMESTOP(T4);
							link_request();
				  TIMESTART(T0);
				   bSTATE=RESET_WAIT;
			  }	else
#endif
			  {

				if( pckt_outof_sn(Buff) )
				{
				   TIMESTOP(T2);
				  // OUTPUT link_acknowledge
					link_ack();
				   TIMESTART(T3);
				} else {
				  if(receive_credit_available() )
				  {
 					// OUTPUT network layer packet
			 		// DTENL_input(I_LTR, Buff, bLen );
					increment_receive_state();
 					// decrement_receive_credit();
					if( inmediate_reply_request(Buff) )
					{
					  store_receive_credit();
					  TIMESTOP(T2);
 					  // OUTPUT link acknowledge */
						link_ack();
					  TIMESTART(T3);
					} else {
					  store_receive_credit();
					  TIMESTART(T2);
					} // inmediate_reply_request
					 DTENL_input(I_LTR, Buff, bLen );
				  } else {
					  store_receive_credit();
					  TIMESTOP(T2);
					  // OUTPUT link acknowledge
						 link_ack();
					  TIMESTART(T3);
				  } // receive_credit_available
				} // pckt_outof_sn
				bSTATE = READY;
			  }	// packet_outside_win

				  OutputDebugString("\tREADY LTR\n");
		} break;
		case I_ATO: {  // acknowledge  timout T2
         //  OUTPUT link acknowledge
			  link_ack();
			  TIMESTART(T3);
			  bSTATE=READY;
				  OutputDebugString("\tREADY ATO\n");
		} break;
		case I_ACO: { // Activity timout T3
			  TIMESTOP(T2);
         //  OUTPUT link acknowledge
				link_ack();
			  TIMESTART(T3);
			  bSTATE=READY;
				  OutputDebugString("\tREADY ACO\n");
		} break;
		case I_NLP: { // Network layer packet
			 if( send_credit_available() )
			 {
               // OUTPUT packet accepted
			   DTENL_input(I_NLP, NULL, 1);
			   store_packet(Buff, bLen);
			   set_retransmision_counter();
               // OUTPUT link_transfer
				link_transfer();
			   increment_send_state();
			   decrement_send_credit();
			   TIMESTART(T1);
			 } else {
 				// OUTPUT packet rejected
				  DTENL_input(I_NLP, NULL, 0);
			 }
			 bSTATE = READY;
				  OutputDebugString("\tREADY NLP\n");
		} break;
		case I_LFD: { /* link failure detection timeout T4 */
			   maximise_link_parameters();
			  /* OUTPUT link_failure */
					 DTENL_input(I_LFD, NULL, 0 );
			  TIMESTOP(T1);
			  TIMESTOP(T2);
			  TIMESTOP(T3);
			  TIMESTOP(T4);
			  link_request();
			  TIMESTART(T0);
			  bSTATE = RESET_WAIT;
				  OutputDebugString("\tREADY LFD\n");
		} break;
		case I_CVE: { /* Credit value */
			  update_receive_credit(bLen);
			  if( receive_credit_available() )
			  {
				TIMESTOP(T2);
/* 				OUTPUT link acknowledge */
				link_ack();
				TIMESTART(T3);
 			  }
			  bSTATE=READY;
		   /* Ready to send packages ACTIVE */
/* 		   DTENL_input(I_LRQ, NULL, 1); */
				  OutputDebugString("\tREADY CVE\n");
		} break;
	}
} /* ready_handler */

/**
 * Link wait state dispatcher
 * @param type - message type
 * @param Buff - last received buffer contents
 * @param len  - number of bytes in buffer
 */
static void link_wait_handler( uint8_t type, uint8_t *Buff, uint8_t len)
{
	(void)len;

	switch( type )
	{
		case I_NLR: { /* network_layer_reset */
			  PI_Initialise();
			  maximise_link_parameters();
			  TIMESTOP(T1);
			  TIMESTOP(T2);
			  TIMESTOP(T3);
			  TIMESTOP(T4);
			  link_request();
			  TIMESTART(T0);
			  bSTATE = RESET_WAIT;
				  OutputDebugString("\tWAIT NLR\n");
		} break;
		case I_PON:{  /* power_on  */
			  PI_Initialise();
			  maximise_link_parameters();
			  /* OUTPUT link_failure */
					 DTENL_input(I_PON, NULL, 0 );
			  TIMESTOP(T1);
			  TIMESTOP(T2);
			  TIMESTOP(T3);
			  TIMESTOP(T4);
			  link_request();
			  TIMESTART(T0);
			  bSTATE = RESET_WAIT;
				  OutputDebugString("\tWAIT PON\n");
		} break;
		case I_LRQ: {  /* link request */
		   adjust_link_parameters(Buff);
			   if( received_parameters_acceptable( Buff )  && bLRSent==true)
			   {
				  initialise_variables();
					TIMESTOP(T0);
/* 				  OUTPUT link_acknowledge */
				   link_ack();
/* 				  OUTPUT link_ready */
				   DTENL_input(I_LRQ, NULL, 1);
				  TIMESTART(T3);
				  TIMESTART(T4);
				  bSTATE=READY;
				  OutputDebugString("READY\n");
			   } else {
				   link_request();
				   TIMESTART(T0);
				  bSTATE=LINK_WAIT;
			   }
				  OutputDebugString("\tWAIT LRQ\n");
		} break;
		case I_LAK: {  /* link acknowledge */
			if(bLRSent==true)
			{
				initialise_variables();
				record_send_credit(Buff);
				TIMESTOP(T0);
/* 				OUTPUT link acknowledge */
				link_ack();
/* 				OUTPUT link ready */
				   DTENL_input(I_LRQ, NULL, 1);
				TIMESTART(T3);
				TIMESTART(T4);
				bSTATE=READY;
				  OutputDebugString("\tWAIT LAK\n");
			}
			else
			{
				link_request();
				TIMESTART(T0);
			}
		} break;
		case I_LTR:  /* link transfer */
		case I_LET: { /* link establishment timout T0 */
				link_request();
				TIMESTART(T0);
				bSTATE=RESET_WAIT;
				  OutputDebugString("\tWAIT LET\n");
		} break;
		case I_NLP: { /* Network layer packet */
/* 			 OUTPUT packet_rejected */
		   DTENL_input(I_NLP, NULL, 0);
				  OutputDebugString("\tWAIT NLP\n");
		} break;
	}
} /* link_wait_handler */

/**
 * Reset wait state dispatcher
 * @param type - message type
 * @param Buff - last received buffer
 * @param len  - number of bytes in buffer
 */
static void reset_wait_handler( uint8_t type, uint8_t *Buff, uint8_t len)
{
	(void) Buff;
	(void) len;

	switch( type )
	{
		case I_NLR:  /* network_layer_reset */
		case I_PON:  /* power_on  */
			  PI_Initialise();
			initialise_variables();
			maximise_link_parameters();
			  /* Stop timers */
			  TIMESTOP(T1);
			  TIMESTOP(T2);
			  TIMESTOP(T3);
			  TIMESTOP(T4);
					link_request();
				 TIMESTART(T0);
			   bSTATE = RESET_WAIT;
				  OutputDebugString("\tRESET PON\n");
		break;
		case I_LRQ:  /* link request */
			   adjust_link_parameters(Buff);
			   link_request();
			   TIMESTART(T0);
			   bSTATE = LINK_WAIT;
				  OutputDebugString("\tRESET LRQ\n");
		break;
		case I_LAK:  /* link acknowledge */
		case I_LTR:  /* link transfer */
		case I_LET:  /* link establishment timout T0 */
			link_request();
			TIMESTART(T0);
			   bSTATE = RESET_WAIT;
				  OutputDebugString("\tRESET LET\n");
		break;
		case I_NLP:  /* Network layer packet */
/* 			 OUTPUT packet_rejected */
			   DTENL_input(I_NLP, NULL, 0);
			   bSTATE = RESET_WAIT;
				  OutputDebugString("\tRESET NLP\n");
		break;
	}

}  /* reset_wait_handler */

/**
 * Send bLen byte to device on port x
 * @param bBuffer - buffer to send
 * @param bLen    - number of bytes in buffer
 * @return   if TXBUFFER overflow DLLREJECT
 *           true otherwise.
 */
uint8_t DLL_Send(uint8_t *bBuffer, uint8_t bLen )
{
uint16_t fcs;
uint8_t res;

	  if( bLen > TX_MSG_BUFF+8 )
			return DLLREJECT;
   umemmove( &bBuffer[3], bBuffer, bLen );
   bBuffer[0]=SYN;
   bBuffer[1]=DLE;
   bBuffer[2]=STX;
   bLen += 3;
   bBuffer[bLen]=DLE;
   bBuffer[++bLen]=ETX;
   fcs = DLL_Crc( &bBuffer[3], bLen-2);
   bBuffer[++bLen]=HIBYTE(fcs);
   bBuffer[++bLen]=LOBYTE(fcs);
	bLen = MakeDLEStuff(bBuffer, bLen+1 );
	bBuffer[bLen]=0;
    res = PI_Send( bBuffer, bLen );
return res;
} /* DLL_Send */

/**
 * process last received packet
 */
static void process_packet(void)
{
#ifdef CRC_ACTIVE
	uint16_t fcs;
#endif
uint8_t bLen;
uint8_t type;

    type = UNDEFINED;
	bLen = CutDLEStuff(bRX_BUFFER, bRxIndex);

		/* FCS check */
#ifdef CRC_ACTIVE
	fcs = DLL_Crc( bRX_BUFFER, bLen-2);

	  if( bRX_BUFFER[bLen-2]!=HIBYTE(fcs) ||
		  bRX_BUFFER[bLen-1]!=LOBYTE(fcs) )
		return;
#endif
		/* type of packet */
	switch( bRX_BUFFER[0] )		/* process packet */
	{
	  case LR:
		   OutputDebugString("RCV LRQ\n");
			type = I_LRQ;
	  break;
	  case LA:
		   OutputDebugString("RCV LAK\n");
			type = I_LAK;
	  break;
	  case LT:
		   OutputDebugString("RCV LTR\n");
			type = I_LTR;
	  break;
	}
	DLL_input( type, bRX_BUFFER, bLen-4 );
} /* process_packet */

/**
 * Makes DLE stuffing
 * @param bBuff - buffer to process
 * @param bLen  - number of bytes in buffer
 * @return  number of bytes processed
 */
static uint8_t MakeDLEStuff(uint8_t *bBuff, uint8_t bLen)
{
uint8_t  x;
uint8_t  magic;
	for(x=3; x<(bLen-4); x++)
	{
		if( bBuff[x]== DLE )
		{
			magic=x+1;
			umemmove( &bBuff[magic], &bBuff[x], bLen-x );
			bBuff[x]=DLE;
			bLen++;
			x++;
		}
	}
return bLen;
} /* MakeDLEStuff */

/**
 * Checks and remove DLE stuffing
 * @param bBuff - buffer to process
 * @param bLen  - number of bytes to process
 * @return number of bytes processed
 */
static uint8_t CutDLEStuff(uint8_t *bBuff, uint8_t bLen)
{
uint8_t  x;
uint8_t  magic;
	for(x=0; x<(bLen-4); x++)
	{
		if( (bBuff[x]== DLE) && (bBuff[x+1]==DLE) )
		{
			magic=x+1;
			umemmove( &bBuff[x], &bBuff[magic], bLen-(x+1) );
			bLen--;
		}
	}
return bLen;
} /* CutDLEStuff */

/**
 * Send a link transfer packet using TXBUFF
 */
static void link_transfer(void)
{
	  umemmove( &bTX_BUFFER[4], bTX_BUFFER, bTXLen );
	  bTX_BUFFER[0]=LT;
	  bTX_BUFFER[1]=VS;   /* NS; */
	  bTX_BUFFER[2]=AR;
	  bTX_BUFFER[3]=NULL;
	  DLL_Send(bTX_BUFFER, bTXLen+4 );
}

/**
 * Sends a link ACK packet using TXBUFF
 */
static void link_ack(void)
{
   	  txLNK=txNK;
	  txNR=VR;
	  txNK=Rk;
	  Sk=Rk;
	  bTX_BUFFER[0]=LA;
	  bTX_BUFFER[1]=VR;
	  bTX_BUFFER[2]=Rk;
	  bTX_BUFFER[3]=NULL;
	  OutputDebugString("SEND LACK\n");
	  DLL_Send(bTX_BUFFER, 4 );
}

/**
 * Send a link request packet using TXBUFF
 */
static void link_request(void)
{
	  bTX_BUFFER[0]=LR;
	  bTX_BUFFER[1]=N1;
	  bTX_BUFFER[2]=k;
	  bTX_BUFFER[3]=CV;
	  OutputDebugString("SEND LRQ\n");
	  DLL_Send(bTX_BUFFER, 4 );
      bLRSent=true;
}

/**
 * Validate current conexion parameters
 * @param Buff - Last received packet
 * @return true/false
 */
static bool received_parameters_acceptable( uint8_t *Buff )
{
bool res;
	res = true;
	  if( k>Buff[2] )
	  {
			 res = false;
	  }
	  if( N1>Buff[1])
	  {
			 res = false;
	  }
	  if( CV>Buff[3])
	  {
			 res = false;
	  }
return res;
}

/**
 * Stores a packet in the TX queue
 * @param Buff   - Buffer to store
 * @param length - Number of bytes in buffer
 */
static void store_packet(uint8_t *Buff, uint8_t length)
{
	umemcpy(bTX_BUFFER,Buff,length);
	bTXLen = length;
}

/**
 * Initialize protocol variables
 */
static void initialise_variables(void)
{
  AR=0;
  VS=1;
  VR=1;
  Rk=1;
  Sk=0;
  SNR=1;
  umemset( bRX_BUFFER, 0, TX_MSG_BUFF+11+1 );
  PI_Clear();
  C1=N2;
  bLRSent=false;
}

#ifdef  USE_adjust_link_parameters
/**
 * 	sets values of variables maximum_length, window_size and current_version
 *	to the minima of (unit_maximun_length, unit_maximum_window_size, and unit_version,
 *	and the values of the corresponding parameters received in the most recently
 *	received LR message.
 *	@param  LastReceivedLR - Last received LR
 *	@return Always true.
 */
bool adjust_link_parameters(uint8_t *LastReceivedLR)
{
	N1= (LastReceivedLR[1]<U_MAX_LEN)? LastReceivedLR[1]: U_MAX_LEN;
	k = (LastReceivedLR[2]<U_MAX_WIN)? LastReceivedLR[2]: U_MAX_WIN;
	CV= (LastReceivedLR[3]<U_VERSION)? LastReceivedLR[3]: U_VERSION;
	return true;
}
#endif

/**
 * 	Returns true if the tx_sequence_number of the last received link_transfer message is
 *	outside the range of values given by the expression
 *	[ (rx_seq_number-previous_credit_number) to (rx_seq_number-1+rx_credit_number) ]
 *	these variables being the values of parameters in the last transmitted link_acknowledge
 *	messages.
 * @param LastReceivedLT - the LT message LastReceived
 */
bool packet_outside_window(uint8_t *LastReceivedLT)
{
	uint8_t tx_seq_number;
	tx_seq_number=LastReceivedLT[1];
	return (tx_seq_number>=(txNR-txLNK) && tx_seq_number<=(txNR-1+txNK));
}

/**
 * Calculate FCS
 * @param bBuffer - buffer to compute
 * @param bLen    - number of bytes in buffer
 * @return Word - FCS
 */
uint16_t DLL_Crc(uint8_t *bBuffer, uint8_t bLen)
{

 uint16_t fcs;
 uint16_t q;

	fcs=0xffff;
	while(bLen--)
	{
		q=*(wMtab+(*bBuffer++^(fcs>>8)));
		fcs=((q&0xff00)^(fcs<<8))|(q&0x00ff);
	}
return (fcs^0xffff);
}
/****************************************************[END]******/






