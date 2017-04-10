/**
 * DTENL.H
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
#ifndef _DTENL_H_
#define _DTENL_H_ 1

// Status messaging
#define SEND_STATUS    0x80  /* D->R */
#define RECEIVE_STATUS 0x80	 /* R->D */
#define STACK_POS      0xC0	 /* R->D */
#define STACK_QUE	   0xD0	 /* R->D */
#define STACK_NEG      0xE0	 /* R->D */

// Short data (SST) messaging
#define SEND_SST       0x81	 /* D->R */
#define RECEIVE_SST	   0x81	 /* R->D */
#define SST_POS		   0xC0	 /* R->D */
#define SST_QUE		   0xD0	 /* R->D */
#define SST_NEG		   0xE0	 /* R->D */

// Extended data (MST) messaging
#define SEND_MST       0x82	 /* D->R */
#define RECEIVE_MST	   0x82	 /* R->D */
#define MST_POS		   0xC0	 /* R->D */
#define MST_QUE		   0xD0	 /* R->D */
#define MST_NEG		   0xE0	 /* R->D */

// Voice and modem (non-prescribed data) call
#define SETUP_VM       0xA4  /* D->R */
#define SETUP_EVM      0xA5	 /* D->R */
#define SETUP_POS      0xC4  /* R->D */
#define SETUP_QUE      0xD4	 /* R->D */
#define SETUP_NEG      0xE4	 /* R->D */
#define INCOM_VMC      0xA4	 /* R->D */
#define INCOM_EVMC     0xA5	 /* R->D */
#define RECEIVE_POS    0xC5	 /* R->D */
#define RECEIVE_WAR    0xD5	 /* R->D */
#define RECEIVE_CNC	   0xE5	 /* R->D */

// Modem call data transfer
#define SEND_MD	       0xA3  /* D->R */
#define RECEIVE_MD	   0xA3	 /* R->D */

// Connection clearing
#define DISC_NOR	   0x86	 /* D->R */
#define DISC_CAN	   0xA6	 /* D->R */
#define CLEAR_NOR	   0x86	 /* R->D */
#define CLEAR_ABN	   0xA6	 /* R->D */

// Diversion Control
#define DIV_REQ		   0x87	 /* D->R */
#define DIV_CAN		   0xA7	 /* D->R */
#define DIVACK_POS	   0xC7	 /* R->D */
#define DIVACK_NEG	   0xE7	 /* R->D */

// Radio controlling and miscelaneous messages
#define RADIO_INT      0xB0	 /* D->R */
#define RADIO_PER	   0xB0	 /* R->D */
#define NUM_INFO	   0xB1	 /* R->D */
#define RADIO_CONT	   0xB2	 /* D->R */
#define OPER_COND	   0xB2	 /* R->D */
#define NET_INFO	   0xB5	 /* R->D */
#define RADIO_MAN	   0xB3	 /* D->R */
#define RADIO_SET	   0xB3	 /* R->D */
#define RPROTO_INFO	   0xB4	 /* R->D */
#define DPROTO_INFO	   0xB4	 /* D->R */
#define VOL_CTR		   0xB6	 /* D->R */
#define DIAL_STR	   0xB7	 /* D->R */
#define RADIO_TST	   0x01	 /* D->R */
#define RADIO_TSTRES   0x01	 /* R->D */

// Others
#define NL_ESCAPE      0x00

// Adress Presentations
#define NOADD_ADDR  0x00
#define SAMIS_BCD   0x10
#define SPARE       0x20
#define PABX        0x30
#define PABX_PI		0x40
#define MPT_PI      0x50
#define MPT_PIS		0x60
#define MMI			0x70

// State Machine
#define T_IDLE      0x00
#define T_OCW     	0x01 /* outgoing call wait */
#define T_OICW      0x02 /* outgoing include call wait */
#define T_ACTIVE    0x03
#define T_ICW       0x04 /* incoming call wait */
#define T_SW        0x05 /* Status Wait */
#define T_STW       0x06 /* Segment Transaction wait */
#define T_NS        0x07 /* no service */

#define T_PCKPEND   0x08 /* Pending Packet to process */

// CODING INFO
#define MPT1343_BCD_RP   0x17
#define MPT1343_CCITT_RP 0x27
#define MPT1343_BINARY   0x87
#define MPT1343_BCD      0x97
#define MPT1343_CCITT    0xA7
#define MPT1343_CCITT_5  0xB7
#define MPT1343_PC8      0xF7

// User Messages
#define M_PCKPEND   0x08 /* PPP */
#define M_ACTIVE    0x03 /* Link Ready */

#define SET_IDENT(A) A
#define GET_IDENT(A) A

typedef struct {
uint8_t Msgtyp;
uint8_t PFIX;
uint16_t Ident;  // Invert store
uint8_t AdescLength;
uint8_t *Address;
} NLHEADER;


void DTENL_GetMessage(void);
void DTENL_input(uint8_t type, uint8_t *Buff, uint8_t bLen );
/* User Entry Point */
void DTENL_UserApp(uint8_t type, uint8_t *Buff, uint8_t Len);

extern 	uint8_t LastTrans;
#define DTENL_IsSpace()	          (LastTrans==T_IDLE)
#define DTENL_RemovePackets()     DLL_input( I_CVE, NULL, 1)
#define DTENL_PowerOn()			  DLL_input( I_PON, NULL, 0); LastTrans=T_NS;
#define DTENL_ProcessMessage()    LastTrans = T_IDLE


bool DTENL_PostSSTMessage(uint8_t prefix, uint16_t ident, uint8_t *messg, uint8_t len);
bool DTENL_PostMSTMessage(uint8_t prefix, uint16_t ident, uint8_t *messg, uint8_t len);
bool DTENL_Status(uint8_t prefix, uint16_t ident, uint8_t St);
bool DTENL_NorDisconnect(uint8_t prefix, uint16_t ident, uint8_t cause);
bool DTENL_AbnorDisconnect(uint8_t prefix, uint16_t ident, uint8_t cause);


#endif
/****************************************************[END]******/
