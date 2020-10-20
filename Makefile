default: test

CC      := cc
CFLAGS  := -g -Wall -fPIC
LDFLAGS := -lm -shared

SRC_DIRS  := src src/engine src/game src/mario src/tools test
BUILD_DIR := build
DIST_DIR  := dist
ALL_DIRS  := $(addprefix $(BUILD_DIR)/,$(SRC_DIRS))

LIB_FILE   := $(DIST_DIR)/libsm64.so
LIB_H_FILE := $(DIST_DIR)/include/libsm64.h
TEST_FILE  := run-test

C_IMPORTED := src/mario/geo.inc.c src/mario/model.inc.c
H_IMPORTED := $(C_IMPORTED:.c=.h)
IMPORTED   := $(C_IMPORTED) $(H_IMPORTED)

C_FILES   := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)) $(C_IMPORTED)
O_FILES   := $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o))
DEP_FILES := $(O_FILES:.o=.d)
TEST_OBJS := build/test/context.o build/test/level.o build/test/main.o

DUMMY != mkdir -p src/mario
DUMMY != mkdir -p $(ALL_DIRS)
DUMMY != mkdir -p $(DIST_DIR)/include

$(filter-out src/mario/geo.inc.c,$(IMPORTED)): src/mario/geo.inc.c
src/mario/geo.inc.c: ./import-mario-geo.py
	./import-mario-geo.py

test/level.c test/level.h: ./import-test-collision.py
	./import-test-collision.py

test/main.c: test/level.h

$(BUILD_DIR)/%.o: %.c $(IMPORTED)
	@$(CC) $(CFLAGS) -MM -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<

$(LIB_FILE): $(O_FILES)
	$(CC) $(LDFLAGS) -o $@ $^

$(LIB_H_FILE): src/libsm64.h
	cp -f $< $@

$(TEST_FILE): $(LIB_FILE) $(TEST_OBJS)
	$(CC) -o $@ $(TEST_OBJS) $(LIB_FILE) -lGLEW -lGL -lSDL2 -lSDL2main -lm

lib: $(LIB_FILE)

test: $(TEST_FILE)

run: test
	./$(TEST_FILE)

clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR) src/mario test/level.?

-include $(DEP_FILES)