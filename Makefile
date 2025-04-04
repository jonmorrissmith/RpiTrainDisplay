# Train Display - an RGB matrix departure board for the Raspberry Pi
# Makefile

# Directories
SRCDIR = Src
OBJDIR = Obj

# Compiler and flags
CXX = g++
# CXXFLAGS = -std=c++11 -O3 -Wall -Wextra -I/home/display/rpi-rgb-led-matrix/include -I$(SRCDIR)
CXXFLAGS = -std=c++11 -O3 -I/home/display/rpi-rgb-led-matrix/include -I$(SRCDIR)
LDFLAGS = -L/home/display/rpi-rgb-led-matrix/lib
LDLIBS = -lrgbmatrix -lcurl -lpthread

# Target executable
TARGET = traindisplay

# Source files (in Src directory)
SOURCES = $(SRCDIR)/api_client.cpp \
          $(SRCDIR)/config.cpp \
          $(SRCDIR)/display_text.cpp \
          $(SRCDIR)/traindisplay.cpp \
          $(SRCDIR)/train_service_display.cpp \
          $(SRCDIR)/train_service_parser.cpp

# Object files (maintained in separate directory)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

# Ensure obj directory exists
$(shell mkdir -p $(OBJDIR))

# Main rules
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Pattern rule for object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Special targets for testing
parser_test: $(OBJDIR)/parser_test.o $(OBJDIR)/train_service_parser.o $(OBJDIR)/api_client.o $(OBJDIR)/config.o $(OBJDIR)/display_text.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/parser_test.o: $(SRCDIR)/parser_test.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(TARGET) parser_test $(OBJDIR)/*.o
	rmdir $(OBJDIR) 2>/dev/null || true

# Phony targets
.PHONY: all clean
