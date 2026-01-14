################################################################################
# MRS Version: 2.2.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Debug/debug.c 

C_DEPS += \
./Debug/debug.d 

OBJS += \
./Debug/debug.o 


EXPANDS += \
./Debug/debug.c.234r.expand 



# Each subdirectory must supply rules for building sources it contributes
Debug/%.o: ../Debug/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Debug" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Core" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/User" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Peripheral/inc" -I"c:/Users/jiangzuohai/Desktop/jinggai/×îÖÕ°å-v2/Hardware" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

