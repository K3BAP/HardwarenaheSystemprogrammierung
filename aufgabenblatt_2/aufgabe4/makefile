# Makefile for compiling fft_analyzer

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -O2

# Libraries
LIBS = -lsndfile -lm -lOpenCL

# Source files
SRC = fourier_opencl.cpp

# Output executable
TARGET = fourier_opencl

# Build target
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# Clean target
clean:
	rm -f $(TARGET) *.o

# Phony targets
.PHONY: all clean
