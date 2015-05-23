#include <newrpl.h>
#include <ui.h>



extern unsigned int __cpu_intoff();
extern void __cpu_inton(unsigned int);
extern void __tmr_eventreschedule();



#define DEBOUNCE  16  // 10 SEEMS TO BE ADEQUATE EVEN AT 75 MHz


// KEYBOARD, LOW LEVEL GLOBAL VARIABLES
unsigned short int __keyb_buffer[KEYB_BUFFER] __SYSTEM_GLOBAL__;
volatile int __keyb_lock __SYSTEM_GLOBAL__;
int __keyflags __SYSTEM_GLOBAL__;
int __kused __SYSTEM_GLOBAL__,__kcurrent __SYSTEM_GLOBAL__;
keymatrix __kmat __SYSTEM_GLOBAL__;
int __keyplane __SYSTEM_GLOBAL__;
int __keynumber __SYSTEM_GLOBAL__,__keycount __SYSTEM_GLOBAL__;
int __keyb_repeattime,__keyb_longpresstime __SYSTEM_GLOBAL__,__keyb_debounce __SYSTEM_GLOBAL__;



// QT-BASED KEYBOARD MESSAGES MUST UPDATE THIS MATRIX;
volatile keymatrix __pckeymatrix;







// LOW-LEVEL ROUTINE TO BE USED BY THE IRQ HANDLERS AND EXCEPTION
// HANDLERS ONLY

keymatrix __keyb_getmatrix()
{
return __pckeymatrix;
}


// WRAPPER TO DISABLE INTERRUPTS WHILE READING THE KEYBOARD
// NEEDED ONLY WHEN CALLED FROM WITHIN AN EXCEPTION HANDLER



keymatrix __keyb_getmatrixEX()
{
    unsigned int saved=__cpu_intoff();
    keymatrix m=__keyb_getmatrix();
    thread_processevents();
    __cpu_inton(saved);
    return m;
}






// LOW-LEVEL FUNCTION TO BE USED BY THE
// EXCEPTION SUBSYSTEM ONLY


const unsigned short const __keyb_shiftconvert[8]={
0,
SHIFT_ALPHA|SHIFT_ALPHAHOLD,
SHIFT_LS|SHIFT_LSHOLD,
SHIFT_ALPHA|SHIFT_ALPHAHOLD|SHIFT_LS|SHIFT_LSHOLD,
SHIFT_RS|SHIFT_RSHOLD,
SHIFT_ALPHA|SHIFT_ALPHAHOLD|SHIFT_RS|SHIFT_RSHOLD,
SHIFT_LS|SHIFT_LSHOLD|SHIFT_RS|SHIFT_RSHOLD,
SHIFT_LS|SHIFT_LSHOLD|SHIFT_ALPHA|SHIFT_ALPHAHOLD|SHIFT_RS|SHIFT_RSHOLD
};

int __keyb_getkey(int wait)
{

    keymatrix m;
    m=__keyb_getmatrixEX();

    if(wait) {
        // wait for a non-shift key to be pressed
        while( (m&0x8fffffffffffffffLL )==0LL ) m=__keyb_getmatrixEX();
    }

    int kcode,shft=(m>>60)&0x7;
    unsigned char *mbytes=(unsigned char *)&m;
    int k;
    for(k=0,kcode=0;k<8;++mbytes,++k,kcode+=8)
    {
        if(*mbytes!=0) {
            k=*mbytes;
            while( !(k&1) ) {
                k>>=1;
                ++kcode;
                }
        break;
        }
    }


    if(wait) {
        while( (m&0x8fffffffffffffffLL )!=0 ) m=__keyb_getmatrixEX();
    if(kcode>=60) kcode=63;
    }

    if(kcode<60) return kcode | __keyb_shiftconvert[shft];
    if(kcode<64) return kcode;
    return 0;

}


void __keyb_waitrelease()
{
    keymatrix m=1;
    // DO NOT LOCK THE THREAD
    while(m!=0LL) { m=__keyb_getmatrixEX(); }
    __pckeymatrix=0;
}



