/*
 * MCXA266.c
 */

#include "FreeRTOS.h"
#include "task.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_lpadc.h"
#include "fsl_common.h"
#include "fsl_reset.h"
#include "fsl_clock.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "ff.h"

/* USB Stack Includes */
#include "usb_host_config.h"
#include "usb.h"
#include "usb_host.h"
#include "usb_host_msd.h"

/* Global Handles */
usb_host_handle g_HostHandle = NULL;
FATFS g_fileSystem;

extern usb_host_class_handle g_UsbFatfsClassHandle;
static usb_host_class_handle g_UsbMsdClassHandle = NULL;
static usb_host_interface_t *s_msdInterfaceHandle = NULL;
static usb_device_handle s_msdDeviceHandle = NULL;
static volatile bool s_usbMsdReady = false;

/* Define FreeRTOS Heap_5 Buffer */
#define BOARD_HEAP_SIZE (24 * 1024) /* 24 KB heap */
static uint8_t s_freeRtosHeap[BOARD_HEAP_SIZE] __attribute__((aligned(8)));

/* USB Host MSD interface open callback */
static void USB_HostMsdSetInterfaceCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    if (status == kStatus_USB_Success)
    {
        PRINTF("USB_HostEvent: MSD interface opened successfully. USB disk ready.\r\n");
        s_usbMsdReady = true;
    }
    else
    {
        PRINTF("USB_HostEvent: MSD interface open failed. status=%d\r\n", status);
        s_usbMsdReady = false;
    }
}

/* USB Host Event Callback */
usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                           usb_host_configuration_handle configHandle,
                           uint32_t eventCode)
{
    usb_host_configuration_t *configuration = (usb_host_configuration_t *)configHandle;
    usb_host_interface_t *interface;
    uint8_t interfaceIndex;
    usb_status_t status = kStatus_USB_Success;

    switch ((usb_host_event_t)eventCode)
    {
        case kUSB_HostEventAttach:
            if (configuration == NULL)
            {
                return kStatus_USB_Error;
            }
            s_msdDeviceHandle = NULL;
            s_msdInterfaceHandle = NULL;
            s_usbMsdReady = false;
            for (interfaceIndex = 0; interfaceIndex < configuration->interfaceCount; ++interfaceIndex)
            {
                interface = &configuration->interfaceList[interfaceIndex];
                if ((interface->interfaceDesc->bInterfaceClass == USB_HOST_MSD_CLASS_CODE) &&
                    ((interface->interfaceDesc->bInterfaceSubClass == USB_HOST_MSD_SUBCLASS_CODE_UFI) ||
                     (interface->interfaceDesc->bInterfaceSubClass == USB_HOST_MSD_SUBCLASS_CODE_SCSI)) &&
                    (interface->interfaceDesc->bInterfaceProtocol == USB_HOST_MSD_PROTOCOL_BULK))
                {
                    s_msdDeviceHandle = deviceHandle;
                    s_msdInterfaceHandle = interface;
                    PRINTF("USB_HostEvent: USB MSD device attached. Waiting for enumeration...\r\n");
                    return kStatus_USB_Success;
                }
            }
            return kStatus_USB_NotSupported;

        case kUSB_HostEventEnumerationDone:
            if ((s_msdDeviceHandle == deviceHandle) && (s_msdInterfaceHandle != NULL))
            {
                PRINTF("USB_HostEvent: USB MSD enumeration done. Initializing MSD class...\r\n");
                status = USB_HostMsdInit(deviceHandle, &g_UsbMsdClassHandle);
                if (status != kStatus_USB_Success)
                {
                    PRINTF("USB_HostEvent: USB MSD init failed. status=%d\r\n", status);
                    g_UsbMsdClassHandle = NULL;
                    g_UsbFatfsClassHandle = NULL;
                    return status;
                }
                g_UsbFatfsClassHandle = g_UsbMsdClassHandle;
                status = USB_HostMsdSetInterface(g_UsbMsdClassHandle, s_msdInterfaceHandle, 0,
                                                 USB_HostMsdSetInterfaceCallback, NULL);
                if (status != kStatus_USB_Success)
                {
                    PRINTF("USB_HostEvent: USB MSD set interface failed. status=%d\r\n", status);
                    g_UsbMsdClassHandle = NULL;
                    g_UsbFatfsClassHandle = NULL;
                    s_usbMsdReady = false;
                    return status;
                }
                return kStatus_USB_Success;
            }
            break;

        case kUSB_HostEventDetach:
            PRINTF("USB_HostEvent: USB device detached. Cleaning up MSD state...\r\n");
            if (g_UsbMsdClassHandle != NULL)
            {
                (void)USB_HostMsdDeinit(deviceHandle, g_UsbMsdClassHandle);
                g_UsbMsdClassHandle = NULL;
            }
            g_UsbFatfsClassHandle = NULL;
            s_usbMsdReady = false;
            s_msdDeviceHandle = NULL;
            s_msdInterfaceHandle = NULL;
            break;

        case kUSB_HostEventNotSupported:
            PRINTF("USB_HostEvent: Unsupported USB device attached.\r\n");
            break;

        default:
            break;
    }
    return kStatus_USB_Success;
}

