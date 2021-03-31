#
# Path to my SDK installation
#
PICO_SDK_PATH ?= ~/coding/pico-sdk

#
# Path to the mount point for my pico
#
PICO_MOUNT ?= /Volumes/RPI-RP2

#
# Path to my UF2 image in the build directory
#
IMAGE = build/sbi-weather.uf2

all : $(IMAGE)

install : all
	#mount $(PICO_MOUNT)
	cp $(IMAGE) $(PICO_MOUNT)/foo
	sync

clean :
	rm -rf build pico_sdk_import.cmake

build :
	mkdir build

$(IMAGE): build/Makefile
	( cd build ; make all )

build/Makefile : CMakeLists.txt pico_sdk_import.cmake | build
	( cd build ; cmake -DPICO_SDK_PATH=$(PICO_SDK_PATH) .. )

pico_sdk_import.cmake : $(PICO_SDK_PATH)/external/pico_sdk_import.cmake
	cp $< $@

.PHONY : all install clean $(IMAGE)

