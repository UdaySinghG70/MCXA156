################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/fsl_clock.c \
../drivers/fsl_common.c \
../drivers/fsl_common_arm.c \
../drivers/fsl_gpio.c \
../drivers/fsl_lpadc.c \
../drivers/fsl_lpuart.c \
../drivers/fsl_reset.c \
../drivers/fsl_spc.c 

C_DEPS += \
./drivers/fsl_clock.d \
./drivers/fsl_common.d \
./drivers/fsl_common_arm.d \
./drivers/fsl_gpio.d \
./drivers/fsl_lpadc.d \
./drivers/fsl_lpuart.d \
./drivers/fsl_reset.d \
./drivers/fsl_spc.d 

OBJS += \
./drivers/fsl_clock.o \
./drivers/fsl_common.o \
./drivers/fsl_common_arm.o \
./drivers/fsl_gpio.o \
./drivers/fsl_lpadc.o \
./drivers/fsl_lpuart.o \
./drivers/fsl_reset.o \
./drivers/fsl_spc.o 


# Each subdirectory must supply rules for building sources it contributes
drivers/%.o: ../drivers/%.c drivers/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_MCXA156VLL -DCPU_MCXA156VLL_cm33 -DSDK_OS_BAREMETAL -DSERIAL_PORT_TYPE_UART=1 -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\board" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\source" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\drivers" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\CMSIS" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\CMSIS\m-profile" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\utilities" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\utilities\debug_console" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\device" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\device\periph1" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\component\serial_manager" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\component\lists" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\utilities\str" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\component\uart" -I"C:\Users\Desktop\Desktop\MCUXpressoIDE\workspace\MCXA156_Potentiometer\utilities\debug_console\config" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-drivers

clean-drivers:
	-$(RM) ./drivers/fsl_clock.d ./drivers/fsl_clock.o ./drivers/fsl_common.d ./drivers/fsl_common.o ./drivers/fsl_common_arm.d ./drivers/fsl_common_arm.o ./drivers/fsl_gpio.d ./drivers/fsl_gpio.o ./drivers/fsl_lpadc.d ./drivers/fsl_lpadc.o ./drivers/fsl_lpuart.d ./drivers/fsl_lpuart.o ./drivers/fsl_reset.d ./drivers/fsl_reset.o ./drivers/fsl_spc.d ./drivers/fsl_spc.o

.PHONY: clean-drivers

