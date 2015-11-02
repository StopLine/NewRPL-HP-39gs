/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// LIBRARY 20 DEFINES THE COMMENTS OBJECTS
// AND ASSOCIATED FUNCTIONS


#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  20
#define LIB_ENUM lib20enum
#define LIB_NAMES lib20_names
#define LIB_HANDLER lib20_handler
#define LIB_NUMBEROFCMDS LIB20_NUMBEROFCMDS
#define ROMPTR_TABLE    romptr_table20

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,LIBRARY_NUMBER+1,LIBRARY_NUMBER+2,LIBRARY_NUMBER+3,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(STRIPCOMMENTS)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
//#define CMD_EXTRANAME \
//    "→STR", \

//#define CMD_EXTRAENUM \
//    TOSTR, \



// INTERNAL DECLARATIONS



// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST , /*CMD_EXTRAENUM , */ LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
const char * const LIB_NAMES[]= { CMD_LIST /*, CMD_EXTRANAME */  };
#undef CMD




/*
// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)empty_string,
    0
};

*/



// FIX THE PROLOG OF A STRING TO MATCH THE DESIRED LENGTH IN CHARACTERS
// LOW-LEVEL FUNCTION, DOES NOT ACTUALLY RESIZE THE OBJECT
void rplSetCommentLength(WORDPTR string,BINT length)
{
    BINT padding=(4-((length)&3))&3;

    *string=MKPROLOG(LIBRARY_NUMBER+padding,(length+3)>>2);
}



