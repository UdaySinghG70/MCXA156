#include "mcc.h"
#include <LCD_KBD.h>
#include "menu.h"

unsigned char KBD_DATA = 0;
unsigned char KBD_TEMP = 0;


unsigned char display_buffer[] = {'U','N','I','V','E','R','S','A','L',' ','I','N','S','T','R','.',
                                  'R','D','S',' ','C','O','N','T','R','O','L','L','E','R','-','1'};


unsigned long BlinkFlags = 0x00000000;
unsigned char BlinkData[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};//8bit * 10 = 80 bits for 80 chars 
unsigned char TEMP_KBD_DATA = 0;
//unsigned char lcd_sm = 0;
//unsigned short blink_counter = 0;

unsigned char Read_KBD_PORT(void)
{
    unsigned char x1;
    x1 = 0;
    if(PORTAbits.RA0)x1 |= 0x8;
    if(PORTAbits.RA1)x1 |= 0x4;
    if(PORTAbits.RA2)x1 |= 0x2;
    if(PORTAbits.RA3)x1 |= 0x1;

    return x1;

}
/*
void SetBlinkLocation(unsigned char loc)
{
    unsigned char i,j;
    if(loc > 79)return;
    i = 0;j = 1;
    while(loc > 7)
    {
        loc = loc - 8;
        i++;
    }
    BlinkData[i] |= (j << loc);
 }

void ClearBlinkLocation(unsigned char loc)
{
    unsigned char i,j;
    if(loc > 79)return;
    i = 0;j = 1;
    while(loc > 7)
    {
        loc = loc - 8;
        i++;
    }
    BlinkData[i] = BlinkData[i] ^ (j << loc);
 }

unsigned char GetBlinkLocation(unsigned char loc)
{
    unsigned char i,j;
    if(loc > 79)return 0;
    i = 0;j = 1;
    while(loc > 7)
    {
        loc = loc - 8;
        i++;
    }
    if(BlinkData[i] & (j << loc))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
*/

unsigned char GetBlinkLocation(unsigned char loc)
{
    unsigned long bf;
    unsigned char rt;
	bf = 1 ;
	bf = bf << loc;
    if(BlinkFlags & bf)rt = 1;
    else rt = 0;
}

void SetBlinkFlag(unsigned char BlinkLocation)
{
	unsigned long bf;

	bf = 1 ;
	bf = bf << BlinkLocation;
	BlinkFlags = BlinkFlags | bf;
}



void ClearBlinkFlags(unsigned char BlinkLocation)
{
	unsigned long bf;

	bf = 1 ;
	bf = bf << BlinkLocation;
	bf = ~bf;
	BlinkFlags = BlinkFlags & bf;
}
void SetBlinkLocation(unsigned char loc)
{
    unsigned long bf;

	bf = 1 ;
	bf = bf << loc;
	BlinkFlags = BlinkFlags | bf;
}

void ClearAllBlinkFlags(void)
{
	BlinkFlags = 0;
}
void ServiceKBD4X1(void)
{
	static unsigned char kbd_sm = 0;
	unsigned char x1;

	switch(kbd_sm)
	{
		case 0:
			KBD_TEMP |= Read_KBD_PORT();
			kbd_sm++;
			//if(KBD_TEMP & 0xC0){KBD_COUNTER++;if(KBD_COUNTER > 10){KBD_DATA |= (KBD_TEMP & 0xC0);KBD_COUNTER -= 1;KBD_TEMP &= 0x3F;}}
			//else{KBD_COUNTER = 0;}
			break;
		case 1:
			x1 = Read_KBD_PORT();
			if(KBD_TEMP & 0x0F)
			{
				if((KBD_TEMP & 0x08) && ((x1 & 0x08) == 0x0)){KBD_DATA |= 0x08;KBD_TEMP &= 0xF7;}
				if((KBD_TEMP & 0x04) && ((x1 & 0x04) == 0x0)){KBD_DATA |= 0x04;KBD_TEMP &= 0xFB;}
				if((KBD_TEMP & 0x02) && ((x1 & 0x02) == 0x0)){KBD_DATA |= 0x02;KBD_TEMP &= 0xFD;}
				if((KBD_TEMP & 0x01) && ((x1 & 0x01) == 0x0)){KBD_DATA |= 0x01;KBD_TEMP &= 0xFE;}
			}
			kbd_sm = 0;
			break;
	}
}

void WriteLCDPort(unsigned char data)
{
	LCD_DATA = data;
}

void send_lcd_cmd1(unsigned char cmd_char)
{

		WriteLCDPort(cmd_char);
		LCD_RS = 0;
		__delay_us(100);
		LCD_EN1 = 1;
		__delay_us(100);
		LCD_EN1 = 0;
}

