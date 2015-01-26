CCP=g++
#CFLAGS=-g -W -Wall -O0 -DDEBUG -std=c++11
CFLAGS=-W -Wall -O3 -std=c++11
LIBS=-lsdsl -ldivsufsort -ldivsufsort64 -lboost_filesystem -lboost_system
FILES= CR.cpp util.cpp fm_wrapper.cpp

default:
	$(CCP) $(CFLAGS) -w -o CR_example CR_example.cpp $(FILES) $(LIBS)
	
clean:
	rm -f *~ *.o CR_example
