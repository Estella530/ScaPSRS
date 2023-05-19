CXX=mpic++

MAIN_SRC=main.cpp ScaPSRS.cpp

OTHER_SRC=utils/ProcessConfig.cpp utils/Log.cpp

all:
	$(CXX) -g -w -O3 $(MAIN_SRC) $(OTHER_SRC) -std=c++11 -o scapsrs
clean:
	rm scapsrs
