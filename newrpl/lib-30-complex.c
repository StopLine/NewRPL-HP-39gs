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
#define LIBRARY_NUMBER  30


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(RE,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(IM,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(ARG,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(CONJ,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    ECMD(CPLX2REAL,"C→R",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(REAL2CPLX,"R→C",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE

#define ERROR_LIST \
ERR(COMPLEXEXPECTED,0), \
ERR(COMPLEXORREALEXPECTED,1), \
ERR(COMPLEXNOTSUPPORTED,2), \
ERR(NOTALLOWEDINCOMPLEX,3)


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

// USED TO DEFINE A REAL CONSTANT ZERO
const uint32_t const zero_data[1]={0};

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib30_menu);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib30_menu,
    0
};

// DECODE A COMPLEX NUMBER INTO A REAL
// DOES NOT ALLOCATE ANY MEMORY, MIGHT USE RREG[8] IF STORAGE IS NEEDED

void rplRealPart(WORDPTR complex,REAL *real)
{
    WORDPTR part=++complex;
    if(ISANGLE(*part)) ++part;
    rplReadNumberAsReal(part,real);
}

void rplImaginaryPart(WORDPTR complex,REAL *imag)
{
    WORDPTR part=rplSkipOb(++complex);
    if(ISANGLE(*part)) ++part;
    rplReadNumberAsReal(part,imag);
}

// RETURN -1 IF NOT POLAR, OTHERWISE RETURN THE ANGLE MODE
BINT rplPolarComplexMode(WORDPTR complex)
{
    if(!ISCOMPLEX(*complex)) return ANGLENONE;
    WORDPTR part=rplSkipOb(++complex);
    if(ISANGLE(*part)) return ANGLEMODE(*part);
    return ANGLENONE;
}

BINT rplComplexClass(WORDPTR complex)
{
    if(!ISCOMPLEX(*complex)) {
        if(ISBINT(*complex)) {
            // CAN ONLY BE ZERO
            BINT64 n=rplReadBINT(complex);
            if(n==0) return CPLX_ZERO;
            return CPLX_NORMAL;
        }
        if(ISREAL(*complex)) {
            REAL r;
            rplReadReal(complex,&r);
            if(iszeroReal(&r)) return CPLX_ZERO;
            switch(r.flags&F_UNDINFINITY)
            {
            case F_UNDINFINITY:
                return CPLX_UNDINF;
            case F_INFINITY:
                return CPLX_INF;
            case F_NOTANUMBER:
                return CPLX_NAN;
            }

            return CPLX_NORMAL;
        }
        return CPLX_NAN;

    }

    BINT cclass=0;
    WORDPTR re,im;
    re=++complex;
    im=rplSkipOb(re);

    if(ISANGLE(*im)) { cclass|=CPLX_POLAR; ++im; }

    REAL r,i;

    rplReadNumberAsReal(re,&r);
    rplReadNumberAsReal(im,&i);

    if(i.flags&F_NOTANUMBER) return CPLX_NAN;


    switch(r.flags&F_UNDINFINITY)
    {
    case F_UNDINFINITY:
        return CPLX_UNDINF;
    case F_INFINITY:
        cclass|=CPLX_INF;
        break;
    case F_NOTANUMBER:
        return CPLX_NAN;
    }

    if(i.flags&F_INFINITY) {
        if(cclass&CPLX_POLAR) return CPLX_NAN;
        if(cclass&CPLX_INF) return CPLX_UNDINF;
        // MALFORMED COMPLEX WITH BAD IMAGINARY PART, BUT IT'S DIRECTED INFINITY
        return CPLX_INF|CPLX_MALFORMED;
    }

    return cclass;

}


// GETS THE REAL PART OF ANY NUMBER: IF BINT OR REAL, GET THE NUMBER. IF COMPLEX, RETURN THE REAL PART.
void rplReadCNumberAsReal(WORDPTR complex,REAL *real)
{
    if(ISCOMPLEX(*complex)) rplRealPart(complex,real);
    else rplReadNumberAsReal(complex,real);
}

void rplReadCNumberAsImag(WORDPTR complex,REAL *imag)
{
    if(ISCOMPLEX(*complex)) rplImaginaryPart(complex,imag);
    else {
        // SET IMAG TO ZERO
        imag->data=(BINT *)zero_data;
        imag->exp=0;
        imag->flags=0;
        imag->len=1;
    }
}

void rplReadCNumber(WORDPTR complex,REAL *real,REAL *imag, BINT *angmode)
{
    if(ISCOMPLEX(*complex)) {
        WORDPTR part=rplSkipOb(complex+1);
        if(ISANGLE(*part)) {
            *angmode=ANGLEMODE(*part);
            rplReadNumberAsReal(part+1,imag);
        }
        else {
            *angmode=ANGLENONE;
            rplReadNumberAsReal(part,imag);
        }
        rplReadNumberAsReal(complex+1,real);
        return;
    }
    // IT'S A REAL NUMBER

    // SET IMAG TO ZERO
    imag->data=(BINT *)zero_data;
    imag->exp=0;
    imag->flags=0;
    imag->len=1;

    *angmode=ANGLENONE;

    rplReadNumberAsReal(complex,real);

}




// CREATE COMPLEX NUMBER FROM 2 RREG'S AND PUSH IT ON THE STACK

WORDPTR rplNewComplex(REAL *real,REAL *imag,BINT angmode)
{
    if(iszeroReal(imag)) {
        // IT'S A REAL NUMBER, THERE'S NO IMAGINARY PART
        return rplNewReal(real);
    }
    BINT size=4+real->len+imag->len;

    if(angmode!=ANGLENONE) ++size;

    ScratchPointer1=(WORDPTR)real->data;
    ScratchPointer2=(WORDPTR)imag->data;

    WORDPTR newobject=rplAllocTempOb(size);
    WORDPTR parts,end;
    if(!newobject) {
        return 0;
    }

    real->data=(BINT *)ScratchPointer1;
    imag->data=(BINT *)ScratchPointer2;

    parts=rplNewRealInPlace(real,newobject+1);
    end=rplNewRealInPlace(imag,parts+((angmode!=ANGLENONE)? 1:0));
    if(angmode!=ANGLENONE) parts[0]=MKPROLOG(DOANGLE+angmode,end-parts-1);
    newobject[0]=MKPROLOG(LIBRARY_NUMBER,end-newobject-1);

    rplTruncateLastObject(end);

    return newobject;

}

void rplNewComplexPush(REAL *real,REAL *imag,BINT angmode)
{
    WORDPTR newobject=rplNewComplex(real,imag,angmode);
    if(!newobject) return;
    rplPushData(newobject);

}
// CREATE COMPLEX NUMBER FROM 2 RREG'S AND PUSH IT ON THE STACK

void rplRRegToComplexPush(BINT real,BINT imag,BINT angmode)
{
    rplNewComplexPush( &RReg[real],&RReg[imag],angmode);
}






// CREATE COMPLEX NUMBER FROM 2 RREG'S AT ADDRESS dest
// AND RETURN POINTER IMMEDIATELY AFTER THE NUMBER
// DOES NOT ALLOCATE MEMORY FROM THE SYSTEM
// USED INTERNALLY FOR COMPOSITES

WORDPTR rplRRegToComplexInPlace(BINT real,BINT imag,WORDPTR dest,BINT angmode)
{
    if(iszeroReal(&RReg[imag])) {
        // IT'S A REAL NUMBER, THERE'S NO IMAGINARY PART
        return rplRRegToRealInPlace(real,dest);
    }
    WORDPTR parts,end;
    parts=rplRRegToRealInPlace(real,dest+1);
    end=rplRRegToRealInPlace(imag,parts+((angmode!=ANGLENONE)? 1:0));
    if(angmode!=ANGLENONE) parts[0]=MKPROLOG(DOANGLE+angmode,end-parts-1);
    dest[0]=MKPROLOG(LIBRARY_NUMBER,end-dest-1);

    return end;
}

// CONVERT TO CARTESIAN COORDINATES, RETURN RESULT IN RReg[0] AND RReg[1]
// USES ALL RREGS FROM 0 TO 7
void rplPolar2Rect(REAL *r,REAL *theta,BINT angmode)
{
    if(angmode==ANGLENONE) {
        copyReal(&RReg[0],r);
        copyReal(&RReg[1],theta);
        return;
    }

    // GET RReg[6]=COS(THETA), RReg[7]=SIN(THETA), BOTH NOT FINALIZED
    trig_sincos(theta,angmode);

    normalize(&RReg[6]);
    normalize(&RReg[7]);

    mulReal(&RReg[0],r,&RReg[6]);
    mulReal(&RReg[1],r,&RReg[7]);
    return;

}

// CONVERT A COMPLEX NUMBER TO POLAR COORDINATES USING THE GIVEN ANGLE MODE
// RESULT IS IN RReg[0]=r, RReg[1]=theta

// WARNING: INPUTS CAN'T BE IN RReg 6 OR 7

void rplRect2Polar(REAL *re,REAL *im,BINT angmode)
{
    if(angmode==ANGLENONE) {
        copyReal(&RReg[0],re);
        copyReal(&RReg[1],im);
        return;
    }

    // GET THE SQUARE OF LENGTH in RReg[8]
    mulReal(&RReg[6],re,re);
    mulReal(&RReg[7],im,im);
    addReal(&RReg[8],&RReg[6],&RReg[7]);

    // NOW USE THE ORIGINAL ARGUMENTS TO GET THE ANGLE
    // GET RReg[0]=theta
    if(iszeroReal(&RReg[8])) rplZeroToRReg(0);
    else trig_atan2(im,re,angmode);
    finalize(&RReg[0]);

    // MOVE IT TO A HIGHER REGISTER, AS ALL TRIG FUNCTIONS USE RRegs 0 TO 7
    swapReal(&RReg[9],&RReg[0]);

    // NOW DO THE SQUARE ROOT
    hyp_sqrt(&RReg[8]);
    finalize(&RReg[0]);

    // AND BRING BACK THE ANGLE TO RReg 1
    swapReal(&RReg[1],&RReg[9]);

    return;
}

// RETURN 1 IF IT'S ZERO, OTHERWISE 0
BINT rplIsZeroComplex(REAL *re,REAL *im,BINT angmode)
{
    if(angmode==ANGLENONE) {
        if(iszeroReal(re)&&iszeroReal(im)) return 1;
        return 0;
    }
    if(iszeroReal(re)) return 1;
    return 0;
}


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // NORMAL BEHAVIOR FOR A COMPLEX IS TO PUSH THE OBJECT ON THE STACK:
        rplPushData(IPtr);
        return;
    }



    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE)
    {
        // THESE ARE OVERLOADABLE COMMANDS DISPATCHED FROM THE
        // OVERLOADABLE OPERATORS LIBRARY.

        // PROVIDE BEHAVIOR FOR OVERLOADABLE OPERATORS HERE
#define arg1 ScratchPointer1
#define arg2 ScratchPointer2

        int nargs=OVR_GETNARGS(CurOpcode);
        REAL Rarg1,Iarg1,Rarg2,Iarg2;
        BINT amode1,amode2=ANGLENONE,cclass1,cclass2=CPLX_NORMAL;

        if(rplDepthData()<nargs) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(nargs==1) {
            // UNARY OPERATORS
            arg1=rplPeekData(1);
            if(!ISCOMPLEX(*arg1)) {
                if(!ISPROLOG(*arg1)) {
                    // ALLOW EXECUTION OF COMMANDS AS OBJECTS
                    if( (OPCODE(CurOpcode)==OVR_EVAL)||
                            (OPCODE(CurOpcode)==OVR_EVAL1)||
                            (OPCODE(CurOpcode)==OVR_XEQ) )
                    {
                        // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
                        WORD saveOpcode=CurOpcode;
                        CurOpcode=*rplPopData();
                        // RECURSIVE CALL
                        LIB_HANDLER();
                        CurOpcode=saveOpcode;
                        return;
                    }



                }
                rplError(ERR_COMPLEXEXPECTED);
                return;
            }

            rplReadCNumberAsReal(arg1,&Rarg1);
            rplReadCNumberAsImag(arg1,&Iarg1);
            cclass1=rplComplexClass(arg1);
            amode1=rplPolarComplexMode(arg1);
            rplDropData(1);
        }
        else {
            arg1=rplPeekData(2);
            arg2=rplPeekData(1);

            if(!ISNUMBERCPLX(*arg1) || !ISNUMBERCPLX(*arg2)) {
                rplError(ERR_COMPLEXORREALEXPECTED);
                return;
            }

            rplReadCNumberAsReal(arg1,&Rarg1);
            rplReadCNumberAsImag(arg1,&Iarg1);
            amode1=rplPolarComplexMode(arg1);
            cclass1=rplComplexClass(arg1);

            rplReadCNumberAsReal(arg2,&Rarg2);
            rplReadCNumberAsImag(arg2,&Iarg2);
            amode2=rplPolarComplexMode(arg2);
            cclass2=rplComplexClass(arg2);

            rplDropData(2);
        }

        switch(OPCODE(CurOpcode))
        {
        case OVR_SUB:
          {
            if(cclass2&CPLX_POLAR) {
                REAL pi;
                // ADD 180 DEGREES
                switch(amode2)
                {
                case ANGLEDEG:
                    decconst_180(&pi);
                    break;
                case ANGLEDMS:
                    decconst_180(&pi);
                    pi.flags|=Iarg2.flags&F_NEGATIVE;
                    break;
                case ANGLEGRAD:
                    decconst_200(&pi);
                    break;
                case ANGLERAD:
                    decconst_PI(&pi);
                    break;
                }
                addReal(&RReg[8],&Iarg2,&pi);
                cloneReal(&Iarg2,&RReg[8]);
                // NO NEED FOR REDUCTION TO +/-PI HERE
                }
            else {
             Rarg2.flags^=F_NEGATIVE;
             Iarg2.flags^=F_NEGATIVE;
            }
              // DELIBERATE FALL THROUGH ADDITION CASE
          }
        case OVR_ADD:
        {


            if( ((cclass1|cclass2)&~CPLX_POLAR)!=CPLX_NORMAL) {
                //  WE HAVE SOME SPECIALS HERE!

                switch(cclass1)
                {
                case CPLX_ZERO:
                    // 0 +/- ANYTHING = ANYTHING
                    rplPushData(arg2);
                    if(OPCODE(CurOpcode)==OVR_SUB) rplCallOvrOperator(CMD_OVR_NEG);
                    else {
                    rplCheckResultAndError(&Rarg2);
                    rplCheckResultAndError(&Iarg2);
                    }
                    return;
                case CPLX_UNDINF:
                {
                    switch(cclass2)
                    {
                    case CPLX_NORMAL:
                    case CPLX_ZERO:
                    case CPLX_NORMAL|CPLX_POLAR:
                    // UNDINF +/- NORMAL = UNDINF
                        rplUndInfinityToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;
                    case CPLX_INF:
                    case CPLX_INF|CPLX_MALFORMED:
                    case CPLX_INF|CPLX_POLAR:
                    case CPLX_UNDINF:
                    default:
                        // UNDINF +/- ANY OTHER INFINITY = NAN
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;

                    }
                }
                case CPLX_INF:
                {
                    switch(cclass2)
                    {
                    case CPLX_NORMAL:
                    case CPLX_ZERO:
                    case CPLX_NORMAL|CPLX_POLAR:
                        // Inf +/- finite = ANYTHING
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    case CPLX_INF:
                        // Inf+Inf = Inf, -Inf-Inf = -Inf
                        if((Rarg1.flags^Rarg2.flags)&F_NEGATIVE) {
                            // ADDING INFINITES OF OPPOSITE SIGNS IS UNDETERMINED
                            rplNANToRReg(0);
                            rplNewRealFromRRegPush(0);
                            rplCheckResultAndError(&RReg[0]);
                            return;
                        }
                        // BOTH HAVE SAME SIGN, SO RETURN THE FIRST ARGUMENT
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    case CPLX_INF|CPLX_POLAR:
                    {
                        // ADDITION ONLY MAKES SENSE IF THEIR DIRECTIONS MATCH
                        // OTHERWISE IT'S UNDEFINED
                        BINT right_direction=0;

                        if(Rarg1.flags&F_NEGATIVE) {
                            // CHECK IF THE POLAR IS GOING IN THE PI DIRECTION

                            switch(amode2)
                            {
                            case ANGLEDMS:
                            case ANGLEDEG:
                            {
                                REAL a_180;
                                decconst_180(&a_180);
                                Iarg2.flags&=~F_NEGATIVE;   // 180 or -180 IS THE SAME DIRECTION
                                if(eqReal(&Iarg2,&a_180)) right_direction=1;
                                break;
                            }
                            case ANGLEGRAD:
                            {
                                REAL a_200;
                                decconst_200(&a_200);
                                Iarg2.flags&=~F_NEGATIVE;   // 180 or -180 IS THE SAME DIRECTION
                                if(eqReal(&Iarg2,&a_200)) right_direction=1;
                                break;
                            }
                            case ANGLERAD:
                            default:
                                // DIRECTION CAN NEVER BE EXACTLY PI, SO IT ALWAYS FAILS
                                break;

                            }
                        }
                        else {
                            if(iszeroReal(&Iarg2)) right_direction=1;
                        }

                        if(right_direction) {
                            // BOTH HAVE SAME SIGN, SO RETURN THE FIRST ARGUMENT
                            rplPushData(arg1);
                            rplCheckResultAndError(&Rarg1);
                            rplCheckResultAndError(&Iarg1);
                            return;
                        }
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;
                    }

                    case CPLX_INF|CPLX_MALFORMED:   // PERP. DIRECTIONS
                    case CPLX_NAN:
                    case CPLX_UNDINF:
                    default:
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;

                    }



                }
                case CPLX_INF|CPLX_POLAR:
                {
                    switch(cclass2)
                    {
                    case CPLX_NORMAL:
                    case CPLX_ZERO:
                    case CPLX_NORMAL|CPLX_POLAR:
                        // Inf +/- finite = ANYTHING
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    case CPLX_INF:
                    {
                        // ADDITION ONLY MAKES SENSE IF THEIR DIRECTIONS MATCH
                        // OTHERWISE IT'S UNDEFINED
                        BINT right_direction=0;

                        // CHECK IF THE POLAR IS GOING IN THE PI DIRECTION
                        if(Rarg2.flags&F_NEGATIVE) {

                        switch(amode1)
                        {
                        case ANGLEDMS:
                        case ANGLEDEG:
                        {
                            REAL a_180;
                            decconst_180(&a_180);
                            Iarg1.flags&=~F_NEGATIVE;   // 180 or -180 IS THE SAME DIRECTION
                            if(eqReal(&Iarg1,&a_180)) right_direction=1;
                            break;
                        }
                        case ANGLEGRAD:
                        {
                            REAL a_200;
                            decconst_200(&a_200);
                            Iarg1.flags&=~F_NEGATIVE;   // 180 or -180 IS THE SAME DIRECTION
                            if(eqReal(&Iarg1,&a_200)) right_direction=1;
                            break;
                        }
                        case ANGLERAD:
                        default:
                            // DIRECTION CAN NEVER BE EXACTLY PI, SO IT ALWAYS FAILS
                            break;

                        }
                    }
                    else {
                        if(iszeroReal(&Iarg1)) right_direction=1;
                    }

                    if(right_direction) {
                        // BOTH HAVE SAME SIGN, SO RETURN THE FIRST ARGUMENT
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    }
                    rplNANToRReg(0);
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;
                    }
                    case CPLX_INF|CPLX_POLAR:
                    {
                        // ADDITION ONLY MAKES SENSE IF THEIR DIRECTIONS MATCH
                        // OTHERWISE IT'S UNDEFINED

                        trig_convertangle(&Iarg2,amode2,amode1);
                        finalize(&RReg[0]);

                        if(eqReal(&Iarg1,&RReg[0])) {
                            // BOTH HAVE SAME DIRECTION, SO RETURN THE FIRST ARGUMENT
                            rplPushData(arg1);
                            rplCheckResultAndError(&Rarg1);
                            rplCheckResultAndError(&Iarg1);
                            return;
                        }
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;
                    }
                    case CPLX_INF|CPLX_MALFORMED:
                    {
                        // ADDITION ONLY MAKES SENSE IF THEIR DIRECTIONS MATCH
                        // OTHERWISE IT'S UNDEFINED
                        BINT right_direction=0;

                        switch(amode1)
                        {
                        case ANGLEDMS:
                        case ANGLEDEG:
                        {
                            REAL a_90;
                            decconst_90(&a_90);
                            a_90.flags|=Iarg2.flags&F_NEGATIVE; // GET THE DIRECTION +/-i
                            if(eqReal(&Iarg1,&a_90)) right_direction=1;
                            break;
                        }
                        case ANGLEGRAD:
                        {
                            REAL a_100;
                            decconst_100(&a_100);
                            a_100.flags|=Iarg2.flags&F_NEGATIVE; // GET THE DIRECTION +/-i
                            if(eqReal(&Iarg1,&a_100)) right_direction=1;
                            break;
                        }
                        case ANGLERAD:
                        default:
                            // DIRECTION CAN NEVER BE EXACTLY +/-PI/2, SO IT ALWAYS FAILS
                            break;

                        }

                    if(right_direction) {
                        // BOTH HAVE SAME SIGN, SO RETURN THE FIRST ARGUMENT
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    }
                    rplNANToRReg(0);
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;
                    }

                    case CPLX_NAN:
                    case CPLX_UNDINF:
                    default:
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;

                    }
                }
                case CPLX_INF|CPLX_MALFORMED:
                {
                    switch(cclass2)
                    {
                    case CPLX_NORMAL:
                    case CPLX_ZERO:
                    case CPLX_NORMAL|CPLX_POLAR:
                        // Inf +/- finite = ANYTHING
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    case CPLX_INF|CPLX_MALFORMED:
                        // Inf+Inf = Inf, -Inf-Inf = -Inf
                        if((Iarg1.flags^Iarg2.flags)&F_NEGATIVE) {
                            // ADDING INFINITES OF OPPOSITE SIGNS IS UNDETERMINED
                            rplNANToRReg(0);
                            rplNewRealFromRRegPush(0);
                            rplCheckResultAndError(&RReg[0]);
                            return;
                        }
                        // BOTH HAVE SAME SIGN, SO RETURN THE FIRST ARGUMENT
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    case CPLX_INF|CPLX_POLAR:
                    {
                        // ADDITION ONLY MAKES SENSE IF THEIR DIRECTIONS MATCH
                        // OTHERWISE IT'S UNDEFINED
                        BINT right_direction=0;

                        switch(amode2)
                        {
                        case ANGLEDMS:
                        case ANGLEDEG:
                        {
                            REAL a_90;
                            decconst_90(&a_90);
                            a_90.flags|=Iarg1.flags&F_NEGATIVE; // GET THE DIRECTION +/-i
                            if(eqReal(&Iarg2,&a_90)) right_direction=1;
                            break;
                        }
                        case ANGLEGRAD:
                        {
                            REAL a_100;
                            decconst_100(&a_100);
                            a_100.flags|=Iarg1.flags&F_NEGATIVE; // GET THE DIRECTION +/-i
                            if(eqReal(&Iarg2,&a_100)) right_direction=1;
                            break;
                        }
                        case ANGLERAD:
                        default:
                            // DIRECTION CAN NEVER BE EXACTLY +/-PI/2, SO IT ALWAYS FAILS
                            break;

                        }

                    if(right_direction) {
                        // BOTH HAVE SAME SIGN, SO RETURN THE FIRST ARGUMENT
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    }
                    rplNANToRReg(0);
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;
                    }
                    case CPLX_INF:   // PERP. DIRECTIONS
                    case CPLX_NAN:
                    case CPLX_UNDINF:
                    default:
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;

                    }



                }

                case CPLX_NORMAL:
                case CPLX_NORMAL|CPLX_POLAR:
                {
                    switch(cclass2)
                    {
                    case CPLX_ZERO:
                        // FINITE +/- 0 = FINITE
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;

                    case CPLX_INF:
                    case CPLX_INF|CPLX_POLAR:
                    case CPLX_INF|CPLX_MALFORMED:

                    // FINITE +/- INFINITY = INFINITY
                        rplPushData(arg2);
                        if(OPCODE(CurOpcode)==OVR_SUB) rplCallOvrOperator(CMD_OVR_NEG);
                        else {
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        }
                        return;

                    case CPLX_UNDINF:
                        // FINITE +/- UNDINF = UNDINF
                        rplUndInfinityToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;

                    case CPLX_NAN:

                    default:
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;

                    }



                }

                default:
                case CPLX_NAN:
                    // NAN +/- ANYTHING = NAN
                    rplNANToRReg(0);
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;

                }


            } // END OF SPECIALS HANDLING



            if(amode1==ANGLENONE) {
                if(amode2==ANGLENONE) {
                    // ADD THE REAL PART FIRST
                    addReal(&RReg[0],&Rarg1,&Rarg2);
                    addReal(&RReg[1],&Iarg1,&Iarg2);
                    rplRRegToComplexPush(0,1,ANGLENONE);

                    rplCheckResultAndError(&RReg[0]);
                    rplCheckResultAndError(&RReg[1]);

                    return;
                }
                // CONVERT BOTH ARGUMENTS TO CARTESIAN, THEN ADD
                rplPolar2Rect(&Rarg2,&Iarg2,amode2);
                addReal(&RReg[2],&Rarg1,&RReg[0]);
                addReal(&RReg[3],&Iarg1,&RReg[1]);

                // RESULT IN CARTESIAN IS OK
                rplRRegToComplexPush(2,3,ANGLENONE);
                rplCheckResultAndError(&RReg[2]);
                rplCheckResultAndError(&RReg[3]);

                return;


            }

            if(amode2!=ANGLENONE) {
                // CONVERT SECOND ARGUMENT TO CARTESIAN

                // WARNING: Iarg2 COULD BE RReg[8] WHEN SUBTRACTING
                rplPolar2Rect(&Rarg2,&Iarg2,amode2);

                swapReal(&RReg[8],&RReg[0]);    // SAVE TO HIGHER REGISTERS
                swapReal(&RReg[9],&RReg[1]);

                // CONVERT FIRST ARGUMENT TO CARTESIAN
                rplPolar2Rect(&Rarg1,&Iarg1,amode1);

                addReal(&RReg[3],&RReg[8],&RReg[0]);
                addReal(&RReg[4],&RReg[9],&RReg[1]);

            }
            else {
                // CONVERT FIRST ARGUMENT TO CARTESIAN
                rplPolar2Rect(&Rarg1,&Iarg1,amode1);

                // ADD DIRECTLY
                addReal(&RReg[3],&RReg[0],&Rarg2);
                addReal(&RReg[4],&RReg[1],&Iarg2);
                
            }

            // HERE WE HAVE THE RESULT IN CARTESIAN COORDINATES IN RReg 3 AND 4
            
            rplRect2Polar(&RReg[3],&RReg[4],amode1);

            rplRRegToComplexPush(0,1,amode1);
            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[1]);


            return;

        }


        case OVR_MUL:
        {


            if( ((cclass1|cclass2)&~CPLX_POLAR)!=CPLX_NORMAL) {
                //  WE HAVE SOME SPECIALS HERE!

                switch(cclass1)
                {
                case CPLX_ZERO:
                    // 0 * FINITE = 0, OTHERWISE UNDEFINED
                    switch(cclass2)
                    {
                    case CPLX_ZERO:
                    case CPLX_NORMAL:
                    case CPLX_NORMAL|CPLX_POLAR:
                        rplPushData(zero_bint);
                        return;
                    case CPLX_INF:
                    case CPLX_INF|CPLX_POLAR:
                    case CPLX_INF|CPLX_MALFORMED:
                    case CPLX_UNDINF:
                    case CPLX_NAN:
                    default:
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;
                    }
                case CPLX_UNDINF:
                {
                    switch(cclass2)
                    {
                    case CPLX_NORMAL:
                    case CPLX_NORMAL|CPLX_POLAR:
                    case CPLX_INF:
                    case CPLX_INF|CPLX_MALFORMED:
                    case CPLX_INF|CPLX_POLAR:
                    case CPLX_UNDINF:

                    // UNDINF +/- NORMAL = UNDINF
                        rplUndInfinityToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;
                    case CPLX_ZERO:
                    case CPLX_NAN:
                    default:
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;

                    }
                }
                case CPLX_INF:
                {
                    switch(cclass2)
                    {
                    case CPLX_NORMAL:
                    {
                        BINT resmode;

                        if(amode1!=ANGLENONE) resmode=amode1;
                        else if(amode2!=ANGLENONE) resmode=amode2;
                        else resmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);


                        // INF * (A,B) = DIRECTED INFINITY
                        // CONVERT TO POLAR AND RETURN DIRECTED INFINITY
                        // Z = r1*r2 * e^(i*(Theta1+Theta2)) = Inf * e^i*(Theta1+Theta2)

                        if(Rarg1.flags&F_NEGATIVE) {
                            // -Inf --> INVERT DIRECTION
                            Iarg2.flags^=F_NEGATIVE;
                            Rarg2.flags^=F_NEGATIVE;
                        }
                        trig_atan2(&Iarg2,&Rarg2,resmode);
                        finalize(&RReg[0]);


                        // HERE RReg[0] = RESULTING ANGLE
                        // RETURN DIRECTED INFINITY

                        rplNewComplexPush(&Rarg1,&RReg[0],amode2);

                        rplCheckResultAndError(&RReg[0]);
                        rplCheckResultAndError(&Rarg1);
                        return;
                    }



                    case CPLX_NORMAL|CPLX_POLAR:
                       {
                        // Inf * r*e^i*Theta

                        // RETURN DIRECTED INFINITY

                        if(Rarg1.flags&F_NEGATIVE) {
                            // ADD pi TO THE ANGLE
                            REAL pi;
                            // ADD 180 DEGREES
                            switch(amode2)
                            {
                            case ANGLEDEG:
                                decconst_180(&pi);
                                break;
                            case ANGLEDMS:
                                decconst_180(&pi);
                                pi.flags|=Iarg2.flags&F_NEGATIVE;
                                break;
                            case ANGLEGRAD:
                                decconst_200(&pi);
                                break;
                            case ANGLERAD:
                                decconst_PI(&pi);
                                break;
                            }
                            addReal(&RReg[9],&Iarg2,&pi);
                            // NEED REDUCTION TO +/-PI HERE
                            trig_reduceangle(&RReg[9],amode2);
                        }
                        else copyReal(&RReg[0],&Iarg2);

                        Rarg1.flags&=~F_NEGATIVE;   // MAKE SURE INFINITY IS POSITIVE
                        rplNewComplexPush(&Rarg1,&RReg[0],amode2);

                        rplCheckResultAndError(&RReg[0]);
                        rplCheckResultAndError(&Rarg1);
                        return;

                    }


                    case CPLX_INF:
                    {
                        BINT sign=(Rarg1.flags^Rarg2.flags)&F_NEGATIVE;

                        Rarg1.flags=(Rarg1.flags&~F_NEGATIVE)|sign;
                        rplNewRealPush(&Rarg1);
                        rplCheckResultAndError(&Rarg1);
                        return;
                    }


                    case CPLX_INF|CPLX_POLAR:
                    {
                        // Inf * Inf *e^i*Theta

                        // RETURN DIRECTED INFINITY

                        if(Rarg1.flags&F_NEGATIVE) {
                            // ADD pi TO THE ANGLE
                            REAL pi;
                            // ADD 180 DEGREES
                            switch(amode2)
                            {
                            case ANGLEDEG:
                                decconst_180(&pi);
                                break;
                            case ANGLEDMS:
                                decconst_180(&pi);
                                pi.flags|=Iarg2.flags&F_NEGATIVE;
                                break;
                            case ANGLEGRAD:
                                decconst_200(&pi);
                                break;
                            case ANGLERAD:
                                decconst_PI(&pi);
                                break;
                            }
                            addReal(&RReg[9],&Iarg2,&pi);
                            // NEED REDUCTION TO +/-PI HERE
                            trig_reduceangle(&RReg[9],amode2);
                        }
                        else copyReal(&RReg[0],&Iarg2);

                        rplNewComplexPush(&Rarg2,&RReg[0],amode2);

                        rplCheckResultAndError(&RReg[0]);
                        rplCheckResultAndError(&Rarg2);
                        return;

                    }


                    case CPLX_UNDINF:
                        // UNDINF * INF = UNDINF
                            rplUndInfinityToRReg(0);
                            rplNewRealFromRRegPush(0);
                            rplCheckResultAndError(&RReg[0]);
                            return;



                    case CPLX_INF|CPLX_MALFORMED:
                    {
                        // Inf * i * Inf = i* Inf


                        // RETURN DIRECTED INFINITY
                        REAL pi2;
                        BINT amode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);
                        switch(amode)
                        {
                        case ANGLEDEG:
                        case ANGLEDMS:
                            decconst_90(&pi2);
                            break;
                        case ANGLEGRAD:
                            decconst_100(&pi2);
                            break;
                        case ANGLERAD:
                            decconst_PI_2(&pi2);
                            break;
                        }
                        pi2.flags|=(Rarg1.flags^Iarg2.flags)&F_NEGATIVE;

                        Rarg1.flags&=F_NEGATIVE;

                        rplNewComplexPush(&Rarg1,&pi2,amode);

                        rplCheckResultAndError(&Rarg1);
                        return;



                    }

                    case CPLX_ZERO:
                    case CPLX_NAN:
                    default:
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;

                    }



                }

                    // *** DONE UP TO HERE ***


                case CPLX_INF|CPLX_POLAR:
                {
                    switch(cclass2)
                    {
                    case CPLX_NORMAL:
                    case CPLX_ZERO:
                    case CPLX_NORMAL|CPLX_POLAR:
                        // Inf +/- finite = ANYTHING
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    case CPLX_INF:
                    {
                        // ADDITION ONLY MAKES SENSE IF THEIR DIRECTIONS MATCH
                        // OTHERWISE IT'S UNDEFINED
                        BINT right_direction=0;

                        // CHECK IF THE POLAR IS GOING IN THE PI DIRECTION
                        if(Rarg2.flags&F_NEGATIVE) {

                        switch(amode1)
                        {
                        case ANGLEDMS:
                        case ANGLEDEG:
                        {
                            REAL a_180;
                            decconst_180(&a_180);
                            Iarg1.flags&=~F_NEGATIVE;   // 180 or -180 IS THE SAME DIRECTION
                            if(eqReal(&Iarg1,&a_180)) right_direction=1;
                            break;
                        }
                        case ANGLEGRAD:
                        {
                            REAL a_200;
                            decconst_200(&a_200);
                            Iarg1.flags&=~F_NEGATIVE;   // 180 or -180 IS THE SAME DIRECTION
                            if(eqReal(&Iarg1,&a_200)) right_direction=1;
                            break;
                        }
                        case ANGLERAD:
                        default:
                            // DIRECTION CAN NEVER BE EXACTLY PI, SO IT ALWAYS FAILS
                            break;

                        }
                    }
                    else {
                        if(iszeroReal(&Iarg1)) right_direction=1;
                    }

                    if(right_direction) {
                        // BOTH HAVE SAME SIGN, SO RETURN THE FIRST ARGUMENT
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    }
                    rplNANToRReg(0);
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;
                    }
                    case CPLX_INF|CPLX_POLAR:
                    {
                        // ADDITION ONLY MAKES SENSE IF THEIR DIRECTIONS MATCH
                        // OTHERWISE IT'S UNDEFINED

                        trig_convertangle(&Iarg2,amode2,amode1);
                        finalize(&RReg[0]);

                        if(eqReal(&Iarg1,&RReg[0])) {
                            // BOTH HAVE SAME DIRECTION, SO RETURN THE FIRST ARGUMENT
                            rplPushData(arg1);
                            rplCheckResultAndError(&Rarg1);
                            rplCheckResultAndError(&Iarg1);
                            return;
                        }
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;
                    }
                    case CPLX_INF|CPLX_MALFORMED:
                    {
                        // ADDITION ONLY MAKES SENSE IF THEIR DIRECTIONS MATCH
                        // OTHERWISE IT'S UNDEFINED
                        BINT right_direction=0;

                        switch(amode1)
                        {
                        case ANGLEDMS:
                        case ANGLEDEG:
                        {
                            REAL a_90;
                            decconst_90(&a_90);
                            a_90.flags|=Iarg2.flags&F_NEGATIVE; // GET THE DIRECTION +/-i
                            if(eqReal(&Iarg1,&a_90)) right_direction=1;
                            break;
                        }
                        case ANGLEGRAD:
                        {
                            REAL a_100;
                            decconst_100(&a_100);
                            a_100.flags|=Iarg2.flags&F_NEGATIVE; // GET THE DIRECTION +/-i
                            if(eqReal(&Iarg1,&a_100)) right_direction=1;
                            break;
                        }
                        case ANGLERAD:
                        default:
                            // DIRECTION CAN NEVER BE EXACTLY +/-PI/2, SO IT ALWAYS FAILS
                            break;

                        }

                    if(right_direction) {
                        // BOTH HAVE SAME SIGN, SO RETURN THE FIRST ARGUMENT
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    }
                    rplNANToRReg(0);
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;
                    }

                    case CPLX_NAN:
                    case CPLX_UNDINF:
                    default:
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;

                    }
                }
                case CPLX_INF|CPLX_MALFORMED:
                {
                    switch(cclass2)
                    {
                    case CPLX_NORMAL:
                    case CPLX_ZERO:
                    case CPLX_NORMAL|CPLX_POLAR:
                        // Inf +/- finite = ANYTHING
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    case CPLX_INF|CPLX_MALFORMED:
                        // Inf+Inf = Inf, -Inf-Inf = -Inf
                        if((Iarg1.flags^Iarg2.flags)&F_NEGATIVE) {
                            // ADDING INFINITES OF OPPOSITE SIGNS IS UNDETERMINED
                            rplNANToRReg(0);
                            rplNewRealFromRRegPush(0);
                            rplCheckResultAndError(&RReg[0]);
                            return;
                        }
                        // BOTH HAVE SAME SIGN, SO RETURN THE FIRST ARGUMENT
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    case CPLX_INF|CPLX_POLAR:
                    {
                        // ADDITION ONLY MAKES SENSE IF THEIR DIRECTIONS MATCH
                        // OTHERWISE IT'S UNDEFINED
                        BINT right_direction=0;

                        switch(amode2)
                        {
                        case ANGLEDMS:
                        case ANGLEDEG:
                        {
                            REAL a_90;
                            decconst_90(&a_90);
                            a_90.flags|=Iarg1.flags&F_NEGATIVE; // GET THE DIRECTION +/-i
                            if(eqReal(&Iarg2,&a_90)) right_direction=1;
                            break;
                        }
                        case ANGLEGRAD:
                        {
                            REAL a_100;
                            decconst_100(&a_100);
                            a_100.flags|=Iarg1.flags&F_NEGATIVE; // GET THE DIRECTION +/-i
                            if(eqReal(&Iarg2,&a_100)) right_direction=1;
                            break;
                        }
                        case ANGLERAD:
                        default:
                            // DIRECTION CAN NEVER BE EXACTLY +/-PI/2, SO IT ALWAYS FAILS
                            break;

                        }

                    if(right_direction) {
                        // BOTH HAVE SAME SIGN, SO RETURN THE FIRST ARGUMENT
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;
                    }
                    rplNANToRReg(0);
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;
                    }
                    case CPLX_INF:   // PERP. DIRECTIONS
                    case CPLX_NAN:
                    case CPLX_UNDINF:
                    default:
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;

                    }



                }

                case CPLX_NORMAL:
                case CPLX_NORMAL|CPLX_POLAR:
                {
                    switch(cclass2)
                    {
                    case CPLX_ZERO:
                        // FINITE +/- 0 = FINITE
                        rplPushData(arg1);
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        return;

                    case CPLX_INF:
                    case CPLX_INF|CPLX_POLAR:
                    case CPLX_INF|CPLX_MALFORMED:

                    // FINITE +/- INFINITY = INFINITY
                        rplPushData(arg2);
                        if(OPCODE(CurOpcode)==OVR_SUB) rplCallOvrOperator(CMD_OVR_NEG);
                        else {
                        rplCheckResultAndError(&Rarg1);
                        rplCheckResultAndError(&Iarg1);
                        }
                        return;

                    case CPLX_UNDINF:
                        // FINITE +/- UNDINF = UNDINF
                        rplUndInfinityToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;

                    case CPLX_NAN:

                    default:
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;

                    }



                }

                default:
                case CPLX_NAN:
                    // NAN +/- ANYTHING = NAN
                    rplNANToRReg(0);
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;

                }


            } // END OF SPECIALS HANDLING


            /*
            // OLD SPECIALS HANDLING HERE - OBSOLETE
            if(cclass1==CPLX_) {

                if(isundinfiniteReal(&Rarg1) || isinfiniteReal(&Iarg1)) {
                    // UNDEFINED INFINITY OR ANOTHER MALFORMED COMPLEX
                    // TREAT (Inf,Inf) = UndInf

                    if(iszeroReal(&Rarg2)&&iszeroReal(&Iarg2)) {
                        // UNDINF * 0 = UNDEFINED
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;
                    }

                    // UNDINF * ANYTHING ELSE = UNDINF

                    rplUndInfinityToRReg(0);
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;
                }

                // TREAT AS REAL +/-INFINITY SINCE DIRECTED INFINITY IS ALWAYS IN POLAR FORM

                // HANDLE SPECIAL CASES
                if(iszeroReal(&Rarg2)&&iszeroReal(&Iarg2)) {
                    // INFINITE * 0 = UNDEFINED
                    rplNANToRReg(0);
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;
                }

                if(isinfiniteReal(&Rarg2)) {
                    if(isundinfiniteReal(&Rarg2) || isinfiniteReal(&Iarg2)) {
                        // MALFORMED COMPLEX
                        // TREAT AS UNDIRECTED INFINITY

                        // UNDINF * INF = UNDINF
                        rplUndInfinityToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;
                    }
                    // TREAT AS REAL INFINITY

                    // INF * INF = INF
                    rplInfinityToRReg(0);
                    RReg[0].flags|=(Rarg1.flags^Rarg2.flags)&F_NEGATIVE;
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;

                }

                // INF * (A,B) = DIRECTED INFINITY


                // CONVERT TO POLAR AND RETURN DIRECTED INFINITY

                // Z = r1*r2 * e^(i*(Theta1+Theta2)) = Inf * e^i*(Theta1+Theta2)

                amode2=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

                trig_atan2(&Iarg2,&Rarg2,amode2);
                finalize(&RReg[0]);


                // HERE RReg[0] = RESULTING ANGLE
                // RETURN DIRECTED INFINITY

                rplNewComplexPush(&Rarg1,&RReg[0],amode2);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&Rarg1);
                return;



            }


            // SPECIALS HANDLING FOR SECOND ARGUMENT
            else if(isinfiniteReal(&Rarg2)) {

                if(isundinfiniteReal(&Rarg2) || isinfiniteReal(&Iarg2)) {
                    // UNDEFINED INFINITY OR ANOTHER MALFORMED COMPLEX
                    // TREAT (Inf,Inf) = UndInf

                    if(iszeroReal(&Rarg1)&&iszeroReal(&Iarg1)) {
                        // UNDINF * 0 = UNDEFINED
                        rplNANToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;
                    }

                    // UNDINF * ANYTHING ELSE = UNDINF

                    rplUndInfinityToRReg(0);
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;
                }

                // IT'S A MALFORMED NUMBER, INFINITE COMPLEX
                // SHOULD ALWAYS BE POLAR
                // IN ANY CASE, TREAT AS REAL +/-INFINITY

                // HANDLE SPECIAL CASES
                if(iszeroReal(&Rarg1)&&iszeroReal(&Iarg1)) {
                    // INFINITE * 0 = UNDEFINED
                    rplNANToRReg(0);
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;
                }

                if(isinfiniteReal(&Rarg1)) {
                    if(isinfiniteReal(&Iarg1)) {
                        // MALFORMED COMPLEX
                        // TREAT AS UNDIRECTED INFINITY

                        // UNDINF * INF = UNDINF
                        rplUndInfinityToRReg(0);
                        rplNewRealFromRRegPush(0);
                        rplCheckResultAndError(&RReg[0]);
                        return;
                    }
                    // TREAT AS REAL INFINITY
                    // INF * INF = +/-INF

                    rplInfinityToRReg(0);
                    RReg[0].flags|=(Rarg1.flags^Rarg2.flags)&F_NEGATIVE;
                    rplNewRealFromRRegPush(0);
                    rplCheckResultAndError(&RReg[0]);
                    return;


                }

                // INF * (A,B) = DIRECTED INFINITY


                // CONVERT TO POLAR AND RETURN DIRECTED INFINITY

                // Z = r1*r2 * e^(i*(Theta1+Theta2)) = Inf * e^i*(Theta1+Theta2)

                amode2=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

                trig_atan2(&Iarg1,&Rarg1,amode2);
                finalize(&RReg[0]);


                // HERE RReg[0] = RESULTING ANGLE
                // RETURN DIRECTED INFINITY

                rplNewComplexPush(&Rarg2,&RReg[0],amode2);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&Rarg2);
                return;




            }

                    // Rarg2 IS A POLAR COMPLEX
                    if(isinfiniteReal(&Rarg2)) {
                        // IF IT'S A POLAR INFINITY, DON'T TRY TO WORK IN CARTESIAN COORDINATES

                        if(isNANorinfiniteReal(&Iarg2)) {
                            // MALFORMED POLAR COMPLEX NUMBER
                            rplNANToRReg(0);
                            rplNewRealFromRRegPush(0);
                            rplCheckResultAndError(&RReg[0]);
                            return;

                        }

                        // HANDLE SPECIAL CASES WHEN FIRST ARGUMENT IS INFINITY TOO
                        if(isinfiniteReal(&Rarg1)) {

                            if(isundinfiniteReal(&Rarg1) || isinfiniteReal(&Iarg1)) {
                                // UNDEFINED INFINITY OR ANOTHER MALFORMED COMPLEX
                                // TREAT (Inf,Inf) = UndInf


                                // UNDINF * ANYTHING ELSE = UNDINF

                                rplUndInfinityToRReg(0);
                                rplNewRealFromRRegPush(0);
                                rplCheckResultAndError(&RReg[0]);
                                return;
                            }

                            // FIRST ARGUMENT IS REAL +/-INFINITY

                            // SECOND ARGUMENT IS DIRECTED INFINITY


                            // CONVERT TO POLAR AND RETURN DIRECTED INFINITY


                        }


                        // CONVERT TO POLAR AND RETURN DIRECTED INFINITY

                        // Z = r1*r2 * e^(i*(Theta1+Theta2)) = Inf * e^i*(Theta1+Theta2)

                        trig_atan2(&Iarg1,&Rarg1,amode2);

                        normalize(&RReg[0]);
                        swapReal(&RReg[8],&RReg[0]);

                        // ADD ANGLES, BE CAREFUL WITH THE DMS MODE
                        if(amode2==ANGLEDMS) {
                            // ADDING IN DMS NEEDS SPECIAL TREATMENT
                            trig_convertangle(&RReg[8],amode2,ANGLEDEG);
                            swapReal(&RReg[0],&RReg[6]);
                            trig_convertangle(&Iarg2,amode2,ANGLEDEG);
                            addReal(&RReg[7],&RReg[6],&RReg[0]);
                            trig_reduceangle(&RReg[7],ANGLEDEG);
                            swapReal(&RReg[7],&RReg[0]);
                            trig_convertangle(&RReg[7],ANGLEDEG,amode2);
                        }
                        else {
                            addReal(&RReg[7],&RReg[8],&Iarg2);
                            trig_reduceangle(&RReg[7],amode2);
                        }

                        // HERE RReg[0] = RESULTING ANGLE
                        // RETURN DIRECTED INFINITY

                        rplNewComplexPush(&Rarg2,&RReg[0],amode2);

                        rplCheckResultAndError(&RReg[0]);
                        rplCheckResultAndError(&Rarg2);
                        return;

                    }

            */





            if(amode1==ANGLENONE) {
                // RESULT WILL BE IN CARTESIAN COORDINATES
                if(amode2==ANGLENONE) {
                    // WORK IN CARTESIAN COORDINATES

                    // FINALLY, DO THE NORMAL MULTIPLICATION

                    Context.precdigits+=8;
                    mulReal(&RReg[0],&Rarg1,&Rarg2);
                    mulReal(&RReg[1],&Iarg1,&Iarg2);
                    subReal(&RReg[2],&RReg[0],&RReg[1]);
                    mulReal(&RReg[0],&Rarg1,&Iarg2);
                    mulReal(&RReg[1],&Iarg1,&Rarg2);
                    Context.precdigits-=8;
                    addReal(&RReg[3],&RReg[0],&RReg[1]);
                    finalize(&RReg[2]);
                    rplRRegToComplexPush(2,3,ANGLENONE);
                    rplCheckResultAndError(&RReg[0]);
                    rplCheckResultAndError(&RReg[1]);
                    return;
                }

                else {




                    // CONVERT TO CARTESIAN COORDINATES
                    rplPolar2Rect(&Rarg2,&Iarg2,amode2);

                    // THEN DO IT ALL IN CARTESIAN
                    swapReal(&RReg[8],&RReg[0]);
                    swapReal(&RReg[9],&RReg[1]);

                    Context.precdigits+=8;
                    mulReal(&RReg[0],&Rarg1,&RReg[8]);
                    mulReal(&RReg[1],&Iarg1,&RReg[9]);
                    subReal(&RReg[2],&RReg[0],&RReg[1]);
                    mulReal(&RReg[0],&Rarg1,&RReg[9]);
                    mulReal(&RReg[1],&Iarg1,&RReg[8]);
                    Context.precdigits-=8;
                    addReal(&RReg[3],&RReg[0],&RReg[1]);
                    finalize(&RReg[2]);
                    rplRRegToComplexPush(2,3,ANGLENONE);

                    rplCheckResultAndError(&RReg[0]);
                    rplCheckResultAndError(&RReg[1]);
                    return;

                }
            }

            // WORK DIRECTLY IN POLAR COORDINATES

            if(amode1!=ANGLEDMS) {
                trig_convertangle(&Iarg2,amode2,amode1);
                addReal(&RReg[1],&Iarg1,&RReg[0]);
                trig_reduceangle(&RReg[1],amode1);
                swapReal(&RReg[1],&RReg[0]);

            } else {
                // ADDING IN DMS NEEDS SPECIAL TREATMENT
                trig_convertangle(&Iarg1,amode1,ANGLEDEG);
                swapReal(&RReg[0],&RReg[6]);
                trig_convertangle(&Iarg2,amode2,ANGLEDEG);
                addReal(&RReg[7],&RReg[6],&RReg[0]);
                trig_reduceangle(&RReg[7],ANGLEDEG);
                swapReal(&RReg[7],&RReg[0]);
                trig_convertangle(&RReg[7],ANGLEDEG,amode1);
                swapReal(&RReg[0],&RReg[1]);
            }
            mulReal(&RReg[0],&Rarg1,&Rarg2);

            // CHECK FOR UNDINF/NAN MAGNITUDE
            if(isundinfiniteReal(&RReg[0])||isNANReal(&RReg[0])) {
                // RETURN WHATEVER IT IS IN RREG
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            }
            // CHECK FOR UNDIF/NAN ANGLE
            if(isNANorinfiniteReal(&RReg[1])) {

                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            }







            rplRRegToComplexPush(0,1,amode1);
            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[1]);
            return;

        }
        case OVR_DIV:
        {
            // CHECK FOR DIVIDE BY ZERO
            if(iszeroReal(&Rarg2)) {
                if(amode2!=ANGLENONE) {
                    // Z/(DIRECTED ZERO) = DIRECTED INFINITY

                    if(amode1==ANGLENONE) {
                        // CONVERT TO POLAR IN THE SAME SYSTEM AS THE ZERO
                        rplRect2Polar(&Rarg1,&Iarg1,amode2);
                        swapReal(&RReg[0],&RReg[8]);
                        swapReal(&RReg[1],&RReg[9]);
                        cloneReal(&Rarg1,&RReg[8]);
                        cloneReal(&Iarg1,&RReg[9]);
                        amode1=amode2;
                    }

                    if(amode1!=ANGLEDMS) {
                    trig_convertangle(&Iarg2,amode2,amode1);
                    swapReal(&RReg[0],&RReg[7]);
                    divReal(&RReg[2],&Rarg1,&Rarg2);
                    subReal(&RReg[1],&Iarg1,&RReg[7]);
                    trig_reduceangle(&RReg[1],amode1);
                    }
                    else {
                        // SUBTRACTING IN DMS NEEDS SPECIAL TREATMENT
                        trig_convertangle(&Iarg1,amode1,ANGLEDEG);
                        swapReal(&RReg[0],&RReg[6]);
                        trig_convertangle(&Iarg2,amode2,ANGLEDEG);
                        subReal(&RReg[7],&RReg[6],&RReg[0]);
                        trig_reduceangle(&RReg[7],amode1);
                        swapReal(&RReg[0],&RReg[7]);
                        trig_convertangle(&RReg[7],ANGLEDEG,amode1);
                        divReal(&RReg[2],&Rarg1,&Rarg2);
                    }

                    // HERE RReg[0]=ARGUMENT
                    // RReg[2]=INFINITE MAGNITUDE

                    if((RReg[2].flags&(F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW|F_ERROR))
                        || (RReg[0].flags&(F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW|F_ERROR)))
                    {
                        // IT'S UNDEFINED
                        rplNANToRReg(1);
                        rplNewRealFromRRegPush(1);
                        rplCheckResultAndError(&RReg[1]);
                        return;
                    }

                    // EVEN IF MAGNITUDE IS INFINITE, IT'S A DIRECTED INFINITY
                    rplRRegToComplexPush(2,0,amode1);

                    rplCheckResultAndError(&RReg[2]);
                    rplCheckResultAndError(&RReg[0]);

                    return;
                }
                else if(iszeroReal(&Iarg2)) {
                // Z/0 = UNDIRECTED INFINITY

                rplUndInfinityToRReg(0);
                rplNewRealFromRRegPush(0);

                rplCheckResultAndError(&RReg[0]);

                return;
                }
            }



            if(amode1==ANGLENONE) {
                if(amode2==ANGLENONE) {
                    // (a+b*i)/(c+d*i) = (a+b*i)*(c-d*i)/((c+d*i)*(c-d*i)) = (a*c+b*d)/(c^2+d^2) + (b*c-a*d)/(c^2+d^2)*i
                    Context.precdigits+=8;
                    mulReal(&RReg[0],&Rarg1,&Rarg2);
                    mulReal(&RReg[1],&Iarg1,&Iarg2);
                    addReal(&RReg[2],&RReg[0],&RReg[1]);
                    mulReal(&RReg[0],&Iarg1,&Rarg2);
                    mulReal(&RReg[1],&Rarg1,&Iarg2);
                    subReal(&RReg[3],&RReg[0],&RReg[1]);
                    mulReal(&RReg[0],&Rarg2,&Rarg2);
                    mulReal(&RReg[1],&Iarg2,&Iarg2);
                    addReal(&RReg[4],&RReg[0],&RReg[1]);

                    Context.precdigits-=8;

                    // TODO: CHECK FOR DIV BY ZERO AND ISSUE ERROR
                    divReal(&RReg[0],&RReg[2],&RReg[4]);
                    divReal(&RReg[1],&RReg[3],&RReg[4]);

                    rplRRegToComplexPush(0,1,ANGLENONE);
                    rplCheckResultAndError(&RReg[0]);
                    rplCheckResultAndError(&RReg[1]);


                    return;
                }
                else {
                    // CONVERT TO RECTANGULAR COORDINATES FIRST

                    rplPolar2Rect(&Rarg2,&Iarg2,amode2);

                    // (a+b*i)/(c+d*i) = (a+b*i)*(c-d*i)/((c+d*i)*(c-d*i)) = (a*c+b*d)/(c^2+d^2) + (b*c-a*d)/(c^2+d^2)*i
                    Context.precdigits+=8;
                    mulReal(&RReg[6],&Rarg1,&RReg[0]);
                    mulReal(&RReg[7],&Iarg1,&RReg[1]);
                    addReal(&RReg[2],&RReg[6],&RReg[7]);
                    mulReal(&RReg[6],&Iarg1,&RReg[0]);
                    mulReal(&RReg[7],&Rarg1,&RReg[1]);
                    subReal(&RReg[3],&RReg[6],&RReg[7]);
                    mulReal(&RReg[4],&Rarg2,&Rarg2);

                    Context.precdigits-=8;

                    divReal(&RReg[0],&RReg[2],&RReg[4]);
                    divReal(&RReg[1],&RReg[3],&RReg[4]);

                    rplRRegToComplexPush(0,1,ANGLENONE);

                    rplCheckResultAndError(&RReg[0]);
                    rplCheckResultAndError(&RReg[1]);


                    return;


                }
                }

            // RESULT WILL BE IN POLAR COORDINATES

            if(amode2==ANGLENONE) {
                rplRect2Polar(&Rarg2,&Iarg2,amode1);
                swapReal(&RReg[0],&RReg[6]);
                swapReal(&RReg[1],&RReg[7]);
                divReal(&RReg[2],&Rarg1,&RReg[6]);
                subReal(&RReg[1],&Iarg1,&RReg[7]);
                trig_reduceangle(&RReg[1],amode1);
            }
            else {
                if(amode1!=ANGLEDMS) {
                trig_convertangle(&Iarg2,amode2,amode1);
                swapReal(&RReg[0],&RReg[7]);
                divReal(&RReg[2],&Rarg1,&Rarg2);
                subReal(&RReg[1],&Iarg1,&RReg[7]);
                trig_reduceangle(&RReg[1],amode1);
                }
                else {
                    // SUBTRACTING IN DMS NEEDS SPECIAL TREATMENT
                    trig_convertangle(&Iarg1,amode1,ANGLEDEG);
                    swapReal(&RReg[0],&RReg[6]);
                    trig_convertangle(&Iarg2,amode2,ANGLEDEG);
                    subReal(&RReg[7],&RReg[6],&RReg[0]);
                    trig_reduceangle(&RReg[7],amode1);
                    swapReal(&RReg[0],&RReg[7]);
                    trig_convertangle(&RReg[7],ANGLEDEG,amode1);
                    divReal(&RReg[2],&Rarg1,&Rarg2);
                }
            }

            rplRRegToComplexPush(2,0,amode1);
            rplCheckResultAndError(&RReg[2]);
            rplCheckResultAndError(&RReg[0]);


            return;


        }

        case OVR_POW:
        {
            BINT resmode=amode1;
            if(amode1==ANGLENONE) {
                rplRect2Polar(&Rarg1,&Iarg1,ANGLERAD);
                amode1=ANGLERAD;
            }
            else {
                copyReal(&RReg[0],&Rarg1);
                copyReal(&RReg[1],&Iarg1);
            }


            // HERE WE HAVE 'Z' IN POLAR COORDINATES

            if( (amode2==ANGLENONE) && iszeroReal(&Iarg2)) {
                // REAL POWER OF A COMPLEX NUMEBR
                //Z^n= e^(ln(Z^n)) = e^(n*ln(Z)) = e^(n*[ln(r)+i*Theta)
                //Z^n= e^(n*ln(r))*e^(i*Theta*n) = r^n * e(i*Theta*n)
                //Z^n= r^n * cos(Theta*n) + i* r^n * sin(Theta*n)

                if(iszeroReal(&Rarg2)) {
                    // Z^0 = 1, UNLESS Z=0

                    if(iszeroReal(&RReg[0])) {
                        // 0^0 is undefined
                        rplError(ERR_UNDEFINEDRESULT);
                        return;
                    }

                    rplPushData((WORDPTR)one_bint);
                    return;

                }

                // RReg[9]=Theta
                swapReal(&RReg[9],&RReg[1]);

                // NOTE: powReal USES ALL RREGS FROM 0 TO 8, ONLY 9 IS PRESERVED
                // RReg[8]=r^n
                powReal(&RReg[8],&RReg[0],&Rarg2);

                mulReal(&RReg[2],&Rarg2,&RReg[9]);


                if(resmode==ANGLENONE) {

                trig_sincos(&RReg[2],amode1);
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                // RESULT IS RReg[6]=cos(Theta*n) AND RReg[7]=sin(Theta*n)

                // RReg[0]=r^n*cos(Theta*n)
                mulReal(&RReg[0],&RReg[6],&RReg[8]);
                mulReal(&RReg[1],&RReg[7],&RReg[8]);

                rplRRegToComplexPush(0,1,ANGLENONE);
                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[1]);

                return;
                }

                // PUT TOGETHER A POLAR COMPLEX NUMBER

                rplRRegToComplexPush(8,2,resmode);

                rplCheckResultAndError(&RReg[8]);
                rplCheckResultAndError(&RReg[2]);

                return;


            }

            // COMPLEX NUMBER TO COMPLEX POWER

            // Z^w = e^ ( [a*ln(r) - b*Theta] ) * (cos[[b*ln(r)+a*Theta]+ i * sin[b*ln(r)+a*Theta] )
            // SO FAR WE HAVE RREG[0]=r, RREG[1]=Theta

            swapReal(&RReg[0],&RReg[8]);
            swapReal(&RReg[1],&RReg[9]);

            if(amode1!=ANGLERAD) {
            // ANGLES MUST BE IN RADIANS FOR THIS OPERATION
            trig_convertangle(&RReg[9],amode1,ANGLERAD);
            swapReal(&RReg[9],&RReg[0]);
            }

            hyp_ln(&RReg[8]);
            normalize(&RReg[0]);

            swapReal(&RReg[0],&RReg[8]);    // RReg[8]=ln(r), RReg[9]=Theta

            // NOW WE NEED THE EXPONENT IN CARTESIAN COORDINATES
            if(amode2!=ANGLENONE) rplPolar2Rect(&Rarg2,&Iarg2,amode2);
            else { copyReal(&RReg[0],&Rarg2); copyReal(&RReg[1],&Iarg2); }
            // RREG[0]=a, RReg[1]=b

            Context.precdigits+=8;

            mulReal(&RReg[7],&RReg[8],&RReg[0]); // RReg[7]=a*ln(r)
            mulReal(&RReg[2],&RReg[8],&RReg[1]); // RReg[2]=b*ln(r)
            mulReal(&RReg[3],&RReg[9],&RReg[0]); // RReg[3]=a*Theta
            mulReal(&RReg[4],&RReg[9],&RReg[1]); // RReg[3]=b*Theta
            subReal(&RReg[8],&RReg[7],&RReg[4]);   // RReg[8]=a*ln(r)-b*Theta
            addReal(&RReg[9],&RReg[2],&RReg[3]);   // RReg[9]=b*ln(r)+a*Theta

            Context.precdigits-=8;

            hyp_exp(&RReg[8]);
            normalize(&RReg[0]);

            swapReal(&RReg[8],&RReg[0]);

            // RReg[8]=r'=e^(a*ln(r)-b*Theta)
            // RReg[9]=Theta'=a*Theta+b*ln(r)

            if(resmode==ANGLENONE) {
                rplPolar2Rect(&RReg[8],&RReg[9],ANGLERAD);
                rplRRegToComplexPush(0,1,ANGLENONE);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[1]);

                return;
            }

            // RESULT IS IN POLAR COORDINATES

            trig_convertangle(&RReg[9],ANGLERAD,resmode);
            rplRRegToComplexPush(8,0,resmode);

            rplCheckResultAndError(&RReg[8]);
            rplCheckResultAndError(&RReg[0]);

            return;
        }

        case OVR_SAME:
        case OVR_EQ:
        {
         // CHECK EQUALITY IN POLAR MODE TOO
            if(amode1==ANGLENONE) {
                if(amode2==ANGLENONE) {
                    if(!eqReal(&Rarg1,&Rarg2)||!eqReal(&Iarg1,&Iarg2)) rplPushData((WORDPTR)zero_bint);
                       else rplPushData((WORDPTR)one_bint);
                    return;
                }
                rplPolar2Rect(&Rarg2,&Iarg2,amode2);

                if(!eqReal(&Rarg1,&RReg[0])||!eqReal(&Iarg1,&RReg[1])) rplPushData((WORDPTR)zero_bint);
                   else rplPushData((WORDPTR)one_bint);
                return;
            }


            if(amode2==ANGLENONE) {
                rplPolar2Rect(&Rarg1,&Iarg1,amode1);
                if(!eqReal(&Rarg2,&RReg[0])||!eqReal(&Iarg2,&RReg[1])) rplPushData((WORDPTR)zero_bint);
                   else rplPushData((WORDPTR)one_bint);
                return;
            }

            // BOTH ARE POLAR, DO A POLAR COMPARISON

            if(!eqReal(&Rarg1,&Rarg2)) { rplPushData((WORDPTR)zero_bint); return; }

            if(iszeroReal(&Rarg1)) { rplPushData((WORDPTR)one_bint); return; }

            // SAME MAGNITUDE, NOW COMPARE THE ANGLES

            trig_convertangle(&Iarg1,amode1,ANGLEDEG);
            newRealFromBINT(&RReg[7],180,0);
            divmodReal(&RReg[6],&RReg[9],&RReg[0],&RReg[7]);    // RReg[9]=ANGLE REDUCED TO FIRST QUADRANT

            trig_convertangle(&Iarg2,amode2,ANGLEDEG);
            divmodReal(&RReg[6],&RReg[8],&RReg[0],&RReg[7]);    // RReg[8]=ANGLE REDUCED TO FIRST QUADRANT

            if(!eqReal(&RReg[8],&RReg[9])) { rplPushData((WORDPTR)zero_bint); return; }

            rplPushData((WORDPTR)one_bint);

            return;
        }
        case OVR_NOTEQ:
        {
         // CHECK EQUALITY IN POLAR MODE TOO
            if(amode1==ANGLENONE) {
                if(amode2==ANGLENONE) {
                    if(!eqReal(&Rarg1,&Rarg2)||!eqReal(&Iarg1,&Iarg2)) rplPushData((WORDPTR)one_bint);
                       else rplPushData((WORDPTR)zero_bint);
                    return;
                }
                rplPolar2Rect(&Rarg2,&Iarg2,amode2);

                if(!eqReal(&Rarg1,&RReg[0])||!eqReal(&Iarg1,&RReg[1])) rplPushData((WORDPTR)one_bint);
                   else rplPushData((WORDPTR)zero_bint);
                return;
            }


            if(amode2==ANGLENONE) {
                rplPolar2Rect(&Rarg1,&Iarg1,amode1);
                if(!eqReal(&Rarg2,&RReg[0])||!eqReal(&Iarg2,&RReg[1])) rplPushData((WORDPTR)one_bint);
                   else rplPushData((WORDPTR)zero_bint);
                return;
            }

            // BOTH ARE POLAR, DO A POLAR COMPARISON

            if(!eqReal(&Rarg1,&Rarg2)) { rplPushData((WORDPTR)one_bint); return; }

            if(iszeroReal(&Rarg1)) { rplPushData((WORDPTR)zero_bint); return; }


            // SAME MAGNITUDE, NOW COMPARE THE ANGLES

            trig_convertangle(&Iarg1,amode1,ANGLEDEG);
            newRealFromBINT(&RReg[7],180,0);
            divmodReal(&RReg[6],&RReg[9],&RReg[0],&RReg[7]);    // RReg[9]=ANGLE REDUCED TO FIRST QUADRANT

            trig_convertangle(&Iarg2,amode2,ANGLEDEG);
            divmodReal(&RReg[6],&RReg[8],&RReg[0],&RReg[7]);    // RReg[8]=ANGLE REDUCED TO FIRST QUADRANT

            if(!eqReal(&RReg[8],&RReg[9])) { rplPushData((WORDPTR)one_bint); return; }

            rplPushData((WORDPTR)zero_bint);

            return;
        }
