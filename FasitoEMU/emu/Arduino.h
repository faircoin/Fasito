/*
 * Copyright (c) 2017-2022 by Thomas König <tom@faircoin.world>
 *
 * Arduino.h is part of Fasito, the FairCoin signature token.
 *
 * Fasito is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fasito is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fasito, see file COPYING.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARDUINO_H_
#define ARDUINO_H_

#include <stdint.h>
#include "Serial.h"

#define FTFL_FSEC 0x64
#define OUTPUT 0
#define LOW 0
#define HIGH 0
#define FASTRUN
#define WFI

extern SerialEMU Serial;

void eeprom_read_block(void *buf, const void *addr, uint32_t len);
void eeprom_write_block(const void *buf, void *addr, uint32_t len);
void delay(long n);
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);

#endif /* ARDUINO_H_ */
