default: lib

ifdef LIBSM64_MUSL
  CC      := musl-gcc
  CXX     := musl-g++
  LDFLAGS := -lm -static -shared
else
  CC      := cc
  CXX     := c++
  LDFLAGS := -lm -shared
endif
CFLAGS := -fno-strict-aliasing -g -Wall -Wno-unused-function -fPIC -fvisibility=hidden -DSM64_LIB_EXPORT -DGBI_FLOATS -DVERSION_US -DNO_SEGMENTED_MEMORY

SRC_DIRS  := src src/decomp src/decomp/engine src/decomp/include/PR src/decomp/game src/decomp/pc src/decomp/pc/audio src/decomp/mario src/decomp/tools src/decomp/audio
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

TEST_SRCS_C   := test/context.c test/level.c test/gl33core/gl33core_renderer.c test/gl20/gl20_renderer.c
TEST_SRCS_CPP := test/main.cpp test/audio.cpp
TEST_OBJS     := $(foreach file,$(TEST_SRCS_C),$(BUILD_DIR)/$(file:.c=.o)) $(foreach file,$(TEST_SRCS_CPP),$(BUILD_DIR)/$(file:.cpp=.o))

ifeq ($(OS),Windows_NT)
  LIB_FILE := $(DIST_DIR)/sm64.dll
  TEST_FILE := $(DIST_DIR)/run-test.exe
endif

DUMMY := $(shell mkdir -p $(ALL_DIRS) build/test build/test/gl33core build/test/gl20 src/decomp/mario $(DIST_DIR)/include)


$(filter-out src/decomp/mario/geo.inc.c,$(IMPORTED)): src/decomp/mario/geo.inc.c
src/decomp/mario/geo.inc.c: ./import-mario-geo.py
	./import-mario-geo.py

$(BUILD_DIR)/%.o: %.c $(IMPORTED)
	@$(CC) $(CFLAGS) -I src/decomp/include -MM -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CC) -c $(CFLAGS) -I src/decomp/include -o $@ $<

$(BUILD_DIR)/%.o: %.cpp $(IMPORTED)
	@$(CXX) $(CFLAGS) -I src/decomp/include -MM -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CXX) -c $(CFLAGS) -I src/decomp/include -o $@ $<

$(LIB_FILE): $(O_FILES)
	$(CC) $(LDFLAGS) -o $@ $^

$(LIB_H_FILE): src/libsm64.h
	cp -f $< $@

test/level.c: ./import-test-collision.py
	./import-test-collision.py

test/main.cpp test/gl20/gl20_renderer.c test/gl33core/gl33core_renderer.c: test/level.c

$(BUILD_DIR)/test/%.o: test/%.c
	@$(CC) $(CFLAGS) -MM -MP -MT $@ -MF $(BUILD_DIR)/test/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<

$(TEST_FILE): $(LIB_FILE) $(TEST_OBJS)
ifeq ($(OS),Windows_NT)
	$(CC) -o $@ $(TEST_OBJS) $(LIB_FILE) -lglew32 -lopengl32 -lSDL2 -lSDL2main -lm
else ifeq ($(shell uname -s),Darwin)
	$(CC) -o $@ $(TEST_OBJS) $(LIB_FILE) -framework OpenGL -lGLEW -lSDL2 -lSDL2main -lm -lpthread
else
	$(CC) -o $@ $(TEST_OBJS) $(LIB_FILE) -lGLEW -lGL -lSDL2 -lSDL2main -lm -lpthread
endif

lib: $(LIB_FILE) $(LIB_H_FILE)

test: $(TEST_FILE) $(LIB_H_FILE)

run: test
ifeq ($(OS),Windows_NT)
	cd dist && ./run-test
else
	./$(TEST_FILE)
endif

clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR) $(TEST_FILE)

-include $(DEP_FILES)
