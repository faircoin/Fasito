#
# Copyright (c) 2020-2022 by Thomas König <tom@faircoin.world>
# 
# Makefie is part of Fasito, the FairCoin signature token.
#
# Fasito is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Fasito is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Fasito, see file COPYING.
# If not, see <http://www.gnu.org/licenses/>.
#

#
# This is the Makefile to build FasitoEMU the virtual Fasito
#
# make
#

# All Target
all: secp256k1-lib FasitoEMU

obj-%:
	@mkdir $@

secp256k1-lib: lib/include/secp256k1.h

lib/include/secp256k1.h:
	@git clone https://github.com/faircoin/secp256k1-mc-arm.git
	@cd secp256k1-mc-arm && ./autogen.sh && \
	./configure --disable-tests --prefix $(shell pwd)/lib --disable-tests --enable-experimental --enable-module-schnorr --enable-module-ecdh
	$(MAKE) -C secp256k1-mc-arm
	$(MAKE) -C secp256k1-mc-arm install 2>&1 >/dev/null

OBJS += \
obj-emu/Serial.o \
obj-emu/Arduino.o \
obj-emu/main.o \
obj-emu/update.o

OBJS += \
obj-src/commands.o \
obj-src/utils.o \
obj-src/fasito_error.o \
obj-src/main.o

CPP_OPTS=-Iemu -I../src -Ilib/include -Wall -O0 -fmessage-length=0 -DFASITO_EMU

obj-emu/%.o: emu/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: C++ Compiler'
	g++ $(CPP_OPTS) -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

obj-src/%.o: ../src/%.cpp 
	@echo 'Building file: $<'
	@echo 'Invoking: C++ Compiler'
	g++ $(CPP_OPTS) -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

FasitoEMU: obj-emu obj-src $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: C++ Linker'
	g++ --static -Llib/lib -o "FasitoEMU" $(OBJS) -lsecp256k1 -lgmp
	@echo 'Finished building target: $@'
	@echo ' '

clean: clean-build
	-rm -f FasitoEMU

clean-build:
	-rm -rf obj-src obj-emu

distclean: clean
	-rm -rf secp256k1-mc-arm lib eeprom.bin

