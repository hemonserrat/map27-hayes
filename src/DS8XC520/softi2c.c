/**
 * SoftI2C.C
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
#include <hdw.h>
#include <stdint.h>
#include <delays.h>
#include <softi2c.h>


/*---------------------**
**  I/O definitions... **
**  24C64 EEPROM (I2C) **
**---------------------*/
#define SDA  P1_7
#define SCL  P1_6

/**
 * Issue a stop condition
 */
void i2c_stop(void)
{
  SDA=0;
  SCL=1;
	bwait(1);
  SDA=1;
}

/**
 *  issue a start condition
 */
void i2c_start(void)
{
  SDA=1;
  SCL=1;
	bwait(1);
  SDA=0;
	bwait(1);
  SCL=0;
}

/**
 * send a restart condition to the bus
 */
void i2c_restart(void)
{
	bwait(1);
  SCL=1;               /*  Generate re-start condition (Dont use START(), not working */
                       /*  because of SCL/SDA setup seq) */
  SDA=1;
	bwait(2);
  SDA=0;
	bwait(2);
  SDA=1;
	bwait(2);
  SCL=0;
	bwait(2);
}


/**
 * Send a clock pulse
 * @return the status of SDA during the clock high
 */
static uint8_t i2c_clock(void)
{
  uint8_t rc;
	bwait(1);
  SCL=1;
	bwait(1);
  rc=SDA;
  SCL=0;
  return rc;
}

/**
 * issue not acknowledge
 * @return  clock return signal
 */
uint8_t i2c_nack(void)
{
  SDA=1;
  return i2c_clock();
}


/**
 * issue an acknowledge
 * @return  clock return signal
 */
uint8_t i2c_ack(void)
{
  SDA=0;
  return i2c_clock();
}

/**
 * Read 8 bit from device
 * @return   1 uint8_t read
 */
 uint8_t i2c_readByte(void)
{
  uint8_t i;
  uint8_t Data=0;

  SDA=1;
  for (i=0; i<8; i++) {
    Data<<=1;      /*  Advance to next bit */
    if (i2c_clock())     /*  generate clock pulse and read dataline */
      Data |=0x01; /*  it was a '1' */
    else
      Data &=0xFE; /*  it was a '0' */
  }
  return Data;
}

/**
 * Write 8 bit to device
 * @param Data - uint8_t to store
 */
void i2c_writeByte(uint8_t Data)
{
  uint8_t i;
  for (i=0; i<8; i++) {
    SDA=(Data & 0x80); /*  Send out the bit */
    Data<<=1;          /* Advance to next bit */
    i2c_clock();
  }
  i2c_ack();
}

/**
 * Poll device to see when a write sequence actually is finished
 * @return 0 if ack was ok
 *         1 if device newer issued an ack
 */
uint8_t i2c_checkWriteFinish(void)
{
  uint8_t i;
  for (i=0; i<0x80; i++) {
    i2c_start();
    i2c_writeByte(0xA0);
    if (!i2c_nack()) {
    	i2c_stop();
      /*  Ack received from device... */
      return 0;
    }
  }
  /*  No ack received... */
  i2c_stop();
  return 1;
}
