################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Main.cpp \
../RTMPStream.cpp \
../convert_utils.cpp \
../nl_tno_stormcv_util_StreamerHelper.cpp 

OBJS += \
./Main.o \
./RTMPStream.o \
./convert_utils.o \
./nl_tno_stormcv_util_StreamerHelper.o 

CPP_DEPS += \
./Main.d \
./RTMPStream.d \
./convert_utils.d \
./nl_tno_stormcv_util_StreamerHelper.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/openssl -I/usr/local/jdk/include/linux -I/usr/local/jdk/include -includejni.h -includejni_md.h -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


