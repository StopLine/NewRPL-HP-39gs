/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#ifndef ERRORCODES_H
#define ERRORCODES_H

// THIS HEADER EXPORTS ALL KNOWN MESSAGE CODES IN USE
// LIBRARIES USE THE ERROR CODES, THE EXTERNAL UI PROVIDES
// TRANSLATABLE TEXT FOR EACH OF THESE MESSAGES
// THE RPL CORE DOES NOT PROVIDE A TEXT MESSAGE

// MESSAGE ID'S ARE ENCODED USING SINT NUMBER BY THE MACRO MAKEMSG(lib,error)

// LIBRARY 0 IS USED FOR BASIC SYSTEM MESSAGES (COMPILER ERRORS, ETC.)
// THAT DON'T BELONG TO ANY LIBRARY IN PARTICULAR
// ALL OTHER ERRORS SHOULD BE ASSIGNED TO THEIR RESPECTIVE LIBRARIES

enum AllMessages {

    // PROGRAM EXECUTION ERRORS
    ERR_PROGRAMEXPECTED = MAKEMSG(DOCOL,3),

    // COMPILER/DECOMPILER AND GENERAL CORE ERRORS

    // THESE ARE SYSTEM EXCEPTIONS, DO NOT CHANGE THE ORDER
    // ORDER NUMBER MUST MATCH THE EXCEPTION BIT NUMBER
    ERR_EXITRPLEXCEPTION = MAKEMSG(0,0),
    ERR_BKPOINTEXCEPTION = MAKEMSG(0,1),
    ERR_OUTOFMEMORYEXCEPTION = MAKEMSG(0,2),

    ERR_ENDWITHOUTSTART = MAKEMSG(0,3),
    ERR_STARTWITHOUTEND = MAKEMSG(0,4),
    ERR_MISSINGBRACKET = MAKEMSG(0,5),
    ERR_BADARGCOUNT = MAKEMSG(0,6),
    ERR_BADARGTYPE = MAKEMSG(0,6),
    ERR_SYNTAXERROR = MAKEMSG(0,8),
    ERR_INVALIDTOKEN = MAKEMSG(0,9),
    ERR_MALFORMEDOBJECT = MAKEMSG(0,10),
    ERR_NOTALLOWEDINSYMBOLICS = MAKEMSG(0,11),
    ERR_INVALIDOPERATORINSYMBOLIC = MAKEMSG(0,12),
    ERR_UNRECOGNIZEDOBJECT = MAKEMSG(0,13),
    ERR_INVALIDOPCODE = MAKEMSG(0,14),
    ERR_MISSINGLIBRARY = MAKEMSG(0,15),
    ERR_INTERNALEMPTYSTACK = MAKEMSG(0,16),
    ERR_INTERNALEMPTYRETSTACK = MAKEMSG(0,17),
    ERR_BADSTACKINDEX = MAKEMSG(0,18),


    // IDENTIFIERS AND VARIABLES
    ERR_IDENTEXPECTED = MAKEMSG(DOIDENT,0),
    ERR_UNDEFINEDVARIABLE = MAKEMSG(DOIDENT,1),
    ERR_CIRCULARREFERENCE = MAKEMSG(DOIDENT,2),

    // DIRECTORIES AND VARIABLES
    ERR_NONEMPTYDIRECTORY = MAKEMSG(DODIR,0),
    ERR_DIRECTORYNOTFOUND = MAKEMSG(DODIR,1),

    // INTEGER NUMBERS
    ERR_INTEGEREXPECTED = MAKEMSG(DOBINT,0),
    ERR_INTEGERSNOTSUPPORTED = MAKEMSG(DOBINT,1),

    // REAL NUMBERS
    ERR_REALEXPECTED = MAKEMSG(DOREAL,0),
    ERR_REALSNOTSUPPORTED = MAKEMSG(DOREAL,1),
    ERR_INFINITERESULT = MAKEMSG(DOREAL,2),
    ERR_UNDEFINEDRESULT = MAKEMSG(DOREAL,3),
    ERR_NUMBERTOOBIG = MAKEMSG(DOREAL,4),
    ERR_MATHDIVIDEBYZERO = MAKEMSG(DOREAL,5),
    ERR_MATHOVERFLOW = MAKEMSG(DOREAL,6),
    ERR_MATHUNDERFLOW = MAKEMSG(DOREAL,7),


    // COMPLEX NUMBERS
    ERR_COMPLEXEXPECTED = MAKEMSG(DOCMPLX,0),
    ERR_COMPLEXORREALEXPECTED = MAKEMSG(DOCMPLX,1),
    ERR_COMPLEXNOTSUPPORTED = MAKEMSG(DOCMPLX,2),
    ERR_NOTALLOWEDINCOMPLEX = MAKEMSG(DOCMPLX,3),


    // STRINGS
    ERR_STRINGEXPECTED = MAKEMSG(DOSTRING,0),

    // UNITS
    ERR_UNITEXPECTED = MAKEMSG(DOUNIT,0),
    ERR_INCONSISTENTUNITS = MAKEMSG(DOUNIT,1),
    ERR_INVALIDUNITDEFINITION = MAKEMSG(DOUNIT,2),
    ERR_EXPECTEDREALEXPONENT = MAKEMSG(DOUNIT,3),
    ERR_INVALIDUNITNAME = MAKEMSG(DOUNIT,4),
    ERR_UNDEFINEDUNIT = MAKEMSG(DOUNIT,5),





    // LISTS
    ERR_LISTEXPECTED = MAKEMSG(DOLIST,0),
    ERR_INDEXOUTOFBOUNDS = MAKEMSG(DOLIST,1),
    ERR_EMPTYLIST = MAKEMSG(DOLIST,2),
    ERR_INVALIDLISTSIZE = MAKEMSG(DOLIST,3),


    // SYMBOLICS
    ERR_SYMBOLICEXPECTED = MAKEMSG(DOSYMB,0),
    ERR_NOTAVALIDRULE = MAKEMSG(DOSYMB,1),
    ERR_INVALIDUSERDEFINEDFUNCTION = MAKEMSG(DOSYMB,2),

    // MATRIX
    ERR_MATRIXEXPECTED = MAKEMSG(DOMATRIX,0),
    ERR_INVALIDDIMENSION = MAKEMSG(DOMATRIX,1),
    ERR_NOTALLOWEDINMATRIX = MAKEMSG(DOMATRIX,2),
    ERR_INCOMPATIBLEDIMENSION = MAKEMSG(DOMATRIX,3),
    ERR_MATRIXORREALEXPECTED = MAKEMSG(DOMATRIX,4),
    ERR_SQUAREMATRIXONLY = MAKEMSG(DOMATRIX,5),
    ERR_VECTOREXPECTED = MAKEMSG(DOMATRIX,6),
    ERR_MISPLACEDBRACKETS = MAKEMSG(DOMATRIX,7),

    // FLAGS
    ERR_SYSTEMFLAGSINVALID = MAKEMSG(68,0),
    ERR_INVALIDFLAGNUMBER = MAKEMSG(68,1),
    ERR_INVALIDFLAGNAME = MAKEMSG(68,2),
    ERR_IDENTORINTEGEREXPECTED = MAKEMSG(68,3),
    ERR_INVALIDLOCALESTRING = MAKEMSG(68,4),

    // TRANSCENDENTAL FUNCTIONS
    ERR_ARGOUTSIDEDOMAIN = MAKEMSG(66,0),


//    ERR_ = MAKEMSG(  ,  ),


};








#endif // ERRORCODES_H

