PROJECT_PATH=$(realpath ./)
PROJECT_NAME=$(shell basename $(realpath ./))
PROJECT_BUILD_PATH=$(realpath BUILD/)
PROJECT_TOOLS_PATH=$(realpath tools/)
PROJECT_CONFIGS_PATH=$(realpath configs/)
PROJECT_TOOLCHAIN=GCC_ARM
PROJECT_SCM_BRANCH=$(shell git rev-parse --abbrev-ref HEAD)
PROJECT_SCM_HASH=$(shell git rev-parse HEAD)
-include .config

STACK_SIZE ?= 0x30000
HEAP_SIZE ?= 0x30000

$(info STACK_SIZE: ${STACK_SIZE}, HEAP_SIZE: ${HEAP_SIZE})

ALGO_DIR = Algorithms/
DRIVERS_DIR = Drivers/
INTERFACES_DIR = Interfaces/
SCOMMS_DIR = $(INTERFACES_DIR)/SensorComm/
DSINT_DIR = $(INTERFACES_DIR)/DeviceStudio/
SSCOMMS_DIR = $(SCOMMS_DIR)/SmartSensorComm/
SSINT_DIR = $(INTERFACES_DIR)/SmartSensor/
PLATFORMS_DIR = Platforms/

EXTERNAL_SOURCES += mbed-os
EXTERNAL_SOURCES += $(DRIVERS_DIR)USBDevice
PROJECT_SOURCES += $(wildcard *.cpp)
PROJECT_SOURCES += $(wildcard *.h)
PROJECT_SOURCES += $(SCOMMS_DIR)/SensorComm
PROJECT_SOURCES += $(DSINT_DIR)
PROJECT_SOURCES += $(DRIVERS_DIR)MaximSensor
PROJECT_SOURCES += Utilities

## BEGIN PLATFORM ##
ifeq ($(CONFIG_PLATFORM_MAX32600MBED), y)
CONFIG_FLAGS += PLATFORM_MAX32600MBED
CONFIG_PLATFORM_NAME=MAX32600MBED

else ifeq ($(CONFIG_PLATFORM_MAX32625MBED), y)
CONFIG_FLAGS += PLATFORM_MAX32625MBED
CONFIG_PLATFORM_NAME=MAX32625MBED
PROJECT_SOURCES += $(PLATFORMS_DIR)max32625mbed

else ifeq ($(CONFIG_PLATFORM_MAX32630FTHR), y)
CONFIG_FLAGS += PLATFORM_MAX32630FTHR
CONFIG_PLATFORM_NAME=MAX32630FTHR
EXTERNAL_SOURCES += $(PLATFORMS_DIR)max32630fthr

else ifeq ($(CONFIG_PLATFORM_MAX32630HSP), y)
CONFIG_FLAGS += PLATFORM_MAX32630HSP
CONFIG_PLATFORM_NAME=MAX32630FTHR
EXTERNAL_SOURCES += $(PLATFORMS_DIR)max32630hsp
PROJECT_SOURCES += $(PLATFORMS_DIR)max32630hsp

else ifeq ($(CONFIG_PLATFORM_MAX32625PICO), y)
CONFIG_FLAGS += PLATFORM_MAX32625PICO
CONFIG_PLATFORM_NAME=MAX32625PICO

else
$(warning NO MAXIM PLATFORM SELECTED)
endif
CONFIG_FLAGS += MAXIM_PLATFORM_NAME=\"$(CONFIG_MAXIM_PLATFORM_NAME)\"


ifeq ($(CONFIG_ENABLE_LED_STATUS), y)
CONFIG_FLAGS += ENABLE_LED_STATUS
endif

ifeq ($(CONFIG_ENABLE_EVENTSTATS), y)
CONFIG_FLAGS += ENABLE_EVENTSTATS
endif
## END PLATFORM ##

## BEGIN PPG SENSOR ##
ifeq ($(CONFIG_ENABLE_PPG), y)
CONFIG_FLAGS += ENABLE_PPG
PROJECT_SOURCES += $(SCOMMS_DIR)PpgComm/
endif

ifeq ($(CONFIG_PPG_MAX3011X), y)
CONFIG_FLAGS += PPG_MAX3011X
PROJECT_SOURCES += $(DRIVERS_DIR)MAX3011X
endif

ifeq ($(CONFIG_PPG_MAX8614X), y)
CONFIG_FLAGS += PPG_MAX8614X
PROJECT_SOURCES += $(DRIVERS_DIR)MAX8614X
endif
## END PPG SENSOR ##

