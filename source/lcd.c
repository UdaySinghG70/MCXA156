/**
 * @file  lcd.c
 * @brief HD44780-compatible 16x2 LCD driver for MCXA266 (bare-metal safe)
 *
 * This driver uses a shadow buffer (display_buffer[]) that the application
 * writes into.  A 1 kHz state machine (LCD_Service1kHz) pushes one byte
 * per call to the physical LCD over an 8-bit parallel bus.
 *
 * Blink is handled by a 32-bit mask (blink_mask): any character position
 * whose bit is set alternates between its buffer content and a blank space,
 * toggling every 256 ms.
 */

#include "lcd.h"

#include <string.h>
#include <stddef.h>

#include "fsl_gpio.h"
#include "fsl_common.h"

/* =========================================================================
 * COMPILE-TIME CHECK
 * ======================================================================= */
_Static_assert(LCD_TOTAL_CHARS == 32u,
               "LCD_TOTAL_CHARS must be 32 (LCD_COLUMNS * LCD_ROWS)");

/* =========================================================================
 * SHADOW BUFFER & BLINK STATE
 *
 * display_buffer[] — 32 bytes, one per character cell.  Written by the
 *                    application API, read by LCD_Service1kHz().
 * blink_mask       — bit N = 1 means position N blinks.
 * ======================================================================= */

static volatile uint8_t  display_buffer[LCD_TOTAL_CHARS];   /* shadow buffer  */
static volatile uint32_t blink_mask;                        /* per-char blink */

/* =========================================================================
 * REFRESH STATE MACHINE VARIABLES
 *
 * refresh_phase     — which phase of the 4-step cycle we're in.
 * current_char_index — which character position we're writing next.
 * millisecond_counter — free-running tick count; bit 8 controls blink.
 * ======================================================================= */

static uint8_t  refresh_phase;
static uint8_t  current_char_index;
static uint16_t millisecond_counter;

/* =========================================================================
 * HD44780 COMMAND BYTES
 *
 * The HD44780 uses "Set DDRAM Address" commands to position the cursor.
 * Row 0 starts at address 0x00 (command 0x80), row 1 at 0x40 (command 0xC0).
 * ======================================================================= */
#define HD44780_SET_ROW0_ADDRESS   0x80u   /* Set DDRAM address → row 0, col 0 */
#define HD44780_SET_ROW1_ADDRESS   0xC0u   /* Set DDRAM address → row 1, col 0 */

#define HD44780_CMD_FUNCTION_SET   0x38u   /* 8-bit bus, 2-line, 5×8 font      */
#define HD44780_CMD_DISPLAY_OFF    0x08u   /* Display OFF, cursor OFF           */
#define HD44780_CMD_CLEAR_DISPLAY  0x01u   /* Clear display & home cursor       */
#define HD44780_CMD_ENTRY_MODE     0x06u   /* Increment cursor, no shift        */
#define HD44780_CMD_DISPLAY_ON     0x0Cu   /* Display ON, cursor OFF, blink OFF */
#define HD44780_CMD_RESET_BUS      0x30u   /* Force 8-bit mode during init      */

/* =========================================================================
 * REFRESH STATE MACHINE PHASES
 *
 * The state machine cycles through 4 phases to refresh the full display:
 *   PHASE_SET_ROW0_ADDRESS → PHASE_WRITE_ROW0_DATA →
 *   PHASE_SET_ROW1_ADDRESS → PHASE_WRITE_ROW1_DATA → (repeat)
 * ======================================================================= */
#define PHASE_SET_ROW0_ADDRESS   0u   /* Send "set DDRAM address" for row 0 */
#define PHASE_WRITE_ROW0_DATA    1u   /* Write characters 0–15 to row 0     */
#define PHASE_SET_ROW1_ADDRESS   2u   /* Send "set DDRAM address" for row 1 */
#define PHASE_WRITE_ROW1_DATA    3u   /* Write characters 16–31 to row 1    */

/* =========================================================================
 * RS (Register Select) VALUES — passed to lcd_send_byte()
 *
 * RS = 0 → the byte is a command  (e.g. set address, clear display)
 * RS = 1 → the byte is data       (e.g. an ASCII character)
 * ======================================================================= */
#define RS_COMMAND  0u
#define RS_DATA     1u

/* =========================================================================
 * GPIO PIN HELPERS
 *
 * Each function writes a single GPIO pin.  The pin names match the LCD
 * signal they drive.  Hardware wiring:
 *
 *   LCD Signal   MCU Port   MCU Pin
 *   ----------   --------   -------
 *   RS           GPIO4      7
 *   E  (Enable)  GPIO4      6
 *   D0           GPIO2      1
 *   D1           GPIO2      20
 *   D2           GPIO2      21
 *   D3           GPIO4      3
 *   D4           GPIO3      12
 *   D5           GPIO3      13
 *   D6           GPIO3      15
 *   D7           GPIO3      16
 * ======================================================================= */
static inline void LCD_Pin_RS(uint8_t value) { GPIO_PinWrite(GPIO4, 7u, value); }
static inline void LCD_Pin_E (uint8_t value) { GPIO_PinWrite(GPIO4, 6u, value); }