#define LONG_KEYPRESSTIME (__keyb_longpresstime)
#define REPEAT_KEYTIME (__keyb_repeattime)
#define BOUNCE_KEYTIME (__keyb_debounce)

#define KF_RUNNING   1
#define KF_ALPHALOCK 2
#define KF_NOREPEAT  4


/*
#define GPGCON ((unsigned int *)(IO_REGS+0x60))
#define GPGDAT ((unsigned int *)(IO_REGS+0x64))
#define GPGUP ((unsigned int *)(IO_REGS+0x68))
#define GPFCON ((unsigned int *)(IO_REGS+0x50))
#define EXTINT0 ((unsigned int *)(IO_REGS+0x88))
#define EXTINT1 ((unsigned int *)(IO_REGS+0x8c))
#define EINTMASK ((unsigned int *)(IO_REGS+0xa4))
#define EINTPEND ((unsigned int *)(IO_REGS+0xa8))
#define INTMSK   ((unsigned int *)(INT_REGS+0x8))
#define INTPND   ((unsigned int *)(INT_REGS+0x10))
#define SRCPND   ((unsigned int *)(INT_REGS+0x0))
*/




void __keyb_postmsg(unsigned int msg)
{

__keyb_buffer[__kcurrent]=msg;
__kcurrent=(__kcurrent+1)&(KEYB_BUFFER-1);
// CHECK FOR BUFFER OVERRUN
if(__kcurrent==__kused) {
    // BUFFER OVERRUN, DROP LAST KEY
    __kcurrent=(__kcurrent-1)&(KEYB_BUFFER-1);
}

}


void keyb_postmsg(unsigned int msg)
{
    // WARNING: PROBLEMS MAY ARISE IF THE INTERRUPT SERVICE WANTS
    // TO POST A MESSAGE WHILE THE USER IS POSTING ONE.
    while(cpu_getlock(1,&__keyb_lock));

    __keyb_postmsg(msg);
    __keyb_lock=0;

}



unsigned int keyb_getmsg()
{
if(__kused==__kcurrent) return 0;
unsigned int msg=__keyb_buffer[__kused];
__kused=(__kused+1)&(KEYB_BUFFER-1);
return msg;
}

// CHECK IF ANY AVAILABLE KEYSTROKES
int keyb_anymsg()
{
    if(__kused==__kcurrent) return 0;
    return 1;
}

// FLUSH KEYBOARD BUFFER
void keyb_flush()
{
    while(keyb_getmatrix()!=0LL);
    __kused=__kcurrent;
}

// RETURNS THE CURRENT WORKING MATRIX INSTEAD OF
// MESSING WITH THE HARDWARE, BUT ONLY IF KEYBOARD HANDLERS WERE STARTED
keymatrix keyb_getmatrix()
{
    return __pckeymatrix;
}

// ANALYZE CHANGES IN THE KEYBOARD STATUS AND POST MESSAGES ACCORDINGLY

