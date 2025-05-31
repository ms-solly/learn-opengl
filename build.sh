#!/bin/bash

# Compile C++ code with OpenGL/GLFW
g++ ./src/main.cpp -o my_program -lGL -lglfw -lGLEW

# Optional: Add error checking
if [ $? -eq 0 ]; then
  echo "Build successful! Run with: ./my_program"
else
  echo "Build failed!"
fi
