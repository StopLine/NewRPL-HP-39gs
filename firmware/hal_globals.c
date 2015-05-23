#include <ui.h>
// SYSTEM GLOBAL VARIABLES, MAPPED TO RAM 0X08004000 (8 KBYTES MAXIMUM!)
// THESE VARIABLES ARE NOT PERSISTENT ACROSS ON/OFF, REBOOT, ETC.
// VARIABLES CANNOT HAVE STATIC INITIAL VALUES, MUST BE INITIALIZED BY SOFTWARE.


// BASIC HAL VARIABLES
HALSCREEN halScreen __SYSTEM_GLOBAL__;


// CPU SPEED HIGH LEVEL
BINT halFlags __SYSTEM_GLOBAL__;
HEVENT halBusyEvent __SYSTEM_GLOBAL__;




// KEYBOARD DRIVER, HIGH LEVEL
// NOT CONST, THIS IS ACTUALLY A GLOBAL VARIABLE TAKING PERMANENT SPACE
// ONLY TO BE USED BY THE KEYBOARD HANDLER
WORD cmdKeySeco[4] __SYSTEM_GLOBAL__;


// COMMAND LINE EDITOR

