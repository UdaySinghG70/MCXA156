/**
 * @file  lcd.c
 * @brief HD44780-compatible 16x2 LCD driver for MCXA266 (bare-metal safe)
 */

#include "lcd.h"

#include <string.h>
#include <stddef.h>

#include "fsl_gpio.h"
#include "fsl_common.h"

/* =========================================================================
 * CONFIG
 * ======================================================================= */
_Static_assert(LCD_SIZE == 32u, "LCD_SIZE must equal LCD_COLS * LCD_ROWS");

/* =========================================================================
 * INTERNAL STATE
 * ======================================================================= */

static volatile uint8_t  s_buf[LCD_SIZE];
static volatile uint32_t s_blink;

static uint8_t  s_state;
static uint8_t  s_idx;
static uint16_t s_tick;

/* =========================================================================
 * HD44780 CONSTANTS
 * ======================================================================= */
#define HD_DDRAM_ROW0   0x80u
#define HD_DDRAM_ROW1   0xC0u

#define STATE_ADDR_ROW0 0u
#define STATE_DATA_ROW0 1u
#define STATE_ADDR_ROW1 2u
#define STATE_DATA_ROW1 3u

/* =========================================================================
 * GPIO HELPERS
 * ======================================================================= */
static inline void PIN_RS(uint8_t v) { GPIO_PinWrite(GPIO4, 7u, v); }
static inline void PIN_E (uint8_t v) { GPIO_PinWrite(GPIO4, 6u, v); }

static inline void PIN_D0(uint8_t v) { GPIO_PinWrite(GPIO2, 1u,  v); }
static inline void PIN_D1(uint8_t v) { GPIO_PinWrite(GPIO2, 20u, v); }
static inline void PIN_D2(uint8_t v) { GPIO_PinWrite(GPIO2, 21u, v); }
static inline void PIN_D3(uint8_t v) { GPIO_PinWrite(GPIO4, 3u,  v); }
static inline void PIN_D4(uint8_t v) { GPIO_PinWrite(GPIO3, 12u, v); }
static inline void PIN_D5(uint8_t v) { GPIO_PinWrite(GPIO3, 13u, v); }
static inline void PIN_D6(uint8_t v) { GPIO_PinWrite(GPIO3, 15u, v); }
static inline void PIN_D7(uint8_t v) { GPIO_PinWrite(GPIO3, 16u, v); }

/* =========================================================================
 * TIMING (NO RTOS DEPENDENCY)
 * ======================================================================= */

static void delay_setup(void)
{
    for (volatile uint32_t i = 0u; i < 40u; i++) { __NOP(); }
}

static void delay_enable(void)
{
    for (volatile uint32_t i = 0u; i < 50u; i++) { __NOP(); }
}

static void delay_cycle(void)
{
    for (volatile uint32_t i = 0u; i < 90u; i++) { __NOP(); }
}

/* =========================================================================
 * LOW-LEVEL BUS
 * ======================================================================= */

static void write_bus(uint8_t v)
{
    PIN_D0(v & 1u);
    PIN_D1((v >> 1) & 1u);
    PIN_D2((v >> 2) & 1u);
    PIN_D3((v >> 3) & 1u);
    PIN_D4((v >> 4) & 1u);
    PIN_D5((v >> 5) & 1u);
    PIN_D6((v >> 6) & 1u);
    PIN_D7((v >> 7) & 1u);
}

static void lcd_send(uint8_t v, uint8_t rs)
{
    PIN_RS(rs);
    write_bus(v);

    delay_setup();
    PIN_E(1u);
    delay_enable();
    PIN_E(0u);

    delay_setup();
    delay_cycle();
}

/* =========================================================================
 * BLINK API
 * ======================================================================= */

void LCD_SetBlink(uint8_t pos)
{
    if (pos < LCD_SIZE) s_blink |= (1UL << pos);
}

void LCD_ClearBlink(uint8_t pos)
{
    if (pos < LCD_SIZE) s_blink &= ~(1UL << pos);
}

uint8_t LCD_GetBlink(uint8_t pos)
{
    if (pos >= LCD_SIZE) return 0u;
    return (s_blink >> pos) & 1u;
}

