/**
 * SoftI2C.H
 * I2C protocol driver - software implementation
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
#ifndef INC_DS8XC520_SOFTI2C_H_
#define INC_DS8XC520_SOFTI2C_H_

void i2c_start(void);
void i2c_restart(void);
void i2c_stop(void);

uint8_t i2c_nack(void);
inline uint8_t i2c_ack(void);

uint8_t i2c_readByte(void);
void i2c_writeByte(uint8_t Data);
uint8_t i2c_checkWriteFinish(void);

#endif /* INC_DS8XC520_SOFTI2C_H_ */
