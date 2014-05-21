/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"

// THIS IS THE TRAP THAT HANDLES REAL NUMBER EXCEPTIONS AND CONVERTS THEM
// TO RPL KERNEL EXCEPTIONS
void MPDTrapHandler(mpd_context_t *ctx)
{
    // SIMPLY COPY THE MATHEMATIC EXCEPTIONS INTO THE UPPER 16 BITS OF OUR EXCEPTION WORD
    Exceptions|=ctx->status<<16;
    ExceptionPointer=IPtr;
    ctx->newtrap=0;
    // RETURN AND KEEP GOING
    return;
}


// SET THE HANDLER THAT WILL TRAP EXCEPTIONS

void rplSetExceptionHandler(WORDPTR Handler)
{
    // SAVE CURRENT ERROR HANDLERS
    rplPushRet((WORDPTR)ErrorHandler);
    rplPushRet((WORDPTR)ErrorRSTop);
    rplPushRet((WORDPTR)ErrorLAMTop);
    rplPushRet((WORDPTR)ErrornLAMBase);

    ErrorHandler=Handler;
    ErrorLAMTop=LAMTop;
    ErrornLAMBase=nLAMBase;
    ErrorRSTop=RSTop;

}



// REMOVE THE HANDLERS SET BY rplSetExceptionHandler
// AND RESTORE THE PREVIOUS ONE WITHOUT RAISING AN EXCEPTION
void rplRemoveExceptionHandler()
{
    // RESTORE LAM ENVIRONMENT
    nLAMBase=ErrornLAMBase;
    LAMTop=ErrorLAMTop;

    // RESTORE RETURN STACK
    RSTop=ErrorRSTop;

    // RESTORE OLD ERROR HANDLER POINTERS
    ErrornLAMBase=(WORDPTR *)rplPopRet();
    ErrorLAMTop=(WORDPTR *)rplPopRet();
    ErrorRSTop=(WORDPTR *)rplPopRet();
    ErrorHandler=(WORDPTR)rplPopRet();

}



void rplCatchException()
{
    // SAVE THE EXCEPTIONS
    TrappedExceptions=Exceptions;   // THE ERROR HANDLER CAN KNOW THE EXCEPTIONS BY LOOKING AT THIS VARIABLE
                                    // ExceptionPointer STILL POINTS TO THE WORD THAT CAUSED THE EXCEPTION
    Exceptions=0;                   // RESET THE EXCEPTIONS TO ALLOW HANDLER TO RUN



    // SET INSTRUCTION POINTER AND CONTINUE EXECUTION AT THE ERROR HANDLER
    IPtr=ErrorHandler-1;    // MAKE SURE THE FIRST OBJECT AT THE ERROR HANDLER IS NOT SKIPPED
    CurOpcode=0;

    rplRemoveExceptionHandler();

}
