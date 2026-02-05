# ffjpeg

ffjpeg is a lightweight command-line image processing tool written in C.  
It provides basic image manipulation features such as resizing, JPEG quality adjustment, grayscale conversion, vertical flipping, color limiting, and image inspection.

The project is designed to be small, fast, and dependency-light, using the stb single-header libraries.

## Features

- Resize images to arbitrary resolutions
- Adjust JPEG output quality
- Convert images to grayscale
- Flip images vertically
- Limit total color count
- Print basic image information
- Simple command-line interface
- Single-file C implementation
- Minimal external dependencies

## Usage

### Command-Line Flags
```
 --help              Show help message
 --info              Print image information
 --quality N         JPEG quality (1–100)
 --grayscale         Convert image to grayscale
 --flip-y            Flip image vertically
 --colors N          Limit total colors
 --resize WxH        Resize image (example: 1920x1080)
```
## Examples

### Resize an image to 24K resolution with high quality
``` ffjpeg in.jpg out.jpg --resize 23040x12960 --quality 90 ```

### Reduce color count and quality
```ffjpeg in.jpg out.jpg --colors 32 --quality 10```

### Convert to grayscale
```ffjpeg in.jpg out.jpg --grayscale```

### Display image information
```ffjpeg image.jpg out.jpg --info```

## Building

### Windows (MinGW / GCC)
```gcc ffjpeg.c -O3 -o ffjpeg```

### Linux
```gcc ffjpeg.c -O3 -lm -o ffjpeg```

## Dependencies

This project uses:

stb_image.h

stb_image_write.h

Both are public-domain single-header libraries by Sean Barrett.

Place these headers in the same directory as main.c.

## Notes

Very large resolutions (12K+) may require significant memory.

Color limiting is approximate.

Supported input formats include JPG, PNG, BMP, and TGA.

Output format is always JPG.

## Author

Andy
