/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
// LIB 66 PROVIDES COMMANDS THAT DEAL WITH TRANSCENDENTAL FUNCTIONS

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  66
#define LIB_ENUM lib66_enum
#define LIB_NAMES lib66_names
#define LIB_HANDLER lib66_handler
#define LIB_NUMBEROFCMDS LIB66_NUMBEROFCMDS

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(SIN), \
    CMD(COS), \
    CMD(TAN), \
    CMD(ASIN), \
    CMD(ACOS), \
    CMD(ATAN), \
    CMD(ATAN2), \
    CMD(LN), \
    CMD(EXP), \
    CMD(SINH), \
    CMD(COSH), \
    CMD(TANH), \
    CMD(ASINH), \
    CMD(ACOSH), \
    CMD(ATANH), \
    CMD(SQRT)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
/*#define CMD_EXTRANAME \
    "VOID",

#define CMD_EXTRAENUM \
    VOID
*/

// INTERNAL DECLARATIONS


// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST /*, CMD_EXTRAENUM */, LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
char *LIB_NAMES[]= { CMD_LIST /*, CMD_EXTRANAME*/ };
#undef CMD


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case SIN:
    {
        mpd_t dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        trig_sincos(&dec);



        BINT exponent=Context.prec-RReg[7].digits-RReg[7].exp;
        RReg[7].exp+=exponent;
        mpd_round_to_intx(&RReg[2],&RReg[7],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[2].exp-=exponent;
        mpd_reduce(&RReg[1],&RReg[2],&Context);

        rplDropData(1);
        rplRRegToRealPush(1);       // SIN
        return;

    }
    case COS:
    {
        mpd_t dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        trig_sincos(&dec);


        BINT exponent=Context.prec-RReg[6].digits-RReg[6].exp;
        RReg[6].exp+=exponent;
        mpd_round_to_intx(&RReg[2],&RReg[6],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[2].exp-=exponent;
        mpd_reduce(&RReg[0],&RReg[2],&Context);

        rplDropData(1);
        rplRRegToRealPush(0);       // COS
        return;

    }
    case TAN:
    {
        mpd_t dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        trig_sincos(&dec);

        if(mpd_iszero(&RReg[6])) {
            mpd_set_infinity(&RReg[2]);
            if(mpd_isnegative(&RReg[7]))  mpd_set_negative(&RReg[2]);
        }
        else {
            mpd_div(&RReg[2],&RReg[7],&RReg[6],&Context);
        }

        rplDropData(1);
        rplRRegToRealPush(2);       // SIN/COS

        return;

    }
    case ASIN:
    {
        mpd_t y;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&y);

        if(Exceptions) return;
        // WARNING: TRANSCENDENTAL FUNCTIONS OVERWRITE ALL RREGS. INITIAL ARGUMENTS ARE PASSED ON RREG 0, 1 AND 2, SO USING 7 IS SAFE.
        rplOneToRReg(7);
        BINT signy=y.flags&MPD_NEG;
        y.flags^=signy;

        if(mpd_cmp(&y,&RReg[7],&Context)==1) {
            // TODO: INCLUDE COMPLEX ARGUMENTS HERE
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }
        y.flags^=signy;
        trig_asin(&y);

        BINT exponent=Context.prec-RReg[0].digits-RReg[0].exp;
        RReg[0].exp+=exponent;
        mpd_round_to_intx(&RReg[2],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[2].exp-=exponent;
        mpd_reduce(&RReg[0],&RReg[2],&Context);


        rplDropData(1);
        rplRRegToRealPush(0);       // RESULTING ANGLE

        return;

    }

    case ACOS:
    {
        mpd_t y;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&y);

        if(Exceptions) return;
        // WARNING: TRANSCENDENTAL FUNCTIONS OVERWRITE ALL RREGS. INITIAL ARGUMENTS ARE PASSED ON RREG 0, 1 AND 2, SO USING 7 IS SAFE.
        rplOneToRReg(7);
        BINT signy=y.flags&MPD_NEG;
        y.flags^=signy;
        if(mpd_cmp(&y,&RReg[7],&Context)==1) {
            // TODO: INCLUDE COMPLEX ARGUMENTS HERE
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }
        y.flags^=signy;

        trig_acos(&y);

        BINT exponent=Context.prec-RReg[0].digits-RReg[0].exp;
        RReg[0].exp+=exponent;
        mpd_round_to_intx(&RReg[2],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[2].exp-=exponent;
        mpd_reduce(&RReg[0],&RReg[2],&Context);


        rplDropData(1);
        rplRRegToRealPush(0);       // RESULTING ANGLE

        return;

    }



    case ATAN:
        // CALCULATE ATAN FROM ATAN2(Y,1)
    {
        mpd_t y;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&y);

        if(Exceptions) return;
        // WARNING: TRANSCENDENTAL FUNCTIONS OVERWRITE ALL RREGS. INITIAL ARGUMENTS ARE PASSED ON RREG 0, 1 AND 2, SO USING 7 IS SAFE.
        rplOneToRReg(7);


        trig_atan2(&y,&RReg[7]);


        BINT exponent=Context.prec-RReg[0].digits-RReg[0].exp;
        RReg[0].exp+=exponent;
        mpd_round_to_intx(&RReg[2],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[2].exp-=exponent;
        mpd_reduce(&RReg[0],&RReg[2],&Context);

        rplDropData(1);
        rplRRegToRealPush(0);       // RESULTING ANGLE

        return;

    }
    case ATAN2:
        // CALCULATE ATAN IN THE RANGE -PI,+PI
    {
        mpd_t y,x;
        if(rplDepthData()<2) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(2),&y);
        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        trig_atan2(&y,&x);

        BINT exponent=Context.prec-RReg[0].digits-RReg[0].exp;
        RReg[0].exp+=exponent;
        mpd_round_to_intx(&RReg[2],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[2].exp-=exponent;
        mpd_reduce(&RReg[0],&RReg[2],&Context);

        rplDropData(2);
        rplRRegToRealPush(0);       // RESULTING ANGLE

        return;

    }

    case LN:
    {
        mpd_t x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        if(mpd_iszero(&x)) {
            // RETURN -INFINITY AND SET OVERFLOW
            // TODO: IMPLEMENT FLAGS TO AVOID THROWING AN ERROR
            mpd_set_infinity(&RReg[0]);
            Exceptions|=EX_MATHOVERFLOW;
            ExceptionPointer=IPtr;
            return;
        }

        if(mpd_isnegative(&x)) {
            // TODO: RETURN COMPLEX VALUE!
            // FOR NOW JUST THROW AN EXCEPTION
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }

        hyp_ln(&x);

        BINT exponent=Context.prec-RReg[0].digits-RReg[0].exp;
        RReg[0].exp+=exponent;
        mpd_round_to_intx(&RReg[2],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[2].exp-=exponent;
        mpd_reduce(&RReg[1],&RReg[2],&Context);

        rplDropData(1);
        rplRRegToRealPush(1);
        return;

    }

    case EXP:
    {
        mpd_t dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_exp(&dec);

        BINT exponent=Context.prec-RReg[0].digits-RReg[0].exp;
        RReg[0].exp+=exponent;
        mpd_round_to_intx(&RReg[2],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[2].exp-=exponent;
        mpd_reduce(&RReg[1],&RReg[2],&Context);

        rplDropData(1);
        rplRRegToRealPush(1);       // EXP
        return;


    }
    case SINH:
    {
        mpd_t dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_sinhcosh(&dec);

        BINT exponent=Context.prec-RReg[2].digits-RReg[2].exp;
        RReg[2].exp+=exponent;
        mpd_round_to_intx(&RReg[7],&RReg[2],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[7].exp-=exponent;
        mpd_reduce(&RReg[0],&RReg[7],&Context);

        rplDropData(1);
        rplRRegToRealPush(0);       // SINH
        return;

    }

    case COSH:
    {
        mpd_t dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_sinhcosh(&dec);

        BINT exponent=Context.prec-RReg[1].digits-RReg[1].exp;
        RReg[1].exp+=exponent;
        mpd_round_to_intx(&RReg[2],&RReg[1],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[2].exp-=exponent;
        mpd_reduce(&RReg[0],&RReg[2],&Context);

        rplDropData(1);
        rplRRegToRealPush(0);       // COSH
        return;

    }

    case TANH:

    {
        mpd_t dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_sinhcosh(&dec);

        // TANH=SINH/COSH
        mpd_div(&RReg[0],&RReg[7],&RReg[6],&Context);
//        RReg[0].exp+=Context.prec;
//        mpd_round_to_intx(&RReg[7],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
//        RReg[7].exp-=Context.prec;
//        mpd_reduce(&RReg[0],&RReg[7],&Context);

        rplDropData(1);
        rplRRegToRealPush(0);       // TANH
        return;

    }


    case ASINH:
    {
        mpd_t x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        hyp_asinh(&x);

        BINT exponent=Context.prec-RReg[1].digits-RReg[1].exp;
        RReg[1].exp+=exponent;
        mpd_round_to_intx(&RReg[7],&RReg[1],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[7].exp-=exponent;
        mpd_reduce(&RReg[0],&RReg[7],&Context);

        rplDropData(1);
        rplRRegToRealPush(0);
        return;

    }

    return;



    case ACOSH:
    {
        mpd_t x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        rplOneToRReg(0);

        if(mpd_cmp(&RReg[0],&x,&Context)>0) {
            // TODO: EXPAND THIS TO RETURN COMPLEX VALUES
           Exceptions|=EX_BADARGVALUE;
           ExceptionPointer=IPtr;
           return;
        }

        hyp_acosh(&x);

        BINT exponent=Context.prec-RReg[1].digits-RReg[1].exp;
        RReg[1].exp+=exponent;
        mpd_round_to_intx(&RReg[7],&RReg[1],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[7].exp-=exponent;
        mpd_reduce(&RReg[0],&RReg[7],&Context);

        rplDropData(1);
        rplRRegToRealPush(0);
        return;

    }

    case ATANH:
    {
        mpd_t x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        rplOneToRReg(0);
        BINT signx=mpd_sign(&x);
        mpd_set_positive(&x);
        BINT ismorethan1=mpd_cmp(&x,&RReg[0],&Context);
        mpd_set_sign(&x,signx);

        if(ismorethan1==1) {    // x > 1.0
            // TODO: CHANGE THIS ERROR INTO COMPLEX RESULTS!
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }
        if(ismorethan1==0)
        {
            // TODO: IMPLEMENT INFINITY FLAGS TO THROW EXCEPTION ON INFINITY
            mpd_set_infinity(&RReg[0]);
            if(mpd_isnegative(&x)) mpd_set_negative(&RReg[0]);

            rplDropData(1);
            rplRRegToRealPush(0);
            return;
        }

        hyp_atanh(&x);

        BINT exponent=Context.prec-RReg[0].digits-RReg[0].exp;
        RReg[0].exp+=exponent;
        mpd_round_to_intx(&RReg[7],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[7].exp-=exponent;
        mpd_reduce(&RReg[0],&RReg[7],&Context);

        rplDropData(1);
        rplRRegToRealPush(0);
        return;

    }


    return;


    case SQRT:
    {
        mpd_t x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        if(mpd_isnegative(&x)) {
            // TODO: EXPAND THIS TO RETURN COMPLEX VALUES
           Exceptions|=EX_BADARGVALUE;
           ExceptionPointer=IPtr;
           return;
        }

        hyp_sqrt(&x);

        BINT exponent=Context.prec-RReg[0].digits-RReg[0].exp;
        RReg[0].exp+=exponent;
        mpd_round_to_intx(&RReg[7],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[7].exp-=exponent;
        mpd_reduce(&RReg[0],&RReg[7],&Context);

        rplDropData(1);
        rplRRegToRealPush(0);
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
        libCompileCmds(LIBRARY_NUMBER,LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

     return;

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds(LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
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
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;

}




