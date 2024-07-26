SRC_DIR = src
BUILD_DIR = build
LIBS_NAMES_ARGS = -llink_core -lmiddleware_implementation -lplatform_port -lutils_modules -lubus -lubox -lblobmsg_json

SOURCES := $(shell find $(SRC_DIR) -name '*.c')
OBJECTS := $(patsubst $(SRC_DIR)/%.c, %.o, $(SOURCES))
EXECUTABLE := tuyad

vpath %.o $(BUILD_DIR)
vpath %.c $(SRC_DIR)
vpath $(EXECUTABLE) $(BUILD_DIR)

CFLAGS += -g -Wall
CPPFLAGS += -I$(SRC_DIR)
LDFLAGS += $(LIBS_NAMES_ARGS)

all: $(BUILD_DIR)/$(EXECUTABLE)

$(BUILD_DIR)/$(EXECUTABLE): OBJECT_FILES := $(foreach object, $(OBJECTS), $(BUILD_DIR)/$(object))
$(BUILD_DIR)/$(EXECUTABLE): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(OBJECT_FILES) -o $(BUILD_DIR)/$(EXECUTABLE) $(LDFLAGS)

$(OBJECTS): %.o: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $(BUILD_DIR)/$@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	-rm -r $(BUILD_DIR) 2>/dev/null


.PHONY: all clean
