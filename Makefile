 # Makefile  
 #
 # This file is part of MAP27-HAYES Commands Bridge.
 #
 # Copyright (C) 2005,  Hernan Monserrat hemonserrat<at>gmail<dot>com
 #
 # This program is free software: you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation, either version 3 of the License, or
 # (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program.  If not, see <http://www.gnu.org/licenses/>.
LDFLAGS=--model-large

# Includes - root
INCDIR=inc
# Includes 
IDIR =inc -I/Applications/sdcc-3.6.0/share/sdcc/include/mcs51 -I/Applications/sdcc-3.6.0/share/sdcc/include
IDIR += -Iinc/core  -Iinc/DS8XC520
CC=/Applications/sdcc-3.6.0/bin/sdcc 
CFLAGS=-mmcs51 --model-large -I$(IDIR) -DDS520 -DCRC_ACTIVE

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = hdw.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

APP_OBJ = main.rel hayli.rel 
CORE_OBJ =   dtenl.rel dll.rel
DS8XC520_OBJ = ustdlib.rel pi.rel At24cxx.rel softi2c.rel delays.rel

APP_OBJ_PATH = $(patsubst %,$(ODIR)/%,$(APP_OBJ))
CORE_OBJ_PATH = $(patsubst %,$(ODIR)/%,$(CORE_OBJ))
DS8XC520_OBJ_PATH = $(patsubst %,$(ODIR)/%,$(DS8XC520_OBJ))


all: jpc

jpc: $(APP_OBJ_PATH) $(CORE_OBJ_PATH) $(DS8XC520_OBJ_PATH) 
	$(CC) $(LDFLAGS)  $(APP_OBJ_PATH) $(CORE_OBJ_PATH) $(DS8XC520_OBJ_PATH)   -o build/$@.ihx

.PHONY: clean

clean:
	rm -f $(ODIR)/*.asm $(ODIR)/*.lst $(ODIR)/*.rel $(ODIR)/*.sym $(ODIR)/*.rst *~ $(INCDIR)/*~ 

$(APP_OBJ_PATH): 
	$(CC) $(CFLAGS) -c src/$(notdir $(basename $@)).c -o $@
	
$(CORE_OBJ_PATH): 
	$(CC) $(CFLAGS) -c src/core/$(notdir $(basename $@)).c -o $@

$(DS8XC520_OBJ_PATH): 
	$(CC) $(CFLAGS) -c src/DS8XC520/$(notdir $(basename $@)).c -o $@
	