/**
 * HDW.H
 * Hardware definitions
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
#ifndef _HDW_H
#define _HDW_H 1

// SDCC specific
#define MICROCONTROLLER_DS8XC520
#include <mcs51reg.h>
#include <stdint.h>
#include <stdbool.h>

// Eclipse built-int parser
// In order to avoid false warnings on the editor
#ifdef __CDT_PARSER__
	#define XDATA
	#define CODE
	#define IDATA
	typedef char    bit;
	typedef char   bool;
    #define INT(int_num, reg_bank)
    // SDCC x51 registers are not "visible" for the parser
    #define SET(reg, value)

#else
	#define XDATA __xdata
	#define CODE  __code
	#define IDATA __idata
	typedef __bit    bit;
    #define INT(int_num, reg_bank)  __interrupt(int_num) __using(reg_bank)
    #define SET(reg, value)  ((reg)=(value))
#endif


/*-----------------------**
** 	General definitions  **
**-----------------------*/
typedef uint16_t          HTIME;
typedef uint8_t           HTIMESTOP;


#ifndef NULL
	#define NULL           0
#endif
	
#define LOBYTE(w)           ((uint8_t)(w))
#define HIBYTE(w)           ((uint8_t)((uint16_t)(w) >> 8))

/*----------------------**
**    Timer definitions **
**----------------------*/

extern  HTIME  hdw_secs;
#define HDW_Time()   (hdw_secs)


/*-------------------------**
**   LED Indicators macros **
**-------------------------*/
#ifdef MICROCONTROLLER_DS8XC520

	#define LON(A)  SET(A,0)
	#define LOFF(A) SET(A,1)

	#define LTX     T0
	#define LRX     INT0
	#define	LCA     T1
	#define LXX     INT1

	#define LOFFALL() { LOFF(LRX);LOFF(LTX);\
					   LOFF(LCA);LOFF(LXX); }
	#define LONALL()  { LON(LRX);LON(LTX);\
					   LON(LCA);LON(LXX); }
#else
	#define LON(A)
	#define LOFF(A)
	#define LRX
	#define LTX
	#define	LCA
	#define LXX
	#define LOFFALL()
	#define LONALL()
#endif



#endif
/**************************************************[END]*********/


				
