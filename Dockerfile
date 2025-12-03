# Stage 1: Build C++ engine
FROM ubuntu:22.04 AS build_cpp

# Install dependencies
RUN apt-get update && apt-get install -y git unzip curl ca-certificates \
    build-essential \
    ninja-build \
    cmake \
    && update-ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Clone the repo
WORKDIR /app
RUN git clone https://github.com/PZwoodcat/Flock-Chess.git

# Build the C++ engine
WORKDIR /app/Flock-Chess/Flock\ Chess\ -\ public/
RUN mkdir -p build && \
    cmake -S . -B build -G "Ninja Multi-Config" && \
    cmake --build build --config Debug && \
    cmake --build build --config Release

# Stage 2: Python + FastAPI runtime
FROM python:3.11-slim AS final

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Copy the entire cloned repo from build stage
WORKDIR /app
COPY --from=build_cpp /app/Flock-Chess/ ./Flock-Chess/

# Install Python dependencies
WORKDIR /app/Flock-Chess/QE\ chess\ server
RUN pip install --no-cache-dir -r requirements.txt

# Expose FastAPI port
EXPOSE 8000

# Run FastAPI server
CMD ["uvicorn", "simple_fastapi:app", "--host", "0.0.0.0", "--port", "8000", "--log-level", "debug"]
