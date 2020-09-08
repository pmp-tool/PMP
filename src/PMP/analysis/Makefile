ROOT=$(HOME)/PMP
DYNINST_ROOT=$(ROOT)/Dyninst

CXX = g++ -g
CXXFLAGS = -Wall -std=c++11
LIBFLAGS = -fpic -shared

all: analysis

analysis: analysis.o
	$(CXX) $(CXXFLAGS) -o analysis analysis.o \
		-L$(DYNINST_ROOT)/exe/lib -L$(DYNINST_ROOT)/build/tbb/lib -L$(DYNINST_ROOT)/build/elfutils/lib \
		-ldyninstAPI -lsymtabAPI -linstructionAPI -lparseAPI \
		-lboost_system -lcommon -ldw \
		-ltbb -ltbbmalloc -ltbbmalloc_proxy

analysis.o: analysis.cpp
	$(CXX) $(CXXFLAGS) -I$(DYNINST_ROOT)/exe/include -I$(DYNINST_ROOT)/build/tbb/include -I$(DYNINST_ROOT)/build/boost/src/boost -c analysis.cpp

clean:
	rm -f analysis *.o
