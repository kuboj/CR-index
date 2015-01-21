CCP=g++
CFLAGS=-g -W -Wall -O0 -DDEBUG -std=c++11
#CFLAGS=-W -Wall -O3 -std=c++11

INCCDS=./libcds/includes/
INCDIVSUF=./libdivsufsort/build/include/

default: ./libdivsufsort/build/lib/libdivsufsort.a ./libcds/lib/libcds.a
	$(CCP) -I $(INCCDS) -I $(INCDIVSUF) $(CFLAGS) -w -o CR_example CR_example.cpp CR.cpp FM.cpp util.c libcds/lib/libcds.a libdivsufsort/build/lib/libdivsufsort.a -lboost_filesystem -lboost_system

./libcds/lib/libcds.a: 
	$(MAKE) -C libcds
	
libdivsufsort/build/lib/libdivsufsort.a: 
	cd libdivsufsort; mkdir build; cd build; cmake ..; $(MAKE); cd ..
	
clean:
	rm -f fmbuild fmcount fmlocate fmextract fmrecover libfmindex.a *~ *.o ;
	
cleanall:
	rm -f fmbuild fmcount fmlocate fmextract fmrecover libfmindex.a *~ *.o ; cd libdivsufsort; make -f Makefile clean; rm -rf build; cd .. ; cd libcds ; make -f Makefile clean ; cd ..
