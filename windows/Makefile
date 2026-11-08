
CXX := g++

CXXFLAGS := -std=c++17 -O2 -Wall -Wextra \
            -municode -DUNICODE -D_UNICODE

LDFLAGS := -mwindows \
           -municode \
           -lcomdlg32 \
           -lgdi32 \
           -luser32 \
           -lshell32

SRC_DIR := src
DIST_DIR := dist

TARGET := $(DIST_DIR)/DeviceMod.exe

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(DIST_DIR)/%.o,$(SOURCES))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(DIST_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(DIST_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(DIST_DIR)
