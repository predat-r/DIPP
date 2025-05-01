# Distributed Image Processing Pipeline (DIPP)

A scalable, modular system for processing, resizing, watermarking, and compressing images using ZeroMQ for inter-process communication.

## Architecture

The pipeline consists of five independent components that can be mixed and matched based on needs:

1. **Receiver** - Entry point that reads and sends images
2. **Preprocessor** - Enhances image quality using Lab color space
3. **Resizer** - Creates multiple resolution variants
4. **Watermarker** - Applies transparent watermark to each variant
5. **Compressor** - Applies format-specific compression and saves results

### Scalability

Each component can be scaled independently:
- Run multiple instances of compute-heavy components
- Distribute components across different machines
- Load balance across multiple preprocessors or resizers
- Scale horizontally based on workload

## Pipeline Flow

```
[Receiver] → [Preprocessor(s)] → [Resizer(s)] → [Watermarker(s)] → [Compressor(s)]
   :5555         :5556            :5557           :5558             Output
```

## Modular Usage

Components can be used independently or in custom combinations:
- Use just the Resizer for batch image resizing
- Combine Watermarker + Compressor for simple watermarking
- Scale up Preprocessors for heavy image enhancement workloads
- Create custom workflows by mixing components

## Features

- Lab color space conversion for enhanced contrast
- Multiple standard resolution outputs (FHD to QVGA)
- Alpha channel watermarking
- Configurable JPEG/PNG compression
- ZeroMQ-based message passing

## Requirements

- OpenCV 4.x
- ZeroMQ
- C++17 compiler
- CMake 3.x

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Usage

1. Place input image as `image.jpg` in the project root
2. Place watermark as `watermark.png` in the project root
3. Start components in order:

```bash
./compressor
./watermarker
./resizer
./preprocessor
./receiver
```

## Output

The pipeline generates five variants of each input image:
- 1920x1080 (Full HD)
- 1280x720 (HD)
- 800x600 (SVGA)
- 640x480 (VGA)
- 320x240 (QVGA)

## Communication Protocol

Components communicate using a simple protocol:
1. Image format string
2. Binary image data
3. Acknowledgment

## Configuration

Key parameters can be adjusted in the source:
- JPEG quality (default: 60%)
- PNG compression level (default: 6)
- Watermark size ratio (default: 25%)
- Connection timeouts (default: 10s)

## License
MIT