#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_lpadc.h"
#include "fsl_reset.h"
#include "fsl_clock.h"

#define DEMO_LPADC_BASE          ADC1
#define DEMO_LPADC_USER_CHANNEL  8U
#define DEMO_LPADC_USER_CMDID    1U

int main(void) {
    lpadc_config_t lpadcConfigStruct;
    lpadc_conv_trigger_config_t lpadcTriggerConfigStruct;
    lpadc_conv_command_config_t lpadcCommandConfigStruct;
    lpadc_conv_result_t lpadcResultConfigStruct;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_SetClockDiv(kCLOCK_DivADC1, 1u);
    CLOCK_AttachClk(kFRO12M_to_ADC1);
    CLOCK_EnableClock(kCLOCK_GateADC1);
    RESET_ClearPeripheralReset(kADC1_RST_SHIFT_RSTn);

    LPADC_GetDefaultConfig(&lpadcConfigStruct);
    lpadcConfigStruct.enableAnalogPreliminary = true;
    lpadcConfigStruct.referenceVoltageSource = kLPADC_ReferenceVoltageAlt1;
    LPADC_Init(DEMO_LPADC_BASE, &lpadcConfigStruct);

    LPADC_DoOffsetCalibration(DEMO_LPADC_BASE);

    LPADC_GetDefaultConvCommandConfig(&lpadcCommandConfigStruct);
    lpadcCommandConfigStruct.channelNumber = DEMO_LPADC_USER_CHANNEL;
    lpadcCommandConfigStruct.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
    lpadcCommandConfigStruct.hardwareAverageMode = kLPADC_HardwareAverageCount16;
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, &lpadcCommandConfigStruct);

    LPADC_GetDefaultConvTriggerConfig(&lpadcTriggerConfigStruct);
    lpadcTriggerConfigStruct.targetCommandId = DEMO_LPADC_USER_CMDID;
    lpadcTriggerConfigStruct.enableHardwareTrigger = false;
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &lpadcTriggerConfigStruct);

    PRINTF("\r\n--- Minimal 16-bit ADC Read ---\r\n");

    while (1) {
        LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1U);
        while (!LPADC_GetConvResult(DEMO_LPADC_BASE, &lpadcResultConfigStruct)) {}

        PRINTF("ADC: %u\r\n", lpadcResultConfigStruct.convValue);
        SDK_DelayAtLeastUs(500000U, SystemCoreClock);
    }
}
