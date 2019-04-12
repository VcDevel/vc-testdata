CXXFLAGS := -Wall -Wextra -O2 -std=c++17 -march=native
LFLAGS := -lmpfr -lgmp

all: generate-with-mpfr

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LFLAGS)
