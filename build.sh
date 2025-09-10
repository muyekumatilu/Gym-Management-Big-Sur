#!/bin/bash

# Navigate to build directory
cd build

# Clean and build
make clean && make GymManagement_gui

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "Build successful! Starting application..."
    cd ..
    ./build/GymManagement_gui
else
    echo "Build failed!"
    cd ..
    exit 1
fi