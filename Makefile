CC=g++
SRCDIR=src
SRCEXT=cpp
BUILDDIR=build
EXAMPLESDIR=examples
BENCHMARKSDIR=benchmark

#CFLAGS=-g -W -Wall -O0 -DDEBUG -std=c++11
CFLAGS=-W -Wall -O3 -std=c++11 -Wno-unused-variable
#CFLAGS=-W -Wall -O2 -std=c++11 -pg -g

SOURCES=$(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS=$(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
EXAMPLES=$(patsubst $(EXAMPLESDIR)/%.$(SRCEXT),%,$(shell find $(EXAMPLESDIR) -type f -name *.$(SRCEXT)))
BENCHMARKS=$(patsubst $(BENCHMARKSDIR)/%.$(SRCEXT),%,$(shell find $(BENCHMARKSDIR) -type f -name *.$(SRCEXT)))
LIB=-lsdsl -ldivsufsort -ldivsufsort64 -lboost_filesystem -lboost_system
INC=-I include

all: $(OBJECTS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) bin/*"; $(RM) -r $(BUILDDIR) bin/*

examples: $(EXAMPLES)

benchmarks: $(BENCHMARKS)

$(EXAMPLES): $(OBJECTS)
	@echo " Building examples...";
	@echo " $(CC) $(CFLAGS) $^ -o bin/$@ $(EXAMPLESDIR)/$@.$(SRCEXT) $(INC) $(LIB)"; $(CC) $(CFLAGS) $^ -o bin/$@ $(EXAMPLESDIR)/$@.$(SRCEXT) $(INC) $(LIB)

$(BENCHMARKS): $(OBJECTS)
	@echo " Building benchmarks...";
	@echo " $(CC) $(CFLAGS) $^ -o bin/$@ $(BENCHMARKSDIR)/$@.$(SRCEXT) $(INC) $(LIB) -lGkArrays -lz"; $(CC) $(CFLAGS) $^ -o bin/$@ $(BENCHMARKSDIR)/$@.$(SRCEXT) $(INC) $(LIB) -lGkArrays -lz

correctness: $(OBJECTS)
	@echo " $(CC) $(CFLAGS) $^ -o bin/$@ tools/$@.$(SRCEXT) $(INC) $(LIB)"; $(CC) $(CFLAGS) $^ -o bin/$@ tools/$@.$(SRCEXT) $(INC) $(LIB)
