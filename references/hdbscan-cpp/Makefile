SOURCES=$(shell find . -name "*.cpp")
CXXFLAGS= -g -O3 -std=c++11 -Wall $(CPPFLAGS)
OBJECTS=$(SOURCES:%.cpp=%.o)
TARGET=main

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LINK.cpp) $^ -std=c++11 $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -f $(OBJECTS)

