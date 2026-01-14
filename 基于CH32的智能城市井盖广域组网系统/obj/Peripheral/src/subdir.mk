################################################################################
# MRS Version: 2.2.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Peripheral/src/ch32l103_adc.c \
../Peripheral/src/ch32l103_bkp.c \
../Peripheral/src/ch32l103_can.c \
../Peripheral/src/ch32l103_crc.c \
../Peripheral/src/ch32l103_dbgmcu.c \
../Peripheral/src/ch32l103_dma.c \
../Peripheral/src/ch32l103_exti.c \
../Peripheral/src/ch32l103_flash.c \
../Peripheral/src/ch32l103_gpio.c \
../Peripheral/src/ch32l103_i2c.c \
../Peripheral/src/ch32l103_iwdg.c \
../Peripheral/src/ch32l103_lptim.c \
../Peripheral/src/ch32l103_misc.c \
../Peripheral/src/ch32l103_opa.c \
../Peripheral/src/ch32l103_pwr.c \
../Peripheral/src/ch32l103_rcc.c \
../Peripheral/src/ch32l103_rtc.c \
../Peripheral/src/ch32l103_spi.c \
../Peripheral/src/ch32l103_tim.c \
../Peripheral/src/ch32l103_usart.c \
../Peripheral/src/ch32l103_wwdg.c 

C_DEPS += \
./Peripheral/src/ch32l103_adc.d \
./Peripheral/src/ch32l103_bkp.d \
./Peripheral/src/ch32l103_can.d \
./Peripheral/src/ch32l103_crc.d \
./Peripheral/src/ch32l103_dbgmcu.d \
./Peripheral/src/ch32l103_dma.d \
./Peripheral/src/ch32l103_exti.d \
./Peripheral/src/ch32l103_flash.d \
./Peripheral/src/ch32l103_gpio.d \
./Peripheral/src/ch32l103_i2c.d \
./Peripheral/src/ch32l103_iwdg.d \
./Peripheral/src/ch32l103_lptim.d \
./Peripheral/src/ch32l103_misc.d \
./Peripheral/src/ch32l103_opa.d \
./Peripheral/src/ch32l103_pwr.d \
./Peripheral/src/ch32l103_rcc.d \
./Peripheral/src/ch32l103_rtc.d \
./Peripheral/src/ch32l103_spi.d \
./Peripheral/src/ch32l103_tim.d \
./Peripheral/src/ch32l103_usart.d \
./Peripheral/src/ch32l103_wwdg.d 

OBJS += \
./Peripheral/src/ch32l103_adc.o \
./Peripheral/src/ch32l103_bkp.o \
./Peripheral/src/ch32l103_can.o \
./Peripheral/src/ch32l103_crc.o \
./Peripheral/src/ch32l103_dbgmcu.o \
./Peripheral/src/ch32l103_dma.o \
./Peripheral/src/ch32l103_exti.o \
./Peripheral/src/ch32l103_flash.o \
./Peripheral/src/ch32l103_gpio.o \
./Peripheral/src/ch32l103_i2c.o \
./Peripheral/src/ch32l103_iwdg.o \
./Peripheral/src/ch32l103_lptim.o \
./Peripheral/src/ch32l103_misc.o \
./Peripheral/src/ch32l103_opa.o \
./Peripheral/src/ch32l103_pwr.o \
./Peripheral/src/ch32l103_rcc.o \
./Peripheral/src/ch32l103_rtc.o \
./Peripheral/src/ch32l103_spi.o \
./Peripheral/src/ch32l103_tim.o \
./Peripheral/src/ch32l103_usart.o \
./Peripheral/src/ch32l103_wwdg.o 


EXPANDS += \
./Peripheral/src/ch32l103_adc.c.234r.expand \
./Peripheral/src/ch32l103_bkp.c.234r.expand \
./Peripheral/src/ch32l103_can.c.234r.expand \
./Peripheral/src/ch32l103_crc.c.234r.expand \
./Peripheral/src/ch32l103_dbgmcu.c.234r.expand \
./Peripheral/src/ch32l103_dma.c.234r.expand \
./Peripheral/src/ch32l103_exti.c.234r.expand \
./Peripheral/src/ch32l103_flash.c.234r.expand \
./Peripheral/src/ch32l103_gpio.c.234r.expand \
./Peripheral/src/ch32l103_i2c.c.234r.expand \
./Peripheral/src/ch32l103_iwdg.c.234r.expand \
./Peripheral/src/ch32l103_lptim.c.234r.expand \
./Peripheral/src/ch32l103_misc.c.234r.expand \
./Peripheral/src/ch32l103_opa.c.234r.expand \
./Peripheral/src/ch32l103_pwr.c.234r.expand \
./Peripheral/src/ch32l103_rcc.c.234r.expand \
./Peripheral/src/ch32l103_rtc.c.234r.expand \
./Peripheral/src/ch32l103_spi.c.234r.expand \
./Peripheral/src/ch32l103_tim.c.234r.expand \
./Peripheral/src/ch32l103_usart.c.234r.expand \
./Peripheral/src/ch32l103_wwdg.c.234r.expand 



# Each subdirectory must supply rules for building sources it contributes
Peripheral/src/%.o: ../Peripheral/src/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Debug" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Core" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/User" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Peripheral/inc" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Hardware" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