static inline void LCD_Pin_D0(uint8_t value) { GPIO_PinWrite(GPIO2, 1u,  value); }
static inline void LCD_Pin_D1(uint8_t value) { GPIO_PinWrite(GPIO2, 20u, value); }
static inline void LCD_Pin_D2(uint8_t value) { GPIO_PinWrite(GPIO2, 21u, value); }
static inline void LCD_Pin_D3(uint8_t value) { GPIO_PinWrite(GPIO4, 3u,  value); }
static inline void LCD_Pin_D4(uint8_t value) { GPIO_PinWrite(GPIO3, 12u, value); }
static inline void LCD_Pin_D5(uint8_t value) { GPIO_PinWrite(GPIO3, 13u, value); }
static inline void LCD_Pin_D6(uint8_t value) { GPIO_PinWrite(GPIO3, 15u, value); }
static inline void LCD_Pin_D7(uint8_t value) { GPIO_PinWrite(GPIO3, 16u, value); }

/* =========================================================================
 * TIMING DELAYS (NOP loops — no RTOS dependency)
 *
 * At 180 MHz, one NOP ≈ 5.5 ns.
 *   delay_before_enable:  ~220 ns  — data-setup time before E rises
 *   delay_enable_pulse:   ~275 ns  — E high time (HD44780 min ≈ 450 ns)
 *   delay_after_enable:   ~500 ns  — hold time after E falls
 * ======================================================================= */

static void delay_before_enable(void)
{
    for (volatile uint32_t i = 0u; i < 40u; i++) { __NOP(); }
}

static void delay_enable_pulse(void)
{
    for (volatile uint32_t i = 0u; i < 50u; i++) { __NOP(); }
}

static void delay_after_enable(void)
{
    for (volatile uint32_t i = 0u; i < 90u; i++) { __NOP(); }
}

/* =========================================================================
 * LOW-LEVEL BUS — write 8 data bits to D0–D7
 * ======================================================================= */

static void lcd_write_data_pins(uint8_t byte)
{
    LCD_Pin_D0( byte       & 1u);
    LCD_Pin_D1((byte >> 1) & 1u);
    LCD_Pin_D2((byte >> 2) & 1u);
    LCD_Pin_D3((byte >> 3) & 1u);
    LCD_Pin_D4((byte >> 4) & 1u);
    LCD_Pin_D5((byte >> 5) & 1u);
    LCD_Pin_D6((byte >> 6) & 1u);
    LCD_Pin_D7((byte >> 7) & 1u);
}

/**
 * @brief Send one byte to the LCD with the specified RS level.
 *
 * @param byte          The 8-bit value (command or ASCII character).
 * @param register_sel  RS_COMMAND (0) for commands, RS_DATA (1) for characters.
 */
static void lcd_send_byte(uint8_t byte, uint8_t register_sel)
{
    LCD_Pin_RS(register_sel);
    lcd_write_data_pins(byte);

    delay_before_enable();
    LCD_Pin_E(1u);              /* rising edge latches data on the bus */
    delay_enable_pulse();
    LCD_Pin_E(0u);              /* falling edge completes the transfer */

    delay_before_enable();
    delay_after_enable();
}

/* =========================================================================
 * BLINK API
 * ======================================================================= */

void LCD_SetBlink(uint8_t position)
{
    if (position < LCD_TOTAL_CHARS)
        blink_mask |= (1UL << position);
}

void LCD_ClearBlink(uint8_t position)
{
    if (position < LCD_TOTAL_CHARS)
        blink_mask &= ~(1UL << position);
}

uint8_t LCD_GetBlink(uint8_t position)
{
    if (position >= LCD_TOTAL_CHARS) return 0u;
    return (blink_mask >> position) & 1u;
}

void LCD_ClearAllBlink(void)
{
    blink_mask = 0u;
}

/* =========================================================================
 * LCD INIT — HD44780 power-on reset sequence (blocking delays)
 * ======================================================================= */

