#!/bin/bash

# Build script for makcu-cpp on Linux

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== MAKCU C++ Linux Build Script ===${NC}"

# Check for required dependencies
echo -e "${YELLOW}Checking dependencies...${NC}"

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake is not installed${NC}"
    echo "Install with: sudo apt-get install cmake"
    exit 1
fi

# Check for pkg-config
if ! command -v pkg-config &> /dev/null; then
    echo -e "${RED}Error: pkg-config is not installed${NC}"
    echo "Install with: sudo apt-get install pkg-config"
    exit 1
fi

# Check for libudev development files
if ! pkg-config --exists libudev; then
    echo -e "${RED}Error: libudev development files are not installed${NC}"
    echo "Install with: sudo apt-get install libudev-dev"
    exit 1
fi

# Check for build essentials
if ! command -v g++ &> /dev/null; then
    echo -e "${RED}Error: g++ compiler is not installed${NC}"
    echo "Install with: sudo apt-get install build-essential"
    exit 1
fi

echo -e "${GREEN}All dependencies found!${NC}"

# Create build directory
echo -e "${YELLOW}Creating build directory...${NC}"
mkdir -p build
cd build

# Run CMake configuration
echo -e "${YELLOW}Configuring project...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
echo -e "${YELLOW}Building project...${NC}"
make -j$(nproc)

echo -e "${GREEN}=== Build completed successfully! ===${NC}"
echo -e "${BLUE}Executable location: ${PWD}/bin/makcu-cpp${NC}"

# Test if the executable was created
if [ -f "bin/makcu-cpp" ]; then
    echo -e "${GREEN}✓ Executable built successfully${NC}"
else
    echo -e "${RED}✗ Executable not found${NC}"
    exit 1
fi

echo ""
echo -e "${BLUE}To run the demo:${NC}"
echo -e "${YELLOW}cd build && ./bin/makcu-cpp${NC}"