
void __tmr_eventreschedule();


// TIMERS
volatile long long __systmr __attribute__ ((section (".system_globals")));
volatile long long __evtmr __attribute__ ((section (".system_globals")));
unsigned int __sysfreq __attribute__ ((section (".system_globals")));
volatile int __tmr_lock __attribute__ ((section (".system_globals")));
timed_event tmr_events[NUM_EVENTS] __attribute__ ((section (".system_globals")));





// SAVE TIMERS CONFIGURATION - 13 WORDS REQUIRED
void tmr_save(unsigned int *tmrbuffer)
{

tmrbuffer[0]=*HWREG(TMR_REGS,0);
tmrbuffer[1]=*HWREG(TMR_REGS,0x4);
tmrbuffer[2]=*HWREG(TMR_REGS,0x8);
tmrbuffer[3]=*HWREG(TMR_REGS,0xc);
tmrbuffer[4]=*HWREG(TMR_REGS,0x10);

tmrbuffer[5]=*HWREG(TMR_REGS,0x18);
tmrbuffer[6]=*HWREG(TMR_REGS,0x1c);
tmrbuffer[7]=*HWREG(TMR_REGS,0x24);
tmrbuffer[8]=*HWREG(TMR_REGS,0x28);
tmrbuffer[9]=*HWREG(TMR_REGS,0x30);
tmrbuffer[10]=*HWREG(TMR_REGS,0x34);
tmrbuffer[11]=*HWREG(TMR_REGS,0x3c);
tmrbuffer[12]=*HWREG(INT_REGS,0x8);	// GET TIMERS INTERRUPT MASK
}


// RESTORE TIMERS

void tmr_restore(unsigned int *tmrbuffer)
{
// MASK ALL TIMER INTERRUPTS
*HWREG(INT_REGS,0x8)|=0x7c00;

// STOP ALL RUNNING TIMERS
*HWREG(TMR_REGS,8)&=0xFF8000e0;

// CLEAN ALL PENDING INTERRUPTS
*HWREG(INT_REGS,0x10)=0x7c00;
*HWREG(INT_REGS,0)=0x7c00;



// RESTORE REGISTERS
*HWREG(TMR_REGS,0)=tmrbuffer[0];
*HWREG(TMR_REGS,4)=tmrbuffer[1];
*HWREG(TMR_REGS,0xc)=tmrbuffer[3];
*HWREG(TMR_REGS,0x10)=tmrbuffer[4];

*HWREG(TMR_REGS,0x18)=tmrbuffer[5];
*HWREG(TMR_REGS,0x1c)=tmrbuffer[6];
*HWREG(TMR_REGS,0x24)=tmrbuffer[7];
*HWREG(TMR_REGS,0x28)=tmrbuffer[8];
*HWREG(TMR_REGS,0x30)=tmrbuffer[9];
*HWREG(TMR_REGS,0x34)=tmrbuffer[10];
*HWREG(TMR_REGS,0x3c)=tmrbuffer[11];

*HWREG(INT_REGS,0x8)=(*HWREG(INT_REGS,0x8)&(~0x7c00))|(tmrbuffer[12]&0x7c00);

// FINALLY RESTART ALL TIMERS
*HWREG(TMR_REGS,8)=tmrbuffer[2];
}









// THIS EVENT IS CALLED TO CHECK IF WE HAVE AN EVENT
void __tmr_newirqeventsvc()
{
    if(cpu_getlock(2,&__tmr_lock)) return;    // GET A LOCK ON THE TIMERS, ABORT IF SOMEBODY ELSE HAS IT
    __tmr_lock=1;

    // GET THE CURRENT SYSTEM TICKS
    tmr_t current_ticks=tmr_ticks();
    int f;

    if(__evtmr && (current_ticks>=__evtmr)) {
        // PROCESS THE NEXT EVENT
        for(f=0;f<NUM_EVENTS;++f)
        {
        if(tmr_events[f].status&1) {
            if(tmr_events[f].ticks<=__evtmr) {
            // EXECUTE EVENT
            (*(tmr_events[f].eventhandler))();
            if((tmr_events[f].status&3)==3) {
                // AUTORELOAD
                tmr_events[f].ticks+=tmr_events[f].delay;
            }
            else { tmr_events[f].status=0; }
            }
        }
        }
    }

    __tmr_lock=0;

    __tmr_eventreschedule();

}

void __tmr_irqservice()
{
__systmr+=0x10000;

if( __evtmr && (__evtmr-__systmr<0x10000) && !(*HWREG(TMR_REGS,0x8)&0x100)) __tmr_newirqeventsvc();

}

