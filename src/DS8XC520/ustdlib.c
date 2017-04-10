/**
 * uSTDLIB.C
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
#include <stdint.h>
#include <ustdlib.h>

/**
 * Copies len bytes form src to dest
 * @param dest - destination buffer
 * @param src  - source buffer
 * @param len  - number of bytes to copy
 */
void umemcpy(uint8_t *dest, const uint8_t *src, uint8_t len )
{
  while( len-- ) *dest++=*src++;
}


/**
 * sets len bytes from dest with c
 * @param dest - destination buffer
 * @param c    - byte pattern
 * @param len  - number of bytes to set
 */
void umemset(uint8_t *dest, uint8_t c, uint8_t len )
{
  while( len-- ) *dest++=c;
}

/**
 * moves len bytes from src to dest
 * @param dest - destination buffer
 * @param src  - source buffer
 * @param len  - bytes to move
 */
void umemmove(uint8_t *dest, const uint8_t *src, uint8_t len)
{
	if( dest < src ) {
		umemcpy((uint8_t*)dest,(uint8_t*)src, len );
	} else  {
		while(len--)
			*(dest+len)=*(src+len);
	}
}


/**
 * return the number of bytes in s
 * @param s - NULL terminated buffer.
 * @return number of bytes in s without NULL.
 */
uint8_t ustrlen(const uint8_t *s )
{
	uint8_t res;
	res = 0;
	while( *s++ ) res++;
	return res;
}

/**
 * compares src and dest len bytes
 * @param dest - destination buffer
 * @param src  - source buffer
 * @param len  - number of bytes to compare
 * @return   0  - src==dest
 *           0> - dest > src
 *           0< - dest < src
 */
char umemcmp(uint8_t *dest, const uint8_t *src, uint8_t len )
{
	char dif;
	for (; len-- >0; src++, dest++ )
	{
		dif = *dest - *src;
		if (dif != 0)
			return(dif);
	}
	return(0);
}


/**
 * compares src and dest len bytes, ignore case
 * @param dest - destination buffer
 * @param src  - source buffer
 * @param len  - number of bytes to compare
 * @return  0  - src==dest
 *          0> - dest > src
 *          0< - dest < src
 */
char umemicmp(uint8_t *dest, const uint8_t *src, uint8_t len )
{
	char dif;
	uint8_t bSrc;
	uint8_t bDest;
	for (; len-- >0; src++, dest++ )
	{
		bSrc = utoupr(*src);
		bDest = utoupr(*dest);
		dif = bSrc - bDest;
		if (dif != 0)
			return(dif);
	}
	return(0);
}

/**
 * Reverse the contents of string s
 * @param s - NULL terminated string
 */
void ureverse(uint8_t *s)
{
	uint8_t *j;
	uint8_t c;
	j = s + ustrlen(s) - 1;
	while(s < j) {
		c = *s;
		*s++ = *j;
		*j-- = c;
	}
}

/**
 * converts an integer to an ASCII string representation
 * @param n - number to be converted
 * @param s - Buffer to store result
 * @param pad - right alignment padding
 */
void uitoa(uint16_t n, uint8_t *s, uint8_t pad)
{
	uint8_t *ptr;
	ptr = s;
	do {
		*ptr++ = n % 10 + '0';
		pad--;
	} while ((n = n / 10) > 0);
	while(pad--) *ptr++=' ';
	*ptr = 0x00;
	ureverse(s);
}

/**
 * converts an ASCII string to an Integer
 * @param s - NULL terminated string to convert
 * @return uint16_t number
 */
uint16_t uatoi(uint8_t *s)
{
	uint8_t *ptr;
	uint16_t n;
	uint16_t r;
	uint16_t x;
	ptr = s;
	ureverse(s);
	x = 1;
	n = 255;
	r = 0;
	do {
		n = *ptr++ - '0';
		if( n < 10 ){
			n =(x==1)? n:n*x;
			r += n;
			x *= 10;
		}
	} while (*ptr);
	*ptr = 0x00;
	ureverse(s);
	return r;
}

/**
 * turns n to upper case
 * @param n - Byte to convert
 * @return  n in upper case
 */
uint8_t utoupr(uint8_t n)
{
	uint8_t c;
	c = n - 'a';
	if( c < ('z'-'a')+1 )
		n = c + 'A';
	return n;
}
		 
/**
 * convert s to upper case
 * @param s - NULL terminated string to convert
 */
void ustrupr( uint8_t *s )
{
	uint8_t *ptr;

	ptr = s;
	while(*ptr)
	{
		*ptr = utoupr(*ptr);
		ptr++;
	}
}

/*  ************************************************[ENDL]******** */
