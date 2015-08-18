#include <newrpl.h>
#include <libraries.h>
#include <ui.h>



//#include "fw_globals.c"
//#include "sys/lcd.c"
//#include "sys/cpu.c"
//#include "sys/irq.c"
//#include "sys/timer.c"
//#include "sys/battery.c"
//#include "sys/keyboard.c"
//#include "sys/exception.c"
//#include "sys/stdlib.c"

//#include "sys/minifont.c"
//#include "sys/font5.c"
//#include "sys/font6.c"
//#include "sys/font7.c"
//#include "sys/font8.c"
//#include "sys/graphics.c"

//#include "hal_screen.c"
//#include "hal_battery.c"
//#include "hal_keyboard.c"
//#include "hal_memory.c"

#define enter_mode(mode) call_swi(mode)

void switch_mode(int mode) __attribute__ ((naked));
void switch_mode(int mode)
{
    asm volatile ("and r0,r0,#0x1f");
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0x1f");
    asm volatile ("orr r1,r1,r0");
    asm volatile ("mov r0,lr");         // GET THE RETURN ADDRESS **BEFORE** MODE CHANGE
    asm volatile ("msr cpsr_all,r1");
    asm volatile ("bx r0");
}

unsigned int get_mode()
{
    register unsigned int cpsr;

    asm volatile ("mrs %0,cpsr_all" : "=r" (cpsr) );

    return cpsr&0x1f;
}

void call_swi(unsigned int arg1) __attribute ((noinline));
void call_swi(unsigned int arg1)
{
    asm volatile ("swi #0" : : : "r0","r1");
}


void enable_interrupts()
{
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");
}


void disable_interrupts()
{
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");
}



void set_stack(unsigned int *) __attribute__ ((naked));
void set_stack(unsigned int *newstackptr)
{

       asm volatile ("mov sp,r0");
       asm volatile ("bx lr");

}




void setup_hardware()
{
volatile unsigned int *ptr=(unsigned int *)0x48000000;

// SETUP MEMORY CONTROLLER, TO MAKE SURE WE CAN ACCESS ROM AND RAM CORRECTLY
 *ptr=0x11111192;
  ptr[1]=0x200;
  ptr[2]=0x300;

// SETUP POWER MANAGEMENT
  ptr=(unsigned int *)0x4C000000;
  ptr[0]=0x5a55a5;
  ptr[3]=0xE330;    // ENABLE CLOCK FOR ADC, RTC, GPIO, PWM, SD, LCD, FLASH, DISABLE EVERYTHING ELSE (INCLUDING USB)
  // THE REST OF THE REGISTERS WILL BE PROGRAMMED BY cpu_setspeed

  // SETUP GPIO
  ptr=(unsigned int *)0x56000000;
  ptr[0]=0x103f;    // GPACON
  ptr[4]=0x155555;  // GPBCON
  ptr[6]=0x7ff;     // GPB PULLUPS
  ptr[8]=0xaa56a9;  // GPCCON
  ptr[10]=0xffff;    // GPC PULLUPS
  ptr[12]=0x55555515; // GPDCON
  ptr[14]=0xffff;     // GPD PULLUPS
  ptr[16]=0x56aa955;    // GPECON
  ptr[18]=0xf83f;       // GPE PULLUPS
  ptr[20]=0x6a96;       // GPFCON
  ptr[22]=0xff;         // GPF PULLUPS
  ptr[24]=0x5555aaa9;   // GPGCON
  ptr[26]=0x1;          // GPG PULLUPS
  ptr[28]=0x155555;     // GPHCON
  ptr[30]=0x7ff;        // GPH PULLUPS

  ptr[41]=0x00fffff0;   // EINTMASK = MASK ALL EXTERNAL INTS
  // SETUP MISCELLANEOUS

}



// REAL PROGRAM STARTUP (main)
// WITH VIRTUAL MEMORY ALREADY INITIALIZED