void LCD_Init(void)
{
    /* --- Clear shadow buffer and reset state machine ------------------- */
    for (uint8_t i = 0u; i < LCD_TOTAL_CHARS; i++)
        display_buffer[i] = (uint8_t)' ';

    blink_mask          = 0u;
    refresh_phase       = PHASE_SET_ROW0_ADDRESS;
    current_char_index  = 0u;
    millisecond_counter = 0u;

    /* --- 50 ms power-up delay (HD44780 requirement) -------------------- */
    SDK_DelayAtLeastUs(50000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    LCD_Pin_RS(0u);
    LCD_Pin_E(0u);

    /* --- Reset sequence: send 0x30 three times with 15 ms gaps --------- */
    lcd_write_data_pins(HD44780_CMD_RESET_BUS);

    for (uint8_t attempt = 0u; attempt < 3u; attempt++)
    {
        delay_before_enable();
        LCD_Pin_E(1u);
        delay_enable_pulse();
        LCD_Pin_E(0u);

        SDK_DelayAtLeastUs(15000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));
    }

    /* --- Function Set: 8-bit, 2-line, 5×8 font ------------------------- */
    lcd_send_byte(HD44780_CMD_FUNCTION_SET, RS_COMMAND);
    SDK_DelayAtLeastUs(1000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    /* --- Display OFF --------------------------------------------------- */
    lcd_send_byte(HD44780_CMD_DISPLAY_OFF, RS_COMMAND);
    SDK_DelayAtLeastUs(1000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    /* --- Clear Display ------------------------------------------------- */
    lcd_send_byte(HD44780_CMD_CLEAR_DISPLAY, RS_COMMAND);
    SDK_DelayAtLeastUs(2000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    /* --- Entry Mode: cursor increments, no display shift ---------------- */
    lcd_send_byte(HD44780_CMD_ENTRY_MODE, RS_COMMAND);
    SDK_DelayAtLeastUs(1000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    /* --- Display ON, cursor OFF, blink OFF ----------------------------- */
    lcd_send_byte(HD44780_CMD_DISPLAY_ON, RS_COMMAND);
}

/* =========================================================================
 * BUFFER API — write into the shadow buffer (never blocks)
 * ======================================================================= */

void LCD_Clear(void)
{
    for (uint8_t i = 0u; i < LCD_TOTAL_CHARS; i++)
        display_buffer[i] = (uint8_t)' ';
}

void LCD_SetChar(char character, uint8_t position)
{
    if (position < LCD_TOTAL_CHARS)
        display_buffer[position] = (uint8_t)character;
}

void LCD_Print(const char *text, uint8_t length, uint8_t position)
{
    if (!text || position >= LCD_TOTAL_CHARS) return;

    uint8_t end_position = position + length;
    if (end_position > LCD_TOTAL_CHARS) end_position = LCD_TOTAL_CHARS;

    for (uint8_t i = position; i < end_position; i++)
        display_buffer[i] = (uint8_t)text[i - position];
}

void LCD_PrintStr(const char *text, uint8_t position)
{
    if (!text) return;
    LCD_Print(text, (uint8_t)strlen(text), position);
}

/* =========================================================================
 * 1 kHz SERVICE ROUTINE — call exactly once per millisecond
 *
 * State machine cycle (34 calls = 1 full refresh):
 *
 *   Call  0:       Send "Set DDRAM address row 0"  (command 0x80)
 *   Calls 1–16:   Write display_buffer[0]–[15]    (row 0 characters)
 *   Call  17:      Send "Set DDRAM address row 1"  (command 0xC0)
 *   Calls 18–33:  Write display_buffer[16]–[31]   (row 1 characters)
 *   → repeat
 *
 * Blink: millisecond_counter bit 8 toggles every 256 calls (256 ms).
 *        When this bit is 1, blinking positions show a blank space.
 * ======================================================================= */

void LCD_Service1kHz(void)
{
    millisecond_counter++;

    /* Blink toggle: bit 8 of the counter flips every 256 ms.
     * When is_blink_blank_phase == 1, blinking characters show as spaces. */
    const uint8_t is_blink_blank_phase = (millisecond_counter & 0x100u) ? 1u : 0u;

    switch (refresh_phase)
    {
        /* ---- Step 1: Set the cursor to the start of row 0 ------------- */
        case PHASE_SET_ROW0_ADDRESS:
            lcd_send_byte(HD44780_SET_ROW0_ADDRESS, RS_COMMAND);
            current_char_index = 0u;
            refresh_phase = PHASE_WRITE_ROW0_DATA;
            break;

        /* ---- Step 2: Write row 0 characters one at a time ------------- */
        case PHASE_WRITE_ROW0_DATA:
        {
            uint8_t character = display_buffer[current_char_index];

            /* If this position is blinking and we're in the blank phase,
             * replace the character with a space. */
            if (LCD_GetBlink(current_char_index) && is_blink_blank_phase)
                character = ' ';

            lcd_send_byte(character, RS_DATA);

            if (++current_char_index >= LCD_COLUMNS)
                refresh_phase = PHASE_SET_ROW1_ADDRESS;

            break;
        }

        /* ---- Step 3: Set the cursor to the start of row 1 ------------- */
        case PHASE_SET_ROW1_ADDRESS:
            lcd_send_byte(HD44780_SET_ROW1_ADDRESS, RS_COMMAND);
            current_char_index = LCD_COLUMNS;   /* row 1 starts at index 16 */
            refresh_phase = PHASE_WRITE_ROW1_DATA;
            break;

        /* ---- Step 4: Write row 1 characters one at a time ------------- */
        case PHASE_WRITE_ROW1_DATA:
        {
            uint8_t character = display_buffer[current_char_index];

            if (LCD_GetBlink(current_char_index) && is_blink_blank_phase)
                character = ' ';

            lcd_send_byte(character, RS_DATA);

            if (++current_char_index >= LCD_TOTAL_CHARS)
                refresh_phase = PHASE_SET_ROW0_ADDRESS;   /* full cycle done */

            break;
        }

        /* ---- Safety fallback ------------------------------------------ */
        default:
            refresh_phase = PHASE_SET_ROW0_ADDRESS;
            break;
    }
}
