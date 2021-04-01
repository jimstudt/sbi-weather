#
# Path to my SDK installation
#
GUESS_SDK := $(firstword $(wildcard ~/coding/pico-sdk ~/Documents/coding/pico-sdk) )
PICO_SDK_PATH ?= $(GUESS_SDK)

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
	[ -d $(PICO_MOUNT) ] || mount $(PICO_MOUNT)
	cp $(IMAGE) $(PICO_MOUNT)/foo
	sync
	# will get an error for the disk going offline without an unmount
	# but I can't find a sequence of commands that will get it 
	# done without the error.

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

