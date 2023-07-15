MAKEFLAGS += --no-builtin-rules

ENABLE_DEBUG := 1

ifeq ($(ENABLE_DEBUG), 1)
    DEBUG = -g
else
    DEBUG =
endif

CXX = g++ $(CXXSTD)
CXXSTD = -std=gnu++20
WARN = -Wall
CXXFLAGS = $(WARN) -Wextra $(DEBUG) -pedantic -I/usr/include/gdk-pixbuf-2.0
CXXFLAGS += $(shell pkg-config --cflags gtk+-3.0 --cflags opencv 2>/dev/null) $(shell pkg-config --cflags $(ALLPKGS) 2>/dev/null) -fPIC 
CXXFLAGS += -Wno-deprecated-enum-enum-conversion -w

LDFLAGS = -lgtk-3 -lgdk-3 -ljpeg $(shell pkg-config --libs gtk+-3.0 --libs opencv 2>/dev/null) $(shell pkg-config --libs $(ALLPKGS) 2>/dev/null)

IFACEPKGS = hidapi-libusb
DEPPKGS = opencv4 gtk+-3.0
ALLPKGS = $(IFACEPKGS) $(DEPPKGS)

INCLUDES = $(INCLUDES-$@)
LIBS = $(LIBS-$@)

INCLUDES-mintdeck.o = $(shell pkg-config --cflags $(ALLPKGS))
INCLUDES-decklib.o = $(shell pkg-config --cflags $(ALLPKGS))
INCLUDES-decklib.os = $(shell pkg-config --cflags $(ALLPKGS))
LIBS-mintdeck = $(shell pkg-config --libs $(ALLPKGS))
LIBS-decklib.so = $(shell pkg-config --libs $(ALLPKGS))

# Source files and objects
SRC = src/main.cpp src/mintdeck.cpp src/decklib.cpp src/keymap.cpp src/deck.cpp src/test.cpp
OBJ = obj/main.o obj/mintdeck.o obj/decklib.o obj/keymap.o obj/deck.o obj/test.o

# Library names and paths
LIB_NAME = decklibrary
LIB_STATIC = lib$(LIB_NAME).a
LIB_DIR = lib

# Targets
all: static mintdeck

static: $(LIB_STATIC)

mintdeck: obj/main.o obj/mintdeck.o $(LIB_STATIC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

package: static
	mkdir -p dist/$(LIB_DIR)
	cp $(LIB_STATIC) $(LIB_DIR)
	tar czvf dist/$(LIB_NAME).tar.gz $(LIB_DIR)

# Static library
$(LIB_STATIC): $(OBJ)
	ar rcs $@ $^

# Object files
obj/main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/mintdeck.o: src/mintdeck.cpp src/mintdeck.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/decklib.o: src/decklib.cpp src/decklib.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/keymap.o: src/keymap.cpp src/keymap.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/deck.o: src/deck.cpp src/deck.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/test.o: src/test.cpp src/test.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(LIB_STATIC) mintdeck
	rm -rf dist
	rm -rf doc/html

.PHONY: all static mintdeck package clean
