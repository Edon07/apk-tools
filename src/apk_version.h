/* apk_version.h - Alpine Package Keeper (APK)
 *
 * Copyright (C) 2005-2008 Natanael Copa <n@tanael.org>
 * Copyright (C) 2008 Timo Teräs <timo.teras@iki.fi>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation. See http://www.gnu.org/ for details.
 */

#ifndef APK_VERSION_H
#define APK_VERSION_H

#include "apk_blob.h"

#define APK_VERSION_LESS		-1
#define APK_VERSION_EQUAL		0
#define APK_VERSION_GREATER		1

#define APK_VERSION_RESULT_MASK(r)	(1 << ((r)+1))

int apk_version_validate(apk_blob_t ver);
int apk_version_compare(apk_blob_t a, apk_blob_t b);

#endif