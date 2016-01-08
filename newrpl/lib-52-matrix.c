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
#define LIBRARY_NUMBER  52


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    ECMD(TOARRAY,"→ARRY",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ARRAYDECOMP,"ARRY→",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TOCOL,"→COL",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ADDCOL,"COL+",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(REMCOL,"COL-",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(FROMCOL,"COL→",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TODIAG,"→DIAG",MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    ECMD(FROMDIAG,"DIAG→",MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    ECMD(TOROW,"→ROW",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ADDROW,"ROW+",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(REMROW,"ROW-",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(FROMROW,"ROW→",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TOV2,"→V2",MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    ECMD(TOV3,"→V3",MKTOKENINFO(3,TITYPE_FUNCTION,3,2)), \
    ECMD(FROMV,"V→",MKTOKENINFO(2,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DOMATPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DOMATPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DOMATERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    CMD(AUGMENT,MKTOKENINFO(7,TITYPE_FUNCTION,1,2)), \
    CMD(AXL,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(AXM,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(BASIS,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(CHOLESKY,MKTOKENINFO(8,TITYPE_FUNCTION,1,2)), \
    CMD(CNRM,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(CON,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(COND,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(CROSS,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(CSWP,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(DET,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(DIAGMAP,MKTOKENINFO(7,TITYPE_FUNCTION,1,2)), \
    CMD(DOT,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(EGV,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(EGVL,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(GRAMSCHMIDT,MKTOKENINFO(11,TITYPE_FUNCTION,1,2)), \
    CMD(HADAMARD,MKTOKENINFO(8,TITYPE_FUNCTION,1,2)), \
    CMD(HILBERT,MKTOKENINFO(7,TITYPE_FUNCTION,1,2)), \
    CMD(IBASIS,MKTOKENINFO(6,TITYPE_FUNCTION,1,2)), \
    CMD(IDN,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(IMAGE,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(ISOM,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(JORDAN,MKTOKENINFO(6,TITYPE_FUNCTION,1,2)), \
    CMD(KER,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(LQ,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(LSQ,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(LU,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(MAD,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(MKISOM,MKTOKENINFO(6,TITYPE_FUNCTION,1,2)), \
    CMD(PMINI,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(QR,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(RANK,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(RANM,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(RCI,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(RCIJ,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(RDM,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(REF,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(RNRM,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(RREF,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(RREFMOD,MKTOKENINFO(7,TITYPE_FUNCTION,1,2)), \
    CMD(RSD,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(RSWP,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(SCHUR,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(SNRM,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(SRAD,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(SVD,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(SVL,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(SYLVESTER,MKTOKENINFO(9,TITYPE_FUNCTION,1,2)), \
    CMD(TRACE,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(TRAN,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(TRN,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(VANDERMONDE,MKTOKENINFO(11,TITYPE_FUNCTION,1,2))



// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************


const WORD const matrixeval_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,DOMATPRE),     // PREPARE EACH ELEMENT
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,DOMATPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,DOMATERR),     // ERROR HANDLER
    CMD_SEMI
};

const WORD const matrixeval1_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,DOMATPRE),     // PREPARE EACH ELEMENT
    (CMD_OVR_EVAL1),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,DOMATPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,DOMATERR),     // ERROR HANDLER
    CMD_SEMI
};

const WORD const matrixtonum_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,DOMATPRE),     // PREPARE EACH ELEMENT
    (CMD_OVR_NUM),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,DOMATPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,DOMATERR),     // ERROR HANDLER
    CMD_SEMI
};









void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        rplPushData(IPtr);
        return;
    }


    if(ISUNARYOP(CurOpcode)) {
        if(!ISPROLOG(*rplPeekData(1))) {
            if( (OPCODE(CurOpcode)==OVR_EVAL)||
                    (OPCODE(CurOpcode)==OVR_EVAL1)||
                    (OPCODE(CurOpcode)==OVR_XEQ) )
            {

                WORD saveOpcode=CurOpcode;
                CurOpcode=*rplPopData();
                // RECURSIVE CALL
                LIB_HANDLER();
                CurOpcode=saveOpcode;
                return;
            }
            else {
                rplError(ERR_INVALIDOPCODE);
                return;
            }
        }

        // UNARY OPERATORS
        switch(OPCODE(CurOpcode))
        {

        case OVR_INV:
        {
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR a=rplPeekData(1);

            if(!ISMATRIX(*a)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            rplMatrixInvert();
            return;
        }
        case OVR_ABS:
        {
           // COMPUTE THE FROBENIUS NORM ON MATRICES
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR a=rplPeekData(1);

            if(!ISMATRIX(*a)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            rplMatrixNorm();
            return;

        }
        case OVR_NEG:
        {
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR a=rplPeekData(1);

            if(!ISMATRIX(*a)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            rplMatrixNeg();
            return;

        }
        case OVR_EVAL1:
            // EVAL NEEDS TO SCAN THE MATRIX, EVAL EACH ARGUMENT SEPARATELY AND REBUILD IT.
        {

            WORDPTR object=rplPeekData(1),mainobj;
            if(!ISMATRIX(*object)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }
            mainobj=object;

            // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
            rplCreateLAMEnvironment(IPtr);

            object=rplMatrixGetFirstObj(object);
            WORDPTR endobject=rplSkipOb(mainobj);

            rplCreateLAM((WORDPTR)nulllam_ident,endobject);     // LAM 1 = END OF CURRENT LIST
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,object);     // LAM 2 = NEXT OBJECT TO PROCESS
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,mainobj);     // LAM 3 = ORIGINAL MATRIX
            if(Exceptions) { rplCleanupLAMs(0); return; }

            // GETLAM 1 = END OF MATRIX, GETLAM2 = OBJECT, GETLAM3 = ORIGINAL MATRIX

            // THIS NEEDS TO BE DONE IN 3 STEPS:
            // EVAL WILL PREPARE THE LAMS FOR OPEN EXECUTION
            // MATPRE WILL PUSH THE NEXT OBJECT IN THE STACK
            // MATPOST WILL CHECK IF THE ARGUMENT WAS PROCESSED WITHOUT ERRORS,
            // AND CLOSE THE LOOP TO PROCESS MORE ARGUMENTS

            // THE INITIAL CODE FOR EVAL MUST TRANSFER FLOW CONTROL TO A
            // SECONDARY THAT CONTAINS :: MATPRE EVAL MATPOST ;
            // MATPOST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
            // IN ORDER TO KEEP THE LOOP RUNNING

            rplPushRet(IPtr);
            IPtr=(WORDPTR)  matrixeval1_seco;
            CurOpcode=(CMD_OVR_EVAL1);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

            rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD EVALUATION

            return;
        }
        case OVR_EVAL:
            // EVAL NEEDS TO SCAN THE MATRIX, EVAL EACH ARGUMENT SEPARATELY AND REBUILD IT.
        {
            WORDPTR object=rplPeekData(1),mainobj;
            if(!ISMATRIX(*object)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }
            mainobj=object;

            // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
            rplCreateLAMEnvironment(IPtr);

            object=rplMatrixGetFirstObj(object);
            WORDPTR endobject=rplSkipOb(mainobj);

            rplCreateLAM((WORDPTR)nulllam_ident,endobject);     // LAM 1 = END OF CURRENT LIST
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,object);     // LAM 2 = NEXT OBJECT TO PROCESS
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,mainobj);     // LAM 3 = ORIGINAL MATRIX
            if(Exceptions) { rplCleanupLAMs(0); return; }

            // GETLAM 1 = END OF MATRIX, GETLAM2 = OBJECT, GETLAM3 = ORIGINAL MATRIX

            // THIS NEEDS TO BE DONE IN 3 STEPS:
            // EVAL WILL PREPARE THE LAMS FOR OPEN EXECUTION
            // MATPRE WILL PUSH THE NEXT OBJECT IN THE STACK
            // MATPOST WILL CHECK IF THE ARGUMENT WAS PROCESSED WITHOUT ERRORS,
            // AND CLOSE THE LOOP TO PROCESS MORE ARGUMENTS

            // THE INITIAL CODE FOR EVAL MUST TRANSFER FLOW CONTROL TO A
            // SECONDARY THAT CONTAINS :: MATPRE EVAL MATPOST ;
            // MATPOST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
            // IN ORDER TO KEEP THE LOOP RUNNING

            rplPushRet(IPtr);
            IPtr=(WORDPTR)  matrixeval_seco;
            CurOpcode=(CMD_OVR_EVAL);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

            rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD EVALUATION

            return;
        }
        case OVR_NUM:
            // EVAL NEEDS TO SCAN THE MATRIX, EVAL EACH ARGUMENT SEPARATELY AND REBUILD IT.
        {
            WORDPTR object=rplPeekData(1),mainobj;
            if(!ISMATRIX(*object)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }
            mainobj=object;

            // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
            rplCreateLAMEnvironment(IPtr);

            object=rplMatrixGetFirstObj(object);
            WORDPTR endobject=rplSkipOb(mainobj);

            rplCreateLAM((WORDPTR)nulllam_ident,endobject);     // LAM 1 = END OF CURRENT LIST
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,object);     // LAM 2 = NEXT OBJECT TO PROCESS
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,mainobj);     // LAM 3 = ORIGINAL MATRIX
            if(Exceptions) { rplCleanupLAMs(0); return; }

            // GETLAM 1 = END OF MATRIX, GETLAM2 = OBJECT, GETLAM3 = ORIGINAL MATRIX

            // THIS NEEDS TO BE DONE IN 3 STEPS:
            // EVAL WILL PREPARE THE LAMS FOR OPEN EXECUTION
            // MATPRE WILL PUSH THE NEXT OBJECT IN THE STACK
            // MATPOST WILL CHECK IF THE ARGUMENT WAS PROCESSED WITHOUT ERRORS,
            // AND CLOSE THE LOOP TO PROCESS MORE ARGUMENTS

            // THE INITIAL CODE FOR EVAL MUST TRANSFER FLOW CONTROL TO A
            // SECONDARY THAT CONTAINS :: MATPRE EVAL MATPOST ;
            // MATPOST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
            // IN ORDER TO KEEP THE LOOP RUNNING

            rplPushRet(IPtr);
            IPtr=(WORDPTR)  matrixtonum_seco;
            CurOpcode=(CMD_OVR_NUM);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

            rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD EVALUATION

            return;
        }
        case OVR_XEQ:
            // XEQ NEEDS TO LEAVE THE MATRIX ON THE STACK
            return;

        }

    }

    if(ISBINARYOP(CurOpcode)) {

        // TODO: IMPLEMENT BINARY OPERATORS

        switch(OPCODE(CurOpcode))
        {
        case OVR_ADD:
        {
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);

                return;
            }
            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            if(!ISMATRIX(*a) || !ISMATRIX(*b)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            rplMatrixAdd();
            return;
        }
        case OVR_SUB:
        {
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            if(!ISMATRIX(*a) || !ISMATRIX(*b)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            rplMatrixSub();
            return;
        }
        case OVR_MUL:
        {
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            // NEED TO DETECT SCALAR * MATRIX AND MATRIX * SCALAR CASES
            if((ISNUMBERCPLX(*a)
                  || ISSYMBOLIC(*a)
                  || ISIDENT(*a))) {
                // SCALAR BY MATRIX
                rplMatrixMulScalar();

                return;
            }
            if((ISNUMBERCPLX(*b)
                  || ISSYMBOLIC(*b)
                  || ISIDENT(*b))) {
                // SCALAR BY MATRIX
                rplMatrixMulScalar();

                return;
            }

            // HERE IT HAS TO BE MATRIX x MATRIX

            if(!ISMATRIX(*a) || !ISMATRIX(*b)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            rplMatrixMul();
            return;
        }

        case OVR_DIV:
        {
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            // NEED TO DETECT SCALAR / MATRIX AND MATRIX / SCALAR CASES
            if((ISNUMBERCPLX(*a)
                  || ISSYMBOLIC(*a)
                  || ISIDENT(*a))) {
                // SCALAR BY MATRIX
                rplError(ERR_MATRIXEXPECTED);
                return;
            }
            if((ISNUMBERCPLX(*b)
                  || ISSYMBOLIC(*b)
                  || ISIDENT(*b))) {
                // MATRIX DIV BY SCALAR
                rplCallOvrOperator((CMD_OVR_INV));
                if(Exceptions) return;
                rplMatrixMulScalar();

                return;
            }

            // HERE IT HAS TO BE MATRIX / MATRIX

            if(!ISMATRIX(*a) || !ISMATRIX(*b)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            // PERFORM A*INV(B)

            rplMatrixInvert();
            if(Exceptions) return;
            rplMatrixMul();
            return;


        }

        case OVR_POW:
        {
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            // ONLY MATRIX RAISED TO NUMERIC POWER IS SUPPORTED
            if(!ISMATRIX(*a))
            {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }
            if( !ISNUMBER(*b)) {
                rplError(ERR_INTEGEREXPECTED);
                return;
            }

            if(ISREAL(*b)) {
                REAL real;
                rplReadReal(b,&real);
                if(!isintegerReal(&real)) {
                    rplError(ERR_INTEGEREXPECTED);
                    return;
                }

            }
                BINT rows=MATROWS(a[1]),cols=MATCOLS(a[1]);

                if(rows!=cols) {
                    rplError(ERR_SQUAREMATRIXONLY);
                    return;
                }

                // TODO: CHECK FOR INTEGER RANGE AND ISSUE "Integer too large" ERROR
                BINT64 exp=rplReadNumberAsBINT(b);
                if(Exceptions) return;
                rplPopData();
                if(exp<0) {
                 rplMatrixInvert();
                 if(Exceptions) return;
                 exp=-exp;
                }

                BINT hasresult=0;
                while(exp) {
                if(exp&1) {
                    if(!hasresult) { rplPushData(rplPeekData(1));   // DUP THE CURRENT MATRIX
                                     hasresult=1;
                                }
                                else {
                                        rplPushData(rplPeekData(2));
                                        rplPushData(rplPeekData(2));
                                        rplMatrixMul();
                                        if(Exceptions) return;
                                        rplOverwriteData(3,rplPeekData(1));
                                        rplPopData();
                                 }
                }
                exp>>=1;
                if(exp) {
                rplPushData(rplPeekData(1));
                rplMatrixMul();
                if(Exceptions) return;
                }
                }

                rplPopData();
                return;
            }

        break;
        }
    }

    switch(OPCODE(CurOpcode))
    {
    case TOARRAY:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        BINT64 rows,cols;
        WORDPTR *Savestk=DSTop;

        if(ISLIST(*rplPeekData(1))) {
            rplExplodeList(rplPeekData(1));
            BINT ndims=rplReadNumberAsBINT(rplPopData());
            if((ndims<1) || (ndims>2)) {
                DSTop=Savestk;
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

            cols=rplReadNumberAsBINT(rplPopData());
            if(Exceptions) {
                DSTop=Savestk;
                return;
            }

            if(ndims==2) {

                rows=rplReadNumberAsBINT(rplPopData());
                if(Exceptions) {
                    DSTop=Savestk;
                    return;
                }



            } else rows=0;


            if( (rows<0)||(rows>65535)||(cols<1)||(cols>65535))  {
                DSTop=Savestk;
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

            // REMOVE THE LIST
            rplDropData(1);

            }
        else {
            // IT HAS TO BE A NUMBER

            cols=rplReadNumberAsBINT(rplPopData());
            if(Exceptions) {
                DSTop=Savestk;
                return;
            }

            rows=0;

            if((cols<1)||(cols>65535))  {
                DSTop=Savestk;
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

        }

        // HERE WE HAVE PROPER ROWS AND COLUMNS
        BINT elements=(rows)? rows*cols:cols;

        if(rplDepthData()<elements) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR newmat=rplMatrixCompose(rows,cols);

        if(newmat) {
            rplDropData(elements);
            rplPushData(newmat);
        }

        return;
        
       }
    case ARRAYDECOMP:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISMATRIX(*rplPeekData(1))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        WORDPTR matrix=rplPeekData(1);
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

        WORDPTR *elem=rplMatrixExplode();
        if(Exceptions) return;

        // NOW REMOVE THE ORIGINAL MATRIX FROM THE STACK
        memmovew(elem-1,elem,(DSTop-elem)*(sizeof(WORDPTR *)/sizeof(WORD))); // ADDED sizeof() ONLY FOR 64-BIT COMPATIBILITY

        DSTop--;


        if(rows) rplNewBINTPush(rows,DECBINT);
        rplNewBINTPush(cols,DECBINT);
        rplPushData((WORDPTR)((rows)? two_bint : one_bint));
        rplCreateList();


        return;
    }
    case TOCOL:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISMATRIX(*rplPeekData(1))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        WORDPTR matrix=rplPeekData(1);
        WORDPTR *matptr=DSTop-1;
        WORDPTR elem;
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
        BINT nrows=(rows)? rows:1;
        BINT i,j;

        for(j=1;j<=cols;++j) {
        for(i=1;i<=nrows;++i) rplPushData(rplMatrixFastGet(*matptr,i,j));
        if(rows) {
            elem=rplMatrixCompose(0,nrows);
            if(!elem) return;
            rplDropData(nrows);
        } else elem=rplPopData();

        rplPushData(*matptr);
        rplOverwriteData(2,elem);
        matptr=DSTop-1;
        }

        rplDropData(1);
        rplNewBINTPush(cols,DECBINT);

        return;

    }
    case ADDCOL:
    {
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISMATRIX(*rplPeekData(3))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }
        if(!ISMATRIX(*rplPeekData(2)) && !ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_MATRIXORREALEXPECTED);
            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        BINT64 nelem=rplReadNumberAsBINT(rplPeekData(1));





        WORDPTR matrix=rplPeekData(3);
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

        WORDPTR *matptr=DSTop-3;

        if( (nelem<1)||(nelem>cols+1)) {
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }


        if(!rows) {
            // ADD ELEMENTS TO A VECTOR

            // CHECK VALID TYPES FOR MATRIX ELEMENTS
            if(!ISNUMBERCPLX(*rplPeekData(2)) && !ISSYMBOLIC(*rplPeekData(2))
                  && !ISIDENT(*rplPeekData(2)))
             {
                rplError(ERR_NOTALLOWEDINMATRIX);
                return;
            }

            WORDPTR *first=rplMatrixNewEx(1,cols+1);

            if(!first) {
                return;
            }


            BINT j;
            WORDPTR *stkelem;
            for(j=1;j<nelem;++j) {
                stkelem=rplMatrixFastGetEx(first,cols+1,1,j);
                *stkelem=rplMatrixFastGet(*matptr,1,j);
            }

            stkelem=rplMatrixFastGetEx(first,cols+1,1,j);
            *stkelem=matptr[1]; // THE NEW ELEMENT MIGHT HAVE MOVED, SO GET IT FROM THE STACK
            ++j;

            for(;j<=cols+1;++j) {
                stkelem=rplMatrixFastGetEx(first,cols+1,1,j);
                *stkelem=rplMatrixFastGet(*matptr,1,j-1);
            }

            // MAKE A NEW VECTOR

            WORDPTR newmat=rplMatrixCompose(0,cols+1);
            if(!newmat) {
                DSTop=matptr+3;
                return;
            }

            rplDropData(cols+4);
            rplPushData(newmat);
            return;

        }

        // ADD A VECTOR OR A MATRIX TO A MATRIX

        if(!ISMATRIX(*rplPeekData(2))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }



        WORDPTR mat2=rplPeekData(2);
        BINT rows2=MATROWS(mat2[1]),cols2=MATCOLS(mat2[1]);


        // CHECK PROPER SIZE

        if(!rows2) {
            // MAKE IT A COLUMN VECTOR
            rows2=cols2;
            cols2=1;
        }

        if(rows2!=rows) {
            rplError(ERR_INVALIDDIMENSION);
            return;
        }

        // ADD THE COLUMNS

        WORDPTR *first=rplMatrixNewEx(rows,cols+cols2);

        if(!first) {
            return;
        }


        BINT i,j;
        WORDPTR *stkelem;
        for(j=1;j<nelem;++j) {
            for(i=1;i<=rows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols+cols2,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j);
            }
        }

        for(;j<nelem+cols2;++j) {
            for(i=1;i<=rows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols+cols2,i,j);
            *stkelem=rplMatrixFastGet(matptr[1],i,j-nelem+1);
            }
        }

        for(;j<=cols+cols2;++j) {
            for(i=1;i<=rows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols+cols2,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j-cols2);
            }
        }

        // MAKE A NEW VECTOR

        WORDPTR newmat=rplMatrixCompose(rows,cols+cols2);
        if(!newmat) {
            DSTop=matptr+3;
            return;
        }

        rplDropData(rows*(cols+cols2)+3);
        rplPushData(newmat);
        return;
    }
    case REMCOL:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISMATRIX(*rplPeekData(2))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        BINT64 ncol=rplReadNumberAsBINT(rplPeekData(1));

        WORDPTR matrix=rplPeekData(2);
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
        BINT nrows=(rows)? rows:1;

        WORDPTR *matptr=DSTop-2;

        if(cols<=1) {
            rplError(ERR_INVALIDDIMENSION);
            return;
        }


        if( (ncol<1)||(ncol>cols)) {
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }

        // MAKE THE NEW MATRIX WITHOUT ONE COLUMN

        WORDPTR *first=rplMatrixNewEx(rows,cols-1);

        if(!first) {
            return;
        }


        BINT i,j;
        WORDPTR *stkelem;
        for(j=1;j<ncol;++j) {
            for(i=1;i<=nrows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols-1,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j);
            }
        }

        // SEPARATE THE COLUMN VECTOR/ELEMENT
        for(i=1;i<=nrows;++i) {
        rplPushData(rplMatrixFastGet(*matptr,i,j));
        }


        for(;j<=cols-1;++j) {
            for(i=1;i<=nrows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols-1,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j+1);
            }
        }


        // MAKE THE VECTOR FROM THE ELEMENTS
        WORDPTR newmat;

        if(rows) {
            newmat=rplMatrixCompose(0,nrows);
            if(!newmat) {
                DSTop=matptr+2;
                return;
            }
            rplDropData(nrows);

        } else newmat=rplPopData();

        matptr[1]=newmat;   //  OVERWRITE THE FIRST ARGUMENT WITH THE VECTOR


        // MAKE A NEW VECTOR/MATRIX

        newmat=rplMatrixCompose(rows,cols-1);
        if(!newmat) {
            DSTop=matptr+2;
            return;
        }

        rplDropData(nrows*(cols-1));

        rplOverwriteData(2,newmat);

        return;

    }
    case FROMCOL:
        {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        BINT64 nelem=rplReadNumberAsBINT(rplPeekData(1));

        if(rplDepthData()<nelem+1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        BINT i;
        BINT veclen=0;

        for(i=2;i<=nelem+1;++i) {
            if(ISMATRIX(*rplPeekData(i))) {
                WORDPTR matrix=rplPeekData(i);
                BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

                if(rows) {
                    rplError(ERR_VECTOREXPECTED);
                    return;
                }

                if(veclen) {
                    if(cols!=veclen) {
                        rplError(ERR_INVALIDDIMENSION);
                        return;
                    }
                } else {
                    if(i==2) veclen=cols;
                    else {
                        // DON'T ALLOW MIX OF VECTOR/NUMBERS
                        rplError(ERR_REALEXPECTED);
                        return;
                    }
                }
            }
            else {
                if(! (ISNUMBERCPLX(*LastCompiledObject)
                      || ISSYMBOLIC(*LastCompiledObject)
                      || ISIDENT(*LastCompiledObject))) {
                    rplError(ERR_NOTALLOWEDINMATRIX);
                            return;
                }

                if(veclen) {
                    rplError(ERR_VECTOREXPECTED);
                    return;
                }

            }
        }

        // HERE WE HAVE ALL ELEMENTS PROPERLY VALIDATED





        }
    case TODIAG:
    case FROMDIAG:
    case TOROW:
    case ADDROW:
    case REMROW:
    case FROMROW:
    case TOV2:
    case TOV3:
    case FROMV:
    case DOMATPRE:
    {

        // GETLAM 1 = END OF MATRIX, GETLAM2 = OBJECT, GETLAM3 = ORIGINAL MATRIX


        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE DOLISTERR WORD

        // NOW RECALL THE MATRIX OBJECT TO THE STACK

        rplPushData(*rplGetLAMn(2));

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case DOMATPOST:
    {

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR DOING THE EVALUATION

        // GETLAM 1 = END OF MATRIX, GETLAM2 = OBJECT, GETLAM3 = ORIGINAL MATRIX
        WORDPTR endobj=*rplGetLAMn(1),
                nextobj=rplSkipOb(*rplGetLAMn(2)),
                matrix=*rplGetLAMn(3);

        if(nextobj<endobj) {
            // NEED TO DO ONE MORE LOOP
            rplPutLAMn(2,nextobj);  // STORE NEW OBJECT

            IPtr-=3;   // CONTINUE THE LOOP, MAKE NEXT INSTRUCTION BE DOMATPRE ON ANY LOOP (2 INSTRUCTIONS BACK)
            // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
            return;
        }

        // ALL ELEMENTS WERE PROCESSED
        // FORM A NEW MATRIX WITH ALL THE NEW ELEMENTS

        WORDPTR *prevDStk = rplUnprotectData();

        BINT newdepth=(BINT)(DSTop-prevDStk);

        // COMPUTE THE REQUIRED SIZE

        BINT totalsize=rplMatrixGetFirstObj(matrix)-matrix;
        BINT k;
        for(k=1;k<=newdepth;++k) {
            totalsize+=rplObjSize(rplPeekData(k));
        }

        // NOW ALLOCATE THE NEW MATRIX

         WORDPTR newmat=rplAllocTempOb(totalsize-1);
        if( (!newmat) || Exceptions) {
            DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
            rplCleanupLAMs(0);      // CLEANUP LOCAL VARIABLES
            CurOpcode=*(IPtr-1);    // BLAME THE OPERATOR IN QUESTION
            IPtr=rplPopRet();       // AND RETURN
            return;
        }

        // RE-READ ALL POINTERS, SINCE THEY COULD'VE MOVED
        matrix=*rplGetLAMn(3);
        nextobj=rplMatrixGetFirstObj(matrix);       // FIRST OBJECT = END OF TABLES
        newmat[0]=MKPROLOG(DOMATRIX,totalsize-1);
        newmat[1]=matrix[1];    // SAME SIZE AS ORIGINAL MATRIX

        BINT nelem=nextobj-matrix-2;
        BINT oldidx;
        WORDPTR oldobj,newobj,firstobj,oldfirst,oldptr;

        // FILL THE MATRIX WITH ALL THE OBJECTS FROM THE STACK
        firstobj=newobj=rplMatrixGetFirstObj(newmat);        // STORE NEW OBJECTS HERE
        for(k=newdepth;k>=1;--k) { rplCopyObject(newobj,rplPeekData(k)); newobj=rplSkipOb(newobj); }

        oldfirst=rplMatrixGetFirstObj(matrix);

        for(k=0;k<nelem;++k) {
            // GET THE INDEX NUMBER OF THE OBJECT FROM THE OFFSET
            oldobj=matrix+matrix[2+k];
            for(oldidx=0,oldptr=oldfirst;oldptr<oldobj;++oldidx) oldptr=rplSkipOb(oldptr);
            // FIND THE OBJECT ON THE NEW MATRIX
            for(newobj=firstobj;oldidx>0;--oldidx) newobj=rplSkipOb(newobj);
            // STORE THE NEW OFFSET
            newmat[2+k]=newobj-newmat;
        }



        // HERE THE STACK HAS: MATRIX ELEM1... ELEMN
        rplOverwriteData(newdepth+1,newmat);
        rplDropData(newdepth);

        rplCleanupLAMs(0);
        CurOpcode=*(IPtr-1);    // BLAME THE OPERATOR IN QUESTION
        IPtr=rplPopRet();       // AND RETURN
        return;
    }

    case DOMATERR:
    {
        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        CurOpcode=*(IPtr-2);    // BLAME THE OPERATOR IN QUESTION
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;
        ExceptionPointer=IPtr;
        return;
    }




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

        // CHECK IF THE TOKEN IS THE OPEN BRACKET

        if(*((char * )TokenStart)=='[')
        {
            if(LIBNUM(CurrentConstruct)==LIBRARY_NUMBER) {
                // WE ARE COMPILING OBJECTS INSIDE A MATRIX ALREADY
               if(CurrentConstruct==MKPROLOG(LIBRARY_NUMBER,0)) {
                // WE ARE IN THE OUTER DIMENSION
                // INCREASE DEPTH OF DIMENSION AND ACCEPT
                // WARNING, THIS USES INTERNAL COMPILER WORKINGS
                WORDPTR matrix=*(ValidateTop-1);
                ++*matrix;
                // CHECK IF THERE IS A SIZE WORD YET
                if(CompileEnd==matrix+1) {
                    // THIS IS THE FIRST OBJECT IN THE ARRAY
                    // ADD A DUMMY WORD
                    rplCompileAppend(MATMKSIZE(1,0));
                }
                else {
                    // THERE SHOULD BE A SIZE WORD ALREADY
                    // INCREASE THE ROW COUNT
                    BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
                    if(!rows) {
                        // VECTOR CAN'T OPEN A SECOND DIMENSION
                        RetNum=ERR_SYNTAX;
                        return;
                    }
                    matrix[1]=MATMKSIZE(rows+1,cols);
                }



                if(TokenLen>1) NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                RetNum=OK_CONTINUE_NOVALIDATE;
                return;
                }
                else {
                    // MORE THAN 2 DIMENSIONS ARE NOT SUPPORTED
                    RetNum=ERR_NOTMINE;
                    return;
               }
            }

            rplCompileAppend((WORD) MKPROLOG(LIBRARY_NUMBER,0));
            if(TokenLen>1) {
                NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                RetNum=OK_STARTCONSTRUCT;
            }
            else RetNum=OK_STARTCONSTRUCT;
            return;
        }
        // CHECK IF THE TOKEN IS THE CLOSING BRACKET

        if(*utf8nskip((char * )TokenStart,(char *)BlankStart,TokenLen-1)==']')
        {

            if(TokenLen>1) {
                BlankStart=NextTokenStart=(WORDPTR)utf8nskip((char * )TokenStart,(char *)BlankStart,TokenLen-1);
                RetNum=ERR_NOTMINE_SPLITTOKEN;
                return;
            }

            if(LIBNUM(CurrentConstruct)!=LIBRARY_NUMBER) {
                RetNum=ERR_SYNTAX;
                return;
            }
            WORDPTR matrix=*(ValidateTop-1);
            BINT rows,cols;
            BINT totalelements;

            if(CompileEnd>matrix+1) {
            rows=MATROWS(matrix[1]);
            cols=MATCOLS(matrix[1]);
            if(rows==0) totalelements=cols;
            else totalelements=rows*cols;
            } else rows=cols=totalelements=0;

            if(CurrentConstruct!=MKPROLOG(LIBRARY_NUMBER,0)) {
                // CLOSED AN INNER DIMENSION

                // CAN'T CLOSE AN EMPTY MATRIX
                if(!totalelements) {
                    RetNum=ERR_SYNTAX;
                    return;
                }

                // DECREASE DIMENSION COUNT
                --*matrix;


                // CHECK FULL ROW SIZE IS CORRECT
                // BY CHECKING THE NEXT EMPTY OBJECT IS THE START OF A ROW

                BINT count;
                WORDPTR index=matrix+2;

                count=0;
                while((count<totalelements) && (index<CompileEnd)) { ++count; index=rplSkipOb(index); }

                if(count%cols) {
                    // INVALID MATRIX SIZE
                    RetNum=ERR_SYNTAX;
                    return;
                }


                if(TokenLen>1) NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                RetNum=OK_CONTINUE_NOVALIDATE;
                return;

            }

            // CLOSE THE MATRIX OBJECT
            if(!totalelements) {
                RetNum=ERR_SYNTAX;
                return;
            }

            // STRETCH THE OBJECT, ADD THE INDEX AND REMOVE DUPLICATES
            WORDPTR endofobjects=rplCompileAppendWords(totalelements);
            if(Exceptions) return;

            // MAKE HOLE IN MEMORY
            memmovew(matrix+2+totalelements,matrix+2,endofobjects-(matrix+2));
            endofobjects+=totalelements;

            // NOW WRITE THE INDICES. ALL OFFSETS ARE RELATIVE TO MATRIX PROLOG!
            WORDPTR ptr=matrix+2,objptr=ptr+totalelements,nextobj,index;
            BINT count=0;

            while( (objptr<endofobjects)&&(count<totalelements)) {
                *ptr=objptr-matrix;
                ++ptr;
                ++count;
                objptr=rplSkipOb(objptr);
            }

            if( (count!=totalelements)||(objptr!=endofobjects)) {
                // MALFORMED MATRIX IS MISSING OBJECTS
                RetNum=ERR_INVALID;
                return;
            }

            // COMPACT MATRIX BY REMOVING DUPLICATED OBJECTS
            index=matrix+2;
            objptr=matrix+2+totalelements;

            while(objptr<endofobjects) {
                // CHECK AND REMOVE DUPLICATES OF CURRENT OBJECT
                ptr=rplSkipOb(objptr);
                while(ptr<endofobjects) {
                    if(rplCompareObjects(ptr,objptr)) {
                        // OBJECTS ARE IDENTICAL, REMOVE

                        // REPLACE ALL REFERENCES TO THIS COPY WITH REFERENCES TO THE ORIGINAL
                        for(count=0;count<totalelements;++count) if(index[count]==ptr-matrix) index[count]=objptr-matrix;

                        // AND REMOVE THE COPY
                        nextobj=rplSkipOb(ptr);
                        if(nextobj<endofobjects) {
                            // THERE'S MORE OBJECTS, MOVE ALL MEMORY AND FIX ALL INDICES
                            memmovew(ptr,nextobj,endofobjects-nextobj);
                            for(count=0;count<totalelements;++count) if(index[count]>ptr-matrix) index[count]-=nextobj-ptr;
                        }
                        endofobjects-=nextobj-ptr;
                        rplCompileRemoveWords(nextobj-ptr);
                        // DO NOT ADVANCE ptr, SINCE OBJECTS MOVED
                    }
                    else ptr=rplSkipOb(ptr);
                    }
                objptr=rplSkipOb(objptr);
                }


            RetNum=OK_ENDCONSTRUCT;
            return;
        }

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



        if(ISPROLOG(*DecompileObject)) {
            rplDecompAppendString((BYTEPTR)"[ ");
            BINT rows=MATROWS(*(DecompileObject+1)),cols=MATCOLS(*(DecompileObject+1));
            BINT doublebracket=rows;

            if(!rows) ++rows;

            // SCAN THE INDEX AND OUTPUT ALL OBJECTS INSIDE
            BINT i,j;

            for(i=0;i<rows;++i)
            {
                if(doublebracket) rplDecompAppendString((BYTEPTR)"[ ");
                if(Exceptions) { RetNum=ERR_INVALID; return; }
                for(j=0;j<cols;++j)
                {
                    BINT offset=*(DecompileObject+2+i*cols+j);

                    rplDecompile(DecompileObject+offset,DECOMP_EMBEDDED | ((CurOpcode==OPCODE_DECOMPEDIT)? DECOMP_EDIT:0));    // RUN EMBEDDED
                 if(Exceptions) { RetNum=ERR_INVALID; return; }
                 rplDecompAppendChar(' ');
                }
                if(doublebracket) rplDecompAppendString((BYTEPTR)"] ");
                if(Exceptions) { RetNum=ERR_INVALID; return; }
            }

            rplDecompAppendChar(']');
            if(Exceptions) { RetNum=ERR_INVALID; return; }

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

        // FIRST, CHECK THAT THE OBJECT IS ALLOWED WITHIN AN ARRAY
    {

        if(! (ISNUMBERCPLX(*LastCompiledObject)
              || ISSYMBOLIC(*LastCompiledObject)
              || ISIDENT(*LastCompiledObject))) {
                rplError(ERR_NOTALLOWEDINMATRIX);
                RetNum=ERR_INVALID;
                return;
            }

        WORDPTR matrix=*(ValidateTop-1);
        if(LastCompiledObject==matrix+1) {
            // THIS IS THE FIRST OBJECT IN THE ARRAY
            // ADD A DUMMY WORD
            rplCompileAppend(0);
            // MOVE THE FIRST OBJECT UP IN MEMORY TO MAKE ROOM FOR THE SIZE WORD
            memmovew(LastCompiledObject+1,LastCompiledObject,CompileEnd-1-LastCompiledObject);

            matrix[1]=MATMKSIZE(0,1);

        }

        else {
            // IF THIS IS THE FIRST ROW, INCREASE THE COLUMN COUNT
            BINT dimlevel=OBJSIZE(CurrentConstruct);
            BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
            
            if(rows) {
                // THIS IS A MATRIX, ONLY ALLOW ELEMENTS INSIDE LEVEL 1
                if(!dimlevel) {
                    rplError(ERR_MISPLACEDBRACKETS);
                    RetNum=ERR_INVALID;
                    return;
                }
            } else {
                // THIS IS A VECTOR, ONLY ALLOW ELEMENTS IN THE OUTER LEVEL
                if(dimlevel) {
                    rplError(ERR_MISPLACEDBRACKETS);
                    RetNum=ERR_INVALID;
                    return;
                }

            }
            if(rows<=1) { matrix[1]=MATMKSIZE(rows,cols+1); }

        }



        RetNum=OK_CONTINUE;
        return;
    }

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


        // TODO: CHECK VALIDITY OF A MATRIX, RECURSIVELY VERIFY OBJECTS INSIDE


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