## BEGIN ECG SENSOR ##
ifeq ($(CONFIG_ENABLE_ECG), y)
CONFIG_FLAGS += ENABLE_ECG
PROJECT_SOURCES += $(SCOMMS_DIR)EcgComm/
endif

ifeq ($(CONFIG_ECG_MAX30001), y)
CONFIG_FLAGS += ECG_MAX30001
PROJECT_SOURCES += $(DRIVERS_DIR)MAX30001
endif
## END PPG SENSOR ##

## BEGIN TEMP SENSOR
ifeq ($(CONFIG_ENABLE_TP), y)
CONFIG_FLAGS += ENABLE_TP
PROJECT_SOURCES += $(SCOMMS_DIR)TempComm/
endif

ifeq ($(CONFIG_TP_MAX30205), y)
CONFIG_FLAGS += TP_MAX30205
PROJECT_SOURCES += $(DRIVERS_DIR)MAX30205
endif
## END TEMP SENSOR

## BEGIN PRESSURE SENSOR
ifeq ($(CONFIG_ENABLE_BMP280), y)
CONFIG_FLAGS +=ENABLE_BMP280
PROJECT_SOURCES += $(DRIVERS_DIR)BMP280
endif
## END PRESSURE SENSOR

## BEGIN FLASH CONFIGURATION
ifeq ($(CONFIG_ENABLE_MX25U), y)
CONFIG_FLAGS +=ENABLE_MX25U
PROJECT_SOURCES += $(DRIVERS_DIR)SPIBlockDevice
$(info flash driver is included)
endif
## END FLASH CONFIGURATION

## BEGIN USBMSD CONFIGURATION
ifeq ($(CONFIG_ENABLE_USBMSD), y)
CONFIG_FLAGS +=ENABLE_USBMSD
PROJECT_SOURCES += $(DRIVERS_DIR)USBMSD_BD
$(info USBMSD feature enabled)
endif
## END USBMSD CONFIGURATION

## BEGIN ACCEL ##
ifeq ($(CONFIG_ENABLE_ACCEL), y)
CONFIG_FLAGS += ENABLE_ACCEL
PROJECT_SOURCES += $(DRIVERS_DIR)BMI160
endif

ifeq ($(CONFIG_ACCEL_BMA160), y)
CONFIG_FLAGS += ACCEL_BMA160
PROJECT_SOURCES += $(DRIVERS_DIR)BMA160
endif

ifeq ($(CONFIG_ACCEL_BMA280), y)
CONFIG_FLAGS += ACCEL_BMA280
PROJECT_SOURCES += $(DRIVERS_DIR)BMA280
endif

ifeq ($(CONFIG_ACCEL_LIS2DH), y)
CONFIG_FLAGS += ACCEL_LIS2DH
PROJECT_SOURCES += $(DRIVERS_DIR)LIS2DH
endif
## END ACCEL ##

## BEGIN SMARTSENSOR ##
ifeq ($(CONFIG_ENABLE_SMARTSENSOR), y)
CONFIG_FLAGS += ENABLE_SMARTSENSOR
PROJECT_SOURCES += $(SSINT_DIR)
endif

ifeq ($(CONFIG_ENABLE_SS_BOOTLOADER), y)
CONFIG_FLAGS += ENABLE_SS_BOOTLOADER
PROJECT_SOURCES += $(SSCOMMS_DIR)SSBootloader
endif

ifeq ($(CONFIG_ENABLE_SS_MAX30101), y)
CONFIG_FLAGS += ENABLE_SS_MAX30101
PROJECT_SOURCES += $(SSCOMMS_DIR)SSMAX30101
endif

ifeq ($(CONFIG_ENABLE_SS_MAX8614X), y)
CONFIG_FLAGS += ENABLE_SS_MAX8614X
PROJECT_SOURCES += $(SSCOMMS_DIR)SSMAX8614X
endif
## END SMARTSENSOR ##

## BEGIN SMARTSENSOR ##
ifeq ($(CONFIG_BOOTLOADER_USES_MFIO), y)
CONFIG_FLAGS += BOOTLOADER_USES_MFIO
endif

## BEGIN DISPLAY ##
ifeq ($(CONFIG_ENABLE_DISPLAY), y)
CONFIG_FLASG += ENABLE_DISPLAY
endif

