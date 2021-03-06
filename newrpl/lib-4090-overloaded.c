/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// ******************************
// *** SPECIAL LIBRARY HEADER ***
// ******************************



// REPLACE THE NUMBER
#define LIBRARY_NUMBER  4090


// LIST OF OPERATORS EXPORTED,
// SIMILAR TO THE GENERAL LIBRARY HEADER, BUT FOR OPERATORS ONLY

#define OP_LIST \
    OP(INV,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    OP(NEG,MKTOKENINFO(1,TITYPE_FUNCTION,1,4)), \
    OP(EVAL1,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    OP(EVAL,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    OP(FUNCEVAL,MKTOKENINFO(8,TITYPE_FUNCTION,15,2)), \
    OP(XEQ,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    OP(NUM,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    OP(ABS,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    OP(ISTRUE,MKTOKENINFO(6,TITYPE_FUNCTION,1,2)), \
    OP(NOT,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    OP(EQ,MKTOKENINFO(2,TITYPE_BINARYOP_LEFT,2,10)), \
    OP(NOTEQ,MKTOKENINFO(2,TITYPE_BINARYOP_LEFT,2,10)), \
    OP(LT,MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,9)), \
    OP(GT,MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,9)), \
    OP(LTE,MKTOKENINFO(2,TITYPE_BINARYOP_LEFT,2,9)), \
    OP(GTE,MKTOKENINFO(2,TITYPE_BINARYOP_LEFT,2,9)), \
    OP(SAME,MKTOKENINFO(1,TITYPE_FUNCTION,2,2)), \
    OP(AND,MKTOKENINFO(3,TITYPE_BINARYOP_LEFT,2,11)), \
    OP(OR,MKTOKENINFO(2,TITYPE_BINARYOP_LEFT,2,13)), \
    OP(XOR,MKTOKENINFO(3,TITYPE_BINARYOP_LEFT,2,12)), \
    OP(CMP,MKTOKENINFO(3,TITYPE_FUNCTION,2,9)), \
    OP(ADD,MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,8)), \
    OP(SUB,MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,7)), \
    OP(MUL,MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,6)), \
    OP(DIV,MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,5)), \
    OP(POW,MKTOKENINFO(1,TITYPE_BINARYOP_RIGHT,2,3)), \
    OP(XROOT,MKTOKENINFO(1,TITYPE_FUNCTION,2,2))

    // ADD MORE OPERATORS HERE


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifdef COMMANDS_ONLY_PASS

// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
// THIS IS ONLY FOR THIS LIBRARY
#define LIBRARY_NUMBER 4090

#define OP(a,b) CMD_OVR_##a = MKOPCODE(LIBRARY_NUMBER,OVR_##a)
enum  { OP_LIST , OP(UPLUS,xx), OP(UMINUS,xx) };
#undef OP

// CLEANUP FOR OTHER LIBRARIES TO DEFINE THEIR OWN

#undef LIBRARY_NUMBER


#else

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************


// SPECIAL DECLARATIONS FOR THIS LIBRARY
#define OP(a,b) UNUSED_ENUM_##a
enum { OP_LIST , LIB_NUMBEROFCMDS };
#undef OP



#define OP(a,b) OVRT_ ## a
const char * const LIB_NAMES[]=
{
        OP_LIST
};
#undef OP

#define OP(a,b) OVR_ ## a
const WORD const LIB_OPCODES[]=
{
        OP_LIST
};
#undef OP
#define OP(a,b) b
const BINT const LIB_TOKENINFO[]=
{
        OP_LIST
};
#undef OP

// EXECUTE AN OPERATOR, NOT NECESSARILY AN OVERLOADED ONE
// NO ARGUMENT CHECK! (TO BE DONE BY OPERATOR BEING CALLED)

void rplCallOperator(WORD op)
{
    int libnum=LIBNUM(op);
    if(libnum!=LIBRARY_NUMBER) {
    LIBHANDLER han=rplGetLibHandler(libnum);
    if(han) {
        // EXECUTE THE OTHER LIBRARY DIRECTLY
        BINT SavedOpcode=CurOpcode;
        CurOpcode=op;
        (*han)();
        if(CurOpcode==op) CurOpcode=SavedOpcode;
    }
    else {
        rplError(ERR_MISSINGLIBRARY);
    }
    return;
    }
    rplCallOvrOperator(op);
}



// PERFORM AN INTERNAL CALL TO AN OVERLOADED OPERATOR
// op IS ONE OF THE OVR_XXX OPERATOR CONSTANTS

void rplCallOvrOperator(WORD op)
{
    int nargs=OVR_GETNARGS(op);
    int libnum=0;
    WORDPTR ptr;
    if(nargs>rplDepthData()) {
        rplError(ERR_BADARGCOUNT);
        return;
    }
    while(nargs) {
        // CHECK EACH ARGUMENT
        ptr=rplPeekData(nargs);
        if(LIBNUM(*ptr)>(WORD)libnum) libnum=LIBNUM(*ptr);
        if((LIBNUM(*ptr)==LIBRARY_NUMBER)) {
            // ARGUMENT OF THE OPERATOR IS AN OVERLOADED OPERATOR
            // SO WE NEED TO PROCESS THIS LOCALLY TO AVOID RECURSIVE CALL
            // RESPOND TO OVERLOADED OPERATORS LIKE A LIBRARY HANDLER
            switch(OPCODE(op))
            {
            case OVR_EVAL:
            case OVR_EVAL1:
            case OVR_XEQ:
            // IF IT'S AN OPCODE, EXECUTE IT
            rplCallOvrOperator(*rplPopData());
            return;
            default:
                rplError(ERR_INVALIDOPCODE);
                return;


            }



        }
        --nargs;
    }
    LIBHANDLER han=rplGetLibHandler(libnum);
    if(han) {
        // EXECUTE THE OTHER LIBRARY DIRECTLY
        BINT SavedOpcode=CurOpcode;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,op);
        (*han)();
        if(CurOpcode==MKOPCODE(LIBRARY_NUMBER,op)) CurOpcode=SavedOpcode;
    }
    else {
        // THE LIBRARY DOESN'T EXIST BUT THE OBJECT DOES?
        // THIS CAN ONLY HAPPEN IF TRYING TO EXECUTE WITH A CUSTOM OBJECT
        // WHOSE LIBRARY WAS UNINSTALLED AFTER BEING COMPILED (IT'S AN INVALID OBJECT)
        rplError(ERR_MISSINGLIBRARY);
    }

}





