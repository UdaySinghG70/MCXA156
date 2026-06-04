/**
 * @file  main.c
 * @brief MCXA266 bare-metal application entry point
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_common.h"

#include "lcd.h"

/* =========================================================================
 * DELAY (SDK-based)
 * ======================================================================= */
static void delay_1s(void)
{
    SDK_DelayAtLeastUs(1000000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));
}

/* =========================================================================
 * MAIN
 * ======================================================================= */
int main(void)
{
    /* --- Basic system init --------------------------------------------- */
    BOARD_InitBootPins();
    BOARD_BootClockFROHF180M();
    BOARD_InitDebugConsole();

    PRINTF("\r\n--- BOOT OK ---\r\n");
    PRINTF("Clock: FRO 180 MHz\r\n");
    PRINTF("Bare-metal mode\r\n");
    PRINTF("Creating app flow...\r\n");

    /* --- DIAGNOSTIC: confirm execution reaches LCD --------------------- */
    PRINTF("before LCD_Init\r\n");

    LCD_Init();

    PRINTF("after LCD_Init\r\n");

    /* --- LCD usage ----------------------------------------------------- */
    LCD_Clear();
    LCD_PrintStr("MCXA266  READY  ", 0u);
    LCD_PrintStr("BARE METAL MODE ", 16u);

    LCD_SetBlink(15u);
    LCD_SetBlink(31u);

    PRINTF("LCD text loaded\r\n");

    /* --- Main loop ----------------------------------------------------- */
    uint32_t count = 0U;

    while (1)
    {
        /* Drive LCD state machine */
        LCD_Service1kHz();

        /* slow loop (not real 1kHz; just prevents CPU burn) */
        delay_1s();

        count++;

        PRINTF("running: %lu seconds\r\n", (unsigned long)count);
    }
}
