# Makefile for compiling fft_analyzer

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -O2

# Libraries
LIBS = -lsndfile -lm

# Source files
SRC = fourier_threads.cpp

# Output executable
TARGET = fourier_threads

# Build target
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# Clean target
clean:
	rm -f $(TARGET) *.o

# Phony targets
.PHONY: all clean