static void BOARD_InitPotentiometer(void)
{
    lpadc_config_t lpadcConfig;
    lpadc_conv_trigger_config_t triggerConfig;
    lpadc_conv_command_config_t commandConfig;

    /* Enable clock for PORT1 block so pin mux registers are accessible */
    CLOCK_EnableClock(kCLOCK_GatePORT1);

    /* Release PORT1 and ADC1 peripherals from reset state */
    RESET_ReleasePeripheralReset(kPORT1_RST_SHIFT_RSTn);
    RESET_ReleasePeripheralReset(kADC1_RST_SHIFT_RSTn);

    /* Configure pin function: ALT0 = analog mode for ADC input on this MCU */
    PORT_SetPinMux(PORT1, 14U, kPORT_MuxAlt0);

    /* Configure electrical characteristics of the ADC input pin */
    port_pin_config_t adc_pin_config = {0};
    adc_pin_config.pullSelect = kPORT_PullDisable;
    adc_pin_config.driveStrength = kPORT_LowDriveStrength;
    adc_pin_config.passiveFilterEnable = true;
    adc_pin_config.inputBuffer = kPORT_InputBufferDisable;
    PORT_SetPinConfig(PORT1, 14U, &adc_pin_config);

    /* Divide ADC clock to ensure stable sampling timing */
    CLOCK_SetClockDiv(kCLOCK_DivADC, 2U);

    /* Select internal fast clock (FRO_HF) as ADC clock source */
    CLOCK_AttachClk(kFRO_HF_to_ADC);

    /* Enable ADC peripheral clock */
    CLOCK_EnableClock(kCLOCK_GateADC1);

    /* Load default LPADC configuration */
    PRINTF("  LPADC configuring controller...\r\n");
    LPADC_GetDefaultConfig(&lpadcConfig);
    lpadcConfig.enableAnalogPreliminary = true;
    lpadcConfig.referenceVoltageSource = (lpadc_reference_voltage_source_t)0U;
    LPADC_Init(ADC1, &lpadcConfig);
    PRINTF("  LPADC performing offset calibration...\r\n");
    LPADC_DoOffsetCalibration(ADC1);
    PRINTF("  LPADC offset calibration complete.\r\n");

    /* Conversion command configuration */
    LPADC_GetDefaultConvCommandConfig(&commandConfig);
    commandConfig.channelNumber = 12U;
    commandConfig.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
    commandConfig.hardwareAverageMode = kLPADC_HardwareAverageCount128;
    commandConfig.sampleChannelMode = kLPADC_SampleChannelSingleEndSideA;
    commandConfig.sampleTimeMode = kLPADC_SampleTimeADCK19;
    LPADC_SetConvCommandConfig(ADC1, 1U, &commandConfig);

    /* Trigger configuration */
    LPADC_GetDefaultConvTriggerConfig(&triggerConfig);
    triggerConfig.targetCommandId = 1U;
    triggerConfig.enableHardwareTrigger = false;
    LPADC_SetConvTriggerConfig(ADC1, 0U, &triggerConfig);

    /* Clear stale ADC results */
    lpadc_conv_result_t result;
    while (LPADC_GetConvResult(ADC1, &result)) {}
}

static void BOARD_InitUsbClock(void)
{
    /* Enable USB0 clock gate */
    CLOCK_EnableClock(kCLOCK_GateUSB0);

    /* Switch USB0 to Pll1Clk (which is 240 MHz) */
    CLOCK_AttachClk(kPll1Clk_to_USB0);

    /* Set USB0 clock divider to 5 (240 MHz / 5 = 48 MHz) */
    CLOCK_SetClockDiv(kCLOCK_DivUSB0, 5U);

    /* Release USB0 from reset */
    RESET_ReleasePeripheralReset(kUSB0_RST_SHIFT_RSTn);

    /* Configure USB interrupt priority in NVIC (EnableIRQ will be called after USB Host Init) */
    NVIC_SetPriority(USB0_IRQn, 5U);
}

/* USB Interrupt Service Routine */
void USB0_IRQHandler(void)
{
    USB_HostKhciIsrFunction(g_HostHandle);
}

/* Tasks */
static void USB_Task(void *param) {
    PRINTF("USB_Task: Task execution started.\r\n");
    /* Enable USB0 interrupt in NVIC now that scheduler is running */
    EnableIRQ(USB0_IRQn);
    while (1) {
        /* Calling the correctly declared function */
        USB_HostKhciTaskFunction(g_HostHandle);
        vTaskDelay(pdMS_TO_TICKS(10)); /* 10 ms delay (2 ticks) to prevent priority starvation of App_Task */
    }
}