/*
void __tmr_irqeventsvc()
{

__evtmr+=*HWREG(TMR_REGS,0x1c)+1;	// TCMP1 HOLDS PREVIOUS RELOAD VALUE
*HWREG(TMR_REGS,0x1c)=*HWREG(TMR_REGS,0x18);	// PRESERVE CURRENT LOAD VALUE

int nextticks=0x7fffffff;
register int ntck,active;
int f;

for(active=0,f=0;f<NUM_EVENTS;++f)
{
if(tmr_events[f].status&1) {

		++active;
	// EVENT IS ACTIVE, CHECK IF TIME TO ACTIVATE
    if((tmr_events[f].ticks==__evtmr) || (tmr_events[f].status&4))
	{
		// EXECUTE EVENT
        (*(tmr_events[f].eventhandler))();
        if((tmr_events[f].status&3)==3) {
			// AUTORELOAD
            tmr_events[f].ticks+=tmr_events[f].delay;
            tmr_events[f].status&=~4;
		}
        else { tmr_events[f].status&=~5; continue; }
	}
	
	// CALCULATE NEXT TIMER DELAY
    ntck=tmr_events[f].ticks-__evtmr-*HWREG(TMR_REGS,0x1c)-1;
	
	// IF ntck<0 THE EVENT WILL BE MISSED, EXECUTED LATE ON NEXT INTERRUPT
	// IF ntck==0 THE EVENT WILL HAPPEN NEXT TIMER INTERRUPT

	if(ntck<=0) {
	if(ntck<0) {
		// THIS CAN ONLY HAPPEN WHEN A NEW EVENT IS ADDED/REMOVED,
		// AND ONLY ON RARE CASES
		// THE EVENT WILL BE EXECUTED LATE (FEW usec) ON THE NEXT INTERRUPT
        tmr_events[f].status|=4; // REQUEST FORCED EVENT EXECUTION ON NEXT INTERRUPT
	}
	
	// CATCH UP WITH COUNTERS IF MULTIPLE EVENTS MISSED
	while(ntck<=0) {
        ntck+=tmr_events[f].delay;
    tmr_events[f].ticks+=tmr_events[f].delay;
	}
    tmr_events[f].ticks-=tmr_events[f].delay;
	}
	if( (ntck<nextticks) && (ntck>=0)) nextticks=ntck;	// RECORD AS NEXT CLOCK	
		
	}
}

if(!active) {
// NO MORE EVENTS ARE ACTIVE, STOP TIMER UNTIL NEW EVENTS ARE SCHEDULED
*HWREG(TMR_REGS,0x8)&=~0xf00;
__evtmr=0;
return;
}

if(nextticks>0x10800) nextticks=0x10000;
if(nextticks>0x10000) nextticks=0x8000;

*HWREG(TMR_REGS,0x18)=nextticks-1;

}
*/




// SYSTEM TIMER FREQUENCY = 100 KHz
// ALTERNATIVELY, SWITCH TO 125 KHz IF 100 KHz CANNOT BE CORRECTLY REPRESENTED
#define SYSTIMER_FREQ 100000
#define ALTSYSTIMER_FREQ 125000


// PREPARE TIMERS
void tmr_setup()
{

// MASK ALL TIMER INTERRUPTS
*HWREG(INT_REGS,0x8)|=0x7c00;

// STOP ALL RUNNING TIMERS
*HWREG(TMR_REGS,8)&=0xFF8000e0;

// CLEAN ALL PENDING INTERRUPTS
*HWREG(INT_REGS,0x10)=0x7c00;
*HWREG(INT_REGS,0)=0x7c00;


// START TIMER0 AS 64-BIT RUNNING TIMER AT 100 KHz
__tmr_lock=0;
__systmr=0;

unsigned int pclk=__cpu_getPCLK();

unsigned int divider,prescaler;

prescaler=(pclk<<3)/SYSTIMER_FREQ;
divider=1;

if(prescaler&0xf) {
// WARNING: REQUESTED FREQUENCY CANNOT BE REPRESENTED ACCURATELY
// THIS ONLY HAPPENS AT 75 MHz, ALL OTHER SPEEDS ARE OK
prescaler=(pclk<<3)/ALTSYSTIMER_FREQ;
}

while( prescaler> (1<<(11+divider)) ) { divider++; }

prescaler+= (1<<(2+divider));
prescaler>>=divider+3;

//if(divider>4) PCLK TOO HIGH TO ACHIEVE TIMER FREQUENCY, USE HIGHER MULTIPLE
if(divider>4) divider=4;

// CALCULATE SYSTEM CLOCK FREQUENCY
__sysfreq=(((pclk<<3)/prescaler) +  (1<<(divider+2)) )>>(divider+3);

// SET PRESCALER VALUES FOR TIMERS 0 AND 1
*HWREG(TMR_REGS,0)=(*HWREG(TMR_REGS,0)&(~0xFF)) | (prescaler-1);
*HWREG(TMR_REGS,4)=(*HWREG(TMR_REGS,4)&(~0xf000ff)) | (divider-1) | ((divider-1)<<4);

// SET COUNT VALUES TO MAXIMUM
*HWREG(TMR_REGS,0xc)=0xffff;
*HWREG(TMR_REGS,0x10)=0;
*HWREG(TMR_REGS,0x18)=0xffff;
*HWREG(TMR_REGS,0x1C)=0;

// PREPARE TIMED EVENTS HANDLER
int k;
for(k=0;k< NUM_EVENTS; ++k) tmr_events[k].status=0;

__evtmr=0;


__irq_addhook(10,(__interrupt__) &__tmr_irqservice);
__irq_addhook(11,(__interrupt__) &__tmr_newirqeventsvc);

// UNMASK INTERRUPTS FOR TIMERS 0 AND 1
*HWREG(INT_REGS,0x8)&=~0xc00;



// SET MANUAL UPDATE BIT
*HWREG(TMR_REGS,8)|=2;
// START TIMER0 
*HWREG(TMR_REGS,8)=(*HWREG(TMR_REGS,8)&(~0x1f)) | 0x9;

}


