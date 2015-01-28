CC=g++
SRCDIR=src
SRCEXT=cpp
BUILDDIR=build
EXAMPLESDIR=examples

CFLAGS=-g -W -Wall -O0 -DDEBUG -std=c++11
#CFLAGS=-W -Wall -O3 -std=c++11

SOURCES=$(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS=$(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
EXAMPLES=$(patsubst $(EXAMPLESDIR)/%.$(SRCEXT),%,$(shell find $(EXAMPLESDIR) -type f -name *.$(SRCEXT)))
LIB=-lsdsl -ldivsufsort -ldivsufsort64 -lboost_filesystem -lboost_system
INC=-I include
#TARGET=bin/CR_example

all: $(OBJECTS)

#$(TARGET): $(OBJECTS)
#	@echo " Linking..."
#	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) bin/*"; $(RM) -r $(BUILDDIR) bin/*

$(EXAMPLES): $(OBJECTS)
	@echo " Building examples...";
	@echo " $(CC) $(CFLAGS) $^ -o bin/$@ examples/$@.$(SRCEXT) $(INC) $(LIB)"; $(CC) $(CFLAGS) $^ -o bin/$@ examples/$@.$(SRCEXT) $(INC) $(LIB)

#CR_example: $(OBJECTS)
#	@echo " Linking..."
#	@echo " $(CC) $(CFLAGS) $^ -o bin/CR_example examples/CR_example.cpp $(INC) $(LIB)"
#	$(CC) $(CFLAGS) $^ -o bin/CR_example examples/CR_example.cpp $(INC) $(LIB)