void LCD_ClearAllBlink(void)
{
    s_blink = 0u;
}

/* =========================================================================
 * INIT (BARE-METAL SAFE VERSION)
 * ======================================================================= */

void LCD_Init(void)
{
    uint8_t i;

    for (i = 0u; i < LCD_SIZE; i++)
        s_buf[i] = (uint8_t)' ';

    s_blink = 0u;
    s_state = STATE_ADDR_ROW0;
    s_idx   = 0u;
    s_tick  = 0u;

    /* ---- POWER-UP DELAY (blocking, safe) ---- */
    SDK_DelayAtLeastUs(50000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    PIN_RS(0u);
    PIN_E(0u);

    /* ---- LCD RESET SEQUENCE (0x30 x3) ---- */
    write_bus(0x30u);

    for (i = 0u; i < 3u; i++)
    {
        delay_setup();
        PIN_E(1u);
        delay_enable();
        PIN_E(0u);

        SDK_DelayAtLeastUs(15000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));
    }

    /* ---- FUNCTION SET ---- */
    lcd_send(0x38u, 0u);
    SDK_DelayAtLeastUs(1000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    /* ---- DISPLAY OFF ---- */
    lcd_send(0x08u, 0u);
    SDK_DelayAtLeastUs(1000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    /* ---- CLEAR ---- */
    lcd_send(0x01u, 0u);
    SDK_DelayAtLeastUs(2000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    /* ---- ENTRY MODE ---- */
    lcd_send(0x06u, 0u);
    SDK_DelayAtLeastUs(1000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));

    /* ---- DISPLAY ON ---- */
    lcd_send(0x0Cu, 0u);
}

/* =========================================================================
 * BUFFER API
 * ======================================================================= */

void LCD_Clear(void)
{
    for (uint8_t i = 0u; i < LCD_SIZE; i++)
        s_buf[i] = (uint8_t)' ';
}

void LCD_SetChar(char ch, uint8_t pos)
{
    if (pos < LCD_SIZE)
        s_buf[pos] = (uint8_t)ch;
}

void LCD_Print(const char *s, uint8_t len, uint8_t pos)
{
    if (!s || pos >= LCD_SIZE) return;

    uint8_t end = pos + len;
    if (end > LCD_SIZE) end = LCD_SIZE;

    for (uint8_t i = pos; i < end; i++)
        s_buf[i] = (uint8_t)s[i - pos];
}

void LCD_PrintStr(const char *s, uint8_t pos)
{
    if (!s) return;
    LCD_Print(s, (uint8_t)strlen(s), pos);
}

/* =========================================================================
 * 1 kHz SERVICE ROUTINE
 * ======================================================================= */

void LCD_Service1kHz(void)
{
    s_tick++;

    const uint8_t blink_blank = (s_tick & 0x100u) ? 1u : 0u;

    switch (s_state)
    {
        case STATE_ADDR_ROW0:
            lcd_send(HD_DDRAM_ROW0, 0u);
            s_idx = 0u;
            s_state = STATE_DATA_ROW0;
            break;

        case STATE_DATA_ROW0:
        {
            uint8_t ch = s_buf[s_idx];
            if (LCD_GetBlink(s_idx) && blink_blank) ch = ' ';

            lcd_send(ch, 1u);

            if (++s_idx >= LCD_COLS)
                s_state = STATE_ADDR_ROW1;

            break;
        }

        case STATE_ADDR_ROW1:
            lcd_send(HD_DDRAM_ROW1, 0u);
            s_idx = LCD_COLS;
            s_state = STATE_DATA_ROW1;
            break;

        case STATE_DATA_ROW1:
        {
            uint8_t ch = s_buf[s_idx];
            if (LCD_GetBlink(s_idx) && blink_blank) ch = ' ';

            lcd_send(ch, 1u);

            if (++s_idx >= LCD_SIZE)
                s_state = STATE_ADDR_ROW0;

            break;
        }

        default:
            s_state = STATE_ADDR_ROW0;
            break;
    }
}
