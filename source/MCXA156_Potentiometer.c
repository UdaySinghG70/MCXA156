#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_lpuart.h"
#include "fsl_reset.h"
#include "fsl_clock.h"

#define DEMO_LPUART          LPUART0
#define DEMO_LPUART_CLK_FREQ BOARD_DEBUG_UART_CLK_FREQ

int main(void) {
    lpuart_config_t config;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    /* Attach 12 MHz clock to LPUART0 */
    CLOCK_SetClockDiv(kCLOCK_DivLPUART0, 1u);
    CLOCK_AttachClk(kFRO12M_to_LPUART0);

    /* Initialize LPUART */
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = 115200;
    config.enableTx = true;
    config.enableRx = true;

    LPUART_Init(DEMO_LPUART, &config, DEMO_LPUART_CLK_FREQ);

    uint8_t txbuff[] = "\r\nLPUART Echo Test Started. Type something:\r\n";
    LPUART_WriteBlocking(DEMO_LPUART, txbuff, sizeof(txbuff) - 1);

    while (1) {
        uint8_t ch;
        /* Wait for character */
        LPUART_ReadBlocking(DEMO_LPUART, &ch, 1);
        /* Echo character */
        LPUART_WriteBlocking(DEMO_LPUART, &ch, 1);
    }
}