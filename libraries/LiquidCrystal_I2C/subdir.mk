################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
C:\Sloeber\arduinoPlugin\libraries\LiquidCrystal_I2C\1.1.2\LiquidCrystal_I2C.cpp 

O_SRCS += \
C:\Sloeber\arduinoPlugin\libraries\LiquidCrystal_I2C\1.1.2\LiquidCrystal_I2C.o 

LINK_OBJ += \
.\libraries\LiquidCrystal_I2C\LiquidCrystal_I2C.cpp.o 

CPP_DEPS += \
.\libraries\LiquidCrystal_I2C\LiquidCrystal_I2C.cpp.d 


# Each subdirectory must supply rules for building sources it contributes
libraries\LiquidCrystal_I2C\LiquidCrystal_I2C.cpp.o: C:\Sloeber\arduinoPlugin\libraries\LiquidCrystal_I2C\1.1.2\LiquidCrystal_I2C.cpp
	@echo 'Building file: $<'
	@echo 'Starting C++ compile'
	"C:\Sloeber\arduinoPlugin\packages\arduino\tools\avr-gcc\7.3.0-atmel3.6.1-arduino5/bin/avr-g++" -c -g -Os -Wall -Wextra -std=gnu++11 -fpermissive -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -Wno-error=narrowing -MMD -flto -mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=10802 -DARDUINO_AVR_NANO -DARDUINO_ARCH_AVR     -I"C:\Sloeber\arduinoPlugin\packages\arduino\hardware\avr\1.8.2\cores\arduino" -I"C:\Sloeber\arduinoPlugin\packages\arduino\hardware\avr\1.8.2\variants\eightanaloginputs" -I"C:\Sloeber\arduinoPlugin\libraries\DallasTemperature\3.8.0" -I"C:\Sloeber\arduinoPlugin\libraries\LiquidCrystal_I2C\1.1.2" -I"C:\Sloeber\arduinoPlugin\libraries\OneWire\2.3.5" -I"C:\Sloeber\arduinoPlugin\packages\arduino\hardware\avr\1.8.2\libraries\Wire\src" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -D__IN_ECLIPSE__=1 -x c++ "$<"  -o  "$@"
	@echo 'Finished building: $<'
	@echo ' '


