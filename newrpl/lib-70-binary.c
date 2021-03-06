/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************



// REPLACE THE NUMBER
#define LIBRARY_NUMBER  70


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(STWS,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(RCWS,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(BOR,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(BAND,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(BXOR,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(BLSL,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(BLSR,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(BASR,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(BRL,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(BRR,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(BNOT,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(BADD,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(BSUB,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(BMUL,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(BDIV,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************



void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    switch(OPCODE(CurOpcode))
    {

    case STWS:
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            BINT64 wsize=rplReadNumberAsBINT(rplPeekData(1));

            if(wsize<1) wsize=1;
            if(wsize>63) wsize=63;

            if(!ISLIST(*SystemFlags)) {
                // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
                rplError(ERR_SYSTEMFLAGSINVALID);
                return;
            }
            //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
            WORDPTR low64=SystemFlags+2;
            //WORDPTR hi64=SystemFlags+5;

            low64[0]=(low64[0]&~(0x3f<<4))| (wsize<<4);

            rplDropData(1);
            return;
            }

            // USER FLAGS NOT SUPPORTED FOR NOW
            rplError(ERR_INTEGEREXPECTED);
            return;


    case RCWS:
    {
        if(!ISLIST(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);
            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64=SystemFlags+2;
        //WORDPTR hi64=SystemFlags+5;

        BINT wsize=(low64[0]>>4)&0x3f;

        rplNewBINTPush(wsize,DECBINT);

        return;
    }

    case BOR:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        BINT64 num1,num2;
        BINT base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1=rplReadNumberAsBINT(rplPeekData(2));
            if(Exceptions) return;
            base=LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISLIST(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64=SystemFlags+2;
        BINT wsize=(low64[0]>>4)&0x3f;

        num1|=num2;

        if(num1&(1LL<<wsize)) {
            // SIGN EXTEND THE RESULT
            num1|=~((1LL<<wsize)-1);
        }
        else num1&=((1LL<<wsize)-1);

        rplDropData(2);
        rplNewBINTPush(num1,base);
        return;
    }

    case BAND:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        BINT64 num1,num2;
        BINT base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1=rplReadNumberAsBINT(rplPeekData(2));
            if(Exceptions) return;
            base=LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISLIST(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64=SystemFlags+2;
        BINT wsize=(low64[0]>>4)&0x3f;

        num1&=num2;

        if(num1&(1LL<<wsize)) {
            // SIGN EXTEND THE RESULT
            num1|=~((1LL<<wsize)-1);
        }
        else num1&=((1LL<<wsize)-1);

        rplDropData(2);
        rplNewBINTPush(num1,base);
        return;
    }

    case BXOR:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        BINT64 num1,num2;
        BINT base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1=rplReadNumberAsBINT(rplPeekData(2));
            if(Exceptions) return;
            base=LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISLIST(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64=SystemFlags+2;
        BINT wsize=(low64[0]>>4)&0x3f;

        num1^=num2;

        if(num1&(1LL<<wsize)) {
            // SIGN EXTEND THE RESULT
            num1|=~((1LL<<wsize)-1);
        }
        else num1&=((1LL<<wsize)-1);

        rplDropData(2);
        rplNewBINTPush(num1,base);
        return;
    }

    case BLSL:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        BINT64 num1,num2;
        BINT base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1=rplReadNumberAsBINT(rplPeekData(2));
            if(Exceptions) return;
            base=LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISLIST(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64=SystemFlags+2;
        BINT wsize=(low64[0]>>4)&0x3f;

        if(num2>0) num1<<=num2;
        else num1>>=-num2;

        if(num1&(1LL<<wsize)) {
            // SIGN EXTEND THE RESULT
            num1|=~((1LL<<wsize)-1);
        }
        else num1&=((1LL<<wsize)-1);

        rplDropData(2);
        rplNewBINTPush(num1,base);
        return;
    }

    case BLSR:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        BINT64 num1,num2;
        BINT base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1=rplReadNumberAsBINT(rplPeekData(2));
            if(Exceptions) return;
            base=LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISLIST(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);
            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64=SystemFlags+2;
        BINT wsize=(low64[0]>>4)&0x3f;

        num1&=((1LL<<(wsize+1))-1);

        if(num2>0) num1>>=num2;
        else num1<<=-num2;

        if(num1&(1LL<<wsize)) {
            // SIGN EXTEND THE RESULT
            num1|=~((1LL<<wsize)-1);
        }
        else num1&=((1LL<<wsize)-1);

        rplDropData(2);
        rplNewBINTPush(num1,base);
        return;
    }

    case BASR:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        BINT64 num1,num2;
        BINT base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1=rplReadNumberAsBINT(rplPeekData(2));
            if(Exceptions) return;
            base=LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISLIST(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64=SystemFlags+2;
        BINT wsize=(low64[0]>>4)&0x3f;

        if(num2>0) num1>>=num2;
        else num1<<=-num2;

        if(num1&(1LL<<(wsize-num2))) {
            // SIGN EXTEND THE RESULT
            num1|=~((1LL<<(wsize-num2))-1);
        }
        else num1&=((1LL<<wsize)-1);

        rplDropData(2);
        rplNewBINTPush(num1,base);
        return;
    }


    case BADD:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        BINT64 num1,num2;
        BINT base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1=rplReadNumberAsBINT(rplPeekData(2));
            if(Exceptions) return;
            base=LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISLIST(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64=SystemFlags+2;
        BINT wsize=(low64[0]>>4)&0x3f;

        num1+=num2;

        if(num1&(1LL<<wsize)) {
            // SIGN EXTEND THE RESULT
            num1|=~((1LL<<wsize)-1);
        }
        else num1&=((1LL<<wsize)-1);

        rplDropData(2);
        rplNewBINTPush(num1,base);
        return;
    }

    case BSUB:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        BINT64 num1,num2;
        BINT base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1=rplReadNumberAsBINT(rplPeekData(2));
            if(Exceptions) return;
            base=LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISLIST(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64=SystemFlags+2;
        BINT wsize=(low64[0]>>4)&0x3f;

        num1-=num2;

        if(num1&(1LL<<wsize)) {
            // SIGN EXTEND THE RESULT
            num1|=~((1LL<<wsize)-1);
        }
        else num1&=((1LL<<wsize)-1);

        rplDropData(2);
        rplNewBINTPush(num1,base);
        return;
    }

    case BMUL:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        BINT64 num1,num2;
        BINT base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1=rplReadNumberAsBINT(rplPeekData(2));
            if(Exceptions) return;
            base=LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISLIST(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64=SystemFlags+2;
        BINT wsize=(low64[0]>>4)&0x3f;

        num1*=num2;

        if(num1&(1LL<<wsize)) {
            // SIGN EXTEND THE RESULT
            num1|=~((1LL<<wsize)-1);
        }
        else num1&=((1LL<<wsize)-1);

        rplDropData(2);
        rplNewBINTPush(num1,base);
        return;
    }

    case BDIV:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        BINT64 num1,num2;
        BINT base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1=rplReadNumberAsBINT(rplPeekData(2));
            if(Exceptions) return;
            base=LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISLIST(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64=SystemFlags+2;
        BINT wsize=(low64[0]>>4)&0x3f;

        num1/=num2;

        if(num1&(1LL<<wsize)) {
            // SIGN EXTEND THE RESULT
            num1|=~((1LL<<wsize)-1);
        }
        else num1&=((1LL<<wsize)-1);

        rplDropData(2);
        rplNewBINTPush(num1,base);
        return;
    }


    case BNOT:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        BINT64 num1;
        BINT base;
        if(ISNUMBER(*rplPeekData(1))) {
            num1=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;
            base=LIBNUM(*rplPeekData(1));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISLIST(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64=SystemFlags+2;
        BINT wsize=(low64[0]>>4)&0x3f;

        num1=~num1;

        if(num1&(1LL<<wsize)) {
            // SIGN EXTEND THE RESULT
            num1|=~((1LL<<wsize)-1);
        }
        else num1&=((1LL<<wsize)-1);

        rplDropData(1);
        rplNewBINTPush(num1,base);
        return;
    }


    // ADD MORE OPCODES HERE

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


        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

     return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds((char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
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


    case OPCODE_PROBETOKEN:
        // PROBETOKEN FINDS A VALID WORD AT THE BEGINNING OF THE GIVEN TOKEN AND RETURNS
        // INFORMATION ABOUT IT. THIS OPCODE IS MANDATORY

        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND
        {
        libProbeCmds((char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);

        return;
        }


    case OPCODE_GETINFO:
        libGetInfo2(*DecompileObject,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
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

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
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
    rplError(ERR_INVALIDOPCODE);

    return;


}


#endif





