/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#ifndef ARITHMETIC_H
#define ARITHMETIC_H

// INTERNAL TRANSCENDENTAL FUNCTIONS

void decconst_PI(REAL *real);

void hyp_exp(REAL *);
void hyp_ln(REAL *);
void hyp_log(REAL *);
void hyp_alog(REAL *);

void powReal(REAL *result,REAL *x,REAL *a);
void xrootReal(REAL *result,REAL *x,REAL *a);

void hyp_sqrt(REAL *);
void hyp_sinhcosh(REAL *);
void hyp_atanh(REAL *);
void hyp_asinh(REAL *);
void hyp_acosh(REAL *);

void trig_sincos(REAL *,BINT angmode);
void trig_atan2(REAL *, REAL *, BINT angmode);
void trig_asin(REAL *,BINT angmode);
void trig_acos(REAL *, BINT angmode);


BINT64 factorialBINT(BINT n);
BINT64 nextcbprimeBINT(BINT64 n);
BINT64 nextprimeBINT(BINT64 n);
BINT isprimeBINT(BINT64 n);
void nextprimeReal(BINT regnum,REAL *n);
BINT isprimeReal(REAL *n);

BINT64 powmodBINT(BINT64 a, BINT64 b, BINT64 mod);
void powmodReal(REAL *result,REAL *a,REAL *b,REAL *mod);






#endif // ARITHMETIC_H