void main_virtual(unsigned int mode)
{

    // INITIALIZE SOME SYSTEM VARIABLES


    gglsurface scr;
    int wascleared=0;
    bat_setup();

    // MONITOR BATTERY VOLTAGE TWICE PER SECOND
    HEVENT event=tmr_eventcreate(battery_handler,500,1);

    ggl_initscr(&scr);

    //   CLEAR SCREEN
    ggl_rect(&scr,0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1,0);

    // CAREFUL: THESE TWO ERASE THE WHOLE RAM, SHOULD ONLY BE CALLED AFTER TTRM
    if(!halCheckMemoryMap()) {
        // WIPEOUT MEMORY
    halInitMemoryMap();
    rplInit();
    wascleared=1;
    }
    else rplWarmInit();

    halInitScreen();
    halInitKeyboard();
    halInitBusyHandler();
    halRedrawAll(&scr);

    if(wascleared) halShowMsg("Memory Cleared");
    else halShowMsg("Memory Recovered");
    //halStatusAreaPopup();

    halOuterLoop();

    tmr_eventkill(event);
    //   CLEAR SCREEN
    ggl_rect(&scr,0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1,0x12345678);

}


void clear_globals()
{
    int *ptr=(int *)0x02005000;
    while(ptr!=(int *)0x02006000) *ptr++=0;
}



void startup(int) __attribute__ ((naked,noreturn));
void startup(int prevstate)
{

    // BOOTLOADER LEAVES STACK ON MAIN RAM, MOVE TO SRAM
    // ALSO WE ENTER IN SUPERVISOR MODE

    disable_interrupts();   // THIS REQUIRES SUPERVISOR MODE

    unsigned int mode=get_mode();

    set_stack((unsigned int *)0x40000e00);  // INITIAL STACK:

    setup_hardware();       // SETUP ACCESS TO OUT-OF-CHIP RAM MEMORY AMONG OTHER THINGS, THIS IS DONE BY THE BOOTLOADER BUT JUST TO BE SURE
    create_mmu_tables();

    //set_swivector_phys(__SVM_enter_mode);   // GET READY TO SWITCH PROCESSOR MODES

    //if(get_mode()==USER_MODE) enter_mode(SVC_MODE);   // GO INTO SUPERVISOR MODE, USES 2 WORDS OF SVC STACK AT UNKNOWN LOCATION!! (SET BY BOOTLOADER)
    //else switch_mode(SVC_MODE);
    //set_stack(0x40000f00);  // POINT TO SVC STACK TO SRAM

    switch_mode(SYS_MODE);  // NOW ENTER PRIVILEDGED MODE WITH NO REGISTER BANKING

    set_stack((unsigned int *)0x40000c00);  // POINT TO USER STACK TO SRAM

    enable_mmu();

    // FROM HERE ON, WE ARE RUNNING ON VIRTUAL MEMORY SPACE

    set_stackall(); // SET FINAL STACK POINTERS FOR ALL MODES IN VIRTUAL SPACE

    clear_globals();    // CLEAR TO ZERO ALL NON-PERSISTENT GLOBALS


    __exception_install();  // INITIALIZE IRQ AND EXCEPTION HANDLING

    enable_interrupts();

    tmr_setup();
    __keyb_init();

    __lcd_contrast=7;

    // ADD MORE HARDWARE INITIALIZATION HERE

    // ...

    // DONE WITH SYSTEM INITIALIZATION, SWITCH BACK TO USER MODE
    //switch_mode(SYS_MODE);


    // NON-PRIVILEGED MODE FROM HERE ON...

    cpu_setspeed(6000000);
    lcd_setmode(2,(int *)MEM_PHYS_SCREEN);


    main_virtual(mode);

    while(1) ;
}


