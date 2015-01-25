CCP=g++
#CFLAGS=-g -W -Wall -O0 -DDEBUG -std=c++11
CFLAGS=-W -Wall -O3 -std=c++11
LIBS=-lsdsl -ldivsufsort -ldivsufsort64 -lboost_filesystem -lboost_system

default:
	$(CCP) $(CFLAGS) -w -o CR_example CR_example.cpp CR.cpp $(LIBS)
	
clean:
	rm -f *~ *.o
