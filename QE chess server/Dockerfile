# ============================================================
# Stage 1: Build C++ executable + GoogleTest
# ============================================================
FROM ubuntu:22.04 AS cpp-build

# Install build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# Create working directory
WORKDIR /app/cpp

# Copy C++ source
COPY cpp_src/ .

# ------------------------------------------------------------
# Fetch and build GoogleTest
# ------------------------------------------------------------
# You can also vendor gtest to speed up builds, but this works fine
RUN git clone https://github.com/google/googletest.git -b release-1.12.1
RUN cmake -S googletest -B build_gtest && cmake --build build_gtest --target install

# ------------------------------------------------------------
# Build your C++ project
# ------------------------------------------------------------
RUN cmake -S . -B build && cmake --build build --config Release

# After building, we should have:
#   /app/cpp/build/my_app          (your main executable)
#   /usr/local/lib/libgtest.a      (googletest library)
# Running tests (optional CI step)
# RUN ctest --test-dir build


# ============================================================
# Stage 2: Build Python / FastAPI server
# ============================================================
FROM python:3.11-slim AS fastapi-build

WORKDIR /app

# Install Python dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy FastAPI application code
COPY fastapi_src/ .

# Copy C++ executable from build stage
COPY --from=cpp-build /app/cpp/build/qe_chess_server /usr/local/bin/qe_chess_server

# Make sure executable has permission
RUN chmod +x /usr/local/bin/qe_chess_server

EXPOSE 8080

CMD ["uvicorn", "simple_fastapi:app", "--host", "0.0.0.0", "--port", "8080"]