/* PHYSICAL RAM LAYOUT:

  // FIRST 32 KBYTES ARE USED BY THE BOOTLOADER, SO NOTHING PERSISTENT CAN GO THERE.

  // VOLATILE MEMORY - LOST AFTER RESET/POWEROFF
  0x08000000 - EXCEPTION HANDLERS
  0x08000020 - SYSTEM VARIABLES USED BY BOOTLOADER TO DETERMINE POWER-OFF STATE
  0x08000100 - SCRATCH MEMORY (RREG MEMORY = 504 WORDS PER REGISTER, 8 REGISTERS)
  0x08005000 - SYSTEM GLOBAL VARIABLES (4 KB)
  0x08006000 - 0x08007900h SCREEN (8 KB)

  // PERSISTENT MEMORY
  0x08008000 - MMU TABLE BASE (1 KB) (MUST BE 32 KBYTE ALIGNED) - 1K MAPS FROM 0x000|00000 TO 0x0FF|00000 INCLUSIVE
  0x08008400 - DSTK 2ND TABLE (1 KB) 1K MAPS 256 4K PAGES = 1 MB OF RAM
  0x08008800 - RSTK 2ND TABLE (1 KB)
  0x08008C00 - LAM 2ND TABLE (1 KB)
  0x08009000 - DIRS 2ND TABLE (1 KB)
  0x08009400 - TEMPBLOCKS 2ND TABLE (1 KB)
  0x08009800 - TEMPOB 2ND TABLE (4 KB) 1K MAPS 4 MB OF RAM MAX (SPACE LEFT FOR FUTURE VIRTUAL MEMORY)
  0x0800A800 - REVERSE WALK TABLE (512 bytes) - REVERSE MAP PHYSICAL TO VIRTUAL - INDICATES WHICH RAM PAGES ARE USED/FREE
  0x0800AC00 - PERSISTENT VARIABLES OF THE RPL CORE + MPD library MALLOC AREA (32K)
  0x0800XXXX - FIRST AVAILABLE PAGE OF RAM, AFTER END OF ALL PERSISTENT VARIABLES, ROUNDED TO 4K

 TOTAL USED FOR SYSTEM + MMU TABLES = 26 KBYTES (484 KBYTES AVAILABLE FOR THE USER)

*/

#define MMU_SECTION_RAM(a) (((a)&0xfff00000)| 0xc1e)
#define MMU_SECTION_ROM(a) (((a)&0xfff00000)| 0x01e)
#define MMU_SECTION_DEV(a) (((a)&0xfff00000)| 0xc12)

#define MMU_PAGES_RAM(a) (((a)&0xfffffc00)|0x011)

#define MMU_PAGE(a) (((a)&0xfffff000)|0xffe)

#define MMU_LEVEL1_INDEX(virt) (((virt)>>20)&0xfff)
#define MMU_LEVEL2_INDEX(virt) (((virt)>>12)&0xff)

#define MMU_MAP_SECTION_ROM(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_SECTION_ROM(phys))
#define MMU_MAP_SECTION_RAM(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_SECTION_RAM(phys))
#define MMU_MAP_SECTION_DEV(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_SECTION_DEV(phys))
#define MMU_MAP_COARSE_RAM(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_PAGES_RAM(phys))

#define MMU_MAP_PAGE(phys,virt) ( ( (unsigned int *)(mmu_base[MMU_LEVEL1_INDEX(virt)]&0xfffffc00))[MMU_LEVEL2_INDEX(virt)]=MMU_PAGE(phys))

extern int __last_used_byte;

