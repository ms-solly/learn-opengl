#!/bin/bash

set -e  # Exit on first error

SRC="main.cpp"  # Or change to main.cpp if needed
OUT="ogl_app"

# Auto-detect C or C++
if [[ "$SRC" == *.cpp ]]; then
    COMPILER="g++"
    STD_FLAG="-std=c++17"
else
    COMPILER="gcc"
    STD_FLAG="-std=c99"
fi

# Optional: Validate GLSL shaders (for OpenGL, not Vulkan)
echo "[1/3] Validating GLSL shaders..."
shaders=(shaders/*.vert.glsl shaders/*.frag.glsl)
for shader in "${shaders[@]}"; do
    [[ -f "$shader" ]] && glslangValidator -G "$shader" -o /dev/null
done

# Compile
echo "[2/3] Compiling $SRC..."
$COMPILER -ggdb $STD_FLAG -Wall -Wextra -D_DEBUG \
    "$SRC" \
    -lSDL2 -lGLEW -lGL -lm \
    -o "$OUT"

# Run
echo "[3/3] Running $OUT..."
./"$OUT"

