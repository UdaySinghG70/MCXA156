/*
 * MCXA266 LPADC Simplified 1000Hz Block Average Implementation
 * Sampling: 1000Hz (1ms intervals) | Display: True Average every 100 samples
 * Target Pin: P1_14 (Pin 7 on chip, Breakout Header J4 Pin 2) -> ADC1_A12
 */

#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_lpadc.h"
#include "fsl_reset.h"
#include "fsl_clock.h"
#include "fsl_port.h"
#include "fsl_common.h"

/* ---------------- HARDWARE CONFIGURATION CONSTANTS ---------------- */

/* Base ADC peripheral instance */
#define DEMO_LPADC_BASE          ADC1

/* ADC channel mapped to physical pin P1_14 -> ADC1_A12 */
#define DEMO_LPADC_USER_CHANNEL  12U

/* Conversion command slot inside LPADC sequencer */
#define DEMO_LPADC_USER_CMDID    1U

/* Trigger index used for software triggering */
#define DEMO_LPADC_TRIGGER_ID    0U

/* ---------------- TIMING / AVERAGING CONFIGURATION ---------------- */

/* Sampling period = 1000 microseconds = 1 ms = 1000 Hz sampling rate */
#define SAMPLING_DELAY_US        1000U

/* Number of samples accumulated before computing/displaying average */
#define BLOCK_AVERAGE_COUNT      100U


/* ---------------- PIN CONFIGURATION FUNCTION ---------------- */
/*
 * Configures the physical MCU pin (P1_14) to route analog signal
 * into ADC1 channel 12 (ADC1_A12).
 */
void BOARD_InitLocalAdcPins(void)
{
    /* Enable clock for PORT1 block so pin mux registers are accessible */
    CLOCK_EnableClock(kCLOCK_GatePORT1);

    /* Release PORT1 and ADC1 peripherals from reset state */
    RESET_ReleasePeripheralReset(kPORT1_RST_SHIFT_RSTn);
    RESET_ReleasePeripheralReset(kADC1_RST_SHIFT_RSTn);

    /* Configure pin function:
     * ALT0 = analog mode for ADC input on this MCU
     */
    PORT_SetPinMux(PORT1, 14U, kPORT_MuxAlt0);

    /* Configure electrical characteristics of the ADC input pin */
    port_pin_config_t adc_pin_config = {0};

    /* Disable internal pull resistors (important for analog accuracy) */
    adc_pin_config.pullSelect = kPORT_PullDisable;

    /* Low drive strength since this is input-only analog signal */
    adc_pin_config.driveStrength = kPORT_LowDriveStrength;

    /* Enable passive filter (helps reduce high-frequency noise coupling) */
    adc_pin_config.passiveFilterEnable = true;

    /* Disable digital input buffer to reduce leakage/noise into ADC path */
    adc_pin_config.inputBuffer = kPORT_InputBufferDisable;

    /* Apply configuration to PORT1 pin 14 */
    PORT_SetPinConfig(PORT1, 14U, &adc_pin_config);
}