/*
        // COMPARISONS ARE NOT DEFINED FOR COMPLEX NUMBERS
        case OVR_LT:
        case OVR_GT:
        case OVR_LTE:
        case OVR_GTE:



*/
        case OVR_AND:
            if( rplIsZeroComplex(&Rarg1,&Iarg1,amode1)|| rplIsZeroComplex(&Rarg2,&Iarg2,amode2)) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        case OVR_OR:
            if( rplIsZeroComplex(&Rarg1,&Iarg1,amode1) && rplIsZeroComplex(&Rarg2,&Iarg2,amode2)) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        case OVR_XOR:
            {
            BINT result=rplIsZeroComplex(&Rarg1,&Iarg1,amode1)^rplIsZeroComplex(&Rarg2,&Iarg2,amode2);
            if(result) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
            }

        case OVR_INV:
        {
                // 1/(a+b*i) = (a/(a^2+b^2) - b/(a^2/b^2) i
                if( rplIsZeroComplex(&Rarg1,&Iarg1,amode1)) {
                    rplError(ERR_MATHDIVIDEBYZERO);
                    return;
                }

                if(amode1==ANGLENONE) {

                Context.precdigits+=8;

                mulReal(&RReg[2],&Rarg1,&Rarg1);
                mulReal(&RReg[3],&Iarg1,&Iarg1);
                addReal(&RReg[4],&RReg[2],&RReg[3]);
                Context.precdigits-=8;

                divReal(&RReg[0],&Rarg1,&RReg[4]);
                divReal(&RReg[1],&Iarg1,&RReg[4]);
                RReg[1].flags^=F_NEGATIVE;
                rplRRegToComplexPush(0,1,ANGLENONE);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[1]);


                return;
                }

                // POLAR INVERSION
                REAL one;
                decconst_One(&one);
                divReal(&RReg[0],&one,&Rarg1);
                if(!iszeroReal(&Iarg1)) Iarg1.flags^=F_NEGATIVE;

                rplNewComplexPush(&RReg[0],&Iarg1,amode1);
                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&Iarg1);

                return;
        }
        case OVR_NEG:
        {
            if(amode1==ANGLENONE) {
            if(!iszeroReal(&Rarg1)) Rarg1.flags^=F_NEGATIVE;
            if(!iszeroReal(&Iarg1)) Iarg1.flags^=F_NEGATIVE;

            rplNewComplexPush(&Rarg1,&Iarg1,ANGLENONE);

            return;
            }

            if(amode1==ANGLEDEG) {
                newRealFromBINT(&RReg[6],180,0);
                RReg[6].flags|=(~Iarg1.flags)&F_NEGATIVE;
                if(iszeroReal(&Iarg1)) RReg[6].flags^=F_NEGATIVE;

                addReal(&RReg[1],&Iarg1,&RReg[6]);

                rplNewComplexPush(&Rarg1,&RReg[1],amode1);
                rplCheckResultAndError(&Rarg1);
                rplCheckResultAndError(&RReg[1]);


                return;

            }
            if(amode1==ANGLEGRAD) {
                newRealFromBINT(&RReg[6],200,0);
                RReg[6].flags|=(~Iarg1.flags)&F_NEGATIVE;
                if(iszeroReal(&Iarg1)) RReg[6].flags^=F_NEGATIVE;

                addReal(&RReg[1],&Iarg1,&RReg[6]);

                rplNewComplexPush(&Rarg1,&RReg[1],amode1);
                rplCheckResultAndError(&Rarg1);
                rplCheckResultAndError(&RReg[1]);


                return;

            }
            if(amode1==ANGLERAD) {
                REAL pi;
                decconst_PI(&pi);

                pi.flags|=(~Iarg1.flags)&F_NEGATIVE;
                if(iszeroReal(&Iarg1)) pi.flags^=F_NEGATIVE;

                addReal(&RReg[1],&Iarg1,&RReg[6]);

                rplNewComplexPush(&Rarg1,&RReg[1],amode1);
                rplCheckResultAndError(&Rarg1);
                rplCheckResultAndError(&RReg[1]);


                return;

            }
            // THE ONLY MODE LEFT IS DMS


            newRealFromBINT(&RReg[6],180,0);
            RReg[6].flags|=(~Iarg1.flags)&F_NEGATIVE;
            if(iszeroReal(&Iarg1)) RReg[6].flags^=F_NEGATIVE;


            trig_convertangle(&Iarg1,amode1,ANGLEDEG);

            addReal(&RReg[7],&RReg[0],&RReg[6]);

            trig_convertangle(&RReg[7],ANGLEDEG,amode1);

            rplNewComplexPush(&Rarg1,&RReg[0],amode1);
            rplCheckResultAndError(&Rarg1);
            rplCheckResultAndError(&RReg[0]);


            return;

        }
        case OVR_ABS:
        {
            if(amode1==ANGLENONE) {
                Context.precdigits+=8;
                mulReal(&RReg[2],&Rarg1,&Rarg1);
                mulReal(&RReg[3],&Iarg1,&Iarg1);
                addReal(&RReg[0],&RReg[2],&RReg[3]);

                Context.precdigits-=8;

                hyp_sqrt(&RReg[0]);
                finalize(&RReg[0]);

                rplNewRealFromRRegPush(0);
                return;
            }

            // AVOID CREATING AN OBJECT, JUST RETURN THE FIRST COMPONENT
            rplOverwriteData(1,arg1+1);

            return;


        }
        case OVR_NOT:
        {
            if(rplIsZeroComplex(&Rarg1,&Iarg1,amode1)) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;

        }
        case OVR_EVAL:
        case OVR_EVAL1:
        case OVR_XEQ:
        case OVR_NUM:
            // NOTHING TO DO, JUST KEEP THE ARGUMENT IN THE STACK
            rplPushData(arg1);
            return;


        // ADD MORE case's HERE
        default:
            rplError(ERR_COMPLEXNOTSUPPORTED);
        return;


        }
        return;
