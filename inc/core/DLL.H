/**
 * DLL.H
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
 */
#ifndef _DLL_H
#define _DLL_H 1

//************************************************************
//  DLL Packet format
#define SYN  0x16
#define DLE	 0x10
#define STX	 0x02
#define ETX	 0x03

//************************************************************
//  DLL Packet types
#define LR   0x01
#define LA   0x02
#define LT   0x04

//************************************************************
// Acknowledgement request types
#define AR_NR 0x00
#define AR_AI 0x01

//************************************************************
// DLL State Machine states
#define UNDEFINED  0x00
#define RESET_WAIT 0x01
#define LINK_WAIT  0x02
#define READY	   0x03

//************************************************************
// Rx State Machine states
//#define IDLE	   0x00
#define STARTFLAG1 0x01	
#define STARTFLAG3 0x02
#define STARTFLAG2 0x03
#define WAITDLE	   0x04
#define WAITFCS1   0x05
#define WAITFCS2   0x06
#define PREADY     0x07

//************************************************************
// INPUTS
#define I_PON  0x00 // power_on

#define I_NLR  0x10 // network layer reset
#define I_NLP  0x11 // Network layer packet
#define I_CVE  0x12 // Credit value

#define I_LET  0x30 // link establishment timout T0
#define I_RTO  0x31 // Retry timeout
#define I_ATO  0x32 // acknowledge  timout T2
#define I_ACO  0x33 // Activity timout T3
#define I_LFD  0x34 // link failure detection timeout T4

#define I_LRQ  0x50 // link request
#define I_LAK  0x51 // link acknowledge
#define I_LTR  0x52 // link transfer

#define SUCCESS   0x00
#define LNK_FAIL  0x01
#define DLLREJECT 0x02

//************************************************************
//  Interface to Network layer functions
void DLL_Idle(void);
void DLL_input(uint8_t type, uint8_t *Buff, uint8_t bLen );

// Interface to Physical interface functions
uint8_t DLL_Send(uint8_t *bBuffer, uint8_t bLen );


#endif
//*************************************************[END]*********