void create_mmu_tables()
{

    unsigned int *mmu_base=(unsigned int *)0x08008000;
    volatile unsigned int *ptr=mmu_base;

    // WIPE OUT MAIN MMU TABLE
    while(ptr!=(unsigned int *)0x08008400) *ptr++=0;


//MEM_ROM         0x00000000  // VIRTUAL (AND PHYSICAL) ROM LOCATION (UP TO 4 MBytes)
    MMU_MAP_SECTION_ROM(0x00000000,0x00000000);    // MAP 1ST MEGABYTE SECTION
    MMU_MAP_SECTION_ROM(0x00100000,0x00100000);    // MAP TOTAL 2 MBYTES OF ROM
    
//MEM_DSTK        0x00400000  // DATA STACK VIRTUAL LOCATION (UP TO 4 MB)
    MMU_MAP_COARSE_RAM(0x08008400,0x00400000);  // CREATE THE PAGE MAP, DON'T ADD ANY PAGES YET
    
//MEM_RSTK        0x00800000  // RETURN STACK VIRTUAL LOCATION (UP TO 4 MB)
    MMU_MAP_COARSE_RAM(0x08008800,0x00800000);

//MEM_LAM         0x00C00000  // LOCAL VARIABLES VIRTUAL LOCATION (UP TO 4 MB)
    MMU_MAP_COARSE_RAM(0x08008C00,0x00C00000);

//MEM_DIRS        0x01000000  // GLOBAL DIRECTORIES VIRTUAL LOCATION (UP TO 4 MB)
    MMU_MAP_COARSE_RAM(0x08009000,0x01000000);

//MEM_TEMPBLOCKS  0x01400000  // BLOCK INDEX FOR TEMPOB VIRTUAL LOCATION (UP TO 4 MB)
    MMU_MAP_COARSE_RAM(0x08009400,0x01400000);

//MEM_TEMPOB      0x01800000  // GLOBAL OBJECT ALLOCATION MEMORY VIRTUAL LOCATION (UP TO 8 MB)
    MMU_MAP_COARSE_RAM(0x08009800,0x01800000);
    MMU_MAP_COARSE_RAM(0x08009C00,0x01900000);  // SECOND TABLE FOR UP TO 2 MB
    MMU_MAP_COARSE_RAM(0x0800A000,0x01A00000);  // THIRD TABLE FOR UP TO  3 MB
    MMU_MAP_COARSE_RAM(0x0800A400,0x01B00000);  // FOURTH TABLE FOR UP TO 4 MB


//MEM_SYSTEM      0x02000000  // MAIN RAM FOR SYSTEM VARIABLES
    MMU_MAP_SECTION_RAM(0x08000000,0x02000000);
                              // 1:1 MEMORY MAPPING OF MAIN RAM BUT UNCACHED AND UNBUFFERED
                              // BOOTLOADER EXPECTS RELOCATED EXCEPTION HANDLERS HERE
                              // ALSO SCREEN ACCESS SHOULD NOT BE BUFFERED/CACHED
    MMU_MAP_SECTION_DEV(0x08000000,0x08000000);

//MEM_SRAM        0x03000000
    MMU_MAP_SECTION_RAM(0x40000000,0x03000000);     // SRAM



//MEM_HARDWARE    0x04800000

    MMU_MAP_SECTION_DEV(0x48000000,0x04800000);     // MEMORY CONTROLLLER
    MMU_MAP_SECTION_DEV(0x49000000,0x04900000);     // USB HOST CONTROLLLER
    MMU_MAP_SECTION_DEV(0x4A000000,0x04A00000);     // INTERRUPT CONTROLLER
    MMU_MAP_SECTION_DEV(0x4B000000,0x04B00000);     // DMA
    MMU_MAP_SECTION_DEV(0x4C000000,0x04C00000);     // CLOCK
    MMU_MAP_SECTION_DEV(0x4D000000,0x04D00000);     // LCD
    MMU_MAP_SECTION_DEV(0x4E000000,0x04E00000);     // NAND
    MMU_MAP_SECTION_DEV(0x50000000,0x05000000);     // UART
    MMU_MAP_SECTION_DEV(0x51000000,0x05100000);     // PWM TIMERS
    MMU_MAP_SECTION_DEV(0x52000000,0x05200000);     // USB DEVICE
    MMU_MAP_SECTION_DEV(0x53000000,0x05300000);     // WATCHDOG
    MMU_MAP_SECTION_DEV(0x54000000,0x05400000);     // I2C
    MMU_MAP_SECTION_DEV(0x55000000,0x05500000);     // I2S
    MMU_MAP_SECTION_DEV(0x56000000,0x05600000);     // GPIO
    MMU_MAP_SECTION_DEV(0x57000000,0x05700000);     // RTC
    MMU_MAP_SECTION_DEV(0x58000000,0x05800000);     // ADC
    MMU_MAP_SECTION_DEV(0x59000000,0x05900000);     // SPI
    MMU_MAP_SECTION_DEV(0x5A000000,0x05A00000);     // SD/MMC





}

