LIBS_TUYA_PATH = lib/tuya-iot-core-sdk/build/lib
LIBS_TUYA = $(wildcard $(LIBS_TUYA_PATH)/*.so)
INCLUDES_TUYA_PATH = lib/tuya-iot-core-sdk/include

LIBS_NAMES = $(notdir $(LIBS_TUYA))
LIBS_NAMES_ARGS = $(foreach name, $(LIBS_NAMES), $(name:lib%.so=-l%))
LIBS_PATHS = $(LIBS_TUYA_PATH)
LIBS_PATHS_ARGS = $(foreach path, $(LIBS_PATHS), -L$(path))

INCLUDE_DIRS = $(INCLUDES_TUYA_PATH) includetmp
INCLUDE_DIRS_ARGS = $(foreach dir, $(INCLUDE_DIRS), -I$(dir))


CFLAGS = -g -Wall
CPPFLAGS = $(INCLUDE_DIRS_ARGS)

all:
	gcc $(CFLAGS) $(CPPFLAGS) $(LIBS_PATHS_ARGS) main.c tuya.c -o tuyad $(LIBS_NAMES_ARGS)
	export LD_LIBRARY_PATH=$(shell pwd)/$(LIBS_TUYA_PATH)