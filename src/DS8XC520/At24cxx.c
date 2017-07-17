/**
 * AT24CXX.C
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
#include <hdw.h>
#include <stdint.h>
#include <delays.h>
#include <softi2c.h>

/**
 * Read a uint8_t from the EEPROM
 * @param Address - address to be read
 * @return uint8_t read from specified address
 */
uint8_t AT24CXXRead(uint8_t Address)
{
 uint8_t res;
  /*  Send start condition */
 i2c_start();

  /*  Build slave address for dummy write */
 i2c_writeByte(0xA0);
 i2c_writeByte(Address);
 i2c_restart();
                       /*  Build slave address for read */
  i2c_writeByte(0xA1);        /*  R/W bit =1 */
  res=i2c_readByte();         /*  Read the byte */
  i2c_nack();
  i2c_stop();
  return res;
}

/**
 * Write a byte to the EEPROM
 * @param Address  - address to  write
 * @param Data     - data byte to store
 * @return 0 if ack was ok
 *         1 if device newer issued an ack
 */
uint8_t AT24CXXWrite(uint8_t Address, uint8_t Data)
{

/*   Send start condition */
  i2c_start();

/*    Build slave address for write */
  i2c_writeByte(0xA0);
  i2c_writeByte(Address);

  i2c_writeByte(Data);
  i2c_stop();
  return (i2c_checkWriteFinish());
}

/* **************************************************[ENDL]***** */


