/**
 * @file  lcd.h
 * @brief HD44780-compatible 16x2 LCD driver for MCXA266
 *        8-bit parallel interface, ISR-driven 1 kHz refresh.
 *
 * Usage
 * -----
 *  1. Call LCD_Init() once from a task (uses vTaskDelay internally).
 *  2. Wire LCD_Service1kHz() into vApplicationTickHook() or a 1 kHz timer ISR.
 *  3. Write to the shadow buffer with LCD_SetChar() / LCD_Print() / LCD_Clear().
 *     Changes appear on the next refresh cycle – no blocking, no bus contention.
 *
 * Blink
 * -----
 *  LCD_SetBlink(pos) makes a character position flash at ~2 Hz.
 *  The visible/blank period is determined by bit 8 of the internal tick counter,
 *  giving a 256 ms on / 256 ms off cadence at 1 kHz.
 */

#ifndef LCD_H
#define LCD_H

#include <stdint.h>

/* -----------------------------------------------------------------------
 * Display geometry
 * --------------------------------------------------------------------- */
#define LCD_COLS   16u          /**< Characters per row                  */
#define LCD_ROWS   2u           /**< Number of rows                      */
#define LCD_SIZE   (LCD_COLS * LCD_ROWS)  /**< Total character positions */

/* -----------------------------------------------------------------------
 * Initialisation
 * --------------------------------------------------------------------- */

/**
 * @brief Initialise the LCD hardware and clear the shadow buffer.
 *
 * Follows the HD44780 power-on reset sequence with FreeRTOS delays.
 * Must be called from a task context (not from main() before the
 * scheduler starts).
 */
void LCD_Init(void);

/* -----------------------------------------------------------------------
 * Buffer API
 * All writes go to a shadow buffer; the ISR pushes to hardware.
 * --------------------------------------------------------------------- */

/** @brief Fill every position in the shadow buffer with a space. */
void LCD_Clear(void);

/**
 * @brief Write a single character into the shadow buffer.
 *
 * @param ch   Character to write.
 * @param pos  Linear position 0–(LCD_SIZE-1).  Out-of-range values are
 *             silently ignored.
 */
void LCD_SetChar(char ch, uint8_t pos);

/**
 * @brief Copy a string into the shadow buffer starting at @p pos.
 *
 * Characters that would fall beyond LCD_SIZE are silently dropped.
 *
 * @param s    Source string (need not be NUL-terminated if @p len is exact).
 * @param len  Number of characters to copy.
 * @param pos  Starting linear position (0 = row 0 col 0, 16 = row 1 col 0).
 */
void LCD_Print(const char *s, uint8_t len, uint8_t pos);

/**
 * @brief Convenience wrapper – print a NUL-terminated string.
 *
 * Equivalent to LCD_Print(s, strlen(s), pos).
 */
void LCD_PrintStr(const char *s, uint8_t pos);

/* -----------------------------------------------------------------------
 * Blink API  (32-bit bitmask, one bit per character position)
 * --------------------------------------------------------------------- */

/** @brief Enable blinking at character position @p pos. */
void    LCD_SetBlink(uint8_t pos);

/** @brief Disable blinking at character position @p pos. */
void    LCD_ClearBlink(uint8_t pos);

/** @brief Return non-zero if position @p pos is set to blink. */
uint8_t LCD_GetBlink(uint8_t pos);

/** @brief Disable blinking on all character positions. */
void    LCD_ClearAllBlink(void);

/* -----------------------------------------------------------------------
 * 1 kHz service routine
 * Call from vApplicationTickHook() or a 1 kHz hardware timer ISR.
 * --------------------------------------------------------------------- */

/**
 * @brief Advance the LCD state machine and write one byte to the bus.
 *
 * Each call drives the display one step forward.  A complete 32-character
 * refresh takes 34 calls (1 DDRAM address for row 0, 16 data writes,
 * 1 DDRAM address for row 1, 16 data writes).  At 1 kHz the display is
 * fully refreshed ~29 times per second.
 */
void LCD_Service1kHz(void);

#endif /* LCD_H */