void __SVM_enable_mmu()
{

    asm volatile ("mov r0,#0x08000000");
    asm volatile ("add r0,r0,#0x8000");
    asm volatile ("mcr p15,0,r0,c2,c0,0");      // WRITE MMU BASE REGISTER, ALL CACHES SHOULD'VE BEEN CLEARED BEFORE

    asm volatile ("mrc p15, 0, r0, c1, c0, 0");
    asm volatile ("orr r0,r0,#5");              // ENABLE MMU AND DATA CACHES
    asm volatile ("orr r0,r0,#0x1000");         // ENABLE INSTRUCTION CACHE

    asm volatile ("mcr p15, 0, r0, c1, c0, 0");

    asm volatile ("mov r0,r0");                 // NOP INSTRUCTIONS THAT ARE FETCHED FROM PHYSICAL ADDRESS
    asm volatile ("mov r0,r0");


}


// ALL CACHES AND TLB MUST BE FLUSHED BEFORE DISABLING MMU
void __SVM_disable_mmu()
{

    asm volatile ("mrc p15, 0, r0, c1, c0, 0");
    asm volatile ("bic r0,r0,#5");              // DISABLE MMU AND DATA CACHES
    asm volatile ("bic r0,r0,#0x1000");         // DISABLE INSTRUCTION CACHE

    asm volatile ("mcr p15, 0, r0, c1, c0, 0");

    asm volatile ("mov r0,r0");                 // NOP INSTRUCTIONS THAT ARE FETCHED FROM PHYSICAL ADDRESS
    asm volatile ("mov r0,r0");

}


static void __SVM_flush_Dcache(void)
{
    register unsigned int counter asm("r2");
    register unsigned int cacheaddr asm("r3");

    counter=0;
    while(counter<512) {
    cacheaddr=((counter>>1)&0xe0) | ((counter&63)<<26);
    // CLEAN AND INVALIDATE ENTRY USING INDEX
    asm volatile ("mcr p15, 0, %0, c7, c14, 2" : : "r" (cacheaddr));

    ++counter;
    }

}

static void __SVM_flush_Icache(void)
{
    // CLEAN AND INVALIDATE ENTRY USING INDEX

    register unsigned int value;

    value=0;
    asm volatile ("mcr p15, 0, %0, c7, c5, 0" : : "r" (value));

}

static void __SVM_flush_TLB(void)
{
    // CLEAN AND INVALIDATE ENTRY USING INDEX

    register unsigned int value;

    value=0;
    asm volatile ("mcr p15, 0, %0, c8, c7, 0" : : "r" (value));

}


inline unsigned int get_swivector_phys()
{
    return *(unsigned int *)0x08000008;
}

inline unsigned int get_swivector()
{
    return *(unsigned int *)0x02000008;
}

inline void set_swivector(void *handler)
{
    void **ptr=(void **)0x02000008;
    *ptr=handler;
}

inline void set_swivector_phys(void *handler)
{
    void **ptr=(void **)0x08000008;
    *ptr=handler;
}


void set_stackall()
{
    // THE USER STACK IS ALREADY SETUP PROPERLY

    switch_mode(SVC_MODE);

    set_stack((unsigned int *)0x03000e00);

    switch_mode(ABT_MODE);

    set_stack((unsigned int *)0x03000e00);

    switch_mode(UND_MODE);

    set_stack((unsigned int *)0x03000e00);

    switch_mode(FIQ_MODE);

    set_stack((unsigned int *)0x03000ffc);

    switch_mode(IRQ_MODE);

    set_stack((unsigned int *)0x03000f80);

    switch_mode(SYS_MODE);

    asm volatile ("nop");   // DO SOMETHING IN USER MODE TO PREVENT COMPILER FROM MAKING A TAIL CALL OPTIMIZATION
}

