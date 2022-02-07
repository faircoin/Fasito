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
# This is the Makefile to build Fasito.hex flashable firmware
# (stripped down version of the auto generated build files the from eclipse IDE)
#
# If you use Ubuntu 18.04 install the proper tool chain like this:
#
# sudo apt install build-essential gcc-arm-none-eabi
# make
#

# All Target
all: Fasito.hex clean-build

obj-%:
	@mkdir $@

C_SRCS += \
teensy3/eeprom.c \
teensy3/math_helper.c \
teensy3/mk20dx128.c \
teensy3/nonstd.c \
teensy3/pins_teensy.c \
teensy3/touch.c \
teensy3/usb_desc.c \
teensy3/usb_dev.c \
teensy3/usb_mem.c \
teensy3/usb_rawhid.c \
teensy3/usb_seremu.c \
teensy3/usb_serial.c

CPP_SRCS += \
teensy3/IntervalTimer.cpp \
teensy3/Print.cpp \
teensy3/Stream.cpp \
teensy3/WMath.cpp \
teensy3/WString.cpp \
teensy3/avr_emulation.cpp \
teensy3/main.cpp \
teensy3/new.cpp \
teensy3/usb_inst.cpp \
src/commands.cpp \
src/fasito_error.cpp \
src/main.cpp \
src/update.cpp \
src/utils.cpp

S_UPPER_SRCS += \
teensy3/memcpy-armv7m.S \
teensy3/memset.S

OBJS += \
obj-teensy3/IntervalTimer.o \
obj-teensy3/Print.o \
obj-teensy3/Stream.o \
obj-teensy3/WMath.o \
obj-teensy3/WString.o \
obj-teensy3/avr_emulation.o \
obj-teensy3/eeprom.o \
obj-teensy3/main.o \
obj-teensy3/math_helper.o \
obj-teensy3/memcpy-armv7m.o \
obj-teensy3/memset.o \
obj-teensy3/mk20dx128.o \
obj-teensy3/new.o \
obj-teensy3/nonstd.o \
obj-teensy3/pins_teensy.o \
obj-teensy3/touch.o \
obj-teensy3/usb_desc.o \
obj-teensy3/usb_dev.o \
obj-teensy3/usb_inst.o \
obj-teensy3/usb_mem.o \
obj-teensy3/usb_rawhid.o \
obj-teensy3/usb_seremu.o \
obj-teensy3/usb_serial.o \
obj-src/commands.o \
obj-src/fasito_error.o \
obj-src/main.o \
obj-src/update.o \
obj-src/utils.o

S_UPPER_DEPS += \
obj-teensy3/memcpy-armv7m.d \
obj-teensy3/memset.d

C_DEPS += \
obj-teensy3/eeprom.d \
obj-teensy3/math_helper.d \
obj-teensy3/mk20dx128.d \
obj-teensy3/nonstd.d \
obj-teensy3/pins_teensy.d \
obj-teensy3/touch.d \
obj-teensy3/usb_desc.d \
obj-teensy3/usb_dev.d \
obj-teensy3/usb_mem.d \
obj-teensy3/usb_rawhid.d \
obj-teensy3/usb_seremu.d \
obj-teensy3/usb_serial.d

CPP_DEPS += \
obj-teensy3/IntervalTimer.d \
obj-teensy3/Print.d \
obj-teensy3/Stream.d \
obj-teensy3/WMath.d \
obj-teensy3/WString.d \
obj-teensy3/avr_emulation.d \
obj-teensy3/main.d \
obj-teensy3/new.d \
obj-teensy3/usb_inst.d \
obj-src/commands.d \
obj-src/fasito_error.d \
obj-src/main.d \
obj-src/update.d \
obj-src/utils.d

obj-src/%.o: src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fsingle-precision-constant -Wall  -g -D__MK20DX256__ -DARDUINO=105 -DUSB_SERIAL -DF_CPU=96000000 -I"teensy3" -I"includes" -std=gnu++0x -fabi-version=0 -fno-exceptions -fno-rtti -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

obj-teensy3/%.o: teensy3/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fsingle-precision-constant -Wall  -g -D__MK20DX256__ -DARDUINO=105 -DUSB_SERIAL -DF_CPU=96000000 -I"teensy3" -I"includes" -std=gnu++0x -fabi-version=0 -fno-exceptions -fno-rtti -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

obj-teensy3/%.o: teensy3/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fsingle-precision-constant -Wall  -g -D__MK20DX256__ -DARDUINO=105 -DUSB_SERIAL -DF_CPU=96000000 -I"teensy3" -I"includes" -std=gnu11 -Wstrict-prototypes -Wbad-function-cast -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

obj-teensy3/%.o: teensy3/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fsingle-precision-constant -Wall  -g -x assembler-with-cpp -DMKL25Z4 -DHSE_VALUE=24000000 -I"teensy3" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Fasito.elf: obj-src obj-teensy3 $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C++ Linker'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fsingle-precision-constant -Wall  -g -T "teensy3/mk20dx256.ld" -Xlinker --gc-sections -L"libs" -Wl,-Map,"Fasito.map" --specs=nosys.specs -o "Fasito.elf" $(OBJS) -lsecp256k1
	@echo 'Finished building target: $@'
	@echo ' '

Fasito.hex: Fasito.elf
	@echo 'Invoking: Cross ARM GNU Create Flash Image'
	arm-none-eabi-objcopy -O ihex "Fasito.elf"  "Fasito.hex"
	@echo 'Finished building: $@'
	@echo ' '

clean: clean-build
	-rm -f Fasito.hex

clean-build:
	-rm -rf obj-src obj-teensy3 Fasito.map Fasito.elf
