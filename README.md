# MAP27 Trunked Radio Protocol
(C) 2005 Hernan Monserrat, GNU General Public License version 3

This project implements a protocol converter between a HAYES style subset of ASCII commands to/from 
MAP27, a binary Radio protocol for Trunked Radios on MPT1327 networks.

The core implementation follows the MAP27 specification version 1.5 year 1998.
All the names and conventions used in the source code follow the specs convention.
(not so good programming convention practices but good enogh in order to follow the specs name convention).

The idea of the code is to be embeddable and portable,  keeping in mind low-end 8 bit microcontrollers
with low memory resources.

## Pre-requisites
* current compiler:  SDCC 3.6.0 
* current target uC:  Dallas DS87C520 - Two Full-Duplex Hardware Serial Ports
* memory used:  1KB SRAM 14KB EPROM


## Directory & files organization

- build: output result, binary images
- doc:  doxygen documentation output
- inc:  include files
- obj:  intermediate files, temporary
- src:  source files
   * core:  MAP27 core protocol
   * DS8XC520:  hardware target drivers
   * main.c  - entry point
   * hayli.c - HAYES Lexical interpreter

## To do
1. Porting to ARM-M3 uC


