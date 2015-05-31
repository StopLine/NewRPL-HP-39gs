/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "hal.h"

// EXTERNAL RPL MACHINE REGISTERS



// MEMORY REGIONS. EACH MEMORY REGION CAN GROW INDEPENDENTLY
// GROWTH MAY TRIGGER A GC IF INSUFFICIENT MEMORY.

// TEMPOB AND TEMPBLOCKS

WORDPTR TempOb __DATAORDER1__; // TEMPORARY OBJECT STORAGE
WORDPTR *TempBlocks __DATAORDER1__;    // TEMPOB BLOCK POINTERS STORAGE
WORDPTR *TempBlocksEnd __DATAORDER1__; // END OF THE TEMPOB BLOCKS
BINT TempBlocksSize __DATAORDER1__;


// GLOBAL ARRAY OF POINTERS THAT ARE TO BE UPDATED BY THE GC
WORDPTR GC_PTRUpdate[MAX_GC_PTRUPDATE] __DATAORDER1__;


// RETURN STACK

WORDPTR *RStk __DATAORDER2__;   // BASE OF RETURN STACK
WORDPTR *RSTop __DATAORDER2__; // TOP OF THE RETURN STACK
BINT RStkSize __DATAORDER2__;    // TOTAL SIZE OF RETURN STACK

// DATA STACK

WORDPTR *DStk __DATAORDER2__;   // BASE OF DATA STACK
WORDPTR *DSTop __DATAORDER2__; // TOP OF THE DATA STACK
BINT DStkSize __DATAORDER2__;    // TOTAL SIZE OF DATA STACK
WORDPTR *DStkBottom __DATAORDER2__; // BASE OF THE CURRENT STACK
WORDPTR *DStkProtect __DATAORDER2__; // BASE OF THE PROTECTED STACK

// DIRECTORIES

WORDPTR *Directories __DATAORDER2__;   // BASE OF DIRECTORY STORAGE
WORDPTR *DirsTop __DATAORDER2__;    // POINTER TO END OF USED DIRECTORIES
BINT DirSize __DATAORDER2__;
WORDPTR *CurrentDir __DATAORDER2__;    // POINTER TO CURRENT DIRECTORY

// LAMS

WORDPTR *LAMs __DATAORDER2__;          // BASE OF LOCAL VARIABLES STORAGE
WORDPTR *LAMTop __DATAORDER2__; // TOP OF THE LAM STACK
BINT LAMSize __DATAORDER2__;
WORDPTR *nLAMBase __DATAORDER2__;  // START OF THE LAST LAM ENVIRONMENT
WORDPTR *LAMTopSaved __DATAORDER2__;   // SAVED VALUE OF LAMTOP USED DURING COMPILATION


// TWO MORE SECTIONS AVAILABLE FOR FUTURE USE: DATAORDER3 AND DATAORDERLAST


// *************************************************************
// ** END OF VARIABLES THAT NEED TO BE AT PERSISTENT LOCATION
// ** ACROSS FIRMWARE UPDATES / WARMSTART / RESET
// *************************************************************


// ALL OTHER VARIABLES CAN CHANGE LOCATION, AS THEY ARE REINITIALIZED DURING WARMSTART

// LIBRARIES
LIBHANDLER LowLibRegistry[MAXLOWLIBS];
LIBHANDLER SysHiLibRegistry[MAXSYSHILIBS];
LIBHANDLER HiLibRegistry[MAXHILIBS];
BINT HiLibNumbers[MAXHILIBS];
BINT NumHiLibs;



// OTHER RPL CORE VARIABLES THAT ARE NOT AFFECTED BY GC
WORD CurOpcode; // CURRENT OPCODE (WORD)
WORD Exceptions, TrappedExceptions;  // FLAGS FOR CURRENT EXCEPTIONS
WORDPTR *ValidateTop; // TEMPORARY DATA AFTER THE RETURN STACK USED DURING COMPILATION
WORDPTR *ErrornLAMBase;  // SAVED BASE OF LAM ENVIRONMENT AT ERROR HANDLER
WORDPTR *ErrorLAMTop;   // SAVED VALUE OF LAMTOP AT ERROR HANDLER
WORDPTR *ErrorRSTop;     // SAVED TOP OF RETURN STACK AT ERROR HANDLER


// MULTIPRECISION LIBRARY CONTEXT
mpd_context_t Context;


// ARGUMENTS TO PASS TO LIBRARY HANDLERS
// DURING COMPILATION
UBINT ArgNum1,ArgNum2,ArgNum3,RetNum;

// TEMPORARY SCRATCH MEMORY FOR DIGITS
mpd_uint_t RDigits[REAL_SCRATCHMEM] __SCRATCH_MEMORY__;
// PREALLOCATED STATIC REAL NUMBER REGISTERS FOR TEMPORARY STORAGE
mpd_t RReg[REAL_REGISTERS] __SCRATCH_MEMORY__;


BINT BINT2RealIdx;
