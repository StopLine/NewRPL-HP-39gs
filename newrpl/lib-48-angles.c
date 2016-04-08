/*
 * Copyright (c) 2016, Claudio Lapilli and the newRPL Team
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
#define LIBRARY_NUMBER  48


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    ECMD(TAGDEG,"→∡°",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TAGRAD,"→∡r",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TAGGRAD,"→∡g",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TAGDMS,"→∡d",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ANGTODEG,"A→∡°",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ANGTORAD,"A→∡r",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ANGTOGRAD,"A→∡g",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ANGTODMS,"A→∡d",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TORECT,"→RECT",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TOPOLAR,"→POLAR",MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TOSPHER,"→SPHER",MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2))




#define ERROR_LIST \
        ERR(REALORANGLEEXPECTED,0)



// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER,LIBRARY_NUMBER+1,LIBRARY_NUMBER+2,LIBRARY_NUMBER+3


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************


INCLUDE_ROMOBJECT(LIB_MSGTABLE);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
     (WORDPTR)LIB_MSGTABLE,
    0
};



// CONVERT AN ANGLE FROM ONE SYSTEM TO ANOTHER. LEAVES RESULT IN
// RReg[0] AS A REAL NUMBER
// NO TYPE CHECKS HERE FOR SPEED
// USES RReg[0] TO [3]



void rplConvertAngle(REAL *oldang,BINT oldmode,BINT newmode)
    {

    REAL convfactor;

    switch((oldmode<<2)|newmode)
    {
    // FROM DEGREES TO OTHER MODES
    case (ANGLEDEG<<2)|ANGLEDEG:
        copyReal(&RReg[0],oldang);
        return;
    case (ANGLEDEG<<2)|ANGLERAD:
        decconst_PI_180(&convfactor);
        break;
    case (ANGLEDEG<<2)|ANGLEGRAD:
    {
        RReg[3].data[0]=9;
        RReg[3].exp=-1;
        RReg[3].len=1;
        RReg[3].flags=0;

        divReal(&RReg[0],oldang,&RReg[3]);
        return;
    }

    case (ANGLEDEG<<2)|ANGLEDMS:
    {

        // RReg[2]= T = ANG - FP(ANG)*0.4
        fracReal(&RReg[1],oldang);
        RReg[1].exp--;
        sub_real_mul(&RReg[2],oldang,&RReg[1],4);
        normalize(&RReg[2]);
        // RReg[0]= DMS = T- FP(T*100)*0.004
        RReg[2].exp+=2;
        fracReal(&RReg[1],&RReg[2]);
        RReg[1].exp-=3;
        RReg[2].exp-=2;

        sub_real_mul(&RReg[0],&RReg[2],&RReg[1],4);
        normalize(&RReg[0]);

        return;
    }

    // FROM RADIANS TO OTHER MODES
    case (ANGLERAD<<2)|ANGLEDEG:
        decconst_180_PI(&convfactor);
        break;
    case (ANGLERAD<<2)|ANGLERAD:
        copyReal(&RReg[0],oldang);
        return;

    case (ANGLERAD<<2)|ANGLEGRAD:
        decconst_200_PI(&convfactor);
        break;

    case (ANGLERAD<<2)|ANGLEDMS:
    {
        decconst_180_PI(&convfactor);

        mulReal(&RReg[3],oldang,&convfactor);

        // RReg[2]= T = ANG - FP(ANG)*0.4
        fracReal(&RReg[1],&RReg[3]);
        RReg[1].exp--;
        sub_real_mul(&RReg[2],&RReg[3],&RReg[1],4);
        normalize(&RReg[2]);
        // RReg[0]= DMS = T- FP(T*100)*0.004
        RReg[2].exp+=2;
        fracReal(&RReg[1],&RReg[2]);
        RReg[1].exp-=3;
        RReg[2].exp-=2;

        sub_real_mul(&RReg[0],&RReg[2],&RReg[1],4);
        normalize(&RReg[0]);

        return;
    }

    // FROM GRAD TO OTHER MODES
    case (ANGLEGRAD<<2)|ANGLEDEG:
    {
        RReg[3].data[0]=9;
        RReg[3].exp=-1;
        RReg[3].len=1;
        RReg[3].flags=0;

        mulReal(&RReg[0],oldang,&RReg[3]);
        return;
    }
    case (ANGLEGRAD<<2)|ANGLERAD:
        decconst_PI_200(&convfactor);
        break;
    case (ANGLEGRAD<<2)|ANGLEGRAD:
        copyReal(&RReg[0],oldang);
        return;
    case (ANGLEGRAD<<2)|ANGLEDMS:
    {
        RReg[2].data[0]=9;
        RReg[2].exp=-1;
        RReg[2].len=1;
        RReg[2].flags=0;

        mulReal(&RReg[3],oldang,&RReg[2]);

        // RReg[2]= T = ANG - FP(ANG)*0.4
        fracReal(&RReg[1],&RReg[3]);
        RReg[1].exp--;
        sub_real_mul(&RReg[2],&RReg[3],&RReg[1],4);
        normalize(&RReg[2]);
        // RReg[0]= DMS = T- FP(T*100)*0.004
        RReg[2].exp+=2;
        fracReal(&RReg[1],&RReg[2]);
        RReg[1].exp-=3;
        RReg[2].exp-=2;

        sub_real_mul(&RReg[0],&RReg[2],&RReg[1],4);
        normalize(&RReg[0]);

        return;

    }


    // FROM DMS TO OTHER MODES
    case (ANGLEDMS<<2)|ANGLEDEG:
    {
        RReg[3].data[0]=150;
        RReg[3].exp=0;
        RReg[3].len=1;
        RReg[3].flags=0;

        // RReg[2] = T = DMS + FP(DMS*100)/150;
        oldang->exp+=2;
        fracReal(&RReg[1],oldang);
        divReal(&RReg[0],&RReg[1],&RReg[3]);
        oldang->exp-=2;
        addReal(&RReg[2],oldang,&RReg[0]);

        // RReg[0] = DEG = T + FP(T)/1.5
        fracReal(&RReg[1],&RReg[2]);
        RReg[3].exp-=2;
        divReal(&RReg[1],&RReg[1],&RReg[3]);
        addReal(&RReg[0],&RReg[2],&RReg[1]);

        return;

    }
    case (ANGLEDMS<<2)|ANGLERAD:
        {
            RReg[3].data[0]=150;
            RReg[3].exp=0;
            RReg[3].len=1;
            RReg[3].flags=0;

            // RReg[2] = T = DMS + FP(DMS*100)/150;
            oldang->exp+=2;
            fracReal(&RReg[1],oldang);
            divReal(&RReg[0],&RReg[1],&RReg[3]);
            oldang->exp-=2;
            addReal(&RReg[2],oldang,&RReg[0]);

            // RReg[0] = DEG = T + FP(T)/1.5
            fracReal(&RReg[1],&RReg[2]);
            RReg[3].exp-=2;
            divReal(&RReg[1],&RReg[1],&RReg[3]);
            addReal(&RReg[3],&RReg[2],&RReg[1]);

            decconst_PI_180(&convfactor);

            mulReal(&RReg[0],&RReg[3],&convfactor);

            return;

        }
    case (ANGLEDMS<<2)|ANGLEGRAD:
    {
        RReg[3].data[0]=150;
        RReg[3].exp=0;
        RReg[3].len=1;
        RReg[3].flags=0;

        // RReg[2] = T = DMS + FP(DMS*100)/150;
        oldang->exp+=2;
        fracReal(&RReg[1],oldang);
        divReal(&RReg[0],&RReg[1],&RReg[3]);
        oldang->exp-=2;
        addReal(&RReg[2],oldang,&RReg[0]);

        // RReg[0] = DEG = T + FP(T)/1.5
        fracReal(&RReg[1],&RReg[2]);
        RReg[3].exp-=2;
        divReal(&RReg[1],&RReg[1],&RReg[3]);
        addReal(&RReg[3],&RReg[2],&RReg[1]);

        RReg[2].data[0]=9;
        RReg[2].exp=-1;
        RReg[2].len=1;
        RReg[2].flags=0;

        divReal(&RReg[0],&RReg[3],&RReg[2]);

        return;

    }

    default:            // DEFAULT CASE IS IN CASE OF BAD SYSTEM FLAGS
    case (ANGLEDMS<<2)|ANGLEDMS:
        copyReal(&RReg[0],oldang);
        return;


    }

    mulReal(&RReg[0],oldang,&convfactor);

    return;
}


// HIGHER LEVEL API USING ANGLE OBJECTS DIRECTLY
void rplConvertAngleObj(WORDPTR angleobj,BINT newmode)
{
    BINT oldmode=LIBNUM(*angleobj)&3;
    REAL oldang;

    rplReadNumberAsReal(angleobj+1,&oldang);

    rplConvertAngle(&oldang,oldmode,newmode);
}



WORDPTR rplNewAngleFromNumber(WORDPTR numobj,BINT newmode)
{
    if(!ISNUMBER(*numobj)) {
        rplError(ERR_REALEXPECTED);
        return 0;
    }
    ScratchPointer1=numobj;
    BINT size=rplObjSize(numobj);

    WORDPTR newobj=rplAllocTempOb(size);

    if(!newobj) return 0;

    *newobj=MKPROLOG(DOANGLE + (newmode&3), size);
    memcpyw(newobj+1,ScratchPointer1,size);

    return newobj;

}


WORDPTR rplNewAngleFromReal(REAL *number,BINT newmode)
{
    BINT size=number->len+2;

    WORDPTR newobj=rplAllocTempOb(size);

    if(!newobj) return 0;

    WORDPTR endofobject=rplNewRealInPlace(number,newobj+1);

    *newobj=MKPROLOG(DOANGLE + (newmode&3), endofobject-newobj-1);

    rplTruncateLastObject(endofobject);

    return newobj;

}









void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // NORMAL BEHAVIOR FOR A REAL IS TO PUSH THE OBJECT ON THE STACK:
        rplPushData(IPtr);
        return;
    }


    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE)
    {
        // THESE ARE OVERLOADABLE COMMANDS DISPATCHED FROM THE
        // OVERLOADABLE OPERATORS LIBRARY.

        // PROVIDE BEHAVIOR FOR OVERLOADABLE OPERATORS HERE
        // USE GC-SAFE POINTERS, NEVER LOCAL COPIES OF POINTERS INTO TEMPOB
#define arg1 ScratchPointer1
#define arg2 ScratchPointer2

        int nargs=OVR_GETNARGS(CurOpcode);

        if(rplDepthData()<nargs) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(nargs==1) {
            switch(OPCODE(CurOpcode))
            {
            case OVR_INV:
            {
                // ARGUMENT CHECKS SHOULD NOT BE NECESSARY

                // CONVERT TO CURRENT SYSTEM AND REMOVE THE TAG
                BINT curmode=rplTestSystemFlag(-17)|(rplTestSystemFlag(-18)<<1);

                rplConvertAngleObj(rplPeekData(1),curmode);
                // NEW ANGLE IS IN RReg[0]
                WORDPTR number=rplNewReal(&RReg[0]);
                if(Exceptions) return;

                // THEN CALL THE ORIGINAL OPERATOR
                rplOverwriteData(1,number);
                rplCallOvrOperator(CurOpcode);
                return;

            }

            case OVR_ABS:
            case OVR_NEG:
            {
                // OPERATE, KEEP THE TAG
                rplPushData(rplPeekData(1)+1);  // EXTRACT THE NUMBER
                rplCallOvrOperator(CurOpcode);  // DO THE THING ON THE NUMBER

                WORDPTR newangle=rplNewAngleFromNumber(rplPeekData(1),LIBNUM(*rplPeekData(2))&3);
                rplDropData(1);
                if(Exceptions) return;

                rplOverwriteData(1,newangle);
                return;
            }

            case OVR_EVAL1:
            case OVR_EVAL:
            case OVR_XEQ:
            case OVR_FUNCEVAL:
                // DO NOTHING!
                return;
            case OVR_NUM:
            {
                // ARGUMENT CHECKS SHOULD NOT BE NECESSARY

                // CONVERT TO CURRENT SYSTEM AND REMOVE THE TAG
                BINT curmode=rplTestSystemFlag(-17)|(rplTestSystemFlag(-18)<<1);

                rplConvertAngleObj(rplPeekData(1),curmode);
                // NEW ANGLE IS IN RReg[0]
                WORDPTR number=rplNewReal(&RReg[0]);
                if(Exceptions) return;

                rplOverwriteData(1,number);
                return;

            }
                // CHANGE SIGN, KEEP THE TAG

            case OVR_ISTRUE:
            case OVR_NOT:
                // REMOVE THE TAG, CONVERT TO 1 OR 0
            {
                // ARGUMENT CHECKS SHOULD NOT BE NECESSARY
                // REMOVE THE TAG, NO NEED TO CONVERT
                rplOverwriteData(1,rplPeekData(1)+1);

                // THEN CALL THE ORIGINAL OPERATOR
                rplCallOvrOperator(CurOpcode);
                return;

            }

            }

        }
        else {
            // BINARY OPERATORS

            arg1=rplPeekData(2);
            arg2=rplPeekData(1);

            if(!(ISANGLE(*arg1)||ISNUMBER(*arg1)) || !(ISANGLE(*arg2)||ISNUMBER(*arg2))) {
                rplError(ERR_REALORANGLEEXPECTED);
                return;
            }

            if(ISANGLE(*arg1) && ISANGLE(*arg2)) {
                // ONLY ALLOW ADDITION/SUBTRACTION OF ANGLES

                if(OPCODE(CurOpcode)==OVR_ADD) {

                    // CONVERT 2ND ARGUMENT TO THE SYSTEM OF THE FIRST

                    BINT angmode=LIBNUM(*arg1)&3;
                    if(angmode!=ANGLEDMS) {
                        REAL arg1num;

                        rplReadNumberAsReal(arg1+1,&arg1num);

                        rplConvertAngleObj(arg2,angmode);
                        addReal(&RReg[1],&arg1num,&RReg[0]);
                        WORDPTR newang=rplNewAngleFromReal(&RReg[1],angmode);

                        if(Exceptions) return;
                        rplOverwriteData(2,newang);
                        rplDropData(1);
                        return;

                    }
                        // ADDING NUMBERS IN DMS FORMAT DIRECTLY TAKES MORE WORK

                        rplConvertAngleObj(arg2,ANGLEDEG);
                        copyReal(&RReg[7],&RReg[0]);
                        rplConvertAngleObj(arg1,ANGLEDEG);

                        addReal(&RReg[6],&RReg[0],&RReg[7]);
                        rplConvertAngle(&RReg[6],ANGLEDEG,ANGLEDMS);

                        WORDPTR newang=rplNewAngleFromReal(&RReg[0],angmode);

                        if(Exceptions) return;
                        rplOverwriteData(2,newang);
                        rplDropData(1);
                        return;

                }

                if(OPCODE(CurOpcode)==OVR_SUB) {

                    // CONVERT 2ND ARGUMENT TO THE SYSTEM OF THE FIRST

                    BINT angmode=LIBNUM(*arg1)&3;


                    if(angmode!=ANGLEDMS) {
                        REAL arg1num;
                        rplReadNumberAsReal(arg1+1,&arg1num);
                        rplConvertAngleObj(arg2,angmode);
                        subReal(&RReg[1],&arg1num,&RReg[0]);
                        WORDPTR newang=rplNewAngleFromReal(&RReg[1],angmode);

                        if(Exceptions) return;
                        rplOverwriteData(2,newang);
                        rplDropData(1);
                        return;

                    }
                        // ADDING NUMBERS IN DMS FORMAT DIRECTLY TAKES MORE WORK

                        rplConvertAngleObj(arg2,ANGLEDEG);
                        copyReal(&RReg[7],&RReg[0]);
                        rplConvertAngleObj(arg1,ANGLEDEG);

                        subReal(&RReg[6],&RReg[0],&RReg[7]);
                        rplConvertAngle(&RReg[6],ANGLEDEG,ANGLEDMS);

                        WORDPTR newang=rplNewAngleFromReal(&RReg[0],angmode);

                        if(Exceptions) return;
                        rplOverwriteData(2,newang);
                        rplDropData(1);
                        return;

                }

            }
                // ALL OTHER OPERATORS SHOULD CONVERT TO CURRENT ANGLE SYSTEM AND REMOVE TAGS
                // THEN PROCESS THE OPCODE NORMALLY.

                BINT curmode=rplTestSystemFlag(-17)|(rplTestSystemFlag(-18)<<1);

                if(ISANGLE(*arg1)) {
                    // CONVERT TO CURRENT SYSTEM AND REMOVE THE TAG

                    rplConvertAngleObj(arg1,curmode);
                    // NEW ANGLE IS IN RReg[0]
                    WORDPTR number=rplNewReal(&RReg[0]);
                    if(Exceptions) return;

                    rplOverwriteData(2,number);
                }

                if(ISANGLE(*arg2)) {
                    // CONVERT TO CURRENT SYSTEM AND REMOVE THE TAG

                    rplConvertAngleObj(arg2,curmode);
                    // NEW ANGLE IS IN RReg[0]
                    WORDPTR number=rplNewReal(&RReg[0]);
                    if(Exceptions) return;

                    rplOverwriteData(1,number);
                }

                rplCallOvrOperator(CurOpcode);
                return;




        }

#undef arg1
#undef arg2
    }   // END OF OVERLOADABLE OPERATORS


    switch(OPCODE(CurOpcode))
    {

    case TAGDEG:
    {
    // TAG A NUMBER
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            WORDPTR newang=rplNewAngleFromNumber(rplPeekData(1),ANGLEDEG);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        rplError(ERR_REALEXPECTED);
        return;

    }
    case TAGRAD:
    {
    // TAG A NUMBER
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            WORDPTR newang=rplNewAngleFromNumber(rplPeekData(1),ANGLERAD);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        rplError(ERR_REALEXPECTED);
        return;

    }

    case TAGGRAD:
    {
    // TAG A NUMBER
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            WORDPTR newang=rplNewAngleFromNumber(rplPeekData(1),ANGLEGRAD);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        rplError(ERR_REALEXPECTED);
        return;

    }

    case TAGDMS:
    {
    // TAG A NUMBER
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            WORDPTR newang=rplNewAngleFromNumber(rplPeekData(1),ANGLEDMS);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        rplError(ERR_REALEXPECTED);
        return;

    }

    case ANGTODEG:
    {
        // CONVERT ANGLE TO DEGREES
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            REAL num;
            rplReadNumberAsReal(rplPeekData(1),&num);
            BINT angmode=rplTestSystemFlag(-17)|(rplTestSystemFlag(-18)<<1);
            rplConvertAngle(&num,angmode,ANGLEDEG);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEDEG);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        if(ISANGLE(*rplPeekData(1))) {
            rplConvertAngleObj(rplPeekData(1),ANGLEDEG);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEDEG);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }


        if(ISCOMPLEX(*rplPeekData(1))) {
            BINT angmode=rplPolarComplexMode(rplPeekData(1));

            if(angmode<0) return;   // NOTHING TO DO
            if(angmode==ANGLEDEG) return; // ALREADY IN THE RIGHT SYSTEM

            REAL rp,ip;
            rplRealPart(rplPeekData(1),&rp);
            rplImaginaryPart(rplPeekData(1),&ip);

            rplConvertAngle(&ip,angmode,ANGLEDEG);

            rplDropData(1);
            rplNewComplexPush(&rp,&RReg[0],ANGLEDEG);

            return;

       }

        rplError(ERR_REALEXPECTED);
        return;


    }

    case ANGTORAD:
    {
        // CONVERT ANGLE TO DEGREES
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            REAL num;
            rplReadNumberAsReal(rplPeekData(1),&num);
            BINT angmode=rplTestSystemFlag(-17)|(rplTestSystemFlag(-18)<<1);
            rplConvertAngle(&num,angmode,ANGLERAD);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLERAD);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        if(ISANGLE(*rplPeekData(1))) {
            rplConvertAngleObj(rplPeekData(1),ANGLERAD);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLERAD);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }


        if(ISCOMPLEX(*rplPeekData(1))) {
            BINT angmode=rplPolarComplexMode(rplPeekData(1));

            if(angmode<0) return;   // NOTHING TO DO
            if(angmode==ANGLERAD) return; // ALREADY IN THE RIGHT SYSTEM

            REAL rp,ip;
            rplRealPart(rplPeekData(1),&rp);
            rplImaginaryPart(rplPeekData(1),&ip);

            rplConvertAngle(&ip,angmode,ANGLERAD);

            rplDropData(1);
            rplNewComplexPush(&rp,&RReg[0],ANGLERAD);

            return;

       }

        rplError(ERR_REALEXPECTED);
        return;


    }

    case ANGTOGRAD:
    {
        // CONVERT ANGLE TO DEGREES
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            REAL num;
            rplReadNumberAsReal(rplPeekData(1),&num);
            BINT angmode=rplTestSystemFlag(-17)|(rplTestSystemFlag(-18)<<1);
            rplConvertAngle(&num,angmode,ANGLEGRAD);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEGRAD);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        if(ISANGLE(*rplPeekData(1))) {
            rplConvertAngleObj(rplPeekData(1),ANGLEGRAD);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEGRAD);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }


        if(ISCOMPLEX(*rplPeekData(1))) {
            BINT angmode=rplPolarComplexMode(rplPeekData(1));

            if(angmode<0) return;   // NOTHING TO DO
            if(angmode==ANGLEGRAD) return; // ALREADY IN THE RIGHT SYSTEM

            REAL rp,ip;
            rplRealPart(rplPeekData(1),&rp);
            rplImaginaryPart(rplPeekData(1),&ip);

            rplConvertAngle(&ip,angmode,ANGLEGRAD);

            rplDropData(1);
            rplNewComplexPush(&rp,&RReg[0],ANGLEGRAD);

            return;

       }

        rplError(ERR_REALEXPECTED);
        return;


    }

    case ANGTODMS:
    {
        // CONVERT ANGLE TO DEGREES
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            REAL num;
            rplReadNumberAsReal(rplPeekData(1),&num);
            BINT angmode=rplTestSystemFlag(-17)|(rplTestSystemFlag(-18)<<1);
            rplConvertAngle(&num,angmode,ANGLEDMS);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEDMS);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }
        if(ISANGLE(*rplPeekData(1))) {
            rplConvertAngleObj(rplPeekData(1),ANGLEDMS);

            WORDPTR newang=rplNewAngleFromReal(&RReg[0],ANGLEDMS);
            if(!newang) return;
            rplOverwriteData(1,newang);
            return;
        }


        if(ISCOMPLEX(*rplPeekData(1))) {
            BINT angmode=rplPolarComplexMode(rplPeekData(1));

            if(angmode<0) return;   // NOTHING TO DO
            if(angmode==ANGLEDMS) return; // ALREADY IN THE RIGHT SYSTEM

            REAL rp,ip;
            rplRealPart(rplPeekData(1),&rp);
            rplImaginaryPart(rplPeekData(1),&ip);

            rplConvertAngle(&ip,angmode,ANGLEDMS);

            rplDropData(1);
            rplNewComplexPush(&rp,&RReg[0],ANGLEDMS);

            return;

       }

        rplError(ERR_REALEXPECTED);
        return;


    }

    case TORECT:
    case TOPOLAR:
    case TOSPHER:



    return;





    // STANDARIZED OPCODES:
    // --------------------
    // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES



    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlankLen = blanks length

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors



        // COMPILE A NUMBER TO A REAL
    {
        if(LIBNUM(CurOpcode)!=DOANGLE) {
            // DO NOT COMPILE ANYTHING WHEN CALLED WITH THE UPPER LIBRARY NUMBERS
            RetNum=ERR_NOTMINE;
            return;
        }

        if(!utf8ncmp((char *)TokenStart,"∡",1)) {


        // POINT TO THE LAST CHARACTER
        BINT tlen=TokenLen-1;
        BYTEPTR ptr=(BYTEPTR)utf8nskip((char *)TokenStart,(char *)BlankStart,tlen);
        BINT angmode=rplTestSystemFlag(-17)|(rplTestSystemFlag(-18)<<1);
        BYTEPTR strptr=(BYTEPTR)utf8skip((char *)TokenStart,(char *)BlankStart);
        BINT isapprox=0;


            if(!utf8ncmp((char *)ptr,"°",1)) { angmode=ANGLEDEG; --tlen; }
            if((*ptr=='d')||(*ptr=='D')) { angmode=ANGLEDMS; --tlen; }
            if((*ptr=='g')||(*ptr=='G')) { angmode=ANGLEGRAD; --tlen; }
            if((*ptr=='r')||(*ptr=='R')) { angmode=ANGLERAD; --tlen; }


            WORD  locale=rplGetSystemLocale();


            newRealFromText(&RReg[0],(char *)strptr,utf8nskip((char *)strptr,(char *)BlankStart,tlen),(WORD)locale);


            if(RReg[0].flags&F_ERROR) {
                // THERE WAS SOME ERROR DURING THE CONVERSION, PROBABLY A SYNTAX ERROR
                RetNum=ERR_NOTMINE;
                return;
            }

                if(RReg[0].flags&F_OVERFLOW) {
                    rplError(ERR_MATHOVERFLOW);
                    RetNum=ERR_INVALID;
                    return;
                }
                if(RReg[0].flags&(F_NEGUNDERFLOW|F_POSUNDERFLOW)) {
                    rplError(ERR_MATHUNDERFLOW);
                    RetNum=ERR_INVALID;
                    return;
                }


                if(RReg[0].flags&F_APPROX) isapprox=APPROX_BIT;

                ScratchPointer1=CompileEnd;

                // WRITE THE PROLOG

                rplCompileAppend(MKPROLOG(LIBRARY_NUMBER|angmode,0));

                if(isintegerReal(&RReg[0]) && inBINT64Range(&RReg[0])) {
                        BINT64 num=getBINT64Real(&RReg[0]);
                        rplCompileBINT(num,DECBINT|isapprox);
                 }

                 else {
                rplCompileAppend(MKPROLOG(DOREAL|isapprox,1+RReg[0].len));
                // PACK THE INFORMATION
                REAL_HEADER real;
                real.flags=RReg[0].flags&0xf;
                real.len=RReg[0].len;
                real.exp=RReg[0].exp;

                rplCompileAppend(real.word);      // CAREFUL: THIS IS FOR LITTLE ENDIAN SYSTEMS ONLY!
                BINT count;
                for(count=0;count<RReg[0].len;++count) {
                    rplCompileAppend(RReg[0].data[count]);      // STORE ALL THE MANTISSA WORDS
                }
                }

                *ScratchPointer1|=CompileEnd-ScratchPointer1-1; // FIX THE SIZE


                RetNum=OK_CONTINUE;
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
        // DecompStringEnd = Byte Ptr to end of current string. Write here with rplDecompAppendString(); rplDecompAppendChar();

        if(ISPROLOG(*DecompileObject)) {
        rplDecompAppendString((BYTEPTR)"∡");


        // THIS IS A BASE-10 NUMBER
        // CONVERT TO STRING

        REAL realnum;

        NUMFORMAT fmt;

        BINT Format,sign;

        rplGetSystemNumberFormat(&fmt);

        rplReadNumberAsReal(DecompileObject+1,&realnum);

        sign=realnum.flags&F_NEGATIVE;

        realnum.flags^=sign;

        if(iszeroReal(&realnum)) Format=fmt.MiddleFmt;
        else if(ltReal(&realnum,&(fmt.SmallLimit))) Format=fmt.SmallFmt;
        else if(gtReal(&realnum,&(fmt.BigLimit))) Format=fmt.BigFmt;
        else Format=fmt.MiddleFmt;

        realnum.flags^=sign;

        if(CurOpcode==OPCODE_DECOMPEDIT) Format|=FMT_CODE;

        // ESTIMATE THE MAXIMUM STRING LENGTH AND RESERVE THE MEMORY

        BYTEPTR string;

        BINT len=formatlengthReal(&realnum,Format);

        // RESERVE THE MEMORY FIRST
        rplDecompAppendString2(0,len);

        // NOW USE IT
        string=(BYTEPTR)DecompStringEnd;
        string-=len;

        if(Exceptions) {
            RetNum=ERR_INVALID;
            return;
        }
        DecompStringEnd=(WORDPTR) formatReal(&realnum,(char *)string,Format,fmt.Locale);



        switch(LIBNUM(*DecompileObject)&3)
        {
        case ANGLERAD:
            rplDecompAppendChar('r');
            break;
        case ANGLEGRAD:
            rplDecompAppendChar('g');
            break;
        case ANGLEDEG:
            rplDecompAppendString((BYTEPTR)"°");
            break;
        case ANGLEDMS:
            rplDecompAppendChar('d');
            break;
        }

        RetNum=OK_CONTINUE;
        return;
        }


        // STANDARD COMMAND HANDLING
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

    {
        RetNum=ERR_INVALID;
        return;
    }

    case OPCODE_PROBETOKEN:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // ArgPtr2 = token blanks afterwards
        // ArgNum2 = blanks length

        // COMPILE RETURNS:
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...), or ERR_NOTMINE IF NO TOKEN IS FOUND
    {

        if(LIBNUM(CurOpcode)!=LIBRARY_NUMBER) {
            // DO NOT COMPILE ANYTHING WHEN CALLED WITH THE UPPER (APPROX) LIBRARY NUMBER
            RetNum=ERR_NOTMINE;
            return;
        }

        enum {
            MODE_IP=0,
            MODE_FP,
            MODE_EXPLETTER,
            MODE_EXPSIGN,
            MODE_EXP
        };
        WORD Locale=rplGetSystemLocale();
        BINT mode=MODE_IP;
        BYTE num,tlen=TokenLen;
        int f,exitfor=0;
        BYTEPTR ptr=(BYTEPTR)TokenStart;

        if(utf8ncmp((char *)ptr,"∡",1)) {
            RetNum=ERR_NOTMINE;
            return;
        }
        ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)BlankStart);
        tlen--;

        for(f=0;f<(int)tlen;++f,++ptr) {
            num=*ptr;
            switch(mode)
            {
            case MODE_IP:
                if(num==DECIMAL_DOT(Locale)) { mode=MODE_FP; break; }
                //if(num==THOUSAND_SEP(Locale)) { break; }
                if((f!=0) && (num=='e' || num=='E' || num==EXP_LETTER(Locale))) { mode=MODE_EXPSIGN; break; }
                if(num<'0' || num>'9') { exitfor=1; break; }
                break;
            case MODE_FP:
                //if(num==FRAC_SEP(Locale)) { break; }
                if(num=='.') { mode=MODE_EXPLETTER; break; }
                if(num=='e' || num=='E' || num==EXP_LETTER(Locale)) { mode=MODE_EXPSIGN; break; }
                if(num<'0' || num>'9') { exitfor=1; break; }
                break;
            case MODE_EXPLETTER:
                if(num=='e' || num=='E' || num==EXP_LETTER(Locale)) { mode=MODE_EXPSIGN; break; }
                exitfor=1;
                break;
            case MODE_EXPSIGN:
                if(num=='+' || num=='-') { mode=MODE_EXP; break; }
                if(num<'0' || num>'9') { exitfor=1; break; }
                mode=MODE_EXP;
                break;
            case MODE_EXP:
                if(num<'0' || num>'9') { exitfor=1; break; }
                break;
            }
            if(exitfor) break;
        }

        if(mode==MODE_EXPSIGN) --f;
        if(f==0) RetNum=ERR_NOTMINE;

        else {
            if(f<(int)tlen) {
                if(num=='.') { ++f; ++ptr; num=*ptr; }
                if(f<(int)tlen) {
                    if(!utf8ncmp((char *)ptr,"°",1)) ++f;
                    else
                        if((num=='d')||(num=='D')||(num=='g')||(num=='G')||(num=='r')||(num=='R')) ++f;
                }
            }

            RetNum=OK_TOKENINFO | MKTOKENINFO(f+1,TITYPE_REAL,0,1);
        }

        return;
    }

    case OPCODE_GETINFO:
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_REAL,0,1);
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
            if(OBJSIZE(*ObjectPTR)<2) { RetNum=ERR_INVALID; return; }
            if(rplSkipOb(ObjectPTR)!=rplSkipOb(ObjectPTR+1)) { RetNum=ERR_INVALID; return; }
            if(!ISNUMBER(ObjectPTR[1])) { RetNum=ERR_INVALID; return; }
            if(ISREAL(ObjectPTR[1])) {
            REAL r;
            rplReadReal(ObjectPTR+1,&r);
            // CHECK PROPER LENGTH
            if((WORD)(r.len+1)!=OBJSIZE(*ObjectPTR)) { RetNum=ERR_INVALID; return; }
            // CHECK FOR CORRUPTED DATA
            BINT k;
            for(k=0;k<r.len;++k) {
                // IF THE NUMBER IS NOT NORMALIZED, ASSUME IT WAS CORRUPTED
                if( (r.data[k]<0) || (r.data[k]>=100000000) ) { RetNum=ERR_INVALID; return; }
            }
            }
        }
        RetNum=OK_CONTINUE;
        return;


    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        //RetNum=ERR_NOTMINE;
        return;

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
