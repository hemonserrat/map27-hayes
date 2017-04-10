/**
 * PI.H
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
#ifndef _PIL_H
#define _PIL_H 1
#include <hdw.h>

/*   Debug support    */
#ifdef DEBUG
void  OutputDebugString(const uint8_t *messg );
#else
#define OutputDebugString(A)
#endif

//  Initialise Physical Interface Layer
void PI_Initialise(void);

//  Reset Physical Interface Layer
#define PI_DeInitialise()

//  Clear TX and RX Buffers
void PI_Clear(void);

//  Returns TRUE if a pending byte on port exists
bool PI_IsDataReady(void);

//   Get the first byte pending
uint8_t PI_Receive(void);

//   Send bLen bytes from 	outpkt
uint8_t PI_Send(uint8_t *outpkt, uint8_t bLen );


void hl_init(void);
#endif

/**************************************************[END]*********/

