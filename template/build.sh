#!/bin/bash

set -e  # Exit immediately if any command fails

# 1. Configuration
SRC="main.cpp"         # Change this to your source file
OUT="tiny_glade"     # Output executable name
SHADER_DIR="res/shaders"  # Path to your shaders

# 2. Auto-detect language
if [[ "$SRC" == *.cpp ]]; then
    COMPILER="g++"
    STD_FLAG="-std=c++17"
else
    COMPILER="gcc"
    STD_FLAG="-std=c99"
fi

# 3. Validate shaders
echo "[1/3] Validating GLSL shaders..."
shaders=("$SHADER_DIR"/*.glsl)
for shader in "${shaders[@]}"; do
    if [[ -f "$shader" ]]; then
        echo "Validating $shader"
        glslangValidator -G "$shader" || true  # Continue even if validation fails
    fi
done

# 4. Compile
echo "[2/3] Compiling $SRC..."
$COMPILER -ggdb $STD_FLAG -Wall -Wextra -D_DEBUG \
    "$SRC" \
    -lSDL2 -lGLEW -lGL -lm \
    -o "$OUT"

# 5. Run
echo "[3/3] Running $OUT..."
./"$OUT"
