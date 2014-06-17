/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef HAL_H
#define HAL_H

// CHANGE THE TARGET HERE!!
#define TARGET_PC_SIMULATOR

#define MAX_RAM 16*1024    // 64KBYTES PER AREA = 6*64 TOTAL

// PULL IN ALL TYPE DEFINITIONS
#include "newrpl.h"

// GET COMMON C RUNTIME FUNCTIONS

#ifdef TARGET_PC_SIMULATOR
#include <stdlib.h>
#include <string.h>
#endif

#ifdef TARGET_50G
#include <hpgcc3.h>
#endif

// COMMON DEFINITIONS FOR ALL HARDWARE PLATFORMS
WORDPTR *hal_growmem(WORDPTR *base,BINT newsize);


#endif // HAL_H