ifeq ($(CONFIG_OLED_ADAFRUIT_SSD1306), y)
CONFIG_FLAGS += OLED_ADAFRUIT_SSD1306
EXTERNAL_SOURCES += $(DRIVERS_DIR)Adafruit_FeatherOLED
endif
ifeq ($(CONFIG_LCD_WATCHINTERFACE), y)
CONFIG_FLAGS += LCD_WATCHINTERFACE
endif
ifeq ($(CONFIG_LCD_LS013B7DH03), y)
CONFIG_FLAGS += LCD_LS013B7DH03
PROJECT_SOURCES += $(DRIVERS_DIR)LS013B7DH03
endif
## END DISPLAY ##

## BEGIN ALGORITHMS ##
#ifeq ($(CONFIG_ENABLE_WHRM), y)
#CONFIG_FLAGS += ENABLE_WHRM
#PROJECT_SOURCES += $(ALGO_DIR)WHRM
#endif

ifeq ($(CONFIG_ENABLE_WHRM_AND_SPO2), y)
CONFIG_FLAGS += ENABLE_WHRM_AND_SPO2
#PROJECT_SOURCES += $(ALGO_DIR)WHRM
endif

#ifeq ($(CONFIG_ENABLE_BPT), y)
#CONFIG_FLAGS += ENABLE_BPT
#PROJECT_SOURCES += $(ALGO_DIR)WHRM
#endif
## END ALGORITHMS ##

## SPO2 ##
ifeq ($(CONFIG_ENABLE_SPO2), y)
CONFIG_FLAGS += ENABLE_SPO2
endif
## END ##


## BEGIN BLE ##
ifeq ($(CONFIG_ENABLE_BLE), y)
CONFIG_FLAGS += ENABLE_BLE
PROJECT_SOURCES += $(DRIVERS_DIR)BLE_ICARUS

#ifeq ("$(CONFIG_BLE_ADV_NAME)", "")
#$(error BLE Avertisement name must be specified with CONFIG_BLE_ADV_NAME)
#else
#CONFIG_FLAGS += BLE_ADV_NAME=\"$(CONFIG_BLE_ADV_NAME)\"
#endif

ifeq ($(CONFIG_USE_BLE_TICKER_TO_CHECK_TRANSFER), y)
CONFIG_FLAGS += USE_BLE_TICKER_TO_CHECK_TRANSFER
endif

endif
## END BLE ##

## BEGIN DATA STREAMING DEBUG ##
ifeq ($(CONFIG_ASCII_COMM), y)
CONFIG_FLAGS += ASCII_COMM
endif

ifeq ($(CONFIG_ENABLE_BLETEST_COMM), y)
CONFIG_FLAGS += ENABLE_BLETEST_COMM
PROJECT_SOURCES += $(SCOMMS_DIR)BleTestComm
endif

ifeq ($(CONFIG_ENABLE_SS_GENERICCMDCOMM), y)
CONFIG_FLAGS += ENABLE_SS_GENERICMDCOMM
PROJECT_SOURCES += $(SSCOMMS_DIR)SSGenericCmd
endif
## END DATA STREAMING DEBUG ##

## BEGIN TEST FIRMWARE
ifeq ($(CONFIG_ENABLE_TESTCOMM), y)
CONFIG_FLAGS += ENABLE_TESTCOMM
PROJECT_SOURCES += $(SCOMMS_DIR)TestComm
endif
# END TEST FIRMWARE

## BEGIN MEMORY DEBUG ##
ifeq ($(CONFIG_ENABLE_MEMORY_DEBUG), y)
CONFIG_FLAGS += ENABLE_MEMORY_DEBUG
endif

ifeq ($(CONFIG_MBED_HEAP_STATS_ENABLED), y)
CONFIG_FLAGS += MBED_HEAP_STATS_ENABLED
endif

ifeq ($(CONFIG_MBED_MEM_TRACING_ENABLED), y)
CONFIG_FLAGS += MBED_MEM_TRACING_ENABLED
endif

ifeq ($(CONFIG_MBED_STACK_STATS_ENABLED), y)
CONFIG_FLAGS += MBED_STACK_STATS_ENABLED
endif
## END MEMORY DEBUG ##


## BEGIN DEBUG MSGS TOGGLE
ifeq ($(CONFIG_SHOW_DEBUG_MSGS), y)
CONFIG_FLAGS += SHOW_DEBUG_MSGS
endif
ifeq ($(CONFIG_SHOW_INFO_MSGS), y)
CONFIG_FLAGS += SHOW_INFO_MSGS
endif
ifeq ($(CONFIG_SHOW_I2C_DEBUG_MESSAGES), y)
CONFIG_FLAGS += SHOW_I2C_DEBUG_MESSAGES
endif

ifeq ($(V), 1)
BUILD_VERBOSE = -v
endif