void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE

        // DO ABSOLUTELY NOTHING (IT'S JUST A COMMENT)

        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case STRIPCOMMENTS:
    {
        // TODO: IMPLEMENT THIS

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

        if(*((BYTEPTR)TokenStart)=='@') {
            // START A STRING

            ScratchPointer4=CompileEnd;     // SAVE CURRENT COMPILER POINTER TO FIX THE OBJECT AT THE END

            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER,0));

            union {
                WORD word;
                BYTE bytes[4];
            } temp;

            BINT count=0,mode=1,endmark=0;
            BYTEPTR ptr=(BYTEPTR) TokenStart;
            ++ptr;  // SKIP THE INITIAL AT

            // mode==1 MEANS JUST A STANDARD COMMENT
            // mode==2 MEANS @@ COMMENT THAT IS PERMANENT (SINGLE LINE)
            // mode==3 MEANS @@@ MULTILINE COMMENT

            do {
            while(count<4) {
                if(ptr==(BYTEPTR)NextTokenStart) {
                    // WE ARE AT THE END OF THE GIVEN STRING, STILL NO CLOSING QUOTE, SO WE NEED MORE

                    // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                    if(count) rplCompileAppend(temp.word);
                    *ScratchPointer4=MKPROLOG(LIBRARY_NUMBER+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                    RetNum=OK_NEEDMORE;
                    return;
                }

                if(*ptr=='@') {
                    if(mode<0x40000000) ++mode;
                    else {
                        ++endmark;
                    }
                } else mode|=0x40000000;

               if( ((*ptr=='\n')&&(mode<=0x40000002)) || ((mode&~0x40000000)==endmark))
                {
                    // END OF LINE = END OF COMMENT
                    temp.bytes[count]=*ptr;
                    ++count;
                    ++ptr;
                    // WE HAVE REACHED THE END OF THE COMMENT
                    if(count) {
                    rplCompileAppend(temp.word);
                    }
                    *ScratchPointer4=MKPROLOG(LIBRARY_NUMBER+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);

                    if(ptr<BlankStart) {
                    //   FOUND THE AT SYMBOL WITHIN THE COMMENT ITSELF, SPLIT THE TOKEN
                    TokenStart=ptr;
                    RetNum=OK_SPLITTOKEN;
                    } else RetNum=OK_CONTINUE;
                    return;
                }
                if(count==0) temp.word=0;
                temp.bytes[count]=*ptr;
                ++count;
                ++ptr;
                }
                //  WE HAVE A COMPLETE WORD HERE
                ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                rplCompileAppend(temp.word);
                ptr=(BYTEPTR)ScratchPointer1;

                count=0;


            } while(1);     // DANGEROUS! BUT WE WILL RETURN FROM THE CHECK WITHIN THE INNER LOOP;
            //  THIS IS UNREACHABLE CODE HERE

        }








        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;


    case OPCODE_COMPILECONT:
        // CONTINUE COMPILING STRING
    {
        union {
            WORD word;
            BYTE bytes[4];
        } temp;
        BINT mode=1,endmark=0;
        if(CompileEnd>ScratchPointer4+1) {
            // GET THE FIRST WORD TO EXTRACT THE COMMENT MODE
        temp.word=*(ScratchPointer4+1);

        if(temp.bytes[0]=='@') {
            ++mode;
            if(temp.bytes[1]=='@') ++mode;
        }
        }

        mode|=0x40000000;

        BINT count=(4-(LIBNUM(*ScratchPointer4)&3))&3; // GET NUMBER OF BYTES ALREADY WRITTEN IN LAST WORD

        if(count) {
            --CompileEnd;
            temp.word=*CompileEnd;  // GET LAST WORD
        } else temp.word=0;
        BYTEPTR ptr=(BYTEPTR) TokenStart;
        do {
        while(count<4) {
            if(ptr==(BYTEPTR)NextTokenStart) {
             // WE ARE AT THE END OF THE GIVEN STRING, STILL NO CLOSING QUOTE, SO WE NEED MORE

                // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                if(count) rplCompileAppend(temp.word);
                *ScratchPointer4=MKPROLOG(LIBRARY_NUMBER+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                RetNum=OK_NEEDMORE;
                return;
            }

            if(*ptr=='@') {
                if(mode<0x40000000) ++mode;
                else {
                    ++endmark;
                }
            } else mode|=0x40000000;

           if( ((*ptr=='\n')&&(mode<=0x40000002)) || ((mode&~0x40000000)==endmark))
            {
                // END OF COMMENT
                temp.bytes[count]=*ptr;
                ++count;

                ++ptr;
                // WE HAVE REACHED THE END OF THE STRING
                if(count) rplCompileAppend(temp.word);
                *ScratchPointer4=MKPROLOG(LIBRARY_NUMBER+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);

                if(ptr<BlankStart) {
                //   FOUND THE AT SYMBOL WITHIN THE COMMENT ITSELF, SPLIT THE TOKEN
                TokenStart=ptr;
                RetNum=OK_SPLITTOKEN;
                } else RetNum=OK_CONTINUE;
                return;
            }
            if(count==0) temp.word=0;
            temp.bytes[count]=*ptr;
            ++count;
            ++ptr;


            }
            //  WE HAVE A COMPLETE WORD HERE
            ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
            rplCompileAppend(temp.word);
            ptr=(BYTEPTR)ScratchPointer1;

            count=0;


        } while(1);     // DANGEROUS! BUT WE WILL RETURN FROM THE CHECK WITHIN THE INNER LOOP;
        //  THIS IS UNREACHABLE CODE HERE

    }
    case OPCODE_DECOMPEDIT:
    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors
        if(ISPROLOG(*DecompileObject)) {
            rplDecompAppendChar('@');
            rplDecompAppendString2((BYTEPTR)(DecompileObject+1),(OBJSIZE(*DecompileObject)<<2)-(LIBNUM(*DecompileObject)&3));

            RetNum=OK_CONTINUE;
            return;

        }







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

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        //libGetRomptrID(LIBRARY_NUMBER,(WORDPTR *)ROMPTR_TABLE,ObjectPTR);
        RetNum=ERR_NOTMINE;
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectPTR = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        //libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID);
        RetNum=ERR_NOTMINE;
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
    {
        // STRINGS ARE ALWAYS VALID, EVEN IF THEY CONTAIN INVALID UTF-8 SEQUENCES

        RetNum=OK_CONTINUE;
        return;
    }

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






