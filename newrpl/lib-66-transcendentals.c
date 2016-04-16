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
#define LIBRARY_NUMBER  66


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(SIN,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(COS,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(TAN,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(ASIN,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(ACOS,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(ATAN,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(ATAN2,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(LN,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(EXP,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(SINH,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(COSH,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(TANH,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(ASINH,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(ACOSH,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(ATANH,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(LOG,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(ALOG,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    ECMD(SQRT,"√",MKTOKENINFO(1,TITYPE_PREFIXOP,1,3)), \
    ECMD(PINUM,"π0",MKTOKENINFO(2,TITYPE_PREFIXOP,0,3))

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


    // LIBRARIES THAT DEFINE ONLY COMMANDS STILL HAVE TO RESPOND TO A FEW OVERLOADABLE OPERATORS
    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE) {
        // ONLY RESPOND TO EVAL, EVAL1 AND XEQ FOR THE COMMANDS DEFINED HERE
        // IN CASE OF COMMANDS TREATED AS OBJECTS (WHEN EMBEDDED IN LISTS)
        if( (OPCODE(CurOpcode)==OVR_EVAL)||
                (OPCODE(CurOpcode)==OVR_EVAL1)||
                (OPCODE(CurOpcode)==OVR_XEQ) )
        {
            // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORD saveOpcode=CurOpcode;
            CurOpcode=*rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode=saveOpcode;
            return;
        }
    }

    switch(OPCODE(CurOpcode))
    {
    case PINUM:
    {
        REAL pi;

        decconst_PI(&pi);

        rplNewRealPush(&pi);
        return;

    }
    case SIN:
    {
        REAL dec;
        BINT angmode;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }


        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }

        angmode=ANGLEMODE(*arg);

        if(angmode==ANGLENONE) angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);


        if(ISANGLE(*arg)) rplReadNumberAsReal(arg+1,&dec);
        else rplReadNumberAsReal(arg,&dec);

        if(Exceptions) return;


        trig_sincos(&dec,angmode);

        finalize(&RReg[7]);

        rplDropData(1);
        rplNewRealFromRRegPush(7);       // SIN
        return;

    }
    case COS:
    {
        REAL dec;
        BINT angmode;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }

        angmode=ANGLEMODE(*arg);

        if(angmode==ANGLENONE) angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);


        if(ISANGLE(*arg)) rplReadNumberAsReal(arg+1,&dec);
        else rplReadNumberAsReal(arg,&dec);

        if(Exceptions) return;

        trig_sincos(&dec,angmode);

        finalize(&RReg[6]);
        rplDropData(1);
        rplNewRealFromRRegPush(6);       // COS
        return;

    }
    case TAN:
    {
        REAL dec;
        BINT angmode;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }


        angmode=ANGLEMODE(*arg);

        if(angmode==ANGLENONE) angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);


        if(ISANGLE(*arg)) rplReadNumberAsReal(arg+1,&dec);
        else rplReadNumberAsReal(arg,&dec);

        if(Exceptions) return;


        trig_sincos(&dec,angmode);
        normalize(&RReg[6]);
        normalize(&RReg[7]);
        if(iszeroReal(&RReg[6])) {
            rplInfinityToRReg(2);
            RReg[2].flags^=RReg[7].flags&F_NEGATIVE;
        }
        else {
            divReal(&RReg[2],&RReg[7],&RReg[6]);
        }

        rplDropData(1);
        rplNewRealFromRRegPush(2);       // SIN/COS

        return;

    }
    case ASIN:
    {
        REAL y;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }


        rplReadNumberAsReal(rplPeekData(1),&y);

        if(Exceptions) return;
        // WARNING: TRANSCENDENTAL FUNCTIONS OVERWRITE ALL RREGS. INITIAL ARGUMENTS ARE PASSED ON RREG 0, 1 AND 2, SO USING 7 IS SAFE.
        rplOneToRReg(7);
        BINT signy=y.flags&F_NEGATIVE;
        y.flags^=signy;

        if(gtReal(&y,&RReg[7])) {
            // TODO: INCLUDE COMPLEX ARGUMENTS HERE
            rplError(ERR_ARGOUTSIDEDOMAIN);

            return;
        }

        BINT angmode;
        angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

        y.flags^=signy;
        trig_asin(&y,angmode);

        finalize(&RReg[0]);


        WORDPTR newangle=rplNewAngleFromReal(&RReg[0],angmode);
        if(!newangle) return;

        rplOverwriteData(1,newangle);

        return;

    }

    case ACOS:
    {
        REAL y;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }


        rplReadNumberAsReal(rplPeekData(1),&y);

        if(Exceptions) return;
        // WARNING: TRANSCENDENTAL FUNCTIONS OVERWRITE ALL RREGS. INITIAL ARGUMENTS ARE PASSED ON RREG 0, 1 AND 2, SO USING 7 IS SAFE.
        rplOneToRReg(7);
        BINT signy=y.flags&F_NEGATIVE;
        y.flags^=signy;
        if(gtReal(&y,&RReg[7])) {
            // TODO: INCLUDE COMPLEX ARGUMENTS HERE
            rplError(ERR_ARGOUTSIDEDOMAIN);

            return;
        }

        BINT angmode;
        angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

        y.flags^=signy;

        trig_acos(&y,angmode);
        normalize(&RReg[0]);
        // FIRST ELIMINATE BAD DIGITS SO ACOS(1)=0 INSTEAD OF SMALL NUMBER
        truncReal(&RReg[0],&RReg[0],Context.precdigits+6);
        round_real(&RReg[0],Context.precdigits,0);

        WORDPTR newangle=rplNewAngleFromReal(&RReg[0],angmode);
        if(!newangle) return;

        rplOverwriteData(1,newangle);

        return;

    }



    case ATAN:
        // CALCULATE ATAN FROM ATAN2(Y,1)
    {
        REAL y;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }


        rplReadNumberAsReal(rplPeekData(1),&y);

        if(Exceptions) return;
        // WARNING: TRANSCENDENTAL FUNCTIONS OVERWRITE ALL RREGS. INITIAL ARGUMENTS ARE PASSED ON RREG 0, 1 AND 2, SO USING 7 IS SAFE.
        rplOneToRReg(7);


        BINT angmode;
        angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

        trig_atan2(&y,&RReg[7],angmode);
        finalize(&RReg[0]);

        WORDPTR newangle=rplNewAngleFromReal(&RReg[0],angmode);
        if(!newangle) return;

        rplOverwriteData(1,newangle);

        return;

    }
    case ATAN2:
        // CALCULATE ATAN IN THE RANGE -PI,+PI
    {
        REAL y,x;
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1),arg2=rplPeekData(2);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISSYMBOLIC(*arg2) || ISIDENT(*arg2)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,2);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg) || ISLIST(*arg2)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushDataNoGrow((WORDPTR)two_bint);
            rplPushData(newobj);

            rplCallOperator(CMD_CMDDOLIST);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT DOLIST

            return;
        }


        rplReadNumberAsReal(rplPeekData(2),&y);
        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        BINT angmode;
        angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

        trig_atan2(&y,&x,angmode);

        finalize(&RReg[0]);

        WORDPTR newangle=rplNewAngleFromReal(&RReg[0],angmode);
        if(!newangle) return;

        rplOverwriteData(2,newangle);
        rplDropData(1);


        return;

    }

    case LN:
    {
        REAL x;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        if(iszeroReal(&x)) {
            // RETURN -INFINITY AND SET OVERFLOW
            rplInfinityToRReg(0);
            RReg[0].flags|=F_NEGATIVE;
            rplDropData(1);
            rplNewRealFromRRegPush(0);
            // TODO: IMPLEMENT FLAGS TO AVOID THROWING AN ERROR
            rplError(ERR_INFINITERESULT);
            return;
        }

        if(x.flags&F_NEGATIVE) {
            // TODO: RETURN COMPLEX VALUE!
            // FOR NOW JUST THROW AN EXCEPTION
            rplError(ERR_ARGOUTSIDEDOMAIN);
            return;
        }

        // HANDLE SPECIAL VALUES
        if(isinfiniteReal(&x)) {
           rplError(ERR_INFINITERESULT);
           return;
        }


        hyp_ln(&x);
        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }

    case EXP:
    {
        REAL dec;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        // HANDLE SPECIAL VALUES
        if(isinfiniteReal(&dec)) {
           if(dec.flags&F_NEGATIVE) {
               // e^-Inf = 0
               rplDropData(1);
               rplNewBINTPush(0,DECBINT|((dec.flags&F_APPROX)? APPROX_BIT:0));
               return;
           }
           rplError(ERR_INFINITERESULT);
           return;
        }


        hyp_exp(&dec);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);       // EXP
        return;


    }
    case SINH:
    {
        REAL dec;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_sinhcosh(&dec);

        finalize(&RReg[2]);

        rplDropData(1);
        rplNewRealFromRRegPush(2);       // SINH
        return;

    }

    case COSH:
    {
        REAL dec;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_sinhcosh(&dec);
        finalize(&RReg[1]);

        rplDropData(1);
        rplNewRealFromRRegPush(1);       // COSH
        return;

    }

    case TANH:

    {
        REAL dec;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_sinhcosh(&dec);

        normalize(&RReg[2]);
        normalize(&RReg[1]);

        // TANH=SINH/COSH
        divReal(&RReg[0],&RReg[2],&RReg[1]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);       // TANH
        return;

    }


    case ASINH:
    {
        REAL x;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }


        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        hyp_asinh(&x);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }

    return;



    case ACOSH:
    {
        REAL x;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        rplOneToRReg(0);

        if(gtReal(&RReg[0],&x)) {
            // TODO: EXPAND THIS TO RETURN COMPLEX VALUES
            rplError(ERR_ARGOUTSIDEDOMAIN);
           return;
        }

        hyp_acosh(&x);
        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }

    case ATANH:
    {
        REAL x;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        rplOneToRReg(0);
        BINT signx=x.flags;
        x.flags&=~F_NEGATIVE;
        BINT ismorethan1=cmpReal(&x,&RReg[0]);
        x.flags=signx;

        if(ismorethan1==1) {    // x > 1.0
            // TODO: CHANGE THIS ERROR INTO COMPLEX RESULTS!
            rplError(ERR_ARGOUTSIDEDOMAIN);

            return;
        }
        if(ismorethan1==0)
        {
            // TODO: IMPLEMENT INFINITY FLAGS TO THROW EXCEPTION ON INFINITY
            rplInfinityToRReg(0);
            if(signx&F_NEGATIVE) RReg[0].flags|=F_NEGATIVE;

            rplDropData(1);
            rplNewRealFromRRegPush(0);
            return;
        }

        hyp_atanh(&x);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }


    return;


    case SQRT:
    {
        REAL x,y;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }




        if(ISCOMPLEX(*arg)) {
        rplReadCNumberAsReal(arg,&x);
        rplReadCNumberAsImag(arg,&y);

        // HANDLE SPECIAL VALUES
        if(isinfiniteReal(&y)) {
           rplError(ERR_INFINITERESULT);
           return;
        }
        // HANDLE SPECIAL VALUES
        if(isinfiniteReal(&x)) {
           rplError(ERR_INFINITERESULT);
           return;
        }

            
        if(!iszeroReal(&y)) {
            mulReal(&RReg[0],&x,&x);
            mulReal(&RReg[1],&y,&y);
            addReal(&RReg[7],&RReg[0],&RReg[1]);
            
            hyp_sqrt(&RReg[7]);     // RReg[0]=sqrt(x^2+y^2);
            normalize(&RReg[0]);
            addReal(&RReg[1],&RReg[0],&x);
            subReal(&RReg[2],&RReg[0],&x);  // ONLY POW AND XROOT USE REGISTER 8, SO THIS IS SAFE

            RReg[0].exp=-1;
            RReg[0].len=1;
            RReg[0].flags=0;
            RReg[0].data[0]=5;

            mulReal(&RReg[7],&RReg[1],&RReg[0]);
            mulReal(&RReg[8],&RReg[2],&RReg[0]);

            hyp_sqrt(&RReg[7]);     // THIS IS THE REAL PART OF THE RESULT

            finalize(&RReg[0]);
            swapReal(&RReg[9],&RReg[0]);       // SAVE THIS RESULT

            hyp_sqrt(&RReg[8]);     // THIS IS THE IMAGINARY PART

            finalize(&RReg[0]);

            RReg[0].flags|=y.flags&F_NEGATIVE;

            // DONE, RETURN THE COMPLEX ROOTS
            rplDropData(1);
            rplNewComplexPush(&RReg[9],&RReg[0],ANGLENONE);


            return;

            

        }
            
        }
        else {

            if(!ISNUMBER(*arg)) {
                // ALL OTHER OBJECT TYPES DO
                // obj ^ 0.5

                RReg[0].data[0]=5;
                RReg[0].exp=-1;
                RReg[0].flags=0;
                RReg[0].len=1;

                rplNewRealFromRRegPush(0);

                rplCallOvrOperator(CMD_OVR_POW);

                return;
            }


        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;
        }

        BINT iscplx=x.flags&F_NEGATIVE;

        x.flags&=~F_NEGATIVE;

        // HANDLE SPECIAL VALUES
        if(isinfiniteReal(&x)) {
           rplError(ERR_INFINITERESULT);
           return;
        }


        hyp_sqrt(&x);
        finalize(&RReg[0]);

        rplDropData(1);
        if(iscplx) {
            rplZeroToRReg(1);
            rplNewComplexPush(&RReg[1],&RReg[0],ANGLENONE);
        } else rplNewRealFromRRegPush(0);
        return;

    }


    case LOG:
    {
        REAL x;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        if(iszeroReal(&x)) {
            // RETURN -INFINITY AND SET OVERFLOW
            // TODO: IMPLEMENT FLAGS TO AVOID THROWING AN ERROR
            rplInfinityToRReg(0);
            RReg[0].flags|=F_NEGATIVE;
            rplDropData(1);
            rplNewRealFromRRegPush(0);
            rplError(ERR_INFINITERESULT);
            return;
        }

        if(x.flags&F_NEGATIVE) {
            // TODO: RETURN COMPLEX VALUE!
            // FOR NOW JUST THROW AN EXCEPTION
            rplError(ERR_ARGOUTSIDEDOMAIN);
            return;
        }

        // HANDLE SPECIAL VALUES
        if(isinfiniteReal(&x)) {
           rplError(ERR_INFINITERESULT);
           return;
        }


        hyp_log(&x);
        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }

    case ALOG:
    {
        REAL dec,ten;
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&dec);

        // HANDLE SPECIAL VALUES
        if(isinfiniteReal(&dec)) {
           rplError(ERR_INFINITERESULT);
           return;
        }



        rplReadNumberAsReal((WORDPTR)ten_bint,&ten);

        if(Exceptions) return;
        powReal(&RReg[0],&ten,&dec);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
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
