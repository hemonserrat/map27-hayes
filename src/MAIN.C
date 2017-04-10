/**
 * MAIN.C
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
#include <HDW.h>
#include <PI.h>
#include <DLL.h>
#include <dtenl.h>
#include <hayli.h>


/**
 * Main entry point.
 */
void main(void)
{
  // Initialize microcontroller peripherals
  hl_init();
  // turn off all on board LED's
  LOFFALL();

  OutputDebugString(">INIT");

  // Read config and init HAYLI port
  HLI_Initialize();

  while(1)
  {
	  // Process queue from MAP27 Dinamic Link Layer
 	  DTENL_GetMessage();     
 	  // Free up the queue state
 	  DTENL_ProcessMessage(); 
 	  // Process commands from HAYLI port
	  HLI_Idle();
  }
/* never reached */
}

/***************************************************[end]******/

