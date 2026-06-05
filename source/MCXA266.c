/**
 * @file  MCXA266.c
 * @brief MCXA266 application entry point
 *
 * LCD_Service1kHz() is driven by FreeRTOS's tick hook.
 * FreeRTOS ticks at 1000 Hz (configTICK_RATE_HZ = 1000), so the
 * tick hook fires every 1 ms — giving a true 1 kHz LCD refresh.
 *
 * The scheduler MUST be running for the tick hook to fire.
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_common.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lcd.h"

/* =========================================================================
 * HEAP SETUP (required by heap_5)
 *
 * FreeRTOS heap_5 does NOT allocate memory automatically — you must call
 * vPortDefineHeapRegions() before any pvPortMalloc() (including xTaskCreate).
 *
 * We reserve a static byte array of configTOTAL_HEAP_SIZE (10 KB) and
 * register it as the single heap region.
 * ======================================================================= */

static uint8_t freertos_heap_storage[configTOTAL_HEAP_SIZE] __attribute__((aligned(8)));

static void setup_freertos_heap(void)
{
    static const HeapRegion_t heap_regions[] =
    {
        { freertos_heap_storage, sizeof(freertos_heap_storage) },
        { NULL,                  0                             }  /* end marker */
    };
    vPortDefineHeapRegions(heap_regions);
}

/* =========================================================================
 * 1 kHz TICK HOOK — refreshes the LCD every millisecond
 *
 * FreeRTOS calls vApplicationTickHook() from inside xTaskIncrementTick()
 * on every SysTick interrupt.  With configTICK_RATE_HZ = 1000 this is
 * exactly once per millisecond.
 *
 * We also maintain a free-running millisecond counter for general use.
 * ======================================================================= */

static volatile uint32_t uptime_milliseconds = 0u;

void vApplicationTickHook(void)
{
    uptime_milliseconds++;
    LCD_Service1kHz();
}

/* =========================================================================
 * APPLICATION TASK — periodic heartbeat log
 *
 * Wakes every 100 ms.  Prints a one-line heartbeat to the debug console
 * once per second so you can confirm the system is alive.
 * ======================================================================= */

#define HEARTBEAT_INTERVAL_MS   1000u
#define TASK_SLEEP_MS           100u

static void application_task(void *parameters)
{
    (void)parameters;   /* unused */

    uint32_t next_heartbeat_ms = HEARTBEAT_INTERVAL_MS;
    uint32_t seconds_elapsed   = 0u;

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(TASK_SLEEP_MS));

        uint32_t now_ms = uptime_milliseconds;
        if (now_ms >= next_heartbeat_ms)
        {
            seconds_elapsed++;
            next_heartbeat_ms += HEARTBEAT_INTERVAL_MS;
           // PRINTF("running: %lu seconds\n", (unsigned long)seconds_elapsed);
        }
    }
}

/* =========================================================================
 * MAIN — system startup sequence
 *
 * 1. Initialise board hardware (pins, clocks, debug console)
 * 2. Initialise the LCD (blocking delays — runs before the scheduler)
 * 3. Load initial text into the LCD shadow buffer
 * 4. Set up the FreeRTOS heap (heap_5 requirement)
 * 5. Create the application task
 * 6. Start the FreeRTOS scheduler (tick hook begins firing → LCD refreshes)
 * ======================================================================= */

#define APP_TASK_STACK_WORDS  256u

int main(void)
{
    /* --- Step 1: Board hardware init ----------------------------------- */
    BOARD_InitBootPins();
    BOARD_BootClockFROHF180M();
    BOARD_InitDebugConsole();

    PRINTF("--- BOOT OK ---\n");
    PRINTF("Clock: FRO 180 MHz\n");

    /* --- Step 2: LCD hardware init (blocking, before scheduler) --------- */
    PRINTF("Initialising LCD...\n");
    LCD_Init();
    PRINTF("LCD initialised\n");

    /* --- Step 3: Load initial display text ------------------------------ */
    LCD_Clear();
    LCD_PrintStr("MCXA266",         LCD_ROW0_START);    /* Row 0: "MCXA266" */
    LCD_PrintStr("READY",           8u);                /* Row 0, col 8     */
    LCD_PrintStr("BARE METAL MODE", LCD_ROW1_START);    /* Row 1            */
    LCD_PrintStr("HELLO world", LCD_ROW0_START);    /* Row 1            */


    /* Place a visible character at the blink positions —
     * blinking a space is invisible since blank and space look the same! */
    LCD_SetChar('*', LCD_LAST_COL_ROW0);    /* pos 15: show '*' that blinks */
    LCD_SetChar('*', LCD_LAST_COL_ROW1);    /* pos 31: show '*' that blinks */

    LCD_SetBlink(LCD_LAST_COL_ROW0);    /* blink last column of row 0 (pos 15) */
    LCD_SetBlink(LCD_LAST_COL_ROW1);    /* blink last column of row 1 (pos 31) */
    LCD_SetBlink(30u);
    PRINTF("LCD text loaded\n");

    /* --- Step 4: Set up FreeRTOS heap (heap_5 needs this first) --------- */
    setup_freertos_heap();

    /* --- Step 5: Create application task -------------------------------- */
    xTaskCreate(application_task,
                "app",
                APP_TASK_STACK_WORDS,
                NULL,
                tskIDLE_PRIORITY + 1,
                NULL);

    /* --- Step 6: Start the scheduler (tick hook begins → LCD refreshes) - */
    PRINTF("Starting FreeRTOS scheduler\n");
    vTaskStartScheduler();

    /* Should never reach here — scheduler runs forever */
    for (;;) {}
}
