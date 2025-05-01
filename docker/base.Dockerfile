FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libzmq3-dev \
    libopencv-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN mkdir build
WORKDIR /app/build
RUN cmake .. && make