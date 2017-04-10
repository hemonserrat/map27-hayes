/**
 * HAYLI.H
 * HAYES Commands interpreter 
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
#ifndef _HAYLI_H_
#define _HAYLI_H_ 1

/*-------------------**
**  constants types  **
**-------------------*/

#define MSG_BUFF  255

#define CMD_ECO   'E'
#define CMD_INFO  'I'
#define CMD_RESP  'Q'
#define CMD_CFG   'Z'
#define CMD_FR    'F'
#define CMD_FC    'K'
#define CMD_SI    'V'
#define CMD_SAVE  'W'
#define CMD_LC    'Y'
#define CMD_FC2   'Q'
#define CMD_XON   'X'
#define CMD_HANG  'H'
#define CMD_ENDL  '+'

#define CMD_PER   '&'
#define CMD_SLA   '\\'

#define TERM      '\r'

#define CMD_STATUS  'S'
#define CMD_MST     'D'
#define CMD_BAUD    'B'
#define CMD_CODING  'C'


 
uint8_t GetByte1(void);
void SendByte1(uint8_t c);
void SendString1(const uint8_t *s);

void HLI_Idle(void);
void HLI_Initialize(void);

void Response(const uint8_t *str);
void Reset(void);
uint8_t ATZ(uint8_t n);
void F(void);
void ShowCfgs(void);
uint8_t Save(uint8_t n);


#endif
/* ***********************************************[endl]*** */
