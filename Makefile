
RELATIVE_TOP = ./
TOP = $(realpath $(RELATIVE_TOP))

BIN_DIR = ./bin
OBJ_DIR = ./obj
SRC_DIR = ./src
INC_DIR = ./inc

CC=g++
RM = rm -fv
ECHO = echo

CFLAGS=-c -Wall -ggdb -O0 -std=c++11 -fdiagnostics-color
CFLAGS+= -I./inc -I/usr/include/alsa/
CFLAGS+= -D__MIDI_STUB
LDFLAGS= -lasound -lncurses -lpthread

# Note the order of source files
SOURCES= \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/Composition.cpp \
	$(SRC_DIR)/MidiOut.cpp \
	$(SRC_DIR)/MidiScore.cpp \
	$(SRC_DIR)/UserInterface.cpp \
	$(SRC_DIR)/utility.cpp

# Substitution of file endings
# All strings in SOURCES with ending of .cpp is substituted by .o
OBJECTS=$(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
EXECUTABLE=$(BIN_DIR)/sequencer

# Default make option
all: $(SOURCES) $(EXECUTABLE)

# Note the position of LDFLAGS    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

print-%:
	@echo '$*=$($*)'

clean:
	@$(RM) $(EXECUTABLE)
	@$(RM) $(OBJECTS)
