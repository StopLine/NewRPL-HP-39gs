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
#define LIBRARY_NUMBER  32


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(LSTO,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(LRCL,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ABND,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(NULLLAM,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(LAMEVALPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(LAMEVALPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(LAMEVALERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS    LIBRARY_NUMBER, \
                                    LIBRARY_NUMBER+1, \
                                    LIBRARY_NUMBER+2, \
                                    LIBRARY_NUMBER+3, \
                                    LIBRARY_NUMBER+4, \
                                    LIBRARY_NUMBER+5, \
                                    LIBRARY_NUMBER+6, \
                                    LIBRARY_NUMBER+7, \
                                    LIBRARY_NUMBER+8, \
                                    LIBRARY_NUMBER+9, \
                                    LIBRARY_NUMBER+10, \
                                    LIBRARY_NUMBER+11, \
                                    LIBRARY_NUMBER+12, \
                                    LIBRARY_NUMBER+13, \
                                    LIBRARY_NUMBER+14, \
                                    LIBRARY_NUMBER+15

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************






extern const WORD const symbeval_seco[];
extern const WORD const symbnum_seco[];

ROMOBJECT lameval_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,LAMEVALPRE),
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,LAMEVALPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,LAMEVALERR),     // ERROR HANDLER
    CMD_SEMI
};

ROMOBJECT lamnum_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,LAMEVALPRE),
    (CMD_OVR_NUM),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,LAMEVALPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,LAMEVALERR),     // ERROR HANDLER
    CMD_SEMI
};



// INTERNAL RPL PROGRAM THAT CALLS ABND
ROMOBJECT abnd_prog[]=
{
    (WORD)MKOPCODE(LIBRARY_NUMBER,ABND),  // JUST A WORD THAT WILL BE SKIPPED BY THE COMPILER
    (WORD)MKOPCODE(LIBRARY_NUMBER,ABND)   // THIS IS THE WORD THAT WILL BE EXECUTED
    // SEMI NOT NEEDED SINCE ABND ALREADY DOES SEMI
};

// INTERNAL SINT OBJECTS
ROMOBJECT lam_baseseco_bint[]=
{
    (WORD)LAM_BASESECO
};

// INTERNAL SINT OBJECTS
ROMOBJECT lam_errhandler_bint[]=
{
    (WORD)LAM_ERRHANDLER
};

// INTERNAL NULLLAM IDENT OBJECTS
ROMOBJECT nulllam_ident[]=
{
    (WORD)MKOPCODE(LIBRARY_NUMBER,NULLLAM)
};


//INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib32_menu);


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)nulllam_ident,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib32_menu,
    (WORDPTR)lameval_seco,
    (WORDPTR)abnd_prog,
    (WORDPTR)lam_baseseco_bint,
    (WORDPTR)lam_errhandler_bint,
    0
};



BINT64 powersof10[20];





