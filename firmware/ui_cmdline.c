#include <ui.h>
#include <newrpl.h>
#include <libraries.h>


// COMMAND LINE API
// BASIC PRINCIPLES OF THE COMMAND LINE:
// * ONLY ONE LINE IS EDITED AT ONCE
// * LINE IS EDITED AS THE LAST OBJECT IN TEMPOB, TO BE STRETCHED AT WILL
// * IF LINE IS NOT AT THE END OF TEMPOB, A NEW COPY IS MADE

// SET THE COMMAND LINE TO A GIVEN STRING OBJECT
void uiSetCmdLineText(WORDPTR text)
{
    CmdLineText=text;
    CmdLineCurrentLine=empty_string;
    CmdLineUndoList=empty_list;
    halScreen.LineIsModified=-1;

    // SET CURSOR AT END OF TEXT
    BINT end=rplStrLen(CmdLineText);
    BYTEPTR linestart;
    halScreen.LineCurrent=uiGetLinebyOffset(end,&linestart);
    halScreen.CursorPosition=((BYTEPTR)rplSkipOb(CmdLineText))-linestart;

    if(halScreen.CursorPosition<0) halScreen.CursorPosition=0;
    halScreen.CursorX=StringWidthN(linestart,halScreen.CursorPosition,(FONTDATA *)halScreen.CmdLineFont);


    uiEnsureCursorVisible();

    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;
}

WORDPTR uiGetCmdLineText()
{
    if(halScreen.LineIsModified>0) uiModifyLine();
    if(Exceptions) return NULL;
    return CmdLineText;
}

// SCROLL UP/DOWN AND LEFT/RIGHT TO KEEP CURSOR ON SCREEN
void uiEnsureCursorVisible()
{
    int scrolled=0;
    // CHECK IF SCROLL UP IS NEEDED
    if(halScreen.LineCurrent<halScreen.LineVisible) {
        halScreen.LineVisible=halScreen.LineCurrent;
        scrolled=1;
    }

    // SCROLL DOWN AS NEEDED
    if(halScreen.LineCurrent>=halScreen.LineVisible+halScreen.NumLinesVisible) {
        halScreen.LineVisible=halScreen.LineCurrent-(halScreen.NumLinesVisible-1);
        scrolled=1;
    }

    // SCROLL LEFT AS NEEDED
    if(halScreen.CursorX<halScreen.XVisible+8) {
        if(halScreen.XVisible>0) {
        if(halScreen.XVisible<8) halScreen.XVisible=0;
        else halScreen.XVisible=halScreen.CursorX-8;    // FIXED AT 16 PIXELS
        scrolled=1;
        }
    }

    // SCROLL RIGHT AS NEEDED
    if(halScreen.CursorX>halScreen.XVisible+SCREEN_WIDTH-8) {
        halScreen.XVisible=halScreen.CursorX-(SCREEN_WIDTH-8);
        scrolled=1;
    }

    if(scrolled) halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;

}

void __uicursorupdate()
{
    if(halScreen.CursorState&0x4000) return;    // DON'T UPDATE IF LOCKED
    halScreen.CursorState^=0x8000;              // FLIP STATE BIT
    halScreen.DirtyFlag|=CMDLINE_CURSORDIRTY;
    DRAWSURFACE scr;
    ggl_initscr(&scr);
    halRedrawCmdLine(&scr);
}


// OPEN AN EMPTY COMMAND LINE
void uiOpenCmdLine()
{
    CmdLineText=empty_string;
    CmdLineCurrentLine=empty_string;
    CmdLineUndoList=empty_list;
    halScreen.LineCurrent=1;
    halScreen.LineIsModified=-1;
    halScreen.LineVisible=1;
    halScreen.NumLinesVisible=1;
    halScreen.CursorX=0;
    halScreen.CursorPosition=0;
    halScreen.CursorState='T';
    halScreen.XVisible=0;
    halScreen.CursorTimer=tmr_eventcreate(&__uicursorupdate,200,1);
    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;

}

// OPEN AN EMPTY COMMAND LINE
void uiCloseCmdLine()
{
    tmr_eventkill(halScreen.CursorTimer);

    CmdLineText=empty_string;
    CmdLineCurrentLine=empty_string;
    CmdLineUndoList=empty_list;
    halScreen.LineCurrent=1;
    halScreen.LineIsModified=-1;
    halScreen.LineVisible=1;
    halScreen.NumLinesVisible=1;
    halScreen.CursorX=0;
    halScreen.CursorPosition=0;
    halScreen.CursorState=0;
    halScreen.XVisible=0;
    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;
}


