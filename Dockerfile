FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    clang \
    libstdc++-11-dev

# Set the working directory
WORKDIR /benchmark

# Copy your benchmark source code into the container
COPY . .

# Build the benchmark
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build

# Default command to run the benchmark
CMD ["build/vectorization_neon"]