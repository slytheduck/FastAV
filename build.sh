#!/bin/bash

# FastAV Build Script
# Usage: ./build.sh [clean|release|debug]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔════════════════════════════════════╗${NC}"
echo -e "${BLUE}║       FastAV Build Script          ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════╝${NC}"
echo

# Parse command line arguments
BUILD_TYPE="Release"
CLEAN=false

if [ "$1" == "clean" ]; then
    CLEAN=true
elif [ "$1" == "debug" ]; then
    BUILD_TYPE="Debug"
elif [ "$1" == "release" ]; then
    BUILD_TYPE="Release"
fi

# Clean build directory
if [ "$CLEAN" = true ] || [ "$1" == "clean" ]; then
    echo -e "${YELLOW}🧹 Cleaning build directory...${NC}"
    rm -rf build
    if [ "$1" == "clean" ]; then
        echo -e "${GREEN}✓ Clean complete${NC}"
        exit 0
    fi
fi

# Check dependencies
echo -e "${BLUE}🔍 Checking dependencies...${NC}"

check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}✗ $1 not found. Please install it.${NC}"
        exit 1
    else
        echo -e "${GREEN}✓ $1 found${NC}"
    fi
}

check_command cmake
check_command g++
check_command qmake6 || check_command qmake

# Check ClamAV
if ! command -v clamd &> /dev/null && ! command -v clamdscan &> /dev/null; then
    echo -e "${YELLOW}⚠ ClamAV not found. Install it for scanning functionality.${NC}"
    echo -e "${YELLOW}  Arch: sudo pacman -S clamav${NC}"
    echo -e "${YELLOW}  Ubuntu: sudo apt install clamav clamav-daemon${NC}"
else
    echo -e "${GREEN}✓ ClamAV found${NC}"
fi

echo

# Create build directory
echo -e "${BLUE}📁 Creating build directory...${NC}"
mkdir -p build
cd build

# Configure with CMake
echo -e "${BLUE}⚙️  Configuring with CMake ($BUILD_TYPE)...${NC}"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ CMake configuration failed${NC}"
    exit 1
fi

# Build
echo
echo -e "${BLUE}🔨 Building FastAV...${NC}"
CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
echo -e "${BLUE}   Using $CORES parallel jobs${NC}"

cmake --build . --config $BUILD_TYPE -j$CORES

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
fi

echo
echo -e "${GREEN}╔════════════════════════════════════╗${NC}"
echo -e "${GREEN}║     Build Completed Successfully!  ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════╝${NC}"
echo
echo -e "${BLUE}To run FastAV:${NC}"
echo -e "  ${YELLOW}cd build && ./fastav${NC}"
echo
echo -e "${BLUE}To install:${NC}"
echo -e "  ${YELLOW}cd build && sudo make install${NC}"
echo

# Check if clamd is running
if pgrep -x "clamd" > /dev/null; then
    echo -e "${GREEN}✓ clamd daemon is running${NC}"
else
    echo -e "${YELLOW}⚠ clamd daemon is not running. Start it with:${NC}"
    echo -e "  ${YELLOW}sudo systemctl start clamav-daemon${NC}"
fi

echo