void send_lcd_cmd2(unsigned char cmd_char)
{

		WriteLCDPort(cmd_char);
		LCD_RS = 0;
		__delay_us(100);
		LCD_EN2 = 1;
		__delay_us(100);
		LCD_EN2 = 0;
}
void InitLCD(void)
{
    unsigned char i;
    __delay_ms(50);__delay_ms(50);__delay_ms(50);__delay_ms(50);__delay_ms(50);__delay_ms(50);__delay_ms(50);__delay_ms(50);__delay_ms(50);
    LCD_RS = 0;LCD_EN1 = 0;/*LCD_EN2 = 0;*/
    WriteLCDPort(0x30);
	LCD_RS = 0;
	__delay_ms(20);
	LCD_EN1 = 1;/*LCD_EN2 = 1;*/__delay_ms(5);LCD_EN1 = 0;/*LCD_EN2 = 0;*/ //pulse on enable
	__delay_ms(15);
	LCD_EN1 = 1;/*LCD_EN2 = 1;*/__delay_ms(5);LCD_EN1 = 0;/*LCD_EN2 = 0;*/ //pulse on enable
	__delay_ms(15);
    LCD_EN1 = 1;/*LCD_EN2 = 1;*/__delay_ms(5);LCD_EN1 = 0;/*LCD_EN2 = 0;*/ //pulse on enable
	__delay_ms(15);
    WriteLCDPort(0x38);__delay_ms(1);LCD_EN1 = 1;/*LCD_EN2 = 1;*/__delay_ms(5);LCD_EN1 = 0;/*LCD_EN2 = 0;*/__delay_ms(5);
	WriteLCDPort(0x08);__delay_ms(1);LCD_EN1 = 1;/*LCD_EN2 = 1;*/__delay_ms(5);LCD_EN1 = 0;/*LCD_EN2 = 0;*/__delay_ms(5);
	WriteLCDPort(0x01);__delay_ms(1);LCD_EN1 = 1;/*LCD_EN2 = 1;*/__delay_ms(5);LCD_EN1 = 0;/*LCD_EN2 = 0;*/__delay_ms(5);
	WriteLCDPort(0x06);__delay_ms(1);LCD_EN1 = 1;/*LCD_EN2 = 1;*/__delay_ms(5);LCD_EN1 = 0;/*LCD_EN2 = 0;*/__delay_ms(5);
	WriteLCDPort(0x80);__delay_ms(1);LCD_EN1 = 1;/*LCD_EN2 = 1;*/__delay_ms(5);LCD_EN1 = 0;/*LCD_EN2 = 0;*/__delay_ms(5);
    WriteLCDPort(0x0C);__delay_ms(1);LCD_EN1 = 1;/*LCD_EN2 = 1;*/__delay_ms(5);LCD_EN1 = 0;/*LCD_EN2 = 0;*/__delay_ms(5);
	LCD_RS = 1; 
    LCD_BL = 1;
    
    for(i=0;i<16;i++)display_buffer[i] = LcdLine1_Buff[i];
    for(i=16;i<32;i++)display_buffer[i] = LcdLine2_Buff[i-16];
}



void WriteLCD(void)
{
	static unsigned char lcd_sm = 0;
	static unsigned short blinkcounter = 0;
	static unsigned char lcd_status = 0;
	static unsigned char x1;
	static unsigned char y1;
	unsigned long z1;
	unsigned char i;
	blinkcounter++;

	switch(lcd_sm)
	{
        case 0: //switch to line1
                switch(lcd_status)
                {
                        case 0:
                            WriteLCDPort(Line1);
                            LCD_RS = 0;//LCDRS = 0;
                            lcd_status++;
                            break;
                        case 1:
                            LCD_EN1 = 1;//LCDEN = 1;
                            lcd_status++;
                            break;
                        case 2:
                            LCD_EN1 = 0;//LCDEN = 0;
                            LCD_RS = 1;//LCDRS = 1;
                            lcd_status = 0;
                            lcd_sm++;
                            break;
                }
                break;
        case 17:
                switch(lcd_status)//switch 2 line2
                {
                    case 0:
                            WriteLCDPort(Line2);
                            LCD_RS = 0;//LCDRS = 0;
                            lcd_status++;
                            break;
                    case 1:
                            LCD_EN1 = 1;//LCDEN = 1;
                            lcd_status++;
                            break;
                    case 2:
                            LCD_EN1 = 0;//LCDEN = 0;
                            LCD_RS = 1;//LCDRS = 1;
                            lcd_status = 0;
                            lcd_sm++;
                            break;
                }
                break;
        default:
                 switch(lcd_status)
                 {
                    case 0:
                            if(lcd_sm > 16){y1 = lcd_sm - 2;}else{y1 = lcd_sm - 1;}
                            x1 = display_buffer[y1];
                            if((GetBlinkLocation(y1)) && (blinkcounter & 0x200))x1 = ' ';
                            WriteLCDPort(x1);
                            LCD_RS = 1;//LCDRS = 1;
                            lcd_status++;
                            break;
                    case 1:
                            LCD_EN1 = 1;//LCDEN = 1;
                            lcd_status++;
                            break;
                    case 2:
                            LCD_EN1 = 0;//LCDEN = 1;
                            lcd_status = 0;
                            lcd_sm++;
                            if(lcd_sm > 33)lcd_sm = 0;
                            break;
                    }
                    break;
	}
}



void ClearLCD(void)
{
	unsigned char i;
	for(i=0;i<80;i++)display_buffer[i] = ' ';
}

void PrintLCD(const char * buffer,unsigned char no_of_characters, unsigned char location)
{
	unsigned char i,j;
	j = no_of_characters + location;
	if(j > 32)j=32;
	for(i=location;i<j;i++)display_buffer[i] = *buffer++;
}

/*
void SetBlinkFlag(unsigned char loc)
{
	unsigned char i,j;
    i = 0;j = 1;
    while(loc > 7)
    {
        loc = loc - 8;
        i++;
    }
    BlinkData[i] |= (j << loc);
}



void ClearBlinkFlags(unsigned char loc)
{
	unsigned char i,j;
    i = 0;j = 1;
    while(loc > 7)
    {
        loc = loc - 8;
        i++;
    }
    BlinkData[i] = BlinkData[i] ^ (j << loc);
}

void ClearAllBlinkFlags(void)
{
	unsigned char i;
    BlinkFlags = 0;
    for(i=0;i<10;i++)BlinkData[i] = 0x00;
}
*/
