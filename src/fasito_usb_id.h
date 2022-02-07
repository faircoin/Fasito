/*
 * Copyright (c) 2017-2022 by Thomas König <tom@faircoin.world>
 *
 * fasito_usb_id.h is part of Fasito, the FairCoin signature token.
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


#ifndef SRC_FASITO_USB_ID_H_
#define SRC_FASITO_USB_ID_H_

#include "version.h"

#define MANUFACTURER_NAME {'T','h','e',' ','F','a','i','r','C','o','i','n',' ','d','e','v','e','l','o','p','e','r','s'}
#define MANUFACTURER_NAME_LEN 23
#define PRODUCT_NAME      {'F','a','s','i','t','o',' ','V',('0' + __FASITO_VERSION_MAJOR__),'.',('0' + __FASITO_VERSION_MINOR__)}
#define PRODUCT_NAME_LEN  11

#endif /* SRC_FASITO_USB_ID_H_ */