void uiSetCurrentLine(BINT line)
{

    if(line==halScreen.LineCurrent) return;
    // LOCK CURSOR
    halScreen.CursorState|=0x4000;

    if(halScreen.LineIsModified>0) {
        // INSERT THE MODIFIED TEXT BACK INTO ORIGINAL TEXT

        uiModifyLine();

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            CmdLineText=empty_string;
            CmdLineCurrentLine=empty_string;
            CmdLineUndoList=empty_list;
            return;
        }

    }

    halScreen.LineCurrent=line;
    halScreen.LineIsModified=-1;

    // POSITION THE CURSOR IN THE NEW LINE, TRYING TO PRESERVE THE X COORDINATE

    BINT tryoffset=halScreen.CursorPosition;
    BINT len=rplStrLen(CmdLineCurrentLine);
    BINT targetx;
    BYTEPTR ptr=(BYTEPTR)(CmdLineCurrentLine+1);
    if(tryoffset>len) tryoffset=len;

    targetx=StringWidthN(ptr,tryoffset,halScreen.CmdLineFont);

    while( (targetx<halScreen.CursorX) && (tryoffset<=len) ) {
        targetx+=StringWidthN(ptr+tryoffset,1,halScreen.CmdLineFont);
        ++tryoffset;
    }
    while( (targetx>halScreen.CursorX) && (tryoffset>0) ) {
        --tryoffset;
        targetx-=StringWidthN(ptr+tryoffset,1,halScreen.CmdLineFont);
    }

    halScreen.CursorX=targetx;
    halScreen.CursorPosition=tryoffset;

    uiEnsureCursorVisible();

    // UNLOCK CURSOR
    halScreen.CursorState&=~0x4000;

    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;


}


// MAIN FUNCTION TO INSERT TEXT AT THE CURRENT CURSOR OFFSET

void uiInsertCharacters(BYTEPTR string,BINT length)
{
if(length<=0) return;

// LOCK CURSOR
halScreen.CursorState|=0x4000;


if(halScreen.LineIsModified<0) {

uiExtractLine(halScreen.LineCurrent);

if(Exceptions) {
    throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
    // CLEAN UP AND RETURN
    CmdLineText=empty_string;
    CmdLineCurrentLine=empty_string;
    CmdLineUndoList=empty_list;
    return;
}

}

BINT lenwords=(length+3)>>2;

if(CmdLineCurrentLine==empty_string) {
    WORDPTR newobj=rplAllocTempOb(lenwords);
    if(!newobj) {
        throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        CmdLineText=empty_string;
        CmdLineCurrentLine=empty_string;
        CmdLineUndoList=empty_list;
        return;
    }
    CmdLineCurrentLine=newobj;
    *CmdLineCurrentLine=MKPROLOG(DOSTRING,0);   // MAKE AN EMPTY STRING
}
else {
    if(rplSkipOb(CmdLineCurrentLine)==TempObEnd)  rplResizeLastObject(lenwords);
    else {
        // NOT AT THE END OF TEMPOB
        // MAKE A COPY OF THE OBJECT AT THE END
        WORDPTR newobj=rplAllocTempOb(OBJSIZE(*CmdLineCurrentLine));
        if(!newobj) {
            throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            CmdLineText=empty_string;
            CmdLineCurrentLine=empty_string;
            CmdLineUndoList=empty_list;
            return;
        }
        rplCopyObject(newobj,CmdLineCurrentLine);
        CmdLineCurrentLine=newobj;

    }
}
if(Exceptions) {
    throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
    // CLEAN UP AND RETURN
    CmdLineText=empty_string;
    CmdLineCurrentLine=empty_string;
    CmdLineUndoList=empty_list;
    return;
}

// FINALLY, WE HAVE THE ORIGINAL LINE AT THE END OF TEMPOB, AND ENOUGH MEMORY ALLOCATED TO MAKE THE MOVE

// MOVE THE TAIL TO THE END
memmove( ((BYTEPTR)CmdLineCurrentLine)+4+halScreen.CursorPosition+length,((BYTEPTR)CmdLineCurrentLine)+4+halScreen.CursorPosition,rplStrLen(CmdLineCurrentLine)-halScreen.CursorPosition);
// ADD THE NEW DATA IN
memmove(((BYTEPTR)CmdLineCurrentLine)+4+halScreen.CursorPosition,string,length);

// PATCH THE LENGTH OF THE STRING
BINT newlen=rplStrLen(CmdLineCurrentLine);
newlen+=length;
rplSetStringLength(CmdLineCurrentLine,newlen);

halScreen.LineIsModified=1;

// ADVANCE THE CURSOR
// TODO: IF THE INSERTED TEXT HAD ANY NEWLINES, THE CURRENT COMMAND LINE HAS MULTIPLE LINES IN ONE
// MUST SPLIT THE LINES AND GET THE CURSOR ON THE LAST ONE

halScreen.CursorX+=StringWidthN(((BYTEPTR)CmdLineCurrentLine)+4+halScreen.CursorPosition,length,halScreen.CmdLineFont);
halScreen.CursorPosition+=length;

halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY;

uiEnsureCursorVisible();
// UNLOCK CURSOR
halScreen.CursorState&=~0x4000;

}


