################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../component/lists/fsl_component_generic_list.c 

C_DEPS += \
./component/lists/fsl_component_generic_list.d 

OBJS += \
./component/lists/fsl_component_generic_list.o 


# Each subdirectory must supply rules for building sources it contributes
component/lists/%.o: ../component/lists/%.c component/lists/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_MCXA266VLQ -DCPU_MCXA266VLQ_cm33 -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -DSERIAL_PORT_TYPE_UART=1 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\board" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\source" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\drivers" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\device" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\device\periph5" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\CMSIS" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\CMSIS\m-profile" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\utilities" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\utilities\debug_console" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\component\serial_manager" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\component\lists" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\utilities\str" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\component\uart" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA266\utilities\debug_console\config" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-component-2f-lists

clean-component-2f-lists:
	-$(RM) ./component/lists/fsl_component_generic_list.d ./component/lists/fsl_component_generic_list.o

.PHONY: clean-component-2f-lists