#undef arg1
#undef arg2


    }   // END OF OVERLOADABLE OPERATORS


    switch(OPCODE(CurOpcode))
    {

    case RE:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISNUMBERCPLX(*rplPeekData(1))) {
            rplError(ERR_COMPLEXORREALEXPECTED);
            return;
        }
        if(ISCOMPLEX(*rplPeekData(1))) {
            BINT angmode=rplPolarComplexMode(rplPeekData(1));

            if(angmode==ANGLENONE) {
            // AVOID CREATING A NEW OBJECT
            WORDPTR real=rplPeekData(1)+1;
            rplOverwriteData(1,real);
            return;
            }

            // POLAR COMPLEX NEEDS TO BE CONVERTED TO CARTESIAN

            REAL re,im;

            rplReadCNumber(rplPeekData(1),&re,&im,&angmode);

            if(Exceptions) return;

            rplPolar2Rect(&re,&im,angmode);

            WORDPTR newreal=rplNewRealFromRReg(0);

            if(!newreal) return;

            rplOverwriteData(1,newreal);

            return;

        }
        // IF IT IS A REAL NUMBER, THEN LEAVE THE REAL ON THE STACK
        return;
    }
    case IM:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISNUMBERCPLX(*rplPeekData(1))) {
            rplError(ERR_COMPLEXORREALEXPECTED);
            return;
        }
        if(ISCOMPLEX(*rplPeekData(1))) {

            BINT angmode=rplPolarComplexMode(rplPeekData(1));

            if(angmode==ANGLENONE) {
            // AVOID CREATING A NEW OBJECT
            WORDPTR imag=rplSkipOb(rplPeekData(1)+1);
            rplOverwriteData(1,imag);
            return;
            }

            // POLAR COMPLEX NEEDS TO BE CONVERTED TO CARTESIAN

            REAL re,im;

            rplReadCNumber(rplPeekData(1),&re,&im,&angmode);

            if(Exceptions) return;

            rplPolar2Rect(&re,&im,angmode);

            WORDPTR newreal=rplNewRealFromRReg(1);

            if(!newreal) return;

            rplOverwriteData(1,newreal);

            return;

        }

            // NON-COMPLEX NUMBERS HAVE IMAGINARY PART = 0
            rplDropData(1);
            rplPushData((WORDPTR)zero_bint);
            return;
    }
    case ARG:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISNUMBERCPLX(*rplPeekData(1))) {
            rplError(ERR_COMPLEXORREALEXPECTED);
            return;
        }

        if(ISCOMPLEX(*rplPeekData(1))) {

            BINT angmode=rplPolarComplexMode(rplPeekData(1));

            if(angmode!=ANGLENONE) {
            // AVOID CREATING A NEW OBJECT
                WORDPTR arg=rplSkipOb(rplPeekData(1)+1);
                rplOverwriteData(1,arg);
                return;
            }

            // IT'S A NORMAL COMPLEX, NEED TO COMPUTE THE ARGUMENT

            REAL real,imag;

            rplReadCNumber(rplPeekData(1),&real,&imag,&angmode);

            if(Exceptions) return;

            angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

            // COMPUTE THE ANGLE DIRECTLY FOR SPEED INSTEAD OF USING rplRect2Polar
            trig_atan2(&imag,&real,angmode);
            finalize(&RReg[0]);


            // RETURN AN ANGLE IN THE CURRENT SYSTEM

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],angmode);
            if(!newang) return;
            rplOverwriteData(1,newang);

            return;
        }

        // REAL NUMBERS HAVE ARGUMENT DEPENDING ON THE SIGN

        REAL real;
        rplReadNumberAsReal(rplPeekData(1),&real);

        BINT angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

        if(real.flags&F_NEGATIVE) {
            switch(angmode)
            {
            case ANGLERAD:
            {
                REAL pi;
              decconst_PI(&pi);
              copyReal(&RReg[0],&pi);
              finalize(&RReg[0]);   // ROUND PI TO THE CURRENT NUMBER OF DIGITS
              break;
            }
            case ANGLEGRAD:
                newRealFromBINT(&RReg[0],200,0);
                break;
            case ANGLEDEG:
            case ANGLEDMS:
            default:
                newRealFromBINT(&RReg[0],180,0);
                break;
            }
        }
        else rplZeroToRReg(0);

        WORDPTR newang=rplNewAngleFromReal(&RReg[0],angmode);
        if(!newang) return;
        rplOverwriteData(1,newang);

        return;
    }
    case CONJ:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISNUMBERCPLX(*rplPeekData(1))) {
            rplError(ERR_COMPLEXORREALEXPECTED);
            return;
        }

        if(ISCOMPLEX(*rplPeekData(1))) {
            REAL real,imag;
            BINT angmode;

            rplReadCNumber(rplPeekData(1),&real,&imag,&angmode);

            // INVERT THE SIGN OF THE IMAGINARY PART OR THE ARGUMENT
            if(!iszeroReal(&imag)) imag.flags^=F_NEGATIVE;

            WORDPTR newcplx=rplNewComplex(&real,&imag,angmode);
            if(!newcplx) return;

            rplOverwriteData(1,newcplx);

            return;
        }

        // REAL NUMBERS ARE JUST LEFT ON THE STACK

        return;
    }



    case CPLX2REAL:
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISCOMPLEX(*rplPeekData(1))) {
            rplError(ERR_COMPLEXEXPECTED);
            return;
        }
        ScratchPointer1=rplPeekData(1);
        rplPushData(ScratchPointer1+1);
        rplPushData(rplSkipOb(ScratchPointer1+1));
    return;

    case REAL2CPLX:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!(ISNUMBER(*rplPeekData(1))||ISANGLE(*rplPeekData(1)))) {
            rplError(ERR_REALORANGLEEXPECTED);
            return;
        }
        if(!ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        // CONSTRUCT THE COMPLEX NUMBER
        BINT sizer=rplObjSize(rplPeekData(2));
        BINT sizei=rplObjSize(rplPeekData(1));
        WORDPTR newcplx=rplAllocTempOb(sizer+sizei);
        if(!newcplx) {
            return;
        }

        *newcplx=MKPROLOG(LIBRARY_NUMBER,sizer+sizei);
        rplCopyObject(newcplx+1,rplPeekData(2));
        rplCopyObject(newcplx+1+sizer,rplPeekData(1));

        rplDropData(2);
        rplPushData(newcplx);

    return;
    }







    // STANDARIZED OPCODES:
    // --------------------
    // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES


    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // ArgPtr2 = token blanks afterwards
        // ArgNum2 = blanks length

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors

    {
        // COMPILE COMPLEX OBJECTS IN THE FORM ( X , Y ) BUT ALSO ACCEPT (X,Y) (NO SPACES)

        if(*((char * )TokenStart)=='(')
        {

            rplCompileAppend((WORD) MKPROLOG(LIBRARY_NUMBER,0));
            if(TokenLen>1) {
                NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                RetNum=OK_STARTCONSTRUCT;
            }
            else RetNum=OK_STARTCONSTRUCT;
            return;
        }
        // CHECK IF THE TOKEN IS THE CLOSING BRACKET

        if(*utf8nskip((char *)TokenStart,(char *)BlankStart,TokenLen-1)==')')
        {
            if(TokenLen>1) {
                BlankStart=NextTokenStart=(WORDPTR)utf8nskip((char *)TokenStart,(char *)BlankStart,TokenLen-1);
                RetNum=ERR_NOTMINE_SPLITTOKEN;
                return;
            }


            if(CurrentConstruct!=MKPROLOG(LIBRARY_NUMBER,2)) {
                RetNum=ERR_SYNTAX;
                return;
            }
            RetNum=OK_ENDCONSTRUCT;
            return;
        }

        // CHECK IF THE CURRENT CONSTRUCT IS A COMPLEX NUMBER AND IT CONTAINS A COMMA

        if( (LIBNUM(CurrentConstruct)==LIBRARY_NUMBER) && ISPROLOG(CurrentConstruct))
            {
            BINT count=TokenLen;
            BYTEPTR ptr=(BYTEPTR)TokenStart;
            UBINT64 Locale=rplGetSystemLocale();


            // THERE'S 3 PLACES TO HAVE A COMMA
            // ENDING TOKEN
            // STARTING A TOKEN
            // IN THE MIDDLE WOULD BE PART OF A NUMBER

            if((WORD)utf82cp((char *)ptr,(char *)BlankStart)==ARG_SEP(Locale)) {
               // STARTS WITH COMMA
                    if(TokenLen>1)  NextTokenStart=(WORDPTR)utf8skip((char *)ptr,(char *)BlankStart);
                    // WE DID NOT PRODUCE ANY OUTPUT, SO DON'T VALIDATE
                    RetNum=OK_CONTINUE_NOVALIDATE;
                    return;
            }
            if((WORD)utf82cp(utf8rskip((char *)BlankStart,(char *)ptr),(char *)BlankStart)==ARG_SEP(Locale)) {
                // ENDS WITH A COMMA, SPLIT THE TOKEN
                BlankStart=NextTokenStart=(WORDPTR)utf8rskip((char *)BlankStart,(char *)ptr);
                RetNum=ERR_NOTMINE_SPLITTOKEN;
                return;
            }

            while(count && ((WORD)utf82cp((char *)ptr,(char *)BlankStart)!=ARG_SEP(Locale))) { ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)BlankStart); --count; }

            if(count) {

              // THERE'S A COMMA IN THE MIDDLE
                  // SPLIT THE TOKEN
                  BlankStart=NextTokenStart=(WORDPTR)(ptr);
                  RetNum=ERR_NOTMINE_SPLITTOKEN;
                  return;
              }


            // THERE IS NO COMMA IN THIS TOKEN

            RetNum=ERR_NOTMINE;
            return;


        }



        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

        return;
    }
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        if(ISPROLOG(*DecompileObject)) {
            UBINT64 Locale=rplGetSystemLocale();

            rplDecompAppendString((BYTEPTR)"(");

            // POINT TO THE REAL PART
            DecompileObject++;

            LIBHANDLER libhan=rplGetLibHandler(LIBNUM(*DecompileObject));
            if(libhan) (*libhan)();

            rplDecompAppendUTF8(cp2utf8(ARG_SEP(Locale)));

            // POINT TO THE IMAGINARY PART
            DecompileObject=rplSkipOb(DecompileObject);

            libhan=rplGetLibHandler(LIBNUM(*DecompileObject));
            if(libhan) (*libhan)();

            rplDecompAppendString((BYTEPTR)")");

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
        if(ISNUMBER(*LastCompiledObject) || ISANGLE(*LastCompiledObject)) {
            if( (OBJSIZE(CurrentConstruct)==0)&& ISANGLE(*LastCompiledObject)) {
                // ANGLES ARE NOT ACCEPTABLE IN THE FIRST COMPONENT
                rplError(ERR_NOTALLOWEDINCOMPLEX);
                RetNum=ERR_INVALID;
            }
            else  RetNum=OK_INCARGCOUNT;
        }
        else {
            rplError(ERR_NOTALLOWEDINCOMPLEX);
            RetNum=ERR_INVALID;
        }

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

        if(ISPROLOG(*ObjectPTR)) {
        WORDPTR real,imag;

        // CHECK MINIMUM SIZE
        if(OBJSIZE(*ObjectPTR)<2) { RetNum=ERR_INVALID; return; }
        // CHECK IF REAL PART IS A VALID NUMBER
        real=ObjectPTR+1;
        if(!ISNUMBER(*real)) { RetNum=ERR_INVALID; return; }
        // CHECK IF SIZE OF REAL PART FITS WITHIN COMPLEX OBJECT
        imag=rplSkipOb(real);
        if(imag>=rplSkipOb(ObjectPTR)) { RetNum=ERR_INVALID; return; }
        // CHECK IF IMAGINARY PART IS A VALID NUMBER
        if(!ISNUMBER(*imag)) { RetNum=ERR_INVALID; return; }
        // CHECK IF SIZE OF IMAGINARY PART IS CORRECT
        if(rplSkipOb(imag)!=rplSkipOb(ObjectPTR)) { RetNum=ERR_INVALID; return; }

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
        ObjectPTR=(WORDPTR)lib30_menu;
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

        libFindMsg(LibError,(WORDPTR)LIB_MSGTABLE);
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



