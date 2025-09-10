# Cross-platform Makefile for Gym Management System

# Compiler detection
CXX := g++
ifeq ($(shell which clang++ 2>/dev/null),)
    ifeq ($(shell which g++ 2>/dev/null),)
        $(error No suitable C++ compiler found. Please install g++ or clang++)
    endif
else
    CXX := clang++
endif

# Compiler flags
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS := -lsqlite3

# OS-specific settings
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -lpthread
endif
ifeq ($(UNAME_S),Darwin)
    # macOS - no additional flags needed
endif
ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
    # Windows with MinGW
    LDFLAGS := -lsqlite3
endif

# Directories
SRCDIR := src
INCDIR := include
OBJDIR := obj
DATADIR := data

# Source files
SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Target executable
TARGET := gym_management

# Default target
all: $(TARGET)

# Create target executable
$(TARGET): $(OBJECTS) | $(DATADIR)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete! Run with: ./$(TARGET)"

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create directories
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(DATADIR):
	mkdir -p $(DATADIR)

# Clean build files
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Clean everything including database
distclean: clean
	rm -rf $(DATADIR)

# Install dependencies (Linux/macOS)
install-deps:
ifeq ($(UNAME_S),Linux)
	@echo "Installing SQLite3 for Linux..."
	sudo apt-get update && sudo apt-get install -y libsqlite3-dev
endif
ifeq ($(UNAME_S),Darwin)
	@echo "Installing SQLite3 for macOS..."
	brew install sqlite3
endif

# Help
help:
	@echo "Available targets:"
	@echo "  all         - Build the project (default)"
	@echo "  clean       - Remove build files"
	@echo "  distclean   - Remove build files and database"
	@echo "  install-deps- Install SQLite3 dependencies"
	@echo "  help        - Show this help"

.PHONY: all clean distclean install-deps help