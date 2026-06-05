#include "keypad_lib.h"


void ServiceKBD4X1(void);


unsigned char KBD_DATA = 0;
unsigned char KBD_TEMP = 0;
unsigned char TEMP_KBD_DATA = 0;

unsigned char Read_KBD_PORT(void)
{
    unsigned char x1 = 0;
    
    // Read the 4 input bits from the keypad and pack them into x1
    if(GPIO_PinRead(GPIO3, 22)) x1 |= 0x8; // Bit 3
    if(GPIO_PinRead(GPIO3, 17)) x1 |= 0x4; // Bit 2
    if(GPIO_PinRead(GPIO3, 1))  x1 |= 0x2; // Bit 1
    if(GPIO_PinRead(GPIO2, 22)) x1 |= 0x1; // Bit 0

    return x1;
}