void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE A TYPE, SO THIS IS AN INVALID OPCODE
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    if(OPCODE(CurOpcode)>=MIN_RESERVED_OPCODE) {


    switch(OPCODE(CurOpcode))
    {

    // STANDARIZED OPCODES:
    // --------------------
    // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES


    case OPCODE_COMPILE:

        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,(WORDPTR)LIB_OPCODES,LIB_NUMBEROFCMDS);
     return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:

        // MANUALLY DECOMPILE UNARY PLUS AND UNARY MINUS IN SYMBOLICS
        if(OPCODE(*DecompileObject)==OVR_UMINUS) { rplDecompAppendChar('-'); RetNum=OK_CONTINUE; return; }
        if(OPCODE(*DecompileObject)==OVR_UPLUS) { rplDecompAppendChar('+'); RetNum=OK_CONTINUE; return; }


        libDecompileCmds((char **)LIB_NAMES,(WORDPTR)LIB_OPCODES,LIB_NUMBEROFCMDS);
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
        libProbeCmds((char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_GETINFO:
        // MANUALLY RETURN INFO FOR UNARY PLUS AND MINUS
        if(OPCODE(*DecompileObject)==OVR_UMINUS) { RetNum=OK_TOKENINFO | MKTOKENINFO(1,TITYPE_PREFIXOP,1,4); return; }
        if(OPCODE(*DecompileObject)==OVR_UPLUS) { RetNum=OK_TOKENINFO | MKTOKENINFO(1,TITYPE_PREFIXOP,1,4); return; }


        libGetInfo(*DecompileObject,(char **)LIB_NAMES,(WORDPTR)LIB_OPCODES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
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
    {
        BINT idx;
        do {
        if(LIBNUM(SuggestedOpcode)==LIBRARY_NUMBER) {
            // CONVERT OPCODE INTO INDEX
            BINT k;

            for(k=0;k<LIB_NUMBEROFCMDS;++k)
            {
                if(LIB_OPCODES[k]==OPCODE(SuggestedOpcode)) break;
            }
            if(k==LIB_NUMBEROFCMDS) { RetNum=ERR_NOTMINE; return; }
            SuggestedOpcode=MKOPCODE(LIBRARY_NUMBER,k);
        }

        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        // DO SOME POST-PROCESSING DUE TO DIFFERENT HANDLING OF DATA IN THIS LIBRARY
        if(RetNum==OK_CONTINUE) {
            idx=OPCODE(SuggestedOpcode);
            SuggestedOpcode=MKOPCODE(LIBRARY_NUMBER,LIB_OPCODES[idx]);
        } else idx=0;
        } while(idx==4);    // NEVER OFFER 'FUNCEVAL' FOR AUTOCOMPLETE

        return;
    }

    case OPCODE_LIBINSTALL:
        LibraryList=(WORDPTR)libnumberlist;
        RetNum=OK_CONTINUE;
        return;
    case OPCODE_LIBREMOVE:
        return;


    }

    RetNum=ERR_NOTMINE;
    return;
    }

    if(OPCODE(CurOpcode)<MIN_OVERLOAD_OPCODE) {
        rplError(ERR_INVALIDOPCODE);
        return;
    }


    rplCallOvrOperator(OPCODE(CurOpcode));

}


#endif