void __keyb_update()
{

    if(cpu_getlock(1,&__keyb_lock)) return;

    keymatrix a,b;

doupdate:

    a=__keyb_getmatrix();
    b=a^__kmat;
    __kmat=a;

    // ANALYZE CHANGES
    if(b!=0) {
    // POST MESSAGE
    int key=0;
    while(b!=0) {
    if(b&1) {
    if(a&1) {
        // POST KEYDN MESSAGE
        if(__keynumber==-key) {
            // DISREGARD SPURIOUS KEYPRESS
            __kmat&=~(1LL<<key);	// CLEAR THE KEY
            __keynumber=0;
            __keycount=0;
        }
        else {
        __keyb_postmsg(KM_KEYDN + key);
        if(key<60 || ( (key==KB_ALPHA)&&(__keyplane&(SHIFT_RS|SHIFT_LS)) )) {	// TREAT SHIFT-ALPHA LIKE REGULAR KEYPRESS
        __keyb_postmsg(KM_PRESS + key + (__keyplane&SHIFT_ANY));
        __keynumber=key;
        __keycount=0;

        } else {
            int oldplane=__keyplane;
            if(key==KB_LSHIFT) {
                __keyplane&=~(SHIFT_RSHOLD|SHIFT_RS | (SHIFT_RS<<16));
                __keyplane|=SHIFT_LSHOLD|SHIFT_LS;
                __keyplane^=SHIFT_LS<<16;
            }
            if(key==KB_RSHIFT) {
                __keyplane&=~(SHIFT_LSHOLD|SHIFT_LS| (SHIFT_LS<<16));
                __keyplane|=SHIFT_RSHOLD|SHIFT_RS;
                __keyplane^=SHIFT_RS<<16;
            }
            if(key==KB_ALPHA) {
                if( __keyflags&KF_ALPHALOCK) {
                    if(__keyplane&SHIFT_ALPHA)  {
                        // OTHER KEYS WERE PRESSED, SO END ALPHA MODE
                        __keyplane&=~((SHIFT_ALPHA<<17)|(SHIFT_ALPHA<<16)); // UNLOCK ALPHA

                    } else {
                            __keyplane|=SHIFT_ALPHA<<17; // LOCK ALPHA
                            __keyplane&=~(SHIFT_ALPHA<<16);
                        }

                }
                else {
                if(__keyplane&SHIFT_ALPHA)  {
                    if(__keyplane&(SHIFT_ALPHA<<16)) {
                        // DOUBLE ALPHA KEYPRESS
                        __keyplane|=SHIFT_ALPHA<<17; // LOCK ALPHA
                        __keyplane&=~(SHIFT_ALPHA<<16);
                    }
                    else {
                        // OTHER KEYS WERE PRESSED, SO END ALPHA MODE
                        __keyplane&=~((SHIFT_ALPHA<<17)|(SHIFT_ALPHA<<16)); // UNLOCK ALPHA
                    }
                }
                else __keyplane^=SHIFT_ALPHA<<16;
                }
                __keyplane|=SHIFT_ALPHAHOLD|SHIFT_ALPHA;


            }
            if(key==KB_ON) {
                __keyplane|=SHIFT_ONHOLD;
            }
            // THE KM_SHIFT MESSAGE CARRIES THE OLD PLANE IN THE KEY CODE
            // AND THE NEW PLANE IN THE SHIFT CODE.
            __keyb_postmsg(KM_SHIFT | (__keyplane&SHIFT_ANY) | MKOLDSHIFT(oldplane));

        }
        }
    }
    else {
        __keyb_postmsg(KM_KEYUP + key);

        if(key<60 || (__keynumber==KB_ALPHA)) {
        if(__keynumber>0) __keynumber=-__keynumber;
        __keycount=-BOUNCE_KEYTIME;
        __keyplane&=~((SHIFT_LS|SHIFT_RS|SHIFT_ALPHA)<<16);

        if(!(__keyplane& (SHIFT_HOLD | SHIFT_ONHOLD))) {
            int oldkeyplane=__keyplane;
            __keyplane&=~(SHIFT_LS|SHIFT_RS|SHIFT_ALPHA); // KILL ALL SHIFT PLANES
            __keyplane|=(__keyplane>>17)&SHIFT_ALPHA; // KEEP ALPHA IF LOCKED
            if(oldkeyplane!=__keyplane)	__keyb_postmsg(KM_SHIFT | (__keyplane&SHIFT_ANY) | MKOLDSHIFT(oldkeyplane));
        }
        }
        else {
            int oldkeyplane=__keyplane;
            if(key==KB_LSHIFT) {
                __keyplane&=~((SHIFT_LSHOLD|SHIFT_LS)^((__keyplane>>16)&SHIFT_LS));
                __keyplane&=~((SHIFT_ALPHA)^(((__keyplane>>16)|(__keyplane>>17))&SHIFT_ALPHA));

            }
            if(key==KB_RSHIFT) {
                __keyplane&=~((SHIFT_RSHOLD|SHIFT_RS)^((__keyplane>>16)&SHIFT_RS));
                __keyplane&=~((SHIFT_ALPHA)^(((__keyplane>>16)|(__keyplane>>17))&SHIFT_ALPHA));

            }
            if(key==KB_ALPHA) {
                __keyplane&=~((SHIFT_ALPHAHOLD|SHIFT_ALPHA)^(((__keyplane>>16)|(__keyplane>>17))&SHIFT_ALPHA));

            }
            if(key==KB_ON) {
                __keyplane&=~SHIFT_ONHOLD;
            }
            __keyb_postmsg(KM_SHIFT | (__keyplane&SHIFT_ANY) | MKOLDSHIFT(oldkeyplane));

            __keynumber=-key;
            __keycount=-BOUNCE_KEYTIME;
        }

    }
    }
    b>>=1;
    a>>=1;
    ++key;
    }
    }
    // ANALYZE STATUS OF CURRENT KEYPRESS
    if(__keynumber>=0) {
    if(__kmat & (1LL<<__keynumber)) {
        // KEY STILL PRESSED, INCREASE COUNTER
        ++__keycount;
        if( (__keycount>LONG_KEYPRESSTIME) )
        {
            //if(!(__keyflags&KF_NOREPEAT)) {
            // ONLY CERTAIN KEYS WILL AUTOREPEAT
            switch(__keynumber)
            {
            case KB_SPC:
            case KB_BKS:
                if(__keyplane&SHIFT_ANY) {
                    __keyb_postmsg(KM_LPRESS | __keynumber | (__keyplane&SHIFT_ANY));
                    __keycount=-LONG_KEYPRESSTIME;
                    break;
                }
                // OTHERWISE DO REPEAT
            case KB_UP:
            case KB_DN:
            case KB_LF:
            case KB_RT:
                // THESE ALWAYS REPEAT, EVEN SHIFTED
                __keyb_postmsg(KM_REPEAT | __keynumber | (__keyplane&SHIFT_ANY));
                __keycount=-REPEAT_KEYTIME;
                break;
            default:
                // DO NOT AUTOREPEAT, DO LONG PRESS
                __keyb_postmsg(KM_LPRESS | __keynumber | (__keyplane&SHIFT_ANY));
                __keycount=-LONG_KEYPRESSTIME;
            }

        }

        if(!__keycount) {

            switch(__keynumber)
            {
            case KB_SPC:
            case KB_BKS:
                if(__keyplane&SHIFT_ANY) {
                    __keyb_postmsg(KM_LREPEAT | __keynumber | (__keyplane&SHIFT_ANY));
                    __keycount-=LONG_KEYPRESSTIME;
                    break;
                }
                // OTHERWISE DO REPEAT
            case KB_UP:
            case KB_DN:
            case KB_LF:
            case KB_RT:
                // THESE ALWAYS REPEAT, EVEN SHIFTED
                __keyb_postmsg(KM_REPEAT | __keynumber | (__keyplane&SHIFT_ANY));
                __keycount-=REPEAT_KEYTIME;
                break;
            default:
                // DO NOT AUTOREPEAT, DO LONG PRESS
                __keyb_postmsg(KM_LREPEAT | __keynumber | (__keyplane&SHIFT_ANY));
                __keycount-=LONG_KEYPRESSTIME;
            }

        }

    }
    }


    // REPEATER
    if(__kmat==0) {
        if(__keycount>=0) {
            tmr_events[0].status=0;
            __keynumber=0;
        }
        else { ++__keycount; }
    } else {

        if(!(tmr_events[0].status&1)) {
            // ACTIVATE THE TIMER EVENT IF NOT ALREADY RUNNING
            tmr_events[0].ticks=tmr_ticks()+tmr_events[0].delay;
            tmr_events[0].status=3;
            __tmr_eventreschedule();
    }
    }


    // On-C and On-A-F handling

    if(__kmat== ((1ULL<<KB_ON) | (1ULL<<KB_A)  | (1ULL<<KB_F)))
    {
        // ON-A-F pressed, offer the option to stop the program

        __keyb_lock=0;


        throw_exception("User BREAK requested",__EX_CONT | __EX_WARM | __EX_RESET );

        //  AFTER RETURNING FROM THE EXCEPTION HANDLER, ALL KEYS ARE GUARANTEED TO BE RELEASED
        //  DO AN UPDATE TO SEND KEY_UP MESSAGES TO THE APPLICATION AND CORRECT SHIFT PLANES
        goto doupdate;

    }

    if(__kmat== ((1ULL<<KB_ON) | (1ULL<<KB_A)  | (1ULL<<KB_C)))
    {
        // ON-A-C pressed, offer the option to stop the program

        __keyb_lock=0;


        throw_exception("RPL Break requested",__EX_CONT | __EX_WIPEOUT | __EX_RESET | __EX_RPLREGS );

        //  AFTER RETURNING FROM THE EXCEPTION HANDLER, ALL KEYS ARE GUARANTEED TO BE RELEASED
        //  DO AN UPDATE TO SEND KEY_UP MESSAGES TO THE APPLICATION AND CORRECT SHIFT PLANES
        goto doupdate;

    }


    __keyb_lock=0;

}


