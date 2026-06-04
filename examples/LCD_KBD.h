
// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef LCD_KBD_H
#define	LCD_KBD_H

#include <xc.h> // include processor files - each processor file is guarded.  

#define KBDPORT			PORTA
#define KBD_SCAN1       LATAbits.LATA5
#define KBD_SCAN2       LATAbits.LATA4



#define KEY_ENTER  0x01
#define KEY_MU     0x01
#define KEY_DN     0x02
#define KEY_UP     0x04
#define KEY_ESC    0x08

#define LCD_DATA    LATB
#define LCD_RS      LATDbits.LATD6
#define LCD_EN1     LATDbits.LATD7
#define LCD_EN2     LATDbits.LATD7
#define LCD_BL      LATDbits.LATD2

#define Line1			0x80
#define Line2			0xC0
//LCD COMMAND SET
#define func_set		0x38
#define disp_on			0x0C
#define clr_disp		0x01
#define entry_inc		0x06
#define dd_ram_addr		0x80
#define Line1			0x80
#define Line2			0xC0
#define Line3			0x94
#define Line4			0xD4

extern unsigned char display_buffer[];
extern unsigned long BlinkFlags;
extern unsigned char TEMP_KBD_DATA;

void InitLCD(void);
void WriteLCD(void);
void ClearLCD(void);
void PrintLCD(const char * buffer,unsigned char no_of_characters, unsigned char location);
void SetBlinkFlag(unsigned char BlinkLocation);
void SetBlinkLocation(unsigned char loc);
void ClearBlinkLocation(unsigned char loc);
unsigned char GetBlinkLocation(unsigned char loc);
void ClearBlinkFlags(unsigned char BlinkLocation);
void ClearAllBlinkFlags(void);
unsigned char Read_KBD_PORT(void);
void ServiceKBD4X1(void);
extern unsigned char KBD_DATA;
#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

