/**
 * uSTDLIB.H
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
#ifndef _uSTDLIB_H_
#define _uSTDLIB_H_ 1


void umemcpy(uint8_t *dest, const uint8_t *src, uint8_t len );
void umemset(uint8_t *dest, uint8_t c, uint8_t len );
void umemmove(uint8_t *dest, const uint8_t *src, uint8_t len);
uint8_t ustrlen( const uint8_t *s );
void uitoa(uint16_t n, uint8_t *s, uint8_t pad);
uint16_t uatoi(uint8_t *s);
void ureverse(uint8_t *s);
char umemcmp(uint8_t *dest, const uint8_t *src, uint8_t len );
void ustrupr( uint8_t *s );
char umemicmp(uint8_t *dest, const uint8_t *src, uint8_t len );
uint8_t utoupr(uint8_t n);

#endif

/*   ****************************************************[ENDL]**** */
