/**
 * @file  lcd.h
 * @brief HD44780-compatible 16x2 LCD driver for MCXA266
 *        8-bit parallel interface, ISR-driven 1 kHz refresh.
 *
 * Usage
 * -----
 *  1. Call LCD_Init() once during startup (uses blocking delays internally).
 *  2. Wire LCD_Service1kHz() into vApplicationTickHook() or a 1 kHz timer ISR.
 *  3. Write to the shadow buffer with LCD_SetChar() / LCD_Print() / LCD_Clear().
 *     Changes appear on the next refresh cycle – no blocking, no bus contention.
 *
 * Blink
 * -----
 *  LCD_SetBlink(position) makes a character position flash at ~2 Hz.
 *  The visible/blank period is determined by bit 8 of the internal tick counter,
 *  giving a 256 ms on / 256 ms off cadence at 1 kHz.
 */

#ifndef LCD_H
#define LCD_H

#include <stdint.h>

/* -----------------------------------------------------------------------
 * Display geometry
 * --------------------------------------------------------------------- */
#define LCD_COLUMNS         16u     /**< Characters per row                      */
#define LCD_ROWS            2u      /**< Number of rows on the display           */
#define LCD_TOTAL_CHARS     (LCD_COLUMNS * LCD_ROWS) /**< Total character slots (32) */

/* Convenience position helpers — use these instead of magic numbers */
#define LCD_ROW0_START      0u                  /**< First position of row 0     */
#define LCD_ROW1_START      LCD_COLUMNS         /**< First position of row 1 (16)*/
#define LCD_LAST_COL_ROW0   (LCD_COLUMNS - 1u)  /**< Last column of row 0  (15)  */
#define LCD_LAST_COL_ROW1   (LCD_TOTAL_CHARS - 1u) /**< Last column of row 1 (31) */

/* -----------------------------------------------------------------------
 * Initialisation
 * --------------------------------------------------------------------- */

/**
 * @brief Initialise the LCD hardware and clear the shadow buffer.
 *
 * Follows the HD44780 power-on reset sequence with blocking delays.
 * Call before starting the FreeRTOS scheduler.
 */
void LCD_Init(void);

/* -----------------------------------------------------------------------
 * Buffer API
 * All writes go to a shadow buffer; the 1 kHz ISR pushes data to hardware.
 * --------------------------------------------------------------------- */

/** @brief Fill every position in the shadow buffer with spaces (blank). */
void LCD_Clear(void);

/**
 * @brief Write a single character into the shadow buffer.
 *
 * @param character  The ASCII character to write.
 * @param position   Linear position 0–31.  Row 0 is 0–15, Row 1 is 16–31.
 *                   Out-of-range values are silently ignored.
 */
void LCD_SetChar(char character, uint8_t position);

/**
 * @brief Copy a string into the shadow buffer starting at @p position.
 *
 * Characters that would fall beyond LCD_TOTAL_CHARS are silently dropped.
 *
 * @param text       Source string (need not be NUL-terminated if @p length is exact).
 * @param length     Number of characters to copy from @p text.
 * @param position   Starting linear position (0 = row 0 col 0, 16 = row 1 col 0).
 */
void LCD_Print(const char *text, uint8_t length, uint8_t position);

/**
 * @brief Convenience wrapper – print a NUL-terminated string.
 *
 * Equivalent to LCD_Print(text, strlen(text), position).
 */
void LCD_PrintStr(const char *text, uint8_t position);

/* -----------------------------------------------------------------------
 * Blink API  (32-bit bitmask, one bit per character position)
 * --------------------------------------------------------------------- */

/** @brief Enable blinking at the given character position (0–31). */
void    LCD_SetBlink(uint8_t position);

/** @brief Disable blinking at the given character position (0–31). */
void    LCD_ClearBlink(uint8_t position);

/** @brief Return 1 if the given position is set to blink, 0 otherwise. */
uint8_t LCD_GetBlink(uint8_t position);

/** @brief Disable blinking on all 32 character positions. */
void    LCD_ClearAllBlink(void);

/* -----------------------------------------------------------------------
 * 1 kHz service routine
 * Call from vApplicationTickHook() or a 1 kHz hardware timer ISR.
 * --------------------------------------------------------------------- */

/**
 * @brief Advance the LCD refresh state machine by one step.
 *
 * Each call sends one byte (command or data) to the LCD bus.
 * A complete 32-character refresh takes 34 calls:
 *   1 set-address for row 0  +  16 data writes  +
 *   1 set-address for row 1  +  16 data writes
 *
 * At 1 kHz this gives ~29 full refreshes per second.
 */
void LCD_Service1kHz(void);

#endif /* LCD_H */
