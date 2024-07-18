LIBS_TUYA_PATH = lib/tuya-iot-core-sdk/build/lib
LIBS_TUYA = $(wildcard $(LIBS_TUYA_PATH)/*.so)

LIBS_NAMES = $(notdir $(LIBS_TUYA))
LIBS_NAMES_ARGS = $(foreach name, $(LIBS_NAMES), $(name:lib%.so=-l%))
LIBS_PATHS = $(LIBS_TUYA_PATH)
LIBS_PATHS_ARGS = $(foreach path, $(LIBS_PATHS), -L$(path))

HEADERS := $(dir $(shell find . -name '*.h'))
HEADERS_CLEAN := $(foreach header, $(HEADERS), $(HEADER:%=%))
INC_DIRS := $(sort $(HEADERS_CLEAN))
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

SRC_DIR = src
BUILD_DIR = build

vpath %.o $(BUILD_DIR)
vpath %.c $(SRC_DIR)
vpath $(EXECUTABLE) $(BUILD_DIR)

SOURCES := $(shell find $(SRC_DIR) -name '*.c')
OBJECTS := $(patsubst $(SRC_DIR)/%.c, %.o, $(SOURCES))
EXECUTABLE := tuyad

CFLAGS = -g -Wall
CPPFLAGS = $(INC_FLAGS)
EXEC_TUYA_ARGS = ndcqiihiovexazmo 26233c9f735d8fc3f7sj7d bZCBOQlhPjSmbSiS

all: $(EXECUTABLE)

$(EXECUTABLE): OBJECT_FILES := $(foreach object, $(OBJECTS), $(BUILD_DIR)/$(object))
$(EXECUTABLE): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LIBS_PATHS_ARGS) $(OBJECT_FILES) -o $(BUILD_DIR)/$(EXECUTABLE) $(LIBS_NAMES_ARGS)

$(OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $(BUILD_DIR)/$@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

run: all
	export LD_LIBRARY_PATH=$(shell pwd)/$(LIBS_TUYA_PATH) && ./$(BUILD_DIR)/$(EXECUTABLE) -D $(EXEC_TUYA_ARGS)

run-daemon: all
	export LD_LIBRARY_PATH=$(shell pwd)/$(LIBS_TUYA_PATH) && ./$(BUILD_DIR)/$(EXECUTABLE) $(EXEC_TUYA_ARGS)

clean:
	rm -r $(BUILD_DIR)

.PHONY: all run run-daemon clean