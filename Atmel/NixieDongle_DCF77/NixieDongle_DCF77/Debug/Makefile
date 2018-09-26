################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS := 


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../DCF77.c \
../main.c \
../timers.c \
../usi_i2c_slave.c


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
DCF77.o \
main.o \
timers.o \
usi_i2c_slave.o

OBJS_AS_ARGS +=  \
DCF77.o \
main.o \
timers.o \
usi_i2c_slave.o

C_DEPS +=  \
DCF77.d \
main.d \
timers.d \
usi_i2c_slave.d

C_DEPS_AS_ARGS +=  \
DCF77.d \
main.d \
timers.d \
usi_i2c_slave.d

OUTPUT_FILE_PATH +=NixieDongle_DCF77.elf

OUTPUT_FILE_PATH_AS_ARGS +=NixieDongle_DCF77.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+= 


# AVR32/GNU C Compiler









./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)D:\tools\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"D:\tools\Atmel\Studio\7.0\Packs\atmel\ATtiny_DFP\1.3.147\include"  -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=attiny2313a -B "D:\tools\Atmel\Studio\7.0\Packs\atmel\ATtiny_DFP\1.3.147\gcc\dev\attiny2313a" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP) $(LINKER_SCRIPT_DEP)
	@echo Building target: $@
	@echo Invoking: AVR/GNU Linker : 5.4.0
	$(QUOTE)D:\tools\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="NixieDongle_DCF77.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=attiny2313a -B "D:\tools\Atmel\Studio\7.0\Packs\atmel\ATtiny_DFP\1.3.147\gcc\dev\attiny2313a"  
	@echo Finished building target: $@
	"D:\tools\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "NixieDongle_DCF77.elf" "NixieDongle_DCF77.hex"
	"D:\tools\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "NixieDongle_DCF77.elf" "NixieDongle_DCF77.eep" || exit 0
	"D:\tools\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "NixieDongle_DCF77.elf" > "NixieDongle_DCF77.lss"
	"D:\tools\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "NixieDongle_DCF77.elf" "NixieDongle_DCF77.srec"
	"D:\tools\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-size.exe" "NixieDongle_DCF77.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "NixieDongle_DCF77.elf" "NixieDongle_DCF77.a" "NixieDongle_DCF77.hex" "NixieDongle_DCF77.lss" "NixieDongle_DCF77.eep" "NixieDongle_DCF77.map" "NixieDongle_DCF77.srec" "NixieDongle_DCF77.usersignatures"
	