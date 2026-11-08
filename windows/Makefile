CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -municode -DUNICODE -D_UNICODE
LDFLAGS := -mwindows -lcomdlg32 -lgdi32 -luser32
SRC_DIR := src
DIST_DIR := ../dist
TARGET := $(DIST_DIR)/DeviceMod.exe
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(SOURCES)
	@mkdir -p $(DIST_DIR)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
