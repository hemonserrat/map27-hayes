/**
 * DELAYS.H
 * Delays functions
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
 * @note   8x51  at 11.059 Mhz
 *
 */
#ifndef _DELAYS_H
#define _DELAYS_H 1

void bwait( uint8_t n );
void wwait( uint16_t n );
void delay( uint8_t msecs );
void sleep( uint8_t secs );

#endif
/* ****************************************************************** */
