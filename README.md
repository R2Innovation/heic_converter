# **HEIC/HEIF Converter v1.0**

## **Overview**

A command-line tool for converting HEIC/HEIF image files to other common image formats (JPEG, PNG, BMP, TIFF, WebP). The converter includes embedded HEIC/HEIF codecs, eliminating the need for system-wide codec installation.

**Author:** R Square Innovation Software  
**Version:** v1.0  
**License:** See LICENSE file

## **Features**

- Embedded HEIC/HEIF codecs included if the system deos not have it installed
- Multiple Output Formats: Convert to JPEG, PNG, BMP, TIFF, or WebP
- Batch Processing: Convert entire directories of images
- Parallel Processing: Multi-threaded conversion for improved performance
- Preserve Metadata: Option to keep or strip EXIF data and color profiles
- Cross-Platform: Separate builds for Debian 9 and Debian 12/13
- Recursive Directory Processing: Process nested directories
- Quality Control: Adjustable quality/compression for output formats

## **Supported Formats**

### **Input Formats**

- .heic, .HEIC
- .heif, .HEIF

### **Output Formats**

- .jpg, .jpeg (JPEG)
- .png (PNG)
- .bmp (BMP)
- .tiff, .tif (TIFF)
- .webp (WebP)

## **System Requirements**

### **Debian 9 (Stretch)**

- GCC 6.3 or higher
- CMake 3.7 or higher
- Standard C++17 development libraries

### **Debian 12/13 (Bookworm/Trixie)**

- GCC 11 or higher
- CMake 3.18 or higher
- Standard C++17 development libraries

## **Build Instructions**

### **For Debian 12/13 Systems**

```
bash

# Clone the repository
git clone https://github.com/R-Square-Innovative-Software/heic_converter
cd heic_converter_v1.0

# Run build script for Debian 12/13
./scripts/build_debian12.sh

# The executable will be in build/debian12/bin/
```

### **For Debian 9 Systems**

```
bash

# Clone the repository
git clone https://github.com/R-Square-Innovative-Software/heic_converter
cd heic_converter_v1.0

# Run build script for Debian 9
./scripts/build_debian9.sh
# The executable will be in build/debian9/bin/
```

### **Manual Build (CMake)**

```
bash

mkdir -p build && cd build
cmake -DDEBIAN9_BUILD=ON .. # For Debian 9

# or
cmake -DDEBIAN12_BUILD=ON .. # For Debian 12/13
make -j\$(nproc)
sudo make install # Optional
```

## **Usage**

### **Basic Syntax**

```
heic_converter [options] <input> [output]
```

### **Examples**

**Convert a single file:**

```
bash

heic_converter image.heic
heic_converter image.heic image.jpg
heic_converter -f png image.heic
```

**Convert with quality adjustment:**

```
bash

heic_converter -q 90 image.heic
heic_converter -c 9 image.heic # PNG compression
```

**Batch conversion:**

```
bash

heic_converter -r ./photos ./converted
heic_converter -t 8 -r ./input_dir ./output_dir
```

**Overwrite existing files:**

```
bash

heic_converter -o image.heic image.jpg
```

**Verbose output:**

```
bash

heic_converter -v image.heic
```

### **Command Line Options**

|       **Option**       |              **Description**              | **Default** |
| ---------------------- | ----------------------------------------- | ----------- |
|  \-f, --format FORMAT  | Output format (jpg, png, bmp, tiff, webp) | jpg         |
| \-q, --quality N       | JPEG quality (1-100)                      | 85          |
| \-c, --compression N   | PNG compression level (0-9)               | 6           |
| \-s, --scale FACTOR    | Scale factor (0.1 to 10.0)                | 1.0         |
| \-t, --threads N       | Threads for batch processing              | 4           |
| \-r, --recursive       | Process directories recursively           | false       |
| \-o, --overwrite       | Overwrite existing files                  | false       |
| \-v, --verbose         | Enable verbose output                     | false       |
| \--no-metadata         | Strip metadata from output                | false       |
| \--no-color-profile    | Strip color profile from output           | false       |
| \-h, --help            | Show help message                         |             |
| \--version             | Show version information                  |             |

## **Project Structure**

```
heic_converter_v1.0/
├── data/ # Embedded codecs
│ ├── debian9/ # Codecs for Debian 9
│ └── debian12/ # Codecs for Debian 12/13
├── scripts/ # Build scripts
├── include/ # Header files
├── src/ # Source files
└── tests/ # Test suite
```

### **Key Components**

- main.cpp - Command-line interface and argument parsing
- converter.cpp - Main conversion logic
- heic_decoder.cpp - HEIC/HEIF decoding with embedded codecs
- format_encoder.cpp - Output format encoding using system libraries
- batch_processor.cpp - Batch and directory processing
- file_utils.cpp - File system operations
- config.cpp - Configuration management

## **Embedded Codecs**

The application includes embedded versions of:
  - libheif - HEIC/HEIF decoding library
  - x265 - HEVC/H.265 video codec (required by libheif)

These are compiled into the application and don't require system installation.

## **Output Format Support**

The converter uses system libraries for output formats:

| **Format** | **Library** |       **Notes**       |
| ---------- | ----------- | --------------------- |
| JPEG       | libjpeg     | System library        |
| PNG        | libpng      | System library        |
| BMP        | Built-in    | Simple implementation |
| TIFF       | libtiff     | System library        |
| WebP       | libwebp     | System library        |

## **Performance Tips**

- Use parallel processing for batch conversions: -t 8
- Adjust quality settings for smaller file sizes
- Disable metadata if not needed: --no-metadata
- Use appropriate format for your use case:
  * JPEG for photographs
  * PNG for graphics with transparency
  * WebP for modern web use

## **Troubleshooting**

### **Common Issues**

- "Cannot open file" error:
  * Check file permissions
  * Ensure the file is not corrupted
- "Unsupported format" error
  * Verify the file has a .heic or .heif extension
  * Try a different HEIC/HEIF file
- Memory issues with large files
  * Reduce thread count: -t 2
  * Ensure sufficient system memory
- Build failures on Debian 9
  * Install required dependencies: ./scripts/install_deps_debian9.sh
  * Ensure GCC 6.3+ is installed

### **Error Codes**

|**Code**|        **Description**        |
| ------ | ----------------------------- |
|  0     |  Success                      |
|  1     |  Invalid arguments            |
|  2     |  Unsupported format           |
|  3     |  File not found               |
|  4     |  Read permission error        |
|  5     |  Write permission error       |
|  6     |  Decoding failed              |
|  7     |  Encoding failed              |
|  8     |  Memory allocation error      |
|  9     |  Codec initialization error   |
|  10    |  Batch processing error       |
|  255   |  Unknown error                |

## **Development**

### **Building for Development**

```
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
./bin/heic_converter --help
```

### **Running Tests**

```
cd build
ctest --verbose
```

## **License**

_See the LICENSE file for detailed licensing information.

## **Contributing**

- Fork the repository
- Create a feature branch
- Make changes following the coding style
- Add tests for new functionality
- Submit a pull request

## **Changelog**

See CHANGELOG.md for version history and changes.

## **Support**

For issues, questions, or feature requests:
- Check the troubleshooting section
- Review existing issues
- Submit a new issue with detailed information

_Built by R Square Innovation Software_
