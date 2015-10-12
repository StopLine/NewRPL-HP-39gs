/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// STANDARD COMPILER FOR COMMAND TOKENS
// COMMON TO ALL LIBRARIES THAT DEFINE ONLY COMMANDS
// STARTING TO COUNT FROM COMMAND NUMBER 0
void libCompileCmds(BINT libnum,char *libnames[],WORD libopcodes[],int numcmds)
{
    int idx;
    int len;
    for(idx=0;idx<numcmds;++idx)
    {
        len=utf8len((char *)libnames[idx]);
        if((len!=0) && (len==(BINT)TokenLen) && (!utf8ncmp((char *)TokenStart,(char *)libnames[idx],len)))
       {
            if(libopcodes) rplCompileAppend((WORD) MKOPCODE(libnum,libopcodes[idx]));
           else rplCompileAppend((WORD) MKOPCODE(libnum,idx));
           RetNum=OK_CONTINUE;
           return;
       }
    }
    RetNum=ERR_NOTMINE;
}

// STANDARD DECOMPILER FOR COMMAND TOKENS
// COMMON TO ALL LIBRARIES THAT DEFINE ONLY COMMANDS

void libDecompileCmds(char *libnames[],WORD libopcodes[],int numcmds)
{
WORD opc=OPCODE(*DecompileObject);
int idx;

if(libopcodes) {
    for(idx=0;idx<numcmds;++idx)
    {
        if(libopcodes[idx]==opc) break;
    }
} else idx=opc;
if(idx>=numcmds) {
    RetNum=ERR_INVALID;
    return;
}

rplDecompAppendString((BYTEPTR)libnames[idx]);
RetNum=OK_CONTINUE;
}



// STANDARD PROBETOKEN FOR COMMANDS
// COMMON TO ALL LIBRARIES THAT DEFINE ONLY COMMANDS
// STARTING TO COUNT FROM COMMAND NUMBER 0
void libProbeCmds(char *libnames[],BINT tokeninfo[],int numcmds)
{
    int idx;
    int len;
    int maxidx=-1,maxlen=0;

    // SCAN THROUGH ALL COMMANDS AND FIND LONGEST MATCH
    for(idx=0;idx<numcmds;++idx)
    {
        len=utf8len((char *)libnames[idx]);
        if((len>0) && (len<=(BINT)TokenLen) && (!utf8ncmp((char *)TokenStart,(char *)libnames[idx],len)))
        {
            // WE HAVE A MATCH, STORE THE INDEX BEFORE WE MAKE ANY DECISIONS
            if(len>maxlen) { maxidx=idx; maxlen=len; }
       }
    }

    if(maxlen!=0) {
    if(tokeninfo) {
        RetNum=OK_TOKENINFO | tokeninfo[maxidx];
    } else RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_NOTALLOWED,0,0);
    }
    else RetNum=ERR_NOTMINE;
}


// STANDARD GETINFO FOR COMMANDS
// COMMON TO ALL LIBRARIES THAT DEFINE ONLY COMMANDS
// STARTING TO COUNT FROM COMMAND NUMBER 0
// THIS VERSION TAKES A VECTOR WITH OPCODE NUMBERS
void libGetInfo(WORD opcode,char *libnames[],WORD libopcodes[],BINT tokeninfo[],int numcmds)
    {
        int idx;
        int len;
        opcode=OPCODE(opcode);
        for(idx=0;idx<numcmds;++idx)
        {
            if(libopcodes[idx]==opcode)
           {
                if(tokeninfo) {
                    RetNum=OK_TOKENINFO | tokeninfo[idx];
                } else {
                    len=utf8len(libnames[idx]);
                    RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_NOTALLOWED,0,0);
                }
               return;
           }
        }
        RetNum=ERR_NOTMINE;
    }

// STANDARD GETINFO FOR COMMANDS
// COMMON TO ALL LIBRARIES THAT DEFINE ONLY COMMANDS
// THIS VERSION ASSUMES THE INDEX IS THE OPCODE NUMBER
void libGetInfo2(WORD opcode,char *libnames[],BINT tokeninfo[],int numcmds)
    {
        int idx;
        int len;
        idx=OPCODE(opcode);
        if(idx<numcmds)
        {
                if(tokeninfo) {
                    RetNum=OK_TOKENINFO | tokeninfo[idx];
                } else {
                    len=utf8len(libnames[idx]);
                    RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_NOTALLOWED,0,0);
                }
               return;

        }
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_NOTALLOWED,0,0);
    }


