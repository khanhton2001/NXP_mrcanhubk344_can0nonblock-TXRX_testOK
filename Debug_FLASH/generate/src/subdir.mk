################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../generate/src/C40_Ip_Cfg.c \
../generate/src/Clock_Ip_Cfg.c \
../generate/src/Clock_Ip_PBcfg.c \
../generate/src/Dma_Ip_Cfg.c \
../generate/src/Dma_Ip_Sa_PBcfg.c \
../generate/src/Emios_Mcl_Ip_Sa_PBcfg.c \
../generate/src/Emios_Pwm_Ip_Sa_PBcfg.c \
../generate/src/FlexCAN_Ip_Sa_PBcfg.c \
../generate/src/Flexio_Mcl_Ip_Sa_PBcfg.c \
../generate/src/Igf_Port_Ip_Cfg.c \
../generate/src/IntCtrl_Ip_Cfg.c \
../generate/src/Lpuart_Uart_Ip_Sa_PBcfg.c \
../generate/src/Mcu_Cfg.c \
../generate/src/Mcu_PBcfg.c \
../generate/src/Mem_43_INFLS_Cfg.c \
../generate/src/OsIf_Cfg.c \
../generate/src/Power_Ip_Cfg.c \
../generate/src/Power_Ip_PBcfg.c \
../generate/src/Ram_Ip_Cfg.c \
../generate/src/Ram_Ip_PBcfg.c 

OBJS += \
./generate/src/C40_Ip_Cfg.o \
./generate/src/Clock_Ip_Cfg.o \
./generate/src/Clock_Ip_PBcfg.o \
./generate/src/Dma_Ip_Cfg.o \
./generate/src/Dma_Ip_Sa_PBcfg.o \
./generate/src/Emios_Mcl_Ip_Sa_PBcfg.o \
./generate/src/Emios_Pwm_Ip_Sa_PBcfg.o \
./generate/src/FlexCAN_Ip_Sa_PBcfg.o \
./generate/src/Flexio_Mcl_Ip_Sa_PBcfg.o \
./generate/src/Igf_Port_Ip_Cfg.o \
./generate/src/IntCtrl_Ip_Cfg.o \
./generate/src/Lpuart_Uart_Ip_Sa_PBcfg.o \
./generate/src/Mcu_Cfg.o \
./generate/src/Mcu_PBcfg.o \
./generate/src/Mem_43_INFLS_Cfg.o \
./generate/src/OsIf_Cfg.o \
./generate/src/Power_Ip_Cfg.o \
./generate/src/Power_Ip_PBcfg.o \
./generate/src/Ram_Ip_Cfg.o \
./generate/src/Ram_Ip_PBcfg.o 

C_DEPS += \
./generate/src/C40_Ip_Cfg.d \
./generate/src/Clock_Ip_Cfg.d \
./generate/src/Clock_Ip_PBcfg.d \
./generate/src/Dma_Ip_Cfg.d \
./generate/src/Dma_Ip_Sa_PBcfg.d \
./generate/src/Emios_Mcl_Ip_Sa_PBcfg.d \
./generate/src/Emios_Pwm_Ip_Sa_PBcfg.d \
./generate/src/FlexCAN_Ip_Sa_PBcfg.d \
./generate/src/Flexio_Mcl_Ip_Sa_PBcfg.d \
./generate/src/Igf_Port_Ip_Cfg.d \
./generate/src/IntCtrl_Ip_Cfg.d \
./generate/src/Lpuart_Uart_Ip_Sa_PBcfg.d \
./generate/src/Mcu_Cfg.d \
./generate/src/Mcu_PBcfg.d \
./generate/src/Mem_43_INFLS_Cfg.d \
./generate/src/OsIf_Cfg.d \
./generate/src/Power_Ip_Cfg.d \
./generate/src/Power_Ip_PBcfg.d \
./generate/src/Ram_Ip_Cfg.d \
./generate/src/Ram_Ip_PBcfg.d 


# Each subdirectory must supply rules for building sources it contributes
generate/src/%.o: ../generate/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@generate/src/C40_Ip_Cfg.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


