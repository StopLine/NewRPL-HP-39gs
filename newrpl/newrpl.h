/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef NEWRPL_H
#define NEWRPL_H

#include <stdint.h>
#ifndef UTF8LIB_H
#include "utf8lib.h"
#endif

// EXTERNAL API FOR THE NEWRPL MACHINE - TO BE USED ONLY BY USER LIBRARIES
// BASIC CONSTANTS AND TYPE DEFINITIONS FOR THE RUN ENVIRONMENT

typedef void (*LIBHANDLER)(void);

typedef uint16_t HALFWORD;
typedef uint32_t WORD;
typedef uint8_t BYTE;
typedef WORD *WORDPTR;
typedef BYTE   *BYTEPTR;
typedef int32_t BINT;
typedef uint32_t UBINT;
typedef int64_t BINT64;
typedef uint64_t UBINT64;
#if defined(__LP64__) || defined(_WIN64)
typedef uint64_t PTR2NUMBER;
#else
typedef uint32_t PTR2NUMBER;
#endif



#ifdef __cplusplus
extern "C" {
#endif

// CONSTANTS THAT CONTROL THE MAIN RPL ENGINE

// NUMBER OF STATIC REGISTERS FOR FAST HANDLING OF REAL NUMBERS
#define REAL_REGISTERS 10
// NUMBER OF SIMULTANEOUS CONVERSIONS OF BINT TO REALS THAT CAN BE DONE
#define BINT2REAL      4

// MAXIMUM PRECISION ALLOWED IN THE SYSTEM
// MAKE SURE REAL_SCRATCHMEM CAN HAVE AT LEAST "REAL_REGISTERS*PRECISION_MAX*2/9" WORDS
// WARNING: THIS CONSTANT CANNOT BE CHANGED UNLESS ALL PRECOMPUTED CONSTANT TABLES ARE CHANGED ACCORDINGLY
#define REAL_PRECISION_MAX 2016
#define MAX_USERPRECISION  2000



// SCRATCHPAD MEMORY TO ALLOCATE DIGITS FOR ARBITRARY PRECISION TEMP RESULTS
// THIS IS THE NUMBER OF WORDS, EACH GOOD FOR 9 DIGITS.
#define REAL_REGISTER_STORAGE ((REAL_PRECISION_MAX*2)/8+3)
#define BINT_REGISTER_STORAGE  3
#define EXTRA_STORAGE BINT2REAL*BINT_REGISTER_STORAGE

// DEFINE THE LIMITS FOR THE EXPONENT RANGE FOR ALL REALS
// NOTE: THIS HAS TO FIT WITHIN THE FIELDS OF REAL_HEADER
#define REAL_EXPONENT_MAX   30000
#define REAL_EXPONENT_MIN   -30000


#ifndef DECIMAL_H
#include "decimal.h"
#endif





// HIGH LIBRARIES ARE USER LIBRARIES, SLOWER TO EXECUTE
// LOW LIBRARIES ARE SYSTEM LIBRARIES, WITH FASTER EXECUTION
#define MAXHILIBS 256
#define MAXLOWLIBS 256
#define MAXSYSHILIBS 16
#define MAXLIBNUMBER 4095
// NUMBER OF SCRATCH POINTERS
#define MAX_GC_PTRUPDATE 21


// MINIMUM GUARANTEED STACK LEVELS FOR MEMORY ALLOCATION
#define DSTKSLACK   16
// MINIMUM GUARANTEED STACK LEVELS IN RETURN STACK
#define RSTKSLACK   16
// MINIMUM GUARANTEED STACK LEVELS IN LAM STACK
#define LAMSLACK   16
// MINIMUM GUARANTEED SPACE IN TEMPOB (IN 32-BIT WORDS)
#define TEMPOBSLACK 32
// MINIMUM GUARANTEED SPACE IN TEMPBLOCKS (IN 32-BIT WORDS)
#define TEMPBLOCKSLACK 16

// MINIMUM GUARANTEED STACK LEVELS IN DIRECTORY STACK
#define DIRSLACK   16


#include "sysvars.h"






// INTERNAL TRANSCENDENTAL FUNCTIONS
void hyp_exp(REAL *);
void hyp_ln(REAL *);
void hyp_sqrt(REAL *);
void hyp_sinhcosh(REAL *);
void hyp_atanh(REAL *);
void hyp_asinh(REAL *);
void hyp_acosh(REAL *);

void trig_sincos(REAL *);
void trig_atan2(REAL *,REAL *);
void trig_asin(REAL *);
void trig_acos(REAL *);


// ERROR MANAGEMENT FUNCTIONS
extern void decTrapHandler(BINT error);
extern void rplSetExceptionHandler(WORDPTR Handler);
extern void rplRemoveExceptionHandler();
extern void rplCatchException();


// ENVIRONMENT FUNCTIONS IN RUNSTREAM.C
extern void rplInit(void);
extern void rplSetEntryPoint(WORDPTR ip);
extern void rplRun(void);
extern LIBHANDLER rplGetLibHandler(BINT libnum);
extern WORDPTR rplSkipOb(WORDPTR ip);
extern void rplSkipNext();
extern WORD rplObjSize(WORDPTR ip);



// TEMPOB MEMORY MANAGEMENT IN TEMPOB.C

extern WORDPTR rplAllocTempOb(WORD size);
extern void rplTruncateLastObject(WORDPTR newend);
extern void rplResizeLastObject(WORD additionalsize);
extern void growTempOb(WORD newtotalsize);
extern void shrinkTempOb(WORD newtotalsize);
extern void rplAddTempBlock(WORDPTR block);
extern void growTempBlocks(WORD newtotalsize);
extern void shrinkTempBlocks(WORD newtotalsize);


// COMPILER FUNCTIONS IN COMPILER.C


extern WORDPTR rplCompile(BYTEPTR string, BINT len, BINT addwrapper);
extern void rplCompileAppend(WORD word);
extern WORDPTR rplCompileAppendWords(BINT nwords);
extern void rplCompileRemoveWords(BINT nwords);

#define DECOMP_EMBEDDED     1
#define DECOMP_EDIT         2


// DECOMPILER FUNCTIONS
extern WORDPTR rplDecompile(WORDPTR object, BINT flags);
extern void rplDecompAppendChar(BYTE c);
extern void rplDecompAppendString(BYTEPTR str);
extern void rplDecompAppendString2(BYTEPTR str,BINT len);



// DATA STACK FUNCTIONS IN DATASTACK.C

extern void rplPushData(WORDPTR p);
extern void rplPushDataNoGrow(WORDPTR p);
extern WORDPTR rplPopData();
extern WORDPTR rplPeekData(int level);
extern void rplOverwriteData(int level,WORDPTR ptr);
extern BINT rplDepthData();
extern void rplClearData();
extern void rplDropData(int n);
extern void rplExpandStack(BINT numobjects);
extern void growDStk(WORD newsize);
extern WORDPTR *rplProtectData();
extern WORDPTR *rplUnprotectData();


// SNAPSHOT FUNCTIONS THAT SAVE/RESTORE THE STACK
extern BINT rplCountSnapshots();
extern void rplTakeSnapshot();
extern void rplRemoveSnapshot(BINT numsnap);
extern void rplRestoreSnapshot(BINT numsnap);
extern void rplRevertToSnapshot(BINT numsnap);







// RETURN STACK FUNCTIONS IN RETURNSTACK.C

extern void rplPushRet(WORDPTR p);
extern WORDPTR rplPopRet();
extern void growRStk(WORD newsize);
extern WORDPTR rplPeekRet(int level);
extern void rplClearRStk();

// SYSTEM FLAGS
extern BINT rplGetSystemFlag(BINT flag);
extern void rplSetSystemFlag(BINT flag);
extern void rplClrSystemFlag(BINT flag);
extern void rplClrSystemFlagByName(BYTEPTR name,BINT len);
extern void rplSetSystemFlagByName(BYTEPTR name,BINT len);





// GARBAGE COLLECTION
extern void rplGCollect();

// BACKUP/RESTORE
extern BINT rplBackup(void (*writefunc)(unsigned int));
extern BINT rplRestoreBackup(WORD (*readfunc)());


// SYSTEM SANITY CHECKS
extern BINT rplVerifyObject(WORDPTR obj);
extern BINT rplIsTempObPointer(WORDPTR ptr);
extern BINT rplVerifyObjPointer(WORDPTR ptr);
extern BINT rplVerifyDStack(BINT fix);
extern BINT rplVerifyRStack();
extern BINT rplVerifyTempOb(BINT fix);
extern BINT rplVerifyDirectories(BINT fix);



//IDENTIFIER FUNCTIONS
extern BINT rplGetIdentLength(WORDPTR ident);
extern void rplCompileIDENT(BINT libnum, BYTEPTR tok, BYTEPTR tokend);
extern BINT rplIsValidIdent(BYTEPTR tok,BYTEPTR tokend);




// LAM FUNCTIONS
extern void growLAMs(WORD newtotalsize);
extern void rplCreateLAMEnvironment(WORDPTR owner);
extern BINT rplCreateLAM(WORDPTR nameobj,WORDPTR value);
extern BINT rplCompareIDENT(WORDPTR id1,WORDPTR id2);
extern BINT rplCompareIDENTByName(WORDPTR id1,BYTEPTR name,BINT len);
extern BINT rplCompareObjects(WORDPTR id1,WORDPTR id2);
extern WORDPTR rplGetLAM(WORDPTR nameobj);
extern WORDPTR *rplGetLAMn(BINT idx);
extern WORDPTR *rplGetLAMnName(BINT idx);
extern WORDPTR *rplGetLAMnEnv(WORDPTR *LAMEnv, BINT idx);
extern WORDPTR *rplGetLAMnNameEnv(WORDPTR *LAMEnv, BINT idx);
extern void rplPutLAMn(BINT idx,WORDPTR object);
extern void rplCleanupLAMs(WORDPTR currentseco);
extern void rplClearLAMs();
extern WORDPTR *rplFindLAM(WORDPTR nameobj, BINT scanparents);
extern WORDPTR *rplFindLAMbyName(BYTEPTR name,BINT len,BINT scanparents);
extern WORDPTR *rplGetNextLAMEnv(WORDPTR *startpoint);
extern BINT rplNeedNewLAMEnv();
extern BINT rplNeedNewLAMEnvCompiler();
extern BINT rplLAMCount(WORDPTR *LAMEnvironment);


// GLOBAL VARIABLES AND DIRECTORY FUNCTIONS
extern void growDirs(WORD newtotalsize);
extern WORDPTR rplMakeIdentQuoted(WORDPTR ident);
extern void rplCreateGlobalInDir(WORDPTR nameobj,WORDPTR value,WORDPTR *parentdir);
extern void rplCreateGlobal(WORDPTR nameobj,WORDPTR value);
extern WORDPTR *rplFindDirbyHandle(WORDPTR handle);
extern WORDPTR rplCreateNewDir(WORDPTR name,WORDPTR *parentdir);
extern WORDPTR *rplGetParentDir(WORDPTR *directory);
// VARIOUS WAYS TO RCL GLOBAL VARIABLES
extern WORDPTR *rplFindGlobalbyNameInDir(BYTEPTR name,BINT len,WORDPTR *parent,BINT scanparents);
extern WORDPTR *rplFindGlobalbyName(BYTEPTR name,BINT len,BINT scanparents);
extern WORDPTR *rplFindGlobalByIndexInDir(BINT idx,WORDPTR *directory);
extern WORDPTR *rplFindGlobalByIndex(BINT idx);
extern WORDPTR *rplFindGlobalInDir(WORDPTR nameobj,WORDPTR *parentdir,BINT scanparents);
extern WORDPTR *rplFindGlobal(WORDPTR nameobj,BINT scanparents);
extern WORDPTR *rplFindVisibleGlobalByIndexInDir(BINT idx,WORDPTR *directory);
extern WORDPTR *rplFindVisibleGlobalByIndex(BINT idx);


extern BINT rplGetVarCountInDir(WORDPTR *directory);
extern BINT rplGetVarCount();
extern BINT rplGetVisibleVarCountInDir(WORDPTR *directory);
extern BINT rplGetVisibleVarCount();

extern void rplPurgeGlobal(WORDPTR nameobj);
extern WORDPTR rplGetGlobal(WORDPTR nameobj);
extern WORDPTR *rplMakeNewDir();
extern WORDPTR rplGetDirName(WORDPTR *dir);
extern WORDPTR *rplGetDirfromGlobal(WORDPTR *var);
extern WORDPTR *rplDeepCopyDir(WORDPTR *sourcedir);
extern void rplStoreSettings(WORDPTR nameobject,WORDPTR object);
extern void rplStoreSettingsbyName(BYTEPTR name,BINT namelen,WORDPTR object);
extern WORDPTR rplGetSettings(WORDPTR nameobject);
extern WORDPTR rplGetSettingsbyName(BYTEPTR name,BINT namelen);




// GENERIC OBJECT FUNCTIONS
extern void rplCallOvrOperator(WORD op);
extern void rplCallOperator(WORD op);
extern void rplCopyObject(WORDPTR dest, WORDPTR src);
extern WORDPTR rplMakeNewCopy(WORDPTR object);


// BINT FUNCTIONS
extern WORDPTR rplNewSINT(int num,int base);
extern WORDPTR rplNewBINT(BINT64 num,int base);
extern void rplNewSINTPush(int num,int base);
extern void rplNewBINTPush(BINT64 num,int base);
extern BINT64 rplReadBINT(WORDPTR ptr);
extern WORDPTR rplWriteBINT(BINT64 num,int base,WORDPTR dest);

// TRUE/FALSE FUNCTIONS
extern void rplPushFalse();
extern void rplPushTrue();
extern BINT rplIsFalse(WORDPTR objptr);
extern BINT rplIsTrue(WORDPTR objptr);


// REAL FUNCTIONS
extern void rplOneToRReg(int num);
extern void rplZeroToRReg(int num);
extern void rplInfinityToRReg(int num);
extern void rplNANToRReg(int num);
extern void rplBINTToRReg(int num,BINT64 value);
extern void rplReadReal(WORDPTR real,REAL *dec);
extern void rplCopyRealToRReg(int num,WORDPTR real);
extern WORDPTR rplNewReal(REAL *num);
extern WORDPTR rplNewRealInPlace(REAL *num,WORDPTR addr);
extern WORDPTR rplNewRealFromRReg(int num);
extern void rplNewRealPush(REAL *num);
extern void rplNewRealFromRRegPush(int num);
extern void rplNewApproxRealFromRRegPush(int num);
extern WORDPTR rplRRegToRealInPlace(int num, WORDPTR dest);


// COMPLEX FUNCTIONS
extern void rplRealPart(WORDPTR complex,REAL *real);
extern void rplImaginaryPart(WORDPTR complex,REAL *imag);
extern void rplReadCNumberAsReal(WORDPTR complex,REAL *real);
extern void rplReadCNumberAsImag(WORDPTR complex,REAL *imag);
extern void rplNewComplexPush(REAL *real,REAL *imag);
extern void rplRRegToComplexPush(BINT real,BINT imag);
extern WORDPTR rplRRegToComplexInPlace(BINT real,BINT imag,WORDPTR dest);



// GENERIC FUNCTIONS FOR BINTS AND REALS
extern void rplNumberToRReg(int num,WORDPTR number);
extern BINT64 rplReadNumberAsBINT(WORDPTR number);
extern void rplReadNumberAsReal(WORDPTR number,REAL*dec);

// LIST FUNCTIONS
extern BINT rplListLength(WORDPTR composite);
extern BINT rplListLengthFlat(WORDPTR composite);
extern void rplCreateList();
extern BINT rplExplodeList(WORDPTR composite);
extern WORDPTR rplGetListElement(WORDPTR composite, BINT pos);
extern WORDPTR rplGetListElementFlat(WORDPTR composite, BINT pos);
extern BINT rplIsLastElementFlat(WORDPTR composite, BINT pos);

// SYMBOLIC FUNCTIONS
extern WORDPTR rplSymbUnwrap(WORDPTR symbolic);
extern WORD rplSymbMainOperator(WORDPTR symbolic);
extern BINT rplIsAllowedInSymb(WORDPTR object);
extern void rplSymbApplyOperator(WORD Opcode,BINT nargs);
extern void rplSymbRuleMatch();

// INTERNAL SYMBOLIC API, FOR USE BY OTHER LIBRARIES
extern BINT rplCheckCircularReference(WORDPTR env_owner,WORDPTR object,BINT lamnum);


// STRINGS
// RPL STRING OBJECT
extern void rplSetStringLength(WORDPTR string,BINT length);
extern BINT rplStrLen(WORDPTR string);
extern BINT rplStrSize(WORDPTR string);
extern BINT rplStringGetLinePtr(WORDPTR str,BINT line);


// MATRIX
extern WORDPTR rplMatrixCompose(BINT rows,BINT cols);
extern WORDPTR *rplMatrixExplode();
extern WORDPTR rplMatrixGetFirstObj(WORDPTR matrix);
extern WORDPTR rplMatrixGet(WORDPTR matrix,BINT row,BINT col);
extern WORDPTR rplMatrixFastGet(WORDPTR matrix,BINT row,BINT col);
extern WORDPTR *rplMatrixFastGetEx(WORDPTR *first,BINT cols,BINT i,BINT j);
extern WORDPTR *rplMatrixNewEx(BINT rows,BINT cols);


extern void rplMatrixNorm();
extern void rplMatrixNeg();
extern void rplMatrixEval1();
extern void rplMatrixEval();
extern void rplMatrixToNum();
extern void rplMatrixAdd();
extern void rplMatrixSub();
extern void rplMatrixMul();
extern void rplMatrixMulScalar();
extern void rplMatrixBareiss();
extern void rplMatrixBareissEx(WORDPTR *a,BINT rowsa,BINT colsa);
extern void rplMatrixInvert();
extern void rplMatrixBackSubstEx(WORDPTR *a,BINT rowsa,BINT colsa);


// DEFINED EXCEPTIONS

#define EX_EXITRPL          1
#define EX_BKPOINT          2
#define EX_BADOPCODE        4
#define EX_OUTOFMEM         8
#define EX_CIRCULARREF     16
#define EX_EMPTYSTACK      64
#define EX_EMPTYRSTK      128
#define EX_SYNTAXERROR    256
#define EX_UNDEFINED      512
#define EX_BADARGCOUNT   1024
#define EX_BADARGTYPE    2048
#define EX_BADARGVALUE   4096
#define EX_VARUNDEF      8192
#define EX_NONEMPTYDIR  16384
#define EX_INVALID_DIM  32768
// ADD MORE HERE...
#define EX_MATHDIVZERO     (1<<16)
#define EX_MATHOVERFLOW    (2<<16)











#ifdef __cplusplus
}
#endif


#endif // NEWRPL_H