/* ---------------- MAIN APPLICATION ---------------- */
int main(void)
{
    /* LPADC configuration structures */
    lpadc_config_t lpadcConfig;
    lpadc_conv_trigger_config_t triggerConfig;
    lpadc_conv_command_config_t commandConfig;
    lpadc_conv_result_t result;

    /* Index counter for printed averages */
    uint32_t displayIndex = 0;

    /* ---------------- BLOCK AVERAGING VARIABLES ---------------- */

    /* Accumulates raw ADC samples */
    uint32_t blockSampleSum = 0;

    /* Tracks number of samples collected in current block */
    uint32_t collectedSamples = 0;

    /* ---------------- SYSTEM INITIALIZATION ---------------- */

    /* Initialize board pins (clock muxing, default pin states, etc.) */
    BOARD_InitBootPins();

    /* Initialize system clock tree */
    BOARD_InitBootClocks();

    /* Initialize UART debug console for PRINTF output */
    BOARD_InitDebugConsole();

    /* Configure physical ADC input pin routing */
    BOARD_InitLocalAdcPins();

    /* ---------------- CLOCK / RESET CONFIGURATION ---------------- */

    /* Divide ADC clock to ensure stable sampling timing */
    CLOCK_SetClockDiv(kCLOCK_DivADC, 2U);

    /* Select internal fast clock (FRO_HF) as ADC clock source */
    CLOCK_AttachClk(kFRO_HF_to_ADC);

    /* Enable ADC peripheral clock */
    CLOCK_EnableClock(kCLOCK_GateADC1);

    /* Clear reset state of ADC peripheral (ensures clean startup) */
    RESET_ClearPeripheralReset(kADC1_RST_SHIFT_RSTn);

    /* ---------------- ADC INITIALIZATION ---------------- */

    /* Load default LPADC configuration */
    LPADC_GetDefaultConfig(&lpadcConfig);

    /* Enable internal analog precharge/preliminary circuitry
     * (improves stability of ADC sampling capacitor)
     */
    lpadcConfig.enableAnalogPreliminary = true;

    /* Select reference voltage source (0U = default internal reference) */
    lpadcConfig.referenceVoltageSource = (lpadc_reference_voltage_source_t)0U;

    /* Initialize LPADC peripheral */
    LPADC_Init(DEMO_LPADC_BASE, &lpadcConfig);

    /* Run offset calibration to remove DC error offset in ADC readings */
    LPADC_DoOffsetCalibration(DEMO_LPADC_BASE);

    /* ---------------- CONVERSION COMMAND CONFIGURATION ---------------- */

    LPADC_GetDefaultConvCommandConfig(&commandConfig);

    /* Select ADC channel (ADC1_A12 = external analog input pin) */
    commandConfig.channelNumber = DEMO_LPADC_USER_CHANNEL;

    /* High resolution mode for improved accuracy */
    commandConfig.conversionResolutionMode = kLPADC_ConversionResolutionHigh;

    /* Hardware averaging inside ADC:
     * Each result = average of 128 internal samples
     * Reduces noise significantly before software processing
     */
    commandConfig.hardwareAverageMode = kLPADC_HardwareAverageCount128;

    /* Single-ended input measurement (Channel A side) */
    commandConfig.sampleChannelMode = kLPADC_SampleChannelSingleEndSideA;

    /* Long sample time to support high source impedance signals
     * (e.g., potentiometers, sensors)
     */
    commandConfig.sampleTimeMode = kLPADC_SampleTimeADCK19;

    /* Apply command configuration into command slot 1 */
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, &commandConfig);

    /* ---------------- TRIGGER CONFIGURATION ---------------- */

    LPADC_GetDefaultConvTriggerConfig(&triggerConfig);

    /* Link trigger to conversion command */
    triggerConfig.targetCommandId = DEMO_LPADC_USER_CMDID;

    /* Disable hardware trigger (we use software triggering manually) */
    triggerConfig.enableHardwareTrigger = false;

    /* Apply trigger configuration */
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, DEMO_LPADC_TRIGGER_ID, &triggerConfig);

    /* ---------------- STARTUP MESSAGE ---------------- */
    PRINTF("\r\n--- MCXA266 1000Hz Timed Raw [100 Sample Block Average] Running ---\r\n");

    /* Clear any stale ADC FIFO data before starting measurements */
    while (LPADC_GetConvResult(DEMO_LPADC_BASE, &result))
    {
        /* discard old results */
    }

    /* ---------------- MAIN SAMPLING LOOP ---------------- */
    while (1)
    {
        /* Trigger ADC conversion via software */
        LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, (1U << DEMO_LPADC_TRIGGER_ID));

        /* Wait (polling) until conversion result is available in FIFO */
        while (!LPADC_GetConvResult(DEMO_LPADC_BASE, &result))
        {
            /* busy wait */
        }

        /* Add raw ADC value into accumulation buffer */
        blockSampleSum += result.convValue;

        /* Increment sample counter */
        collectedSamples++;

        /* ---------------- BLOCK AVERAGING LOGIC ---------------- */
        if (collectedSamples >= BLOCK_AVERAGE_COUNT)
        {
            /* Compute arithmetic mean of collected samples */
            uint32_t averagedValue = blockSampleSum / BLOCK_AVERAGE_COUNT;

            /* Print index + averaged ADC value */
            PRINTF("[%4u] %5u\r\n", displayIndex++, averagedValue);

            /* Reset accumulation state for next block */
            blockSampleSum = 0;
            collectedSamples = 0;
        }

        /* ---------------- TIMING CONTROL ---------------- */

        /* Enforce ~1ms sampling period (target 1000 Hz sampling rate)
         * NOTE: this is software delay, not a precise hardware timer
         */
        SDK_DelayAtLeastUs(SAMPLING_DELAY_US, SystemCoreClock);
    }
}
