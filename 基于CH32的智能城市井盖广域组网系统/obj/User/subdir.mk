################################################################################
# MRS Version: 2.2.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/ch32l103_it.c \
../User/main.c \
../User/system_ch32l103.c 

C_DEPS += \
./User/ch32l103_it.d \
./User/main.d \
./User/system_ch32l103.d 

OBJS += \
./User/ch32l103_it.o \
./User/main.o \
./User/system_ch32l103.o 


EXPANDS += \
./User/ch32l103_it.c.234r.expand \
./User/main.c.234r.expand \
./User/system_ch32l103.c.234r.expand 



# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Debug" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Core" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/User" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Peripheral/inc" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Hardware" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