SRC_ARGS += $(addprefix --source $(PROJECT_PATH)/, $(EXTERNAL_SOURCES))
SRC_ARGS += $(addprefix --source $(PROJECT_PATH)/, $(PROJECT_SOURCES))
CFG_ARGS = $(addprefix -D, $(CONFIG_FLAGS))

###$(info SELECTED_CONFIG=$(CONFIG_CONFIG_NAME))
$(info CONFIG_PLATFORM_NAME=$(CONFIG_PLATFORM_NAME) CONFIG_MAXIM_PLATFORM_NAME=$(CONFIG_MAXIM_PLATFORM_NAME) CONFIG_FLAGS=$(CONFIG_FLAGS))
$(info PROJECT_SOURCES=$(PROJECT_SOURCES))
$(info EXTERNAL_SOURCES=$(EXTERNAL_SOURCES))
$(info PROJECT_SCM=$(PROJECT_SCM_BRANCH) ($(PROJECT_SCM_HASH)))

## BEGIN MAKEFILE RULES ##
all: pre_build
###	@if [ -f $(CONFIG_CONFIG_NAME) ]; then \
###		printf "NO CONFIG SELECTED!\n"; \
###		exit 1; \
###	fi;
	@if [ ! -f .checkout ]; then \
		$(MAKE) checkout ; \
	fi;
	mbed compile -t $(PROJECT_TOOLCHAIN) -m $(CONFIG_PLATFORM_NAME) -D__STACK_SIZE=$(STACK_SIZE) -D__HEAP_SIZE=$(HEAP_SIZE) $(BUILD_VERBOSE) $(CFG_ARGS) $(SRC_ARGS)
debug_build:
	mbed compile -t $(PROJECT_TOOLCHAIN) -m $(CONFIG_PLATFORM_NAME) -D__STACK_SIZE=$(STACK_SIZE) -D__HEAP_SIZE=$(HEAP_SIZE) $(BUILD_VERBOSE) $(CFG_ARGS) $(SRC_ARGS) --profile $(PROJECT_PATH)/mbed-os/tools/profiles/debug.json

clean_build:
	mbed compile -t $(PROJECT_TOOLCHAIN) -m $(CONFIG_PLATFORM_NAME) -D__STACK_SIZE=$(STACK_SIZE) -D__HEAP_SIZE=$(HEAP_SIZE) -c $(BUILD_VERBOSE) $(CFG_ARGS) $(SRC_ARGS)

werror_build:
	mbed compile -t $(PROJECT_TOOLCHAIN) -m $(CONFIG_PLATFORM_NAME) -D__STACK_SIZE=$(STACK_SIZE) -D__HEAP_SIZE=$(HEAP_SIZE) -c $(BUILD_VERBOSE) $(CFG_ARGS) $(SRC_ARGS) --profile $(PROJECT_PATH)/werror_build_profile.json

clean:
	rm -rf $(PROJECT_BUILD_PATH)

menuconfig:
	${PROJECT_TOOLS_PATH}/windows_kconfig/kconfig-mconf.exe Kconfig

run:
	python ${PROJECT_TOOLS_PATH}/mbed_run.py ${PROJECT_BUILD_PATH}/${CONFIG_PLATFORM_NAME}/$(PROJECT_TOOLCHAIN)/mbed-os.bin

co: checkout

checkout:
	@echo "Downloading libraries"
	mbed deploy
	$(MAKE) apply_mbedos_patches
	@touch .$@

applypatches:
	@echo "Applying patches"
#	cd mbed-os; \
#	git am ../patches/0001-porting-MAX32625PICO-to-mbed-os-5.patch
#	cd USBDevice; \
#	hg import --no-commit --force ../patches/USBDevice.patch

apply_mbedos_patches:
	@echo "Applying mbed-os patches"
	@cd ./mbed_os_patches; \
	for i in *.patch ; do \
		[ -f "$$i" ] || break ; \
		echo "$$i"; \
		cp "$$i" ../mbed-os; \
		cd ../mbed-os; \
		git apply "$$i"; \
		rm -f "$$i"; \
		cd ..; \
		cd ./mbed_os_patches; \
	done; \
	
selectconfig.%:
	@if [ -f $(PROJECT_CONFIGS_PATH)/$(word 2,$(subst ., ,$@)).config ]; then \
		cp $(PROJECT_CONFIGS_PATH)/$(word 2,$(subst ., ,$@)).config $(PROJECT_PATH)/.config; \
		printf "\n$(word 2,$(subst ., ,$@)).config selected\n"; \
	else \
		printf "\nConfig [$(word 2,$(subst ., ,$@))] is not found!\n"; \
		printf "Available configs are:\n\n"; \
		ls $(PROJECT_CONFIGS_PATH) | cut -d "." -f 1; \
		printf "\n"; \
	fi;

