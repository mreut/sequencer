RELATIVE_TOP = ./
TOP = $(realpath $(RELATIVE_TOP))

CC = g++
RM = rm -fv
ECHO = echo

CFLAGS = -Wall -ggdb -O0 -std=c++11 -fdiagnostics-color
CFLAGS += -I../inc -I/usr/include/alsa/
LDFLAGS = -lasound -lncurses -lpthread

APP1 = ./bin/MulticastSocket_test
SRC1 = ./src/MulticastSocket_test.cpp ../src/MulticastSocket.cpp

all: $(APP1)

$(APP1) : $(SRC1) 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

print-%:
	@echo '$*=$($*)'

clean:
	@$(RM) ./bin/*
