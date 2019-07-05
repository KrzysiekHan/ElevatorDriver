################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
Daniel_winda_v02.lss \

FLASH_IMAGE += \
Daniel_winda_v02.hex \

EEPROM_IMAGE += \
Daniel_winda_v02.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: Daniel_winda_v02.elf secondary-outputs

# Tool invocations
Daniel_winda_v02.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,Daniel_winda_v02.map -mmcu=atmega8 -o "Daniel_winda_v02.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

Daniel_winda_v02.lss: Daniel_winda_v02.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S Daniel_winda_v02.elf  >"Daniel_winda_v02.lss"
	@echo 'Finished building: $@'
	@echo ' '

Daniel_winda_v02.hex: Daniel_winda_v02.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -O ihex Daniel_winda_v02.elf  "Daniel_winda_v02.hex"
	@echo 'Finished building: $@'
	@echo ' '

Daniel_winda_v02.eep: Daniel_winda_v02.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex Daniel_winda_v02.elf  "Daniel_winda_v02.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: Daniel_winda_v02.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega8 Daniel_winda_v02.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(EEPROM_IMAGE)$(FLASH_IMAGE)$(ELFS)$(LSS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS) Daniel_winda_v02.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