void libGetRomptrID(BINT libnum,WORDPTR *table,WORDPTR ptr)
{
    BINT idx=0;
    while(table[idx]) {
        if( (ptr>=table[idx]) && (ptr<table[idx]+rplObjSize(table[idx]))) {
            BINT offset=ptr-table[idx];
            ObjectID=MKROMPTRID(libnum,idx,offset);
            RetNum=OK_CONTINUE;
            return;
        }
        ++idx;
    }
    RetNum=ERR_NOTMINE;
    return;
}

void libGetPTRFromID(WORDPTR *table,WORD id)
{
    BINT idx;
    while(table[idx]) ++idx;
    if(ROMPTRID_IDX(id)>=idx) {
        RetNum=ERR_NOTMINE;
        return;
    }
    ObjectPTR=table[ROMPTRID_IDX(id)]+ROMPTRID_OFF(id);
    RetNum=OK_CONTINUE;
}


// STANDARD AUTOCOMPLETE FOR COMMANDS
// COMMON TO ALL LIBRARIES THAT DEFINE COMMANDS
// STARTING TO COUNT FROM COMMAND NUMBER 0
void libAutoCompleteNext(BINT libnum,char *libnames[],int numcmds)
{
    // TokenStart = token string
    // TokenLen = token length
    // SuggestedOpcode = OPCODE OF THE CURRENT SUGGESTION, OR 0 IF SUGGESTION IS AN OBJECT
    // SuggestedObject = POINTER TO AN OBJECT (ONLY VALID IF SuggestedOpcode==0)

    WORD Prolog=SuggestedOpcode;

    if(LIBNUM(Prolog)<libnum) {
        // COMMANDS ARE SUGGESTED BEFORE ANY OBJECTS
        // SO IF THE PREVIOUS RESULT WAS AN OBJECT, WE ARE DONE HERE
        RetNum=ERR_NOTMINE;
        return;
    }
    BINT idx,len;

    if(LIBNUM(Prolog)==libnum) idx=OPCODE(Prolog)-1;
    else idx=numcmds-1;

    while(idx>=0) {
        len=utf8len((char *)libnames[idx]);
        if((len>=(BINT)TokenLen) && (!utf8ncmp((char *)TokenStart,(char *)libnames[idx],TokenLen)))
        {
            // WE HAVE THE NEXT MATCH
            SuggestedOpcode=MKOPCODE(libnum,idx);
            RetNum=OK_CONTINUE;
            return;
        }
        --idx;
    }

    RetNum=ERR_NOTMINE;
}

void libAutoCompletePrev(BINT libnum,char *libnames[],int numcmds)
{
    // TokenStart = token string
    // TokenLen = token length
    // SuggestedOpcode = OPCODE OF THE CURRENT SUGGESTION, OR 0 IF SUGGESTION IS AN OBJECT
    // SuggestedObject = POINTER TO AN OBJECT (ONLY VALID IF SuggestedOpcode==0)

    WORD Prolog=SuggestedOpcode;

    if(!Prolog) Prolog=*SuggestedObject;

    if(LIBNUM(Prolog)>libnum) {
        // ALREADY PAST HERE
        RetNum=ERR_NOTMINE;
        return;
    }

    BINT idx,len;

    if(LIBNUM(Prolog)==libnum) {
        if(ISPROLOG(Prolog)) idx=0;
        else idx=OPCODE(Prolog)+1;
    }
    else idx=0;

    while(idx<numcmds) {
        len=utf8len((char *)libnames[idx]);
        if((len>=(BINT)TokenLen) && (!utf8ncmp((char *)TokenStart,(char *)libnames[idx],TokenLen)))
        {
            // WE HAVE THE NEXT MATCH
            SuggestedOpcode=MKOPCODE(libnum,idx);
            RetNum=OK_CONTINUE;
            return;
        }
        ++idx;
    }

    RetNum=ERR_NOTMINE;
}