saveconfig:
###	cp $(PROJECT_PATH)/.config $(PROJECT_CONFIGS_PATH)/$(CONFIG_CONFIG_NAME).config

staticcheck:
	@printf "\n\nRATS-2.4 Staic Analyzer\n"
	@printf "=========== BEGIN =============\n"
	${PROJECT_TOOLS_PATH}/Rats-2.4/rats.exe --quiet -w 3 --db ${PROJECT_TOOLS_PATH}/Rats-2.4/rats-c.xml $(PROJECT_SOURCES)
	@printf "============ END ==============\n"
	@printf "\n\nCppcheck-1.78 Static Analyzer\n"
	@printf "=========== BEGIN =============\n"
	${PROJECT_TOOLS_PATH}/Cppcheck-1.78/cppcheck.exe -q --enable=all --force $(PROJECT_SOURCES)
	@printf "============ END ==============\n"
	@printf "\n\nVisualCodeGrepper-2.1.0 Static Analyzer\n"
	@printf "=========== BEGIN =============\n"
	@cd ${PROJECT_TOOLS_PATH}/VisualCodeGrepper-2.1.0; \
	for src in $(PROJECT_SOURCES) ; do \
		${PROJECT_TOOLS_PATH}/VisualCodeGrepper-2.1.0/VisualCodeGrepper.exe -c -t $(PROJECT_PATH)/$$src -r $(PROJECT_PATH)/.tmp_VCG_step.txt ; \
		echo "" >> $(PROJECT_PATH)/.tmp_VCG_result.txt ; \
		echo "$$src:" >> $(PROJECT_PATH)/.tmp_VCG_result.txt ; \
		cat $(PROJECT_PATH)/.tmp_VCG_step.txt >> $(PROJECT_PATH)/.tmp_VCG_result.txt ; \
		rm -rf $(PROJECT_PATH)/.tmp_VCG_step.txt ;\
	done ; \
	cat $(PROJECT_PATH)/.tmp_VCG_result.txt ; \
	rm -rf $(PROJECT_PATH)/.tmp_VCG_result.txt
	@printf "============ END ==============\n"

doxygen:
	mkdir -p $(PROJECT_BUILD_PATH)
	export PROJECT_SOURCES="$(PROJECT_SOURCES)"; \
	export PROJECT_SCM_HASH="$(PROJECT_SCM_HASH)"; \
	doxygen doxygen.conf
	start $(PROJECT_BUILD_PATH)/doc/html/index.html

astyle:
	@printf "=========== BEGIN ASTYLE =============\n"
	${PROJECT_TOOLS_PATH}/AStyle_2.06_windows/bin/AStyle.exe --options=${PROJECT_TOOLS_PATH}/AStyle_2.06_windows/file/custom.ini "*.cpp";
	${PROJECT_TOOLS_PATH}/AStyle_2.06_windows/bin/AStyle.exe --options=${PROJECT_TOOLS_PATH}/AStyle_2.06_windows/file/custom.ini "*.h";
	@for src in $(PROJECT_SOURCES); do \
		if [[ -d $$src ]]; then \
			${PROJECT_TOOLS_PATH}/AStyle_2.06_windows/bin/AStyle.exe --options=${PROJECT_TOOLS_PATH}/AStyle_2.06_windows/file/custom.ini --recursive "$$src/*.cpp" ; \
			${PROJECT_TOOLS_PATH}/AStyle_2.06_windows/bin/AStyle.exe --options=${PROJECT_TOOLS_PATH}/AStyle_2.06_windows/file/custom.ini --recursive "$$src/*.h" ; \
		fi; \
	done;
	@printf "============ END ASTYLE ==============\n"
	
help:
	start https://confluence.maxim-ic.com/x/VSAkB
	
create_flags:
	python ./scripts/py/mbed_convertconfigtoflags.py

pre_build:
	@echo "#ifndef __BUILD_VERSION_H__" > build_version.h
	@echo "#define __BUILD_VERSION_H__" >> build_version.h
	@echo "#define BUILD_SOURCE_BRANCH \"$(PROJECT_SCM_BRANCH)\"" >> build_version.h
	@echo "#define BUILD_SOURCE_HASH \"$(PROJECT_SCM_HASH)\"" >> build_version.h
	@echo "#endif" >> build_version.h
## END MAKEFILE RULES ##
