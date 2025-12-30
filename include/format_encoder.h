// format_encoder.h
// R Square Innovation Software - HEIC Converter v1.1
// Header for format encoding functionality using system libraries

#ifndef FORMAT_ENCODER_H
#define FORMAT_ENCODER_H

#include <vector>
#include <string>
#include <ctime>

// Structure to hold raw image data
struct sImageData {
    unsigned char* pData;
    int iWidth;
    int iHeight;
    int iChannels;
    int iBitDepth;
};

// Structure for encoding options
struct sEncodeOptions {
    std::string sFormat;
    int iQuality; // For JPEG, WebP
    int iCompressionLevel; // For PNG, TIFF
    bool bProgressive; // For JPEG
    bool bInterlace; // For PNG
    bool bLossless; // For WebP
    std::vector<unsigned char> vExifData; // NEW: EXIF metadata
    std::vector<unsigned char> vXmpData;  // NEW: XMP metadata
    std::vector<unsigned char> vIptcData; // NEW: IPTC metadata
    bool bPreserveMetadata;               // NEW: Preserve metadata flag
};

class FormatEncoder {
public:
    // Constructor and Destructor
    FormatEncoder();
    ~FormatEncoder();

    // Main encoding function
    bool fn_encodeImage(
        const sImageData& oImageData,
        const std::string& sOutputPath,
        const sEncodeOptions& oOptions
    );

    // Get supported formats
    std::vector<std::string> fn_getSupportedFormats();

    // Validate output format
    bool fn_validateFormat(const std::string& sFormat);

private:
    // PNG encoding function
    bool fn_encodePNG(
        const sImageData& oImageData,
        const std::string& sOutputPath,
        const sEncodeOptions& oOptions
    );

    // JPEG encoding function
    bool fn_encodeJPEG(
        const sImageData& oImageData,
        const std::string& sOutputPath,
        const sEncodeOptions& oOptions
    );

    // WebP encoding function
    bool fn_encodeWebP(
        const sImageData& oImageData,
        const std::string& sOutputPath,
        const sEncodeOptions& oOptions
    );

    // BMP encoding function
    bool fn_encodeBMP(
        const sImageData& oImageData,
        const std::string& sOutputPath,
        const sEncodeOptions& oOptions
    );

    // TIFF encoding function
    bool fn_encodeTIFF(
        const sImageData& oImageData,
        const std::string& sOutputPath,
        const sEncodeOptions& oOptions
    );

    // Check for format support
    bool fn_checkPNGSupport();
    bool fn_checkJPEGSupport();
    bool fn_checkWebPSupport();
    bool fn_checkBMPSupport();
    bool fn_checkTIFFSupport();

    // NEW: Metadata helper functions
    bool fn_writeJpegWithMetadata(
        const sImageData& oImageData,
        const std::string& sOutputPath,
        const sEncodeOptions& oOptions
    );
    
    bool fn_writePngWithMetadata(
        const sImageData& oImageData,
        const std::string& sOutputPath,
        const sEncodeOptions& oOptions
    );

    // Member variables
    bool bPNGSupported;
    bool bJPEGSupported;
    bool bWebPSupported;
    bool bBMPSupported;
    bool bTIFFSupported;
};

#endif // FORMAT_ENCODER_H
// End format_encoder.h