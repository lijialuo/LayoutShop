# LayoutShop

LayoutShop is a Qt desktop application for generating and refining document layouts, shared for academic reference. The repository includes source code, scoring models, layout templates, and sample inputs.

## Directory Structure

```text
src/                C++ and Qt source files
assets/model/       image scoring model and graph scoring weights
assets/templates/   layout template library
assets/input_json/  sample JSON inputs and accompanying images
assets/resources/   interface icons and resources
CMakeLists.txt      build configuration
```

## Getting Started

The build has been verified on macOS with Apple Silicon. It requires CMake 3.24+, a C++20 compiler, Qt 6, Eigen3, RapidJSON, libtorch, and Gurobi with a valid license.

Install the Homebrew dependencies:

```bash
brew install cmake qt eigen rapidjson pytorch
```

Install Gurobi separately. In `CMakeLists.txt`, find `set(GUROBI_HOME "" CACHE PATH ...)` and replace the empty path with your Gurobi installation directory, for example:

```cmake
set(GUROBI_HOME "/Library/gurobi1001/macos_universal2" CACHE PATH "Path to a Gurobi installation")
```

Then run the following commands from the repository root. If you previously configured this build directory, delete `cmake-build-debug/CMakeCache.txt` first so the updated path takes effect.

```bash
cmake -S . -B cmake-build-debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/qt;/opt/homebrew/opt/pytorch"
cmake --build cmake-build-debug --target LayoutShop -j
./cmake-build-debug/LayoutShop
```