// FIX TIMERS SPEED WHEN CPU CLOCK IS CHANGED, CALLED FROM cpu_setspeed	
void __tmr_fix()
{
// MASK ALL TIMER INTERRUPTS
//*HWREG(INT_REGS,0x8)|=0x7c00;

// STOP ALL RUNNING TIMERS
//*HWREG(TMR_REGS,8)&=0xFF8000e0;

// CLEAN ALL PENDING INTERRUPTS
//*HWREG(INT_REGS,0x10)=0x7c00;
//*HWREG(INT_REGS,0)=0x7c00;


// START TIMER0 AS 64-BIT RUNNING TIMER AT 100 KHz


unsigned int pclk=__cpu_getPCLK();

unsigned int divider,prescaler;

prescaler=(pclk<<3)/SYSTIMER_FREQ;
divider=1;

if(prescaler&0xf) {
// WARNING: REQUESTED FREQUENCY CANNOT BE REPRESENTED ACCURATELY
// THIS ONLY HAPPENS AT 75 MHz, ALL OTHER SPEEDS ARE OK
prescaler=(pclk<<3)/ALTSYSTIMER_FREQ;
}

while( prescaler> (1<<(11+divider)) ) { divider++; }

prescaler+= (1<<(2+divider));
prescaler>>=divider+3;

//if(divider>4) PCLK TOO HIGH TO ACHIEVE TIMER FREQUENCY, USE HIGHER MULTIPLE
if(divider>4) divider=4;

// CALCULATE SYSTEM CLOCK FREQUENCY
__sysfreq=(((pclk<<3)/prescaler) +  (1<<(divider+2)) )>>(divider+3);

// SET PRESCALER VALUES FOR TIMERS 0 AND 1
*HWREG(TMR_REGS,0)=(*HWREG(TMR_REGS,0)&(~0xFF)) | (prescaler-1);
*HWREG(TMR_REGS,4)=(*HWREG(TMR_REGS,4)&(~0xf000ff)) | (divider-1) | ((divider-1)<<4);


}











tmr_t tmr_getsysfreq()
{
    return __sysfreq;
}


tmr_t tmr_ticks()
{
unsigned int *timer0=(unsigned int *)HWREG(TMR_REGS,0x14);

unsigned int before;
unsigned long long ticks1,ticks2;

do {
ticks1=__systmr;
before=*timer0;
ticks2=__systmr;
} while(ticks1!=ticks2);

return ticks1+0x10000-before;	
}


// RETURN DELAY IN MILLISECONDS
int tmr_ticks2ms(tmr_t before, tmr_t after)
{
    return ((after-before)*1000)/__sysfreq;
}

// RETURN DELAY IN MICROSECONDS
int tmr_ticks2us(tmr_t before, tmr_t after)
{
    return ((after-before)*1000000)/__sysfreq;
}

// ADD/SUBTRACT AN INTERVAL IN MILLISECONDS TO THE GIVEN TIME IN TICKS
tmr_t tmr_addms(tmr_t time, int ms)
{
    return time+((ms*__sysfreq)/1000);
}

// ADD/SUBTRACT AN INTERVAL IN MICROSECONDS TO THE GIVEN TIME IN TICKS
tmr_t tmr_addus(tmr_t time, int us)
{
    return time+((us*__sysfreq)/1000000);
}


void tmr_delayms(int milliseconds)
{
	if(milliseconds<=0) return;
tmr_t start=tmr_ticks();

// CALCULATE ENDING TICKS
tmr_t end=start+((milliseconds*__sysfreq)/1000);

// AND WAIT
while(end>tmr_ticks());

}
	