// COPY THE EDITED LINE BACK INTO THE ORIGINAL TEXT
// CREATES A COPY OF THE TEXT
void uiModifyLine()
{
    WORDPTR newobj;
    BINT newsize;

    // GET A NEW OBJECT WITH ROOM FOR THE ENTIRE TEXT
    newobj=rplAllocTempOb( (rplStrLen(CmdLineText)+rplStrLen(CmdLineCurrentLine)+1+ 3)>>2);

    if(Exceptions) {
        throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        CmdLineText=empty_string;
        CmdLineCurrentLine=empty_string;
        CmdLineUndoList=empty_list;
        return;
    }
    BYTEPTR src=(BYTEPTR)(CmdLineText+1),dest=(BYTEPTR)(newobj+1);
    BYTEPTR startline=src+rplStringGetLinePtr(CmdLineText,halScreen.LineCurrent);
    BYTEPTR endline=src+rplStringGetLinePtr(CmdLineText,halScreen.LineCurrent+1);

    if(startline<src) {
        /*
        throw_dbgexception("Bad starting line",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        CmdLineText=empty_string;
        CmdLineCurrentLine=empty_string;
        CmdLineUndoList=empty_list;
        return;
        */
        // LINE DOESN'T EXIST, ADD AT THE END OF TEXT AS A NEW LINE
        startline=src;
    }

    // COPY ALL PREVIOUS LINES TO NEW OBJECT
    newsize=startline-src+rplStrLen(CmdLineCurrentLine);

    memmove(dest,src,startline-src);
    // COPY THE NEW LINE TO THE OBJECT
    memmove(dest+(startline-src),(WORDPTR)(CmdLineCurrentLine+1),rplStrLen(CmdLineCurrentLine));
    // COPY THE REST BACK
    if(endline>=src) {
        // APPEND A NEWLINE AND KEEP GOING
        dest+=startline-src+rplStrLen(CmdLineCurrentLine);
        *dest++='\n';
        newsize+=((BYTEPTR)rplSkipOb(CmdLineText))-endline+1;
        memmove(dest,endline,((BYTEPTR)rplSkipOb(CmdLineText))-endline);
    }

    rplSetStringLength(newobj,newsize);

    CmdLineText=newobj;
    halScreen.LineIsModified=0;

}

// COPY A LINE FROM THE TEXT INTO THE EDITING BUFFER
void uiExtractLine(BINT line)
{
    WORDPTR newobj;

    // GET A NEW OBJECT WITH ROOM FOR THE ENTIRE LINE
    BYTEPTR text=(BYTEPTR)(CmdLineText+1);
    BYTEPTR startline=text+rplStringGetLinePtr(CmdLineText,line);
    BYTEPTR endline=text+rplStringGetLinePtr(CmdLineText,line+1);

    if(startline<text) {
        // CREATE AN EMPTY LINE
        startline=endline=text;
    }

    if(endline<text) endline=text+rplStrLen(CmdLineText);


    newobj=rplAllocTempOb( ((endline-startline)+ 3)>>2);

    if(Exceptions) {
        throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        CmdLineText=empty_string;
        CmdLineCurrentLine=empty_string;
        CmdLineUndoList=empty_list;
        return;
    }

    // ZERO PADDING THE LAST WORD
    newobj[(endline-startline)>>2]=0;

    // COPY LINE TO NEW OBJECT
    memmove(newobj+1,startline,endline-startline);

    rplSetStringLength(newobj,endline-startline);

    CmdLineCurrentLine=newobj;

    halScreen.LineIsModified=0;

}


// RETURN LINE NUMBER FOR A GIVEN OFFSET, AND THE OFFSET OF THE START OF LINE
BINT uiGetLinebyOffset(BINT offset,BINT *linestart)
{

    BYTEPTR ptr=(BYTEPTR)(CmdLineText+1);
    BYTEPTR end=rplSkipOb(CmdLineText);
    BINT len=rplStrLen(CmdLineText);
    BINT f,found,count;
    BYTEPTR *prevfind,*currfind;

    if(offset>len) offset=len;
    if(offset<0) offset=0;
    prevfind=currfind=ptr;
    found=0;
    for(f=0,count=0; (f<len) && (f<offset);++f,++ptr)
    {
    if(*ptr=='\n') {
        found=f+1;
        ++count;
    }
    }

    if(linestart) *linestart=found;
    return count+1;

}

// MOVE THE CURSOR TO THE GIVEN OFFSET WITHIN THE STRING
void uiMoveCursor(BINT offset)
{
if(halScreen.LineIsModified>0) uiModifyLine();

BYTEPTR ptr=(BYTEPTR )(CmdLineText+1);
BINT len=rplStrLen(CmdLineText);
BINT lineoff;

if(offset>len) offset=len;
if(offset<0) offset=0;

BINT line=uiGetLinebyOffset(offset,&lineoff);

uiSetCurrentLine(line);

halScreen.CursorPosition=offset;

halScreen.CursorX=StringWidthN(ptr,offset-lineoff,halScreen.CmdLineFont);
}
