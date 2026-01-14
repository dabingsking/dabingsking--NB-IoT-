################################################################################
# MRS Version: 2.2.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Hardware/ADC.c \
../Hardware/Angle.c \
../Hardware/EC_01G.c \
../Hardware/ICM42688.c \
../Hardware/MyRTC.c \
../Hardware/MySPI.c \
../Hardware/Sensor.c \
../Hardware/Serial.c \
../Hardware/TFT_LCD.c \
../Hardware/TFT_LCD_Init.c \
../Hardware/Timer.c 

C_DEPS += \
./Hardware/ADC.d \
./Hardware/Angle.d \
./Hardware/EC_01G.d \
./Hardware/ICM42688.d \
./Hardware/MyRTC.d \
./Hardware/MySPI.d \
./Hardware/Sensor.d \
./Hardware/Serial.d \
./Hardware/TFT_LCD.d \
./Hardware/TFT_LCD_Init.d \
./Hardware/Timer.d 

OBJS += \
./Hardware/ADC.o \
./Hardware/Angle.o \
./Hardware/EC_01G.o \
./Hardware/ICM42688.o \
./Hardware/MyRTC.o \
./Hardware/MySPI.o \
./Hardware/Sensor.o \
./Hardware/Serial.o \
./Hardware/TFT_LCD.o \
./Hardware/TFT_LCD_Init.o \
./Hardware/Timer.o 


EXPANDS += \
./Hardware/ADC.c.234r.expand \
./Hardware/Angle.c.234r.expand \
./Hardware/EC_01G.c.234r.expand \
./Hardware/ICM42688.c.234r.expand \
./Hardware/MyRTC.c.234r.expand \
./Hardware/MySPI.c.234r.expand \
./Hardware/Sensor.c.234r.expand \
./Hardware/Serial.c.234r.expand \
./Hardware/TFT_LCD.c.234r.expand \
./Hardware/TFT_LCD_Init.c.234r.expand \
./Hardware/Timer.c.234r.expand 



# Each subdirectory must supply rules for building sources it contributes
Hardware/%.o: ../Hardware/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Debug" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Core" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/User" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Peripheral/inc" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Hardware" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