static void App_Task(void *param) {
    lpadc_conv_result_t adcResultStruct;
    uint32_t logIndex = 0;
    uint32_t potValue = 0;
    bool logging_active = false;
    bool mounted = false;
    uint32_t printCounter = 0;

    /* Averaging variables */
    uint32_t blockSampleSum = 0;
    uint32_t collectedSamples = 0;

    PRINTF("App_Task: Task execution started.\r\n");

    while (1) {
        /* 1. Continuously acquire potentiometer data */
        LPADC_DoSoftwareTrigger(ADC1, 1U << 0U);
        while (!LPADC_GetConvResult(ADC1, &adcResultStruct)) {}
        
        blockSampleSum += adcResultStruct.convValue;
        collectedSamples++;

        if (collectedSamples >= 100U) {
            potValue = blockSampleSum / 100U;
            blockSampleSum = 0;
            collectedSamples = 0;

            /* Print potentiometer value periodically if logging is inactive to show life */
            if (!logging_active) {
                printCounter++;
                if (printCounter >= 5) { /* Every 5 averages (500 ms) */
                    printCounter = 0;
                    PRINTF("Potentiometer Raw Value: %u (Logging: INACTIVE, press SW2 to log)\r\n", potValue);
                }
            }

            /* 2. SW2 Pressed: Start logging if USB is active */
            if (GPIO_PinRead(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN) == 0U) {
                if (!logging_active) {
                    if (!s_usbMsdReady) {
                        PRINTF("SW2 Pressed: USB disk not ready. Wait for enumeration to complete.\r\n");
                    } else {
                        PRINTF("SW2 Pressed: Mounting USB disk...\r\n");
                        FRESULT res = f_mount(&g_fileSystem, "1:", 1);
                        if (res == FR_OK) {
                            logging_active = true;
                            mounted = true;
                            PRINTF("SW2 Pressed: Data logging started. pot.csv initialized.\r\n");
                        } else {
                            PRINTF("SW2 Pressed: Mount failed (USB inactive). Error code: %d\r\n", res);
                        }
                    }
                }
            }

            /* 3. SW3 Pressed: Stop logging */
            if (GPIO_PinRead(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN) == 0U) {
                if (logging_active) {
                    logging_active = false;
                    f_mount(NULL, "1:", 0);
                    mounted = false;
                    PRINTF("SW3 Pressed: Data logging stopped.\r\n");
                }
            }

            /* 4. Write data to pot.csv if logging is active */
            if (logging_active) {
                FIL file;
                FRESULT res = f_open(&file, "1:pot.csv", FA_WRITE | FA_OPEN_APPEND);
                if (res == FR_OK) {
                    f_printf(&file, "%u,%u\n", logIndex++, potValue);
                    f_close(&file);
                    PRINTF("Logged [%u]: %u\r\n", logIndex - 1, potValue);
                } else {
                    PRINTF("Failed to open/append pot.csv. Error: %d. Logging disabled.\r\n", res);
                    logging_active = false;
                    mounted = false;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1)); /* 1 ms delay */
    }
}

int main(void) {
    /* Initialize FreeRTOS Heap_5 regions */
    HeapRegion_t xHeapRegions[] = {
        { s_freeRtosHeap, sizeof(s_freeRtosHeap) },
        { NULL, 0 }
    };
    vPortDefineHeapRegions(xHeapRegions);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\n==============================================\r\n");
    PRINTF("  MCXA266 Potentiometer CSV Logger Booting... \r\n");
    PRINTF("==============================================\r\n");

    PRINTF("Initializing buttons (SW2 & SW3)...\r\n");
    BOARD_InitBUTTONsPins();
    PRINTF("Buttons initialized.\r\n");

    PRINTF("Initializing potentiometer (P1_14 -> ADC1_A12)...\r\n");
    BOARD_InitPotentiometer();
    PRINTF("Potentiometer initialized.\r\n");

    PRINTF("Configuring USB FS clock (240MHz PLL1 / 5)...\r\n");
    BOARD_InitUsbClock();
    PRINTF("USB FS clock enabled.\r\n");

    PRINTF("Initializing USB Host controller...\r\n");
    if (USB_HostInit(0U, &g_HostHandle, USB_HostEvent) != kStatus_USB_Success) {
        PRINTF("USB Host Init Failed\r\n");
    } else {
        PRINTF("USB Host Init Successful\r\n");
    }

    PRINTF("Creating FreeRTOS tasks...\r\n");
    PRINTF("  Free heap before tasks: %u bytes\r\n", (unsigned int)xPortGetFreeHeapSize());
    
    BaseType_t usbTaskStatus = xTaskCreate(USB_Task, "USB_Task", 1024, NULL, 3, NULL);
    PRINTF("  USB_Task created: status = %d, Free heap: %u bytes\r\n", (int)usbTaskStatus, (unsigned int)xPortGetFreeHeapSize());
    
    BaseType_t appTaskStatus = xTaskCreate(App_Task, "App_Task", 1000, NULL, 2, NULL);
    PRINTF("  App_Task created: status = %d, Free heap: %u bytes\r\n", (int)appTaskStatus, (unsigned int)xPortGetFreeHeapSize());

    PRINTF("Starting FreeRTOS scheduler...\r\n");
    vTaskStartScheduler();
    
    PRINTF("Error: vTaskStartScheduler() returned!\r\n");
    while (1);
}
