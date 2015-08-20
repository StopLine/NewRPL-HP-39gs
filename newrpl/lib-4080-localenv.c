/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  4080
#define LIB_ENUM lib4080enum
#define LIB_NAMES lib4080_names
#define LIB_HANDLER lib4080_handler
#define LIB_NUMBEROFCMDS LIB4080_NUMBEROFCMDS

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
#define CMD_EXTRANAME \
    "→"
#define CMD_EXTRAENUM \
    NEWLOCALENV


// INTERNAL DECLARATIONS

// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM {  CMD_EXTRAENUM , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
const char * const LIB_NAMES[]= {  CMD_EXTRANAME  };
#undef CMD


#define NEWNLOCALS 0x40000   // SPECIAL OPCODE TO CREATE NEW LOCAL VARIABLES

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
        return;
    }

    switch(OPCODE(CurOpcode))
    {

    // STANDARIZED OPCODES:
    // --------------------
    // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES


    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors

        // CHECK IF THE TOKEN IS THE OBJECT DOCOL
        // BUT ONLY IF WE ARE WITHIN A NEWLOCALENV CONSTRUCT

       if((TokenLen==1) && (!utf8ncmp((char *)TokenStart,"«",1)))
       {
           if(CurrentConstruct!=MKOPCODE(LIBRARY_NUMBER,NEWLOCALENV)) {
               RetNum=ERR_NOTMINE;
               return;
           }


           // COUNT HOW MANY LAMS ARE IN THE CONSTRUCT
           ScratchPointer1=*(ValidateTop-1);
           BINT lamcount=0;

           // INITIALIZE AN ENVIRONMENT FOR COMPILE TIME
           rplCreateLAMEnvironment(ScratchPointer1);
           ++ScratchPointer1;  // SKIP THE START OF CONSTRUCT WORD
           while(ScratchPointer1<CompileEnd) {
               rplCreateLAM(ScratchPointer1,ScratchPointer1);   // CREATE ALL THE LAMS FOR FUTURE COMPILATION
               ScratchPointer1=rplSkipOb(ScratchPointer1);
               ++lamcount;
           }
           // NOW REPLACE THE -> WORD FOR A STANDARD <<

           ScratchPointer1=*(ValidateTop-1);
           *ScratchPointer1=MKPROLOG(SECO,0);  // STANDARD SECONDARY PROLOG SO ALL LAMS ARE CREATED INSIDE OF IT
           CurrentConstruct=MKPROLOG(SECO,0);
           rplCompileAppend((WORD) MKOPCODE(DOIDENT,NEWNLOCALS+lamcount));   // OPCODE TO CREATE ALL THESE LAMS
           RetNum=OK_CONTINUE;
           return;
       }

       // CHECK IF THE TOKEN IS THE NEW LOCAL

       if((TokenLen==1) && (!utf8ncmp((char *)TokenStart,"→",1)))
       {
           rplCompileAppend(CMD_XEQSECO);   // EVAL THE NEXT SECO IN THE RUNSTREAM
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,NEWLOCALENV));  // PUT A MARKER
           RetNum=OK_STARTCONSTRUCT;
           return;
       }


       RetNum=ERR_NOTMINE;
        return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        // THIS LIBRARY DOES NOT GENERATE ANY OPCODES!
        RetNum=ERR_INVALID;
        return;
    case OPCODE_VALIDATE:
        // VALIDATE RECEIVES OPCODES COMPILED BY OTHER LIBRARIES, TO BE INCLUDED WITHIN A COMPOSITE OWNED BY
        // THIS LIBRARY. EVERY COMPOSITE HAS TO EVALUATE IF THE OBJECT BEING COMPILED IS ALLOWED INSIDE THIS
        // COMPOSITE OR NOT. FOR EXAMPLE, A REAL MATRIX SHOULD ONLY ALLOW REAL NUMBERS INSIDE, ANY OTHER
        // OPCODES SHOULD BE REJECTED AND AN ERROR THROWN.
        // Library receives:
        // CurrentConstruct = SET TO THE CURRENT ACTIVE CONSTRUCT TYPE
        // LastCompiledObject = POINTER TO THE LAST OBJECT THAT WAS COMPILED, THAT NEEDS TO BE VERIFIED

        // VALIDATE RETURNS:
        // RetNum =  OK_CONTINUE IF THE OBJECT IS ACCEPTED, ERR_INVALID IF NOT.


        RetNum=OK_CONTINUE;
        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        RetNum=ERR_NOTMINE;
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        RetNum=ERR_NOTMINE;
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID

        if(ISPROLOG(*ObjectPTR)) { RetNum=ERR_INVALID; return; }
        RetNum=OK_CONTINUE;
        return;



    case OPCODE_LIBINSTALL:
        LibraryList=(WORDPTR)libnumberlist;
        RetNum=OK_CONTINUE;
        return;
    case OPCODE_LIBREMOVE:
        return;

    }

    // UNHANDLED OPCODE...

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode)>=MIN_RESERVED_OPCODE) {
        RetNum=ERR_NOTMINE;
        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;

}






