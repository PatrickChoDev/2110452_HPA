BUILD_DIR := build
CMAKE_BUILD_TYPE := Release
CMAKE_GENERATOR := Ninja
TARGET := vectorization_neon
CMAKE_PREFIX_PATH := -DCMAKE_PREFIX_PATH=/opt/homebrew

# Default target
all: $(BUILD_DIR)
	cmake --build $(BUILD_DIR) --target $(TARGET)

# Configure the build directory
$(BUILD_DIR):
	cmake  $(CMAKE_PREFIX_PATH) -S . -B $(BUILD_DIR) -G $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

# Build a specific target
build-%: $(BUILD_DIR)
	cmake --build $(BUILD_DIR) --target $*

# Run a specific executable
run-%: build-%
	./$(BUILD_DIR)/$*

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Reconfigure and rebuild
rebuild: clean all

# Phony targets
.PHONY: all clean rebuild build-% run-%