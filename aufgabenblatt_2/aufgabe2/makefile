# Makefile for compiling fft_analyzer

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -O2

# Libraries
LIBS = 

# Source files
SRC = generate_wav.cpp

# Output executable
TARGET = generate_wav

# Build target
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# Clean target
clean:
	rm -f $(TARGET) *.o

# Phony targets
.PHONY: all clean
