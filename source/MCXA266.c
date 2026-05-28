/*
 * MCXA266 LPADC High-Resolution 16x Averaging Polling Implementation
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

/* Hardware Mapping Definitions */
#define DEMO_LPADC_BASE          ADC1
#define DEMO_LPADC_USER_CHANNEL  12U  /* Target: ADC1_A12 */
#define DEMO_LPADC_USER_CMDID    1U
#define DEMO_LPADC_TRIGGER_ID    0U

/* Local Pin Mux Configuration Function */
void BOARD_InitLocalAdcPins(void)
{
    /* Enable Port 1 Clock Gate */
    CLOCK_EnableClock(kCLOCK_GatePORT1);

    /* Release Port 1 and ADC1 from Reset State */
    RESET_ReleasePeripheralReset(kPORT1_RST_SHIFT_RSTn);
    RESET_ReleasePeripheralReset(kADC1_RST_SHIFT_RSTn);

    /* PORT1_14 (pin 7) is configured as ADC1_A12 */
    PORT_SetPinMux(PORT1, 14U, kPORT_MuxAlt0);

    /* FIXED: Adjusted configuration structure token name to 'inputBuffer' to match SDK target */
    port_pin_config_t adc_pin_config = {0};
    adc_pin_config.pullSelect = kPORT_PullDisable;
    adc_pin_config.driveStrength = kPORT_LowDriveStrength;
    adc_pin_config.passiveFilterEnable = false;
    adc_pin_config.inputBuffer = kPORT_InputBufferDisable; /* Handled via explicit enum name match */

    PORT_SetPinConfig(PORT1, 14U, &adc_pin_config);
}

int main(void)
{
    lpadc_config_t lpadcConfig;
    lpadc_conv_trigger_config_t triggerConfig;
    lpadc_conv_command_config_t commandConfig;
    lpadc_conv_result_t result;

    /* Initialize MCU Core Hardware System Platforms */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Route the physical P1_14 pin tracker to ADC1 Channel 12 input line */
    BOARD_InitLocalAdcPins();

    /* System tree clock token setup with safety divider */
    CLOCK_SetClockDiv(kCLOCK_DivADC, 2U);
    CLOCK_AttachClk(kFRO_HF_to_ADC);
    CLOCK_EnableClock(kCLOCK_GateADC1);

    /* Reset the Peripheral Controller Configuration Registers */
    RESET_ClearPeripheralReset(kADC1_RST_SHIFT_RSTn);

    /* Fetch Base Default Configurations */
    LPADC_GetDefaultConfig(&lpadcConfig);
    lpadcConfig.enableAnalogPreliminary = true;

    /* Direct numerical typecast assignment to reference VREFH safely across driver versions */
    lpadcConfig.referenceVoltageSource = (lpadc_reference_voltage_source_t)0U;

    /* Initialize Driver Block Instance */
    LPADC_Init(DEMO_LPADC_BASE, &lpadcConfig);

    /* Run Hardware Self-Calibration Sequence */
    LPADC_DoOffsetCalibration(DEMO_LPADC_BASE);

    /* ---------------- SYSTEM COMMAND CONFIG ---------------- */
    LPADC_GetDefaultConvCommandConfig(&commandConfig);

    commandConfig.channelNumber = DEMO_LPADC_USER_CHANNEL;
    commandConfig.conversionResolutionMode = kLPADC_ConversionResolutionHigh;

    /* Smooths out physical potentiometer wiper noise & native register layout alignment */
    commandConfig.hardwareAverageMode = kLPADC_HardwareAverageCount16;
    commandConfig.sampleChannelMode = kLPADC_SampleChannelSingleEndSideA;
    commandConfig.sampleTimeMode = kLPADC_SampleTimeADCK7;

    LPADC_SetConvCommandConfig(
        DEMO_LPADC_BASE,
        DEMO_LPADC_USER_CMDID,
        &commandConfig);

    /* ---------------- SYSTEM TRIGGER CONFIG ---------------- */
    LPADC_GetDefaultConvTriggerConfig(&triggerConfig);

    triggerConfig.targetCommandId = DEMO_LPADC_USER_CMDID;
    triggerConfig.enableHardwareTrigger = false;

    LPADC_SetConvTriggerConfig(
        DEMO_LPADC_BASE,
        DEMO_LPADC_TRIGGER_ID,
        &triggerConfig);

    PRINTF("\r\n--- MCXA266 Smooth 16x Averaged LPADC Test Running ---\r\n");

    /* Flush out leftover initialization/calibration data residual values from FIFO */
    while (LPADC_GetConvResult(DEMO_LPADC_BASE, &result))
    {
    }

    while (1)
    {
        /* Issue conversion command execution via Software Bitmask Trigger */
        LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, (1U << DEMO_LPADC_TRIGGER_ID));

        /* Block loop until conversion flag yields true data presence inside FIFO */
        while (!LPADC_GetConvResult(DEMO_LPADC_BASE, &result))
        {
        }

        /* Continuous filtered tracking output stream */
        PRINTF("ADC: %u\r\n", result.convValue);

        /* Half-second settling delay window */
        SDK_DelayAtLeastUs(500000U, SystemCoreClock);
    }
}
