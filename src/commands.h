/*
 * Copyright (c) 2017 by Thomas König <tom@fair-coin.org>
 *
 * commands.h is part of Fasito, the FairCoin signature token.
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

#ifndef SRC_COMMANDS_H_
#define SRC_COMMANDS_H_

extern bool serialEcho;

typedef struct Command
{
    char command[8];
    bool (*handler)(const char **tokens, const uint8_t nTokens);
    uint8_t len;
    bool requireLogin;
} Command;

extern void printHelp();
extern void printVersion();
extern void printStatus();
extern void initNonceStorage();

#endif /* SRC_COMMANDS_H_ */
