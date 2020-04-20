/*
 * Copyright (c) 2019-2020 by Thomas König <tom@fair-coin.org>
 *
 * version.h is part of Fasito, the FairCoin signature token.
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

#ifndef VERSION_H_
#define VERSION_H_

#define __FASITO_VERSION_MAJOR__ 1
#define __FASITO_VERSION_MINOR__ 3

#define STR_NAME(s) #s
#define STR(s) STR_NAME(s)
#define __FASITO_VERSION__ "v" STR(__FASITO_VERSION_MAJOR__) "." STR(__FASITO_VERSION_MINOR__)

#define CONFIG_VERSION 1

#endif /* VERSION_H_ */