void __keyb_init()
{
__keyflags=KF_RUNNING;
__keyplane=0;
__kused=__kcurrent=0;
__keynumber=0;
__kmat=0LL;
__keyb_repeattime=50/KEYB_SCANSPEED;
__keyb_longpresstime=800/KEYB_SCANSPEED;
__keyb_debounce=0; //20/KEYB_SCANSPEED;
__keyb_lock=0;
__pckeymatrix=0;
// INITIALIZE TIMER EVENT 0

tmr_events[0].eventhandler=__keyb_update;
tmr_events[0].delay=(KEYB_SCANSPEED*tmr_getsysfreq())/1000;

tmr_events[0].status=0;

__keyflags|=KF_RUNNING;

}


void __keyb_stop(unsigned int *keysave)
{
UNUSED_ARGUMENT(keysave);

tmr_events[0].status=0;

__keyflags&=~1;
}


void keyb_settiming(int repeat,int longpress,int debounce)
{
    __keyb_repeattime=(repeat+KEYB_SCANSPEED-1)/KEYB_SCANSPEED;
    __keyb_longpresstime=(longpress+KEYB_SCANSPEED-1)/KEYB_SCANSPEED;
    __keyb_debounce=(debounce+KEYB_SCANSPEED-1)/KEYB_SCANSPEED;
}