void tmr_delayus(int microseconds)
{
	if(microseconds<=0) return;

    unsigned long long start=tmr_ticks();

// CALCULATE ENDING TICKS
unsigned long long end=start+((microseconds*tmr_getsysfreq())/1000000);

// AND WAIT
while(end>tmr_ticks());

}

// WAIT UNTIL THE TIME REACHES A CERTAIN VALUE
void tmr_waituntil(tmr_t time)
{
	while(tmr_ticks()<time);
	return;
}




// RETURN AN EVENT HANDLER
HEVENT tmr_eventcreate(__interrupt__ handler,unsigned int ms,int autorepeat)
{
int f;

// NOTE: EVENT 0 IS RESERVED FOR THE SYSTEM (KEYBOARD DRIVER)

for(f=1;f<NUM_EVENTS;++f)
{
    if(!tmr_events[f].status) {
	// FOUND AVAILABLE EVENT	
        tmr_t ticks=tmr_ticks();

        tmr_events[f].eventhandler=handler;
        tmr_events[f].delay=(ms*tmr_getsysfreq())/1000;
        tmr_events[f].ticks=ticks+tmr_events[f].delay;
        tmr_events[f].status=((autorepeat)? 2:0)|1;

		__tmr_eventreschedule();
	return f;
	}
}
return -1;
}


void __tmr_eventreschedule()
{
    if(cpu_getlock(2,&__tmr_lock)) return;    // GET A LOCK ON THE TIMERS, ABORT IF SOMEBODY ELSE HAS IT

    tmr_t current_ticks;
    tmr_t next;
    int f;

        *HWREG(TMR_REGS,0x8)=(*HWREG(TMR_REGS,0x8)&(~0xf00));		// STOP TIMER1

        __tmr_lock=1;

        next=0;
        for(f=0;f<NUM_EVENTS;++f)
        {
        if(tmr_events[f].status&1) {
        // GET THE NEXT EVENT READY
        if(next) {
            if(tmr_events[f].ticks<next) next=tmr_events[f].ticks;
        } else next=tmr_events[f].ticks;
        }
        }

restart:

        // HERE next HAS THE NEXT SCHEDULED EVENT
        __evtmr=next;

        if(next) {

        current_ticks=tmr_ticks();
        if(next-current_ticks>=0x10000) {
            // NO NEED TO DO ANYTHING
        } else {
            // SET THE TIMER1 TO WAKE US UP EXACTLY AT THE RIGHT TIME
            if(next<=current_ticks) {
                *HWREG(TMR_REGS,0x18)=50;   // THIS EVENT IS LATE! RUN IN HALF A MILLISECOND TO CATCH UP
                __evtmr=current_ticks+50;
            }
            else *HWREG(TMR_REGS,0x18)=next-current_ticks;
            *HWREG(TMR_REGS,0x8)=(*HWREG(TMR_REGS,0x8)&(~0xf00))|0x200;		// MANUAL UPDATE
            *HWREG(TMR_REGS,0x8)=(*HWREG(TMR_REGS,0x8)&(~0xf00))|0x100;		// RESTART TIMER, SINGLE SHOT

        }
        }

        while(__tmr_lock==2) {
            // SOMEBODY TRIED TO ACQUIRE THE LOCK WHILE WE WERE USING IT
            // MUST REDO THE TIMING IN CASE THEY ADDED/REMOVED EVENTS
            __tmr_lock=1;
            next=0;
            int f;
            for(f=0;f<NUM_EVENTS;++f)
            {
            if(tmr_events[f].status&1) {
                // GET THE NEXT EVENT READY
                if(next) {
                    if(tmr_events[f].ticks<next) next=tmr_events[f].ticks;
                } else next=tmr_events[f].ticks;
            }
            }

            if(__evtmr!=next) goto restart;
        }

        __tmr_lock=0;

}


void tmr_eventpause(HEVENT event)
{
    if(event<0 || event>=NUM_EVENTS) return;
    tmr_events[event].status|=4;    // PREVENT EVENT FROM BEING REUSED
    tmr_events[event].status&=~1;	// DISABLE EVENT

}

void tmr_eventresume(HEVENT event)
{
    if(event<0 || event>=NUM_EVENTS) return;
    tmr_events[event].ticks=tmr_ticks()+tmr_events[event].delay;
    tmr_events[event].status|=1;    // TURN EVENT ON
    tmr_events[event].status&=~4;    // CLEAR THE DISABLE BIT
    __tmr_eventreschedule();
}



void tmr_eventkill(HEVENT event)
{
	if(event<0 || event>=NUM_EVENTS) return;
    tmr_events[event].status=0;	// KILL EVENT, TIMER WILL STOP AUTOMATICALLY

}