void reset_stackall()
{
    // THE USER STACK IS ALREADY SETUP PROPERLY

    switch_mode(SVC_MODE);

    set_stack((unsigned int *)0x40000e00);

    switch_mode(ABT_MODE);

    set_stack((unsigned int *)0x40000e00);

    switch_mode(UND_MODE);

    set_stack((unsigned int *)0x40000e00);

    switch_mode(FIQ_MODE);

    set_stack((unsigned int *)0x40000ffc);

    switch_mode(IRQ_MODE);

    set_stack((unsigned int *)0x40000f80);

    switch_mode(SYS_MODE);

    asm volatile ("nop");   // DO SOMETHING IN USER MODE TO PREVENT COMPILER FROM MAKING A TAIL CALL OPTIMIZATION
}

void enable_mmu()
{

    // WE MUST BE IN SUPERVISOR MODE ALREADY

    __SVM_flush_Dcache();
    __SVM_flush_Icache();
    __SVM_flush_TLB();

    __SVM_enable_mmu();

// MOVE USER STACK TO VIRTUAL MEMORY, NEEDED HERE TO BE ABLE TO RETURN
   asm volatile ("bic sp,sp,#0xff000000");
   asm volatile ("orr sp,sp,#0x03000000");


}

void disable_mmu()
{

    // WE MUST BE IN SUPERVISOR MODE ALREADY

    __SVM_flush_Dcache();
    __SVM_flush_Icache();
    __SVM_flush_TLB();

    __SVM_disable_mmu();

// MOVE USER STACK TO PHYSICAL MEMORY, NEEDED HERE TO BE ABLE TO RETURN
   asm volatile ("bic sp,sp,#0xff000000");
   asm volatile ("orr sp,sp,#0x40000000");


}



// THIS FUNCTION REBOOTS THE RPL CORE COMPLETELY
// ALL ELEMENTS IN THE STACK WILL BE LOST
// MUST BE ENTERED IN SUPERVISOR MODE
void halWarmStart()
{
    // TODO: ADD RPL ENGINE CLEANUP HERE BEFORE RESET


    // PUT THE CPU IN A KNOWN SLOW SPEED
    cpu_setspeed(6000000);
    // DISABLE THE MMU
    disable_mmu();
    // AND RESTART LIKE NEW
    startup(0);

    // STARTUP NEVER RETURNS...
}

void halWipeoutWarmStart()
{

    int *mmutable=(int *)MEM_REVERSEMMU;

    // INVALIDATE MMU TABLE TO CAUSE A WIPEOUT
    *mmutable=0;

    halWarmStart();
}



void halReset()
{
    // TODO: ADD RPL ENGINE CLEANUP HERE BEFORE RESET


   // PUT THE CPU IN A KNOWN SLOW SPEED
    cpu_setspeed(6000000);

    // MAKE SURE ALL WRITE BUFFERS ARE PROPERLY FLUSHED

    __SVM_flush_Dcache();
    __SVM_flush_Icache();
    __SVM_flush_TLB();


    // SET THE PRESCALER OF THE WATCHDOG AS FAST AS POSSIBLE AND A REASONABLE COUNT (ABOUT 87ms)
   *HWREG(WDT_REGS,8)=0x8000;
   *HWREG(WDT_REGS,0)=0x21;

    // AND WAIT FOR IT TO HAPPEN
    while(1);

}

// ENTER POWER OFF MODE
void halEnterPowerOff()
{
    // TODO: ADD RPL ENGINE CLEANUP HERE BEFORE RESET
    // PUT THE CPU IN A KNOWN SLOW SPEED
    cpu_setspeed(6000000);

    // WAIT FOR ALL KEYS TO BE RELEASED
    __keyb_waitrelease();


    cpu_off_prepare();

    // DISABLE THE MMU
    disable_mmu();
    reset_stackall();

    // AND GO DIE
    //startup(0);
    cpu_off_die();

}

// NEVER EXIT, THIS IS NOT AN APP, IT'S A FIRMWARE
int halExitOuterLoop()
{
    return 0;
}
