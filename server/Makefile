CFLAGS=-Wall -g
LDFLAGS=-Wall -g 

COMPILE=g++ $(CFLAGS) -std=c++11  -c
LINK=g++ $(LDFLAGS) -o 
SRCDIR=.
OBJDIR=../obj
BINDIR=../bin
OBJ=$(OBJDIR)/main.o $(OBJDIR)/myserver.o $(OBJDIR)/memmap.o
BIN=$(BINDIR)/server

all: $(BIN)
$(BIN): $(OBJ) 
	$(LINK) $@ $(OBJ) -lrt
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(COMPILE) $< -o $@
clean: 
	rm -f $(OBJ) $(BIN)
