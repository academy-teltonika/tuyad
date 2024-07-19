LIB_TUYA_ROOT = lib/tuya-iot-core-sdk
LIBS_TUYA_PATH = $(LIB_TUYA_ROOT)/build/lib
LIBS_TUYA = $(wildcard $(LIBS_TUYA_PATH)/*.so)

LIBS_NAMES = $(notdir $(LIBS_TUYA))
LIBS_NAMES_ARGS = $(foreach name, $(LIBS_NAMES), $(name:lib%.so=-l%))
LIBS_PATHS = $(LIBS_TUYA_PATH)
LIBS_PATHS_ARGS = $(foreach path, $(LIBS_PATHS), -L$(path))

HEADERS := $(dir $(shell find . -name '*.h'))
HEADERS_CLEAN := $(sort $(HEADERS))
INC_DIRS = $(foreach header, $(HEADERS_CLEAN), $(header:%/=%))
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

SRC_DIR = src
BUILD_DIR = build

SOURCES := $(shell find $(SRC_DIR) -name '*.c')
OBJECTS := $(patsubst $(SRC_DIR)/%.c, %.o, $(SOURCES))
EXECUTABLE := tuyad

vpath %.o $(BUILD_DIR)
vpath %.c $(SRC_DIR)
vpath $(EXECUTABLE) $(BUILD_DIR)

CFLAGS := -g -Wall
CPPFLAGS := $(INC_FLAGS)

#----------------------------------------------------------------------------------
EXEC_TUYA_ARGS := -p @@@@@@@@@@@@@@ -d @@@@@@@@@@@@@ -s @@@@@@@@@@@@@@@@
#----------------------------------------------------------------------------------

all: $(BUILD_DIR)/$(EXECUTABLE) library

$(BUILD_DIR)/$(EXECUTABLE): OBJECT_FILES := $(foreach object, $(OBJECTS), $(BUILD_DIR)/$(object))
$(BUILD_DIR)/$(EXECUTABLE): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LIBS_PATHS_ARGS) $(OBJECT_FILES) -o $(BUILD_DIR)/$(EXECUTABLE) $(LIBS_NAMES_ARGS)

$(OBJECTS): %.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $(BUILD_DIR)/$@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

run: all
	export LD_LIBRARY_PATH=$(shell pwd)/$(LIBS_TUYA_PATH) && ./$(BUILD_DIR)/$(EXECUTABLE) $(EXEC_TUYA_ARGS)

run-daemon: all
	export LD_LIBRARY_PATH=$(shell pwd)/$(LIBS_TUYA_PATH) && ./$(BUILD_DIR)/$(EXECUTABLE) $(EXEC_TUYA_ARGS) -D

library:
	-cd $(LIB_TUYA_ROOT) && mkdir build 2>/dev/null
	cd $(LIB_TUYA_ROOT)/build && cmake .. && make

clean:
	-rm -r $(BUILD_DIR) 2>/dev/null

clean-library:
	-rm -r $(LIB_TUYA_ROOT)/build 2>/dev/null

.PHONY: all run run-daemon clean library clean-library