/**
 * @file  keypad_lib.h
 * @brief 4-key keypad driver for MCXA266 (aligned with LCD_KBD reference)
 */

#ifndef KEYPAD_LIB_H
#define KEYPAD_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_ENTER  0x01
#define KEY_MU     0x01
#define KEY_DN     0x02
#define KEY_UP     0x04
#define KEY_ESC    0x08

extern unsigned char KBD_DATA;
extern unsigned char TEMP_KBD_DATA;

unsigned char Read_KBD_PORT(void);
void ServiceKBD4X1(void);

#ifdef __cplusplus
}
#endif

#endif /* KEYPAD_LIB_H */
