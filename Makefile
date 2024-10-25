SHELL = /bin/sh 
.SUFFIXES:

# Build options
BUILD_CFG ?= release
SHARED_LIB ?= 1
CFLAGS_EXTRA ?=
LDFLAGS_EXTRA ?=

# Build settings
CFLAGS = -Wall -Wextra -std=c17 -I$(srcdir)/include
ifeq ($(BUILD_CFG),debug)
CFLAGS += -g -O0 -fanalyzer -Wno-analyzer-malloc-leak -DDEBUG
endif

# Directory definitions
srcdir = .
prefix = /usr/local
exec_prefix = $(prefix)
includedir = $(prefix)/include
libdir = $(exec_prefix)/lib

# Project configuration
PROJECT_NAME = cexcept
VERSION_MAJOR = $(shell sed -n -e 's/\#define CEXCEPT_VERSION_MAJOR \([0-9]*\)/\1/p' $(srcdir)/include/$(PROJECT_NAME).h)
VERSION_MINOR = $(shell sed -n -e 's/\#define CEXCEPT_VERSION_MINOR \([0-9]*\)/\1/p' $(srcdir)/include/$(PROJECT_NAME).h)
VERSION_PATCH = $(shell sed -n -e 's/\#define CEXCEPT_VERSION_PATCH \([0-9]*\)/\1/p' $(srcdir)/include/$(PROJECT_NAME).h)

# Source and output
SRC = $(srcdir)/src/$(PROJECT_NAME).c
HDR = $(srcdir)/include/$(PROJECT_NAME).h
OBJ = ./build/$(PROJECT_NAME).o
ifeq ($(SHARED_LIB),1)
LIB = ./output/lib$(PROJECT_NAME)-v$(VERSION_MAJOR)-$(VERSION_MINOR)-$(VERSION_PATCH).so
else
LIB = ./output/lib$(PROJECT_NAME)-v$(VERSION_MAJOR)-$(VERSION_MINOR)-$(VERSION_PATCH).a
endif
TEST_SRCS = $(wildcard $(srcdir)/test/*.c)
TESTS = $(patsubst $(srcdir)/test/%.c,./build/test/%.elf,$(TEST_SRCS))
TEST_OUTS = $(patsubst $(srcdir)/test/%.c,./build/test/%.out,$(TEST_SRCS))

# Build targets
.PHONY: clean

all: $(LIB)

test: $(TESTS)

runtest: $(TEST_OUTS)

./build/%.o : $(srcdir)/src/%.c $(HDR)
	@mkdir -p ./build
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA) -fPIC -c -o $@ $<

ifeq ($(SHARED_LIB),1)
$(LIB): $(OBJ)
	@mkdir -p ./output
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA) -shared $< -o $@ $(LDFLAGS_EXTRA)
else
$(LIB): $(OBJ)
	@mkdir -p ./output
	$(AR) rcs $@ $(OBJ) $(LDFLAGS_EXTRA)
endif

./build/test/%.elf : $(srcdir)/test/%.c $(LIB)
	@mkdir -p ./build/test
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA) -DCEXCEPT_VERBOSITY=3 -o $@ $< $(LIB) $(LDFLAGS_EXTRA)

./build/test/%.out : ./build/test/%.elf
	@$(srcdir)/script/runtest.sh $< $@

clean:
	rm -rf ./build
	rm -rf ./output