void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        // NORMAL BEHAVIOR  ON A IDENT IS TO PUSH THE OBJECT ON THE STACK:
        rplPushData(IPtr);

        if(ISUNQUOTEDIDENT(CurOpcode)) {
            // UNQUOTED LAM, NEED TO ALSO DO XEQ ON ITS CONTENTS
            {
                WORDPTR val=rplGetLAM(rplPeekData(1));
                if(!val) {
                    val=rplGetGlobal(rplPeekData(1));
                    if(!val) {
                        // INEXISTENT IDENT EVALS TO ITSELF, SO RETURN DIRECTLY
                        return;
                    }
                }
                rplOverwriteData(1,val);    // REPLACE THE FIRST LEVEL WITH THE VALUE
                LIBHANDLER han=rplGetLibHandler(LIBNUM(*val));  // AND EVAL THE OBJECT
                if(han) {
                    WORD SavedOpcode=CurOpcode,TmpOpcode;
                    if(ISAPPROX(CurOpcode)) TmpOpcode=CurOpcode=(CMD_OVR_NUM);
                    else TmpOpcode=CurOpcode=(CMD_OVR_XEQ);
                    // EXECUTE THE OTHER LIBRARY DIRECTLY
                    (*han)();
                    // RESTORE THE PREVIOUS ONE ONLY IF THE HANDLER DID NOT CHANGE IT
                    if(CurOpcode==TmpOpcode) CurOpcode=SavedOpcode;
                }
                else {
                    // THE LIBRARY DOESN'T EXIST BUT THE OBJECT DOES?
                    // THIS CAN ONLY HAPPEN IF TRYING TO EXECUTE WITH A CUSTOM OBJECT
                    // WHOSE LIBRARY WAS UNINSTALLED AFTER BEING COMPILED (IT'S AN INVALID OBJECT)
                    rplError(ERR_MISSINGLIBRARY);
                }

            return;
            }
        }
        else return;
    }


    // PROCESS OVERLOADED OPERATORS FIRST
    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE) {

        if(ISUNARYOP(CurOpcode))
        {
        // APPLY UNARY OPERATOR DIRECTLY TO THE CONTENTS OF THE VARIABLE
            switch(OPCODE(CurOpcode))
            {
            case OVR_EVAL1:
            // RCL WHATEVER IS STORED IN THE LAM AND THEN XEQ ITS CONTENTS
            // NO ARGUMENT CHECKS! THAT SHOULD'VE BEEN DONE BY THE OVERLOADED "EVAL" DISPATCHER
            {
                if(!ISPROLOG(*rplPeekData(1))) {
                WORD saveOpcode=CurOpcode;
                CurOpcode=*rplPopData();
                // RECURSIVE CALL
                LIB_HANDLER();
                CurOpcode=saveOpcode;
                return;
                }

                WORDPTR val=rplGetLAM(rplPeekData(1));
                if(!val) {
                    val=rplGetGlobal(rplPeekData(1));
                    if(!val) {
                        // INEXISTENT IDENT EVALS TO ITSELF, SO RETURN DIRECTLY
                        return;
                    }
                }
                rplOverwriteData(1,val);    // REPLACE THE FIRST LEVEL WITH THE VALUE
                CurOpcode=(CMD_OVR_XEQ);
                LIBHANDLER han=rplGetLibHandler(LIBNUM(*val));  // AND EVAL THE OBJECT
                if(han) {
                    // EXECUTE THE OTHER LIBRARY DIRECTLY
                    (*han)();
                }
                else {
                    // THE LIBRARY DOESN'T EXIST BUT THE OBJECT DOES?
                    // THIS CAN ONLY HAPPEN IF TRYING TO EXECUTE WITH A CUSTOM OBJECT
                    // WHOSE LIBRARY WAS UNINSTALLED AFTER BEING COMPILED (IT'S AN INVALID OBJECT)
                    rplError(ERR_MISSINGLIBRARY);
                    CurOpcode=*IPtr;
                }


            }
                return;

            case OVR_EVAL:
            // RCL WHATEVER IS STORED IN THE LAM AND THEN EVAL ITS CONTENTS
            // NO ARGUMENT CHECKS! THAT SHOULD'VE BEEN DONE BY THE OVERLOADED "EVAL" DISPATCHER
            {
                if(!ISPROLOG(*rplPeekData(1))) {
                WORD saveOpcode=CurOpcode;
                CurOpcode=*rplPopData();
                // RECURSIVE CALL
                LIB_HANDLER();
                CurOpcode=saveOpcode;
                return;
                }


                WORDPTR *val=rplFindLAM(rplPeekData(1),1);
                if(!val) {
                    val=rplFindGlobal(rplPeekData(1),1);
                    if(!val) {
                        // INEXISTENT IDENT EVALS TO ITSELF, SO RETURN DIRECTLY
                        return;
                    }
                }

                if(rplCheckCircularReference((WORDPTR)symbeval_seco+2,*(val+1),4)) {
                    rplError(ERR_CIRCULARREFERENCE);
                    return;
                }

                // CREATE A NEW LAM ENVIRONMENT IDENTICAL TO THE ONE USED TO EVAL SYMBOLICS
                // FOR CIRCULAR REFERENCE CHECK
                rplCreateLAMEnvironment((WORDPTR)symbeval_seco+2);

                rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);     // LAM 1 = 0 (DUMMY)
                if(Exceptions) { rplCleanupLAMs(0); return; }

                rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);     // LAM 2 = 0 (DUMMY)
                if(Exceptions) { rplCleanupLAMs(0); return; }

                rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);     // LAM 3 = 0 (DUMMY)
                if(Exceptions) { rplCleanupLAMs(0); return; }

                rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));     // LAM 4 = MAIN VARIABLE NAME, FOR CIRCULAR REFERENCE CHECK
                if(Exceptions) { rplCleanupLAMs(0); return; }


                rplOverwriteData(1,*(val+1));    // REPLACE THE FIRST LEVEL WITH THE VALUE

                rplPushRet(IPtr);
                IPtr=(WORDPTR) lameval_seco;
                CurOpcode=(CMD_OVR_EVAL);
            }
                return;

            case OVR_NUM:
            // RCL WHATEVER IS STORED IN THE LAM AND THEN ->NUM ITS CONTENTS
                // NO ARGUMENT CHECKS! THAT SHOULD'VE BEEN DONE BY THE OVERLOADED "EVAL" DISPATCHER
                {
                    WORDPTR *val=rplFindLAM(rplPeekData(1),1);
                    if(!val) {
                        val=rplFindGlobal(rplPeekData(1),1);
                        if(!val) {
                            // INEXISTENT VARIABLE CANNOT BE CONVERTED TO NUMBER
                            rplError(ERR_UNDEFINEDVARIABLE);
                            return;
                        }
                    }

                    if(rplCheckCircularReference((WORDPTR)symbnum_seco+2,*(val+1),4)) {
                        rplError(ERR_CIRCULARREFERENCE);
                        return;
                    }

                    // CREATE A NEW LAM ENVIRONMENT IDENTICAL TO THE ONE USED TO EVAL SYMBOLICS
                    // FOR CIRCULAR REFERENCE CHECK
                    rplCreateLAMEnvironment((WORDPTR)symbnum_seco+2);

                    rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);     // LAM 1 = 0 (DUMMY)
                    if(Exceptions) { rplCleanupLAMs(0); return; }

                    rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);     // LAM 2 = 0 (DUMMY)
                    if(Exceptions) { rplCleanupLAMs(0); return; }

                    rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);     // LAM 3 = 0 (DUMMY)
                    if(Exceptions) { rplCleanupLAMs(0); return; }

                    rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));     // LAM 4 = MAIN VARIABLE NAME, FOR CIRCULAR REFERENCE CHECK
                    if(Exceptions) { rplCleanupLAMs(0); return; }


                    rplOverwriteData(1,*(val+1));    // REPLACE THE FIRST LEVEL WITH THE VALUE

                    rplPushRet(IPtr);
                    IPtr=(WORDPTR) lamnum_seco;
                    CurOpcode=(CMD_OVR_NUM);
                }
                    return;

            case OVR_XEQ:
                // JUST KEEP THE IDENT ON THE STACK, UNEVALUATED
                if(!ISPROLOG(*rplPeekData(1))) {
                WORD saveOpcode=CurOpcode;
                CurOpcode=*rplPopData();
                // RECURSIVE CALL
                LIB_HANDLER();
                CurOpcode=saveOpcode;
                return;
                }

               return;

            default:
                // PASS AL OTHER OPERATORS DIRECTLY AS A SYMBOLIC OBJECT
            {
                    LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                    (*symblib)();
                    return;
            }
        }


    }   // END OF UNARY OPERATORS

    if(ISBINARYOP(CurOpcode)) {

        // PASS AL OTHER OPERATORS DIRECTLY AS A SYMBOLIC OBJECT
            LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
            (*symblib)();
            return;
        }


    }



    // SPECIAL OPCODES
    if(OPCODE(CurOpcode)&0x70000) {
        // IT'S ONE OF THE COMPACT OPCODES
        BINT op=OPCODE(CurOpcode)>>16;
        BINT num=OPCODE(CurOpcode)&0xffff;
        if(num&0x8000) num|=0xFFFF0000; // GET NEGATIVE LAMS TOO!

        switch(op)
        {
        case 1: // PUTLAMn
        {
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR *local=rplGetLAMn(num);
            *local=rplPopData();
            return;
         }
        case 2: // GETLAMn
            rplPushData(*rplGetLAMn(num));
            return;
        case 3: // GETLAMnEVAL
            rplPushData(*rplGetLAMn(num));
            //rplCallOvrOperator((CMD_OVR_XEQ));
            return;
        case 4: // NEWNLOCALS
            // THIS ONE HAS TO CREATE 'N' LOCALS TAKING THE NAMES AND OBJECTS FROM THE STACK
            // AND ALSO HAS TO 'EVAL' THE NEXT OBJECT IN THE RUNSTREAM
            // THE STACK CONTAINS VAL1 VAL2 ... VALN LAM1 LAM2 ... LAMN
        {
            if(rplDepthData()<2*num) {
                // MALFORMED SECONDARY? THIS SHOULD NEVER HAPPEN
                rplError(ERR_BADARGCOUNT);
                return;
            }

            // CHECK ALL ARGUMENTS
            BINT cnt=num;
            while(cnt) {
                if(!ISIDENT(*rplPeekData(cnt))) {
                    rplError(ERR_IDENTEXPECTED);
                    return;
                }
                --cnt;
            }

            // CREATE A NEW LAM ENVIRONMENT FOR THIS SECONDARY
            rplCreateLAMEnvironment(rplPeekRet(1));
            rplPushRet((WORDPTR)abnd_prog);                          // PUT ABND IN THE STACK TO DO THE CLEANUP
            BINT offset=num;
            // NOW CREATE ALL LOCAL VARIABLES
            while(num) {
                rplCreateLAM(rplPeekData(num),rplPeekData(num+offset));
                --num;
            }
            // CLEAN THE STACK
            rplDropData(2*offset);


         }
            return;
        }
    }

    switch(OPCODE(CurOpcode))
    {
    case LSTO:
    {
        // STORE CONTENT INSIDE A LAM VARIABLE, CREATE A NEW VARIABLE IF NEEDED
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }


        // FIND LOCAL VARIABLE IN THE CURRENT SCOPE ONLY
        WORDPTR *val=rplFindLAM(rplPeekData(1),0);
        BINT neednewenv=rplNeedNewLAMEnv();

        if(val && !neednewenv) {
            val[1]=rplPeekData(2);
            rplDropData(2);
        }
        else {
            // LAM WAS NOT FOUND, CREATE A NEW ONE
            if(neednewenv) {
                // A NEW LAM ENVIRONMENT NEEDS TO BE CREATED
                rplCreateLAMEnvironment(rplPeekRet(1));
                // AND PUSH THE AUTOMATIC CLEANUP ROUTINE
                rplPushRet((WORDPTR)abnd_prog);
            }
                // CREATE THE NEW VARIABLE WITHIN THE CURRENT ENVIRONMENT
                rplCreateLAM(rplPeekData(1),rplPeekData(2));
                rplDropData(2);
            }
    }
    return;
    case LRCL:
    {
        // RCL CONTENT FROM INSIDE A LAM VARIABLE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        WORDPTR val=rplGetLAM(rplPeekData(1));
        if(val) {
            rplOverwriteData(1,val);
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }
    }
        return;
    case ABND:
        // CLEANUP ALL LAMS AND DO SEMI
        {

            if(*(nLAMBase+1)==rplPeekRet(1)) {
                // THIS WILL BE TRUE 99.9% OF THE TIMES
               LAMTop=nLAMBase;
            }
            else {
                // THERE'S SOME OTHER LAM CONSTRUCT OR CORRUPTED MARKERS, SEARCH FOR THE CORRECT MARKER
            WORDPTR *val=LAMTop;
            while( (val=rplGetNextLAMEnv(val)) )
            {
            // PURGE ALL LAMS AFTER THE MARKER
            if(*(val+1)==rplPeekRet(1)) { LAMTop=val; break; }
            }
            // SOMETHING BAD HAPPENED, THIS SECONDARY HAD NO LAM ENVIRONMENT BUT AN ABND WORD!
            if(!val) LAMTop=LAMs;
            }

            nLAMBase=rplGetNextLAMEnv(LAMTop);
            if(!nLAMBase) nLAMBase=LAMs;

            IPtr=rplPopRet();   // GET THE CALLER ADDRESS
            if(IPtr) CurOpcode=*IPtr;    // SET THE WORD SO MAIN LOOP SKIPS THIS OBJECT, AND THE NEXT ONE IS EXECUTED
        }
            return;


    case LAMEVALPRE:
    {
        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE EVAL1ERR WORD

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case LAMEVALPOST:
    {
        // HERE GETLAM1 = OPCODE, GETLAM 2 = END OF SYMBOLIC, GETLAM3 = OBJECT

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR DURING EVALUATION

        rplCleanupLAMs(0);

        IPtr=rplPopRet();
        if(IPtr) CurOpcode=*IPtr;

        return;
    }
    case LAMEVALERR:
        // JUST CLEANUP AND EXIT
        //DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;
        ExceptionPointer=IPtr;
        CurOpcode=(CMD_OVR_EVAL);
        return;

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

        if(LIBNUM(CurOpcode)!=DOIDENT) {
            // WHEN CALLED WITH UPPER LIBRARY NUMBERS, RETURN QUICKLY TO SPEED UP COMPILE TIME
            RetNum=ERR_NOTMINE;
            return;
        }

        // LSTO NEEDS SPECIAL CONSIDERATION TO CREATE LAMS AT COMPILE TIME

        if((TokenLen==4) && (!utf8ncmp((char *)TokenStart,"LSTO",4)))
        {

            // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

            // CHECK IF THE PREVIOUS OBJECT IS A QUOTED IDENT?
            WORDPTR object,prevobject;
            if(ValidateTop<=RSTop) {
                // THERE'S NO ENVIRONMENT
                object=TempObEnd;   // START OF COMPILATION
            } else {
                object=*(ValidateTop-1);    // GET LATEST CONSTRUCT
                ++object;                   // AND SKIP THE PROLOG / ENTRY WORD
            }

            if(object<CompileEnd) {
            do {
                prevobject=object;
                object=rplSkipOb(object);
            } while(object<CompileEnd);

            // HERE PREVOBJECT CONTAINS THE LAST OBJECT THAT WAS COMPILED

            if(ISIDENT(*prevobject)) {
                // WE HAVE A HARD-CODED IDENT, CHECK IF IT EXISTS ALREADY

                // CHECK IF IT'S AN EXISTING LAM, COMPILE TO A PUTLAM OPCODE IF POSSIBLE

                WORDPTR *LAMptr=rplFindLAM(prevobject,1);


                if(LAMptr<LAMTopSaved) {
                    // THIS IS NOT A VALID LAM, LEAVE AS IDENT

                    rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LSTO));

                    // TRACK LAM CREATION IN THE CURRENT ENVIRONMENT

                    // DO WE NEED A NEW ENVIRONMENT?

                    if(rplNeedNewLAMEnvCompiler()) {    // CREATE A NEW ENVIRONMENT IF NEEDED
                        rplCreateLAMEnvironment(*(ValidateTop-1));
                    }
                    rplCreateLAM(prevobject,prevobject);
                    RetNum=OK_CONTINUE;
                    return;
                }

                if(LAMptr<nLAMBase) {
                    // THIS IS A LAM FROM AN UPPER CONSTRUCT
                    // WE CAN USE PUTLAM ONLY INSIDE LOOPS, NEVER ACROSS SECONDARIES

                    WORDPTR *env=nLAMBase;
                    WORD prolog;
                    do {
                        if(LAMptr>env) break;
                        prolog=**(env+1);   // GET THE PROLOG OF THE SECONDARY
                        if(ISPROLOG(prolog) && LIBNUM(prolog)==SECO) {
                        // LAMS ACROSS << >> SECONDARIES HAVE TO BE COMPILED AS IDENTS
                        rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LSTO));

                        RetNum=OK_CONTINUE;
                        return;
                        }
                        env=rplGetNextLAMEnv(env);
                    } while(env);



                }


                // SPECIAL CASE: WHEN A SECO DOESN'T HAVE ANY LOCALS YET
                // BUT LAMS FROM THE PREVIOUS SECO SHOULDN'T BE COMPILED TO GETLAMS

                // SCAN ALL CURRENT CONSTRUCTS TO FIND THE INNERMOST SECONDARY
                // THEN VERIFY IF THAT SECONDARY IS THE CURRENT LAM ENVIRONMENT

                // THIS IS TO FORCE ALL LAMS IN A SECO TO BE COMPILED AS IDENTS
                // INSTEAD OF PUTLAMS

                // LAMS ACROSS DOCOL'S ARE OK AND ALWAYS COMPILED AS PUTLAMS
                WORDPTR *scanenv=ValidateTop-1;

                while(scanenv>=RSTop) {
                    if( (LIBNUM(**scanenv)==SECO)&& (ISPROLOG(**scanenv))) {
                            // FOUND INNERMOST SECONDARY
                            if(*scanenv>*(nLAMBase+1)) {
                                // THE CURRENT LAM BASE IS OUTSIDE THE INNER SECONDARY
                            rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LSTO));
                            if(rplNeedNewLAMEnvCompiler()) {    // CREATE A NEW ENVIRONMENT IF NEEDED
                                rplCreateLAMEnvironment(*(ValidateTop-1));
                            }
                            rplCreateLAM(prevobject,prevobject);


                            RetNum=OK_CONTINUE;
                            return;
                            }
                            break;

                    }
                    --scanenv;
                }


                // IT'S A KNOWN LOCAL VARIABLE, COMPILE AS PUTLAM
                BINT Offset=((BINT)(LAMptr-nLAMBase))>>1;

                // ONLY USE PUTLAM IF OFFSET IS WITHIN RANGE
                if(Offset<=32767 && Offset>=-32768) {
                CompileEnd=prevobject;
                rplCompileAppend(MKOPCODE(DOIDENT,PUTLAMN+(Offset&0xffff)));
                }
                else {
                    rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LSTO));
                }

                RetNum=OK_CONTINUE;
                return;
            }
            else {
                // THIS WOULD CREATE A LOCAL THAT CANNOT BE TRACED
                // BY THE COMPILER. DISABLE PUTLAM/GETLAM OPTIMIZATION
                // FOR THE REST OF THIS ENVIRONMENT

                // ISSUE A SYNTAX ERROR, LSTO REQUIRES A COMPILE-TIME VARIABLE NAME
                RetNum=ERR_SYNTAX;
                return;

            }

            }


            rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LSTO));
            RetNum=OK_CONTINUE;
            return;
        }


        if((TokenLen==4) && (!utf8ncmp((char *)TokenStart,"LRCL",4)))
        {

            // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

            // CHECK IF THE PREVIOUS OBJECT IS A QUOTED IDENT?
            WORDPTR object,prevobject;
            if(ValidateTop<=RSTop) {
                // THERE'S NO ENVIRONMENT
                object=TempObEnd;   // START OF COMPILATION
            } else {
                object=*(ValidateTop-1);    // GET LATEST CONSTRUCT
                ++object;                   // AND SKIP THE PROLOG / ENTRY WORD
            }

            if(object<CompileEnd) {
            do {
                prevobject=object;
                object=rplSkipOb(object);
            } while(object<CompileEnd);

            // HERE PREVOBJECT CONTAINS THE LAST OBJECT THAT WAS COMPILED

            if(ISIDENT(*prevobject)) {
                // WE HAVE A HARD-CODED IDENT, CHECK IF IT EXISTS ALREADY

                // CHECK IF IT'S AN EXISTING LAM, COMPILE TO A GETLAM OPCODE IF POSSIBLE

                WORDPTR *LAMptr=rplFindLAM(prevobject,1);


                if(LAMptr<LAMTopSaved) {
                    // THIS IS NOT A VALID LAM, LEAVE AS IDENT

                    rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LRCL));
                    RetNum=OK_CONTINUE;
                    return;
                }

                if(LAMptr<nLAMBase) {
                    // THIS IS A LAM FROM AN UPPER CONSTRUCT
                    // WE CAN USE GETLAM ONLY INSIDE LOOPS, NEVER ACROSS SECONDARIES

                    WORDPTR *env=nLAMBase;
                    WORD prolog;
                    do {
                        if(LAMptr>env) break;
                        prolog=**(env+1);   // GET THE PROLOG OF THE SECONDARY
                        if(ISPROLOG(prolog) && LIBNUM(prolog)==SECO) {
                        // LAMS ACROSS << >> SECONDARIES HAVE TO BE COMPILED AS IDENTS
                        rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LRCL));

                        RetNum=OK_CONTINUE;
                        return;
                        }
                        env=rplGetNextLAMEnv(env);
                    } while(env);



                }


                // SPECIAL CASE: WHEN A SECO DOESN'T HAVE ANY LOCALS YET
                // BUT LAMS FROM THE PREVIOUS SECO SHOULDN'T BE COMPILED TO GETLAMS

                // SCAN ALL CURRENT CONSTRUCTS TO FIND THE INNERMOST SECONDARY
                // THEN VERIFY IF THAT SECONDARY IS THE CURRENT LAM ENVIRONMENT

                // THIS IS TO FORCE ALL LAMS IN A SECO TO BE COMPILED AS IDENTS
                // INSTEAD OF GETLAMS

                // LAMS ACROSS DOCOL'S ARE OK AND ALWAYS COMPILED AS GETLAMS
                WORDPTR *scanenv=ValidateTop-1;

                while(scanenv>=RSTop) {
                    if( (LIBNUM(**scanenv)==SECO)&& (ISPROLOG(**scanenv))) {
                            // FOUND INNERMOST SECONDARY
                            if(*scanenv>*(nLAMBase+1)) {
                                // THE CURRENT LAM BASE IS OUTSIDE THE INNER SECONDARY
                            rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LRCL));
                            RetNum=OK_CONTINUE;
                            return;
                            }
                            break;

                    }
                    --scanenv;
                }

                // IT'S A KNOWN LOCAL VARIABLE, COMPILE AS GETLAM
                BINT Offset=((BINT)(LAMptr-nLAMBase))>>1;

                if(Offset<=32767 && Offset>=-32768) {
                CompileEnd=prevobject;
                rplCompileAppend(MKOPCODE(DOIDENT,GETLAMN+(Offset&0xffff)));
                }
                else {
                    rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LRCL));
                }
                RetNum=OK_CONTINUE;
                return;
            }


            }


            rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LRCL));
            RetNum=OK_CONTINUE;
            return;
        }


            // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
            // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors
        if(ISPROLOG(*DecompileObject)) {

            // FIND UPPER CONSTRUCT, NOT JUST THE LAST ONE

            // WARNING: THIS USES DECOMPILER INTERNALS, ONLY USE THIS FOR CORE
            WORDPTR *prevconst=ValidateTop-2;
            WORD PreviousConstruct;
            if(prevconst<ValidateBottom) {
                // THERE'S NO UPPER CONSTRUCT
                PreviousConstruct=0;
            } else {
                PreviousConstruct=**prevconst;
            }

            if((ISQUOTEDIDENT(*DecompileObject))&&(PreviousConstruct!=CMD_XEQSECO))
                // THIS IS A QUOTED IDENT
                rplDecompAppendChar('\'');

            BYTEPTR ptr=(BYTEPTR)(DecompileObject+OBJSIZE(*DecompileObject));
            if(ptr[3]==0)
                // WE HAVE A NULL-TERMINATED STRING, SO WE CAN USE THE STANDARD FUNCTION
                rplDecompAppendString((BYTEPTR) (DecompileObject+1));
            else
                rplDecompAppendString2((BYTEPTR)(DecompileObject+1),OBJSIZE(*DecompileObject)<<2);

            if((ISQUOTEDIDENT(*DecompileObject))&&(PreviousConstruct!=CMD_XEQSECO))
                // THIS IS A QUOTED IDENT
                rplDecompAppendChar('\'');

            RetNum=OK_CONTINUE;
            return;

        }

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,LSTO)) {

            rplDecompAppendString((BYTEPTR)"LSTO");

            // CHECK IF THE PREVIOUS OBJECT IS A QUOTED IDENT?
            WORDPTR object,prevobject;
                object=*(ValidateTop-1);    // GET LATEST CONSTRUCT
                ++object;                   // AND SKIP THE PROLOG / ENTRY WORD

            do {
                prevobject=object;
                object=rplSkipOb(object);
            } while(object<DecompileObject);

            // HERE PREVOBJECT CONTAINS THE LAST OBJECT THAT WAS DECOMPILED

            if(!ISIDENT(*prevobject)) {
                // MAKE SURE IT'S AN IDENT, OTHERWISE IT'S FINE BUT NO VARIABLE CREATION IS TRACKED
                RetNum=OK_CONTINUE;
                return;
            }
                // WE HAVE A HARD-CODED IDENT
                // CHECK IF IT'S AN EXISTING LAM


                WORDPTR *LAMptr=rplFindLAM(prevobject,1);


                if(LAMptr<LAMTopSaved) {
                    // THIS IS NOT A VALID LAM, CREATE A NEW ONE

                    // TRACK LAM CREATION IN THE CURRENT ENVIRONMENT

                    // DO WE NEED A NEW ENVIRONMENT?

                    if(rplNeedNewLAMEnvCompiler()) {    // CREATE A NEW ENVIRONMENT IF NEEDED
                        rplCreateLAMEnvironment(*(ValidateTop-1));
                    }
                    rplCreateLAM((WORDPTR)prevobject,(WORDPTR)zero_bint);


                    RetNum=OK_CONTINUE;
                    return;
                }

                if(LAMptr<nLAMBase) {
                    // THIS IS A LAM FROM AN UPPER CONSTRUCT
                    // WE CAN USE PUTLAM ONLY INSIDE LOOPS, NEVER ACROSS SECONDARIES

                    WORDPTR *env=nLAMBase;
                    WORD prolog;
                    do {
                        if(LAMptr>env) break;
                        prolog=**(env+1);   // GET THE PROLOG OF THE SECONDARY
                        if(ISPROLOG(prolog) && LIBNUM(prolog)==SECO) {
                        // LAMS ACROSS << >> SECONDARIES HAVE TO BE COMPILED AS IDENTS
                        // SO WE CREATE A NEW ONE
                            if(rplNeedNewLAMEnvCompiler()) {    // CREATE A NEW ENVIRONMENT IF NEEDED
                                rplCreateLAMEnvironment(*(ValidateTop-1));
                            }
                            rplCreateLAM((WORDPTR)prevobject,(WORDPTR)zero_bint);
                        RetNum=OK_CONTINUE;
                        return;
                        }
                        env=rplGetNextLAMEnv(env);
                    } while(env);



                }


                // SPECIAL CASE: WHEN A SECO DOESN'T HAVE ANY LOCALS YET
                // BUT LAMS FROM THE PREVIOUS SECO SHOULDN'T BE COMPILED TO GETLAMS

                // SCAN ALL CURRENT CONSTRUCTS TO FIND THE INNERMOST SECONDARY
                // THEN VERIFY IF THAT SECONDARY IS THE CURRENT LAM ENVIRONMENT

                // THIS IS TO FORCE ALL LAMS IN A SECO TO BE COMPILED AS IDENTS
                // INSTEAD OF PUTLAMS

                // LAMS ACROSS DOCOL'S ARE OK AND ALWAYS COMPILED AS PUTLAMS
                WORDPTR *scanenv=ValidateTop-1;

                while(scanenv>=RSTop) {
                    if( (LIBNUM(**scanenv)==SECO)&& (ISPROLOG(**scanenv))) {
                            // FOUND INNERMOST SECONDARY
                            if(*scanenv>*(nLAMBase+1)) {
                                // THE CURRENT LAM BASE IS OUTSIDE THE INNER SECONDARY
                            if(rplNeedNewLAMEnvCompiler()) {    // CREATE A NEW ENVIRONMENT IF NEEDED
                                rplCreateLAMEnvironment(*(ValidateTop-1));
                            }
                            rplCreateLAM((WORDPTR)prevobject,(WORDPTR)zero_bint);


                            RetNum=OK_CONTINUE;
                            return;
                            }
                            break;

                    }
                    --scanenv;
                }

                // IT'S A KNOWN LOCAL VARIABLE, NO NEED TO TRACE IT
                RetNum=OK_CONTINUE;
                return;

        }



        switch(OPCODE(*DecompileObject)&0x70000)
        {
        case NEWNLOCALS:
        {

            // WARNING: THIS USES DECOMPILER INTERNALS, ONLY USE THIS FOR CORE
            WORDPTR *prevconst=ValidateTop-2;
            WORD PreviousConstruct;
            if(prevconst<ValidateBottom) {
                // THERE'S NO UPPER CONSTRUCT
                PreviousConstruct=0;
            } else {
                PreviousConstruct=**prevconst;
            }

            if(PreviousConstruct!=CMD_XEQSECO) {
                RetNum=ERR_INVALID;
                return;
            }


            rplCreateLAMEnvironment(*prevconst+1);  // OWNER WILL BE THE SECONDARY
            BINT offset=0;

            // HERE WE HAVE *prevconst POINTING TO THE XEQSECO COMMAND
            // ADD 2 TO SKIP THE WORD + PROLOG OF THE SECO
            // POINTING TO THE FIRST LAM NAME WHEN OFFSET=0

            while((*prevconst)+2+offset < DecompileObject) {
                // CREATE ALL THE NAMED VARIABLES
                rplCreateLAM((*prevconst)+2+offset,(WORDPTR)zero_bint);        // NULLLAM FOR THE COMPILER
                offset+=rplObjSize((*prevconst)+2+offset);
            }

            // DONE CREATING THE ENVIRONMENT

            // NOW REMOVE THE XEQSECO CONSTRUCT WITHOUT ENDING THE SECO
            *prevconst=*(prevconst+1);


            // NOW LOOK FORWARD TO DETERMINE IF THIS IS AN ALGEBRAIC EXPRESSION

            WORDPTR fwd;
            WORD alg=0,eval1=0,semi=0;

            fwd=DecompileObject+1;
            if(fwd<EndOfObject) {
                alg=*fwd;
                fwd=rplSkipOb(fwd);
                if(fwd<EndOfObject) {
                    eval1=*fwd;
                    fwd=rplSkipOb(fwd);
                    if(fwd<EndOfObject) semi=*fwd;
                }
            }

            if(ISSYMBOLIC(alg)&&(eval1==CMD_OVR_EVAL1)&&(semi==CMD_QSEMI)) {
                // THIS IS AN ALGEBRAIC CONSTRUCT
                rplDecompile(DecompileObject+1,DECOMP_EMBEDDED | ((CurOpcode==OPCODE_DECOMPEDIT)? DECOMP_EDIT:0));
                DecompileObject+=3+OBJSIZE(alg);    // SKIP UNTIL END OF SECO
            }
            else             rplDecompAppendString((BYTEPTR)"«");


            RetNum=OK_ENDCONSTRUCT;
            return;
        }
        case GETLAMN:
        {

            BINT num=OPCODE(*DecompileObject)&0xffff;
            if(num&0x8000) num|=0xFFFF0000; // GET NEGATIVE LAMS TOO!

            rplDecompAppendChar('\'');
            WORDPTR name=*rplGetLAMnName(num);
            if(!ISIDENT(*name)) {
                RetNum=ERR_SYNTAX;
                return;
            }
            BINT len=OBJSIZE(*name);

            WORD lastword=name[len];

            len<<=2;

            if(lastword<0x1000000) rplDecompAppendString((BYTEPTR)(name+1));
            else rplDecompAppendString2( ((BYTEPTR)(name+1)),len);

            rplDecompAppendString((BYTEPTR)"\' LRCL");
            RetNum=OK_CONTINUE;
            return;
        }

            /*
            //  LEAVE THIS FOR SOME OBSCURE DEBUG MODE
            rplDecompAppendString((BYTEPTR)"GETLAM");
            BINT result=OPCODE(*DecompileObject)&0xffff;
            if(result&0x8000) result|=0xFFFF0000;
            if(result<0) {
                rplDecompAppendChar('u');
                result=-result;
            }

            BINT digit=0;
            char basechr='0';
            while(result<powersof10[digit]) ++digit;  // SKIP ALL LEADING ZEROS
            // NOW DECOMPILE THE NUMBER
            while(digit<18) {
            while(result>=powersof10[digit]) { ++basechr; result-=powersof10[digit]; }
            rplDecompAppendChar(basechr);
            ++digit;
            basechr='0';
            }
            basechr+=result;
            rplDecompAppendChar(basechr);
        }
            RetNum=OK_CONTINUE;
            return;
            */
        case GETLAMNEVAL:
        {
            BINT num=OPCODE(*DecompileObject)&0xffff;
            if(num&0x8000) num|=0xFFFF0000; // GET NEGATIVE LAMS TOO!

            WORDPTR name=*rplGetLAMnName(num);
            if(!ISIDENT(*name)) {
                RetNum=ERR_SYNTAX;
                return;
            }
            BINT len=OBJSIZE(*name);

            WORD lastword=name[len];

            len<<=2;

            if(lastword<0x1000000) rplDecompAppendString((BYTEPTR)(name+1));
            else rplDecompAppendString2( ((BYTEPTR)(name+1)),len);


            RetNum=OK_CONTINUE;

            return;

        }

            /*
            //  LEAVE THIS FOR SOME OBSCURE DEBUG MODE

            rplDecompAppendString((BYTEPTR)"GETLAM");
            BINT result=OPCODE(*DecompileObject)&0xffff;
            if(result&0x8000) result|=0xFFFF0000;
            if(result<0) {
                rplDecompAppendChar('u');
                result=-result;
            }

            BINT digit=0;
            char basechr='0';
            while(result<powersof10[digit]) ++digit;  // SKIP ALL LEADING ZEROS
            // NOW DECOMPILE THE NUMBER
            while(digit<18) {
            while(result>=powersof10[digit]) { ++basechr; result-=powersof10[digit]; }
            rplDecompAppendChar(basechr);
            ++digit;
            basechr='0';
            }
            basechr+=result;
            rplDecompAppendChar(basechr);
        }
            rplDecompAppendString((BYTEPTR)"EVAL");
            RetNum=OK_CONTINUE;
            return;
            */
        case PUTLAMN:
        {
            BINT num=OPCODE(*DecompileObject)&0xffff;
            if(num&0x8000) num|=0xFFFF0000; // GET NEGATIVE LAMS TOO!

            rplDecompAppendChar('\'');
            WORDPTR name=*rplGetLAMnName(num);
            if(!ISIDENT(*name)) {
                RetNum=ERR_SYNTAX;
                return;
            }
            BINT len=OBJSIZE(*name);

            WORD lastword=name[len];

            len<<=2;

            if(lastword<0x1000000) rplDecompAppendString((BYTEPTR)(name+1));
            else rplDecompAppendString2( ((BYTEPTR)(name+1)),len);

            rplDecompAppendString((BYTEPTR)"\' LSTO");
            RetNum=OK_CONTINUE;
            return;
        }
            /*
            //  LEAVE THIS FOR SOME OBSCURE DEBUG MODE
            rplDecompAppendString((BYTEPTR)"PUTLAM");
            BINT result=OPCODE(*DecompileObject)&0xffff;
            if(result&0x8000) result|=0xFFFF0000;
            if(result<0) {
                rplDecompAppendChar('u');
                result=-result;
            }

            BINT digit=0;
            char basechr='0';
            while(result<powersof10[digit]) ++digit;  // SKIP ALL LEADING ZEROS
            // NOW DECOMPILE THE NUMBER
            while(digit<18) {
            while(result>=powersof10[digit]) { ++basechr; result-=powersof10[digit]; }
            rplDecompAppendChar(basechr);
            ++digit;
            basechr='0';
            }
            basechr+=result;
            rplDecompAppendChar(basechr);
        }
            RetNum=OK_CONTINUE;
            return;
            */

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

    case OPCODE_PROBETOKEN:
        RetNum=ERR_NOTMINE;
        return;
    case OPCODE_GETINFO:
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_IDENT,0,1);
        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER,(WORDPTR *)ROMPTR_TABLE,ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID);
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) {

            // IDENTS ARE ZERO-PADDED STRINGS, DETERMINE THE ACTUAL NUMBER OF BYTES USED
            BINT len=OBJSIZE(*ObjectPTR);
            if(len<1) { RetNum=ERR_INVALID; return; }
            WORD lastword=*(ObjectPTR+len);
            BINT usedbytes=0;
            while( !(lastword&0xff000000) && (usedbytes<4) ) { lastword<<=8; ++usedbytes; }
            usedbytes=4-usedbytes;

            if(!usedbytes) { RetNum=ERR_INVALID; return; }  // IDENT HAS AN EXTRA WORD

            // AND CHECK FOR NAME VALIDITY
            if(!rplIsValidIdent((BYTEPTR)(ObjectPTR+1),((BYTEPTR)ObjectPTR)+(len<<2)+usedbytes)) { RetNum=ERR_INVALID; return; }
        }
        RetNum=OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        return;

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(MENUNUMBER(MenuCodeArg)>0) {
            RetNum=ERR_NOTMINE;
            return;
        }
        ObjectPTR=(WORDPTR)lib32_menu;
        RetNum=OK_CONTINUE;
       return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp,(WORDPTR)LIB_HELPTABLE);
       return;
    }
    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {

       RetNum=ERR_NOTMINE;
       return;
    }

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




