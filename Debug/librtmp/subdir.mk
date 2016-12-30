################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../librtmp/amf.c \
../librtmp/hashswf.c \
../librtmp/log.c \
../librtmp/parseurl.c \
../librtmp/rtmp.c 

OBJS += \
./librtmp/amf.o \
./librtmp/hashswf.o \
./librtmp/log.o \
./librtmp/parseurl.o \
./librtmp/rtmp.o 

C_DEPS += \
./librtmp/amf.d \
./librtmp/hashswf.d \
./librtmp/log.d \
./librtmp/parseurl.d \
./librtmp/rtmp.d 


# Each subdirectory must supply rules for building sources it contributes
librtmp/%.o: ../librtmp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/openssl -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


