/*
 * Copyright (c) 2017 by Thomas König <tom@fair-coin.org>
 *
 * utils.h is part of Fasito, the FairCoin signature token.
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

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include "fasito.h"

#define CLS "\033[2J"
#define pHEX(a, b, l) { Serial.print(a ": "); printHex(b, l, true); }
#define WFI asm("wfi")
#define LED 13

extern FasitoNVRam nvram;

extern void printHex(const uint8_t *buf, const size_t len, const bool addLF = false);
extern bool parseHex(uint8_t *out, const char *in, size_t len);
extern const char **tokenise(char *buf, uint8_t *nTokens);
extern bool readEEPROM(FasitoNVRam *dst);
extern void writeEEPROM(FasitoNVRam *dst);
extern bool fasitoErrorStr(const char *errorStr);
extern bool fasitoError(uint8_t errorNo);
extern bool fasitoError(uint8_t errorNo, int arg);
extern bool comparePins(const UserPIN *userPin, const char *pin);
extern void readMAC(uint8_t *mac);
extern void reverseBytes(uint8_t *buf, size_t len);

#endif /* SRC_UTILS_H_ */
