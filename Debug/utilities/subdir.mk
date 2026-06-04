################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utilities/fsl_assert.c 

C_DEPS += \
./utilities/fsl_assert.d 

OBJS += \
./utilities/fsl_assert.o 


# Each subdirectory must supply rules for building sources it contributes
utilities/%.o: ../utilities/%.c utilities/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_MCXA266VLQ -DCPU_MCXA266VLQ_cm33 -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -DSERIAL_PORT_TYPE_UART=1 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -DATTEST_TOKEN_PROFILE_PSA_IOT_1 -DPLATFORM_DEFAULT_CRYPTO_KEYS -DPS_ENCRYPTION -DSDK_OS_FREE_RTOS -DUSE_RTOS=1 -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\freertos\freertos-kernel\portable\GCC\ARM_CM33\non_secure" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\drivers" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\device" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\device\periph5" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\CMSIS" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\CMSIS\m-profile" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\utilities" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\utilities\debug_console" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\component\serial_manager" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\component\lists" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\utilities\str" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\component\uart" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\utilities\debug_console\config" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\fatfs\source" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\fatfs\source\fsl_usb_disk" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\usb\include" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\usb\host" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\usb\host\class" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\component\osa\config" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\mbedtls3x\include" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tfm\tf-m\platform\include" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tfm\tf-m\interface\include" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tfm\tf-m\interface\include\psa" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tfm\tf-m\interface\include\crypto_keys" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tfm\tf-m\interface\include\psa_manifest" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\platform\ext\driver" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\platform\include" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\platform\ext" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\platform\ext\common" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\platform\ext\target\nxp\common\Device\Config" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\platform\ext\target\nxp\common\Device\Include" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\interface\include" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\interface\include\crypto_keys" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\interface\include\psa" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\interface\include\psa_manifest" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\interface\include\os_wrapper" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\secure_fw\spm\include" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\secure_fw\include" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\secure_fw\partitions\crypto" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\secure_fw\partitions\protected_storage" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\tf-m\config" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\freertos\freertos-kernel\include" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\component\osa" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\board" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\source\config\host\khci" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\source\template\usb" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\source\template" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\freertos\freertos-kernel\template" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\freertos\freertos-kernel\template\ARM_CM33_3_priority_bits" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\source" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-utilities

clean-utilities:
	-$(RM) ./utilities/fsl_assert.d ./utilities/fsl_assert.o

.PHONY: clean-utilities