void keyb_setrepeat(int repeat)
{
    if(!repeat) __keyflags|=KF_NOREPEAT;
    else __keyflags&=~KF_NOREPEAT;
}

void keyb_setalphalock(int single_alpha_lock)
{
    if(single_alpha_lock) __keyflags|=KF_ALPHALOCK;
    else __keyflags&=~KF_ALPHALOCK;

}

void keyb_setshiftplane(int leftshift,int rightshift,int alpha,int alphalock)
{
    while(keyb_getmatrix()!=0LL) ;		// WAIT UNTIL NO MORE KEYS ARE PRESSED TO UPDATE SHIFT STATE

    int oldplane=__keyplane;

    if(leftshift) __keyplane|=SHIFT_LS|(SHIFT_LS<<16);
    else __keyplane&=~(SHIFT_LS|(SHIFT_LS<<16));
    if(rightshift) __keyplane|=SHIFT_RS|(SHIFT_RS<<16);
    else __keyplane&=~(SHIFT_RS|(SHIFT_RS<<16));
    if(alpha||alphalock) __keyplane|=SHIFT_ALPHA|(SHIFT_ALPHA<<16);
    else __keyplane&=~(SHIFT_ALPHA|(SHIFT_ALPHA<<16));
    if(alphalock) {
    __keyplane|=SHIFT_ALPHA<<17; // LOCK ALPHA
    __keyplane&=~(SHIFT_ALPHA<<16);
    }
    else {
        __keyplane&=~(SHIFT_ALPHA<<17);
    }
    keyb_postmsg(KM_SHIFT | (__keyplane&SHIFT_ANY) | MKOLDSHIFT(oldplane));

}


