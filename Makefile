default: lib

ifdef LIBSM64_MUSL
  CC      := musl-gcc
  LDFLAGS := -lm -static -shared
else
  CC      := cc
  LDFLAGS := -lm -shared
endif
CFLAGS := -g -Wall -fPIC -DSM64_LIB_EXPORT

SRC_DIRS  := src src/decomp src/decomp/engine src/decomp/game src/decomp/mario src/decomp/tools
BUILD_DIR := build
DIST_DIR  := dist
ALL_DIRS  := $(addprefix $(BUILD_DIR)/,$(SRC_DIRS))

LIB_FILE   := $(DIST_DIR)/libsm64.so
LIB_H_FILE := $(DIST_DIR)/include/libsm64.h
TEST_FILE  := run-test

C_IMPORTED := src/decomp/mario/geo.inc.c src/decomp/mario/model.inc.c
H_IMPORTED := $(C_IMPORTED:.c=.h)
IMPORTED   := $(C_IMPORTED) $(H_IMPORTED)

C_FILES   := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)) $(C_IMPORTED)
O_FILES   := $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o))
DEP_FILES := $(O_FILES:.o=.d)

TEST_SRCS := test/main.c test/context.c test/level.c
TEST_OBJS := $(foreach file,$(TEST_SRCS),$(BUILD_DIR)/$(file:.c=.o))

ifeq ($(OS),Windows_NT)
  LIB_FILE := $(DIST_DIR)/sm64.dll
endif

DUMMY != mkdir -p $(ALL_DIRS) build/test src/decomp/mario $(DIST_DIR)/include


$(filter-out src/decomp/mario/geo.inc.c,$(IMPORTED)): src/decomp/mario/geo.inc.c
src/decomp/mario/geo.inc.c: ./import-mario-geo.py
	./import-mario-geo.py

$(BUILD_DIR)/%.o: %.c $(IMPORTED)
	@$(CC) $(CFLAGS) -MM -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<

$(LIB_FILE): $(O_FILES)
	$(CC) $(LDFLAGS) -o $@ $^

$(LIB_H_FILE): src/libsm64.h
	cp -f $< $@


test/level.c test/level.h: ./import-test-collision.py
	./import-test-collision.py

test/main.c: test/level.h

$(BUILD_DIR)/test/%.o: test/%.c
	@$(CC) $(CFLAGS) -MM -MP -MT $@ -MF $(BUILD_DIR)/test/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<

$(TEST_FILE): $(LIB_FILE) $(TEST_OBJS)
	$(CC) -o $@ $(TEST_OBJS) $(LIB_FILE) -lGLEW -lGL -lSDL2 -lSDL2main -lm


lib: $(LIB_FILE) $(LIB_H_FILE)

test: $(TEST_FILE) $(LIB_H_FILE)

run: test
	./$(TEST_FILE)

clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR) src/decomp/mario test/level.? $(TEST_FILE)

-include $(DEP_FILES)
