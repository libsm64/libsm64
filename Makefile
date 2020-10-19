default: all

CC      := cc
CFLAGS  := -g -Wall -fPIC
LDFLAGS := -lm -shared

SRC_DIRS  := src src/engine src/game src/mario src/tools
BUILD_DIR := build
DIST_DIR  := dist
ALL_DIRS  := $(addprefix $(BUILD_DIR)/,$(SRC_DIRS))

BIN_FILE   := $(DIST_DIR)/libsm64.so
LIB_H_FILE := $(DIST_DIR)/include/libsm64.h

C_FILES   := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
O_FILES   := $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o))
DEP_FILES := $(O_FILES:.o=.d)

DUMMY != mkdir -p $(ALL_DIRS)
DUMMY != mkdir -p $(DIST_DIR)/include

DUMMY != ./import-mario-geo.py >&2 || echo FAIL
ifeq ($(DUMMY),FAIL)
  $(error Script import-mario-geo.py failed)
endif

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -MM -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<

$(BIN_FILE): $(O_FILES)
	$(CC) $(LDFLAGS) -o $@ $^

dist/include/libsm64.h: src/libsm64.h
	cp -f $< $@

all: $(BIN_FILE) $(LIB_H_FILE)

clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR) src/mario

-include $(DEP_FILES)