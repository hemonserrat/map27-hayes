/**
 * AT24CXX.H
 * AT24CXX EEPROM driver
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
#ifndef _H_AT24CXX
#define _H_AT24CXX

/*--------------------------------------------**
**  Read a byte from the EEPROM               **
**  returns byte read from specified address  **
**--------------------------------------------*/
uint8_t AT24CXXRead(uint8_t Address);
 
/*-------------------------------------------**
**  Write a byte to the EEPROM               **
**  Returns: 0 if ack was ok                 **
**           1 if device newer issued an ack **
**-------------------------------------------*/
uint8_t AT24CXXWrite(uint8_t Address, uint8_t Data);

#endif

