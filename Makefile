CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

TARGET  = prog
SOURCES = prog.cpp algorithms.cpp
OBJECTS = $(SOURCES:.cpp=.o)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean
