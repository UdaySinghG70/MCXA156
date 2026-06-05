/**
 * @file  keypad_lib.c
 * @brief 4-key keypad driver for MCXA266 (aligned with LCD_KBD reference)
 */

#include "keypad_lib.h"
#include "fsl_gpio.h"

unsigned char KBD_DATA = 0;
unsigned char KBD_TEMP = 0;
unsigned char TEMP_KBD_DATA = 0;

unsigned char Read_KBD_PORT(void)
{
    unsigned char x1 = 0;

    /* KEY4 (ESCAPE) -> GPIO3, pin 22 (physical ESC) */
    if (GPIO_PinRead(GPIO3, 22u)) x1 |= 0x08;

    /* KEY3 (UP) -> GPIO3, pin 1 */
    if (GPIO_PinRead(GPIO3, 1u))  x1 |= 0x04;

    /* KEY2 (DOWN) -> GPIO3, pin 17 */
    if (GPIO_PinRead(GPIO3, 17u)) x1 |= 0x02;

    /* KEY1 (ENTER) -> GPIO3, pin 30 (physical ENTER / MENU) */
    if (GPIO_PinRead(GPIO3, 30u)) x1 |= 0x01;

    return x1;
}

void ServiceKBD4X1(void)
{
    static unsigned char kbd_sm = 0;
    unsigned char x1;

    switch (kbd_sm)
    {
        case 0:
            KBD_TEMP |= Read_KBD_PORT();
            kbd_sm++;
            break;

        case 1:
            x1 = Read_KBD_PORT();
            if (KBD_TEMP & 0x0F)
            {
                if ((KBD_TEMP & 0x08) && ((x1 & 0x08) == 0x0)) { KBD_DATA |= 0x08; KBD_TEMP &= 0xF7; }
                if ((KBD_TEMP & 0x04) && ((x1 & 0x04) == 0x0)) { KBD_DATA |= 0x04; KBD_TEMP &= 0xFB; }
                if ((KBD_TEMP & 0x02) && ((x1 & 0x02) == 0x0)) { KBD_DATA |= 0x02; KBD_TEMP &= 0xFD; }
                if ((KBD_TEMP & 0x01) && ((x1 & 0x01) == 0x0)) { KBD_DATA |= 0x01; KBD_TEMP &= 0xFE; }
            }
            kbd_sm = 0;
            break;
    }
}
