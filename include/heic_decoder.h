// heic_decoder.h - Fixed version
#ifndef HEIC_DECODER_H
#define HEIC_DECODER_H

#include <string>
#include <vector>

#ifdef HAVE_LIBHEIF
#include <libheif/heif.h>
#endif

// Object to store decoded image data
struct oDecodedImage
{
    std::vector<unsigned char> vData; // Raw pixel data
    int iWidth;                       // Image width in pixels
    int iHeight;                      // Image height in pixels
    int iChannels;                    // Number of color channels (3 for RGB, 4 for RGBA)
    std::string sColorSpace;          // Color space information
    bool bHasAlpha;                   // Whether image has alpha channel
    std::string sError;               // Error message if decoding failed
}; // End struct oDecodedImage

// Object to store HEIC/HEIF image information
struct oHeicInfo
{
    std::string sFormat;              // Image format (HEIC, HEIF, etc.)
    int iWidth;                       // Image width in pixels
    int iHeight;                      // Image height in pixels
    int iBitDepth;                    // Bit depth
    std::string sColorSpace;          // Color space
    bool bHasAlpha;                   // Whether image has alpha channel
    int iOrientation;                 // EXIF orientation
    std::vector<std::string> vsMetadata; // Image metadata
}; // End struct oHeicInfo

class HeicDecoder
{
public:
    // Constructor and destructor
    HeicDecoder();                               // Local Function
    ~HeicDecoder();                              // Local Function
    
    // Main decoding functions
    oDecodedImage fn_decodeFile(const std::string& sFilePath);               // Local Function
    oDecodedImage fn_decodeMemory(const std::vector<unsigned char>& vData);  // Local Function
    
    // Information functions
    oHeicInfo fn_getImageInfo(const std::string& sFilePath);                // Local Function
    oHeicInfo fn_getImageInfoFromMemory(const std::vector<unsigned char>& vData); // Local Function
    
    // Utility functions
    bool fn_isFormatSupported(const std::string& sFormat);                  // Local Function
    std::vector<std::string> fn_getSupportedFormats();                      // Local Function
    std::string fn_getLastError();                                          // Local Function
    bool fn_isInitialized();                                                // Local Function
    
    // Configuration functions (for embedded codecs only)
    #ifndef HAVE_LIBHEIF
    void fn_setEmbeddedCodecPath(const std::string& sPath);                // Local Function
    std::string fn_getEmbeddedCodecPath();                                 // Local Function
    #endif
    
    // NEW: Set logger for debugging
    void fn_setLogger(class oLogger* pLogger) { m_pLogger = pLogger; }    // Local Function
    
private:
    // Private variables
    std::string sLastError;                      // Last error message
    bool bInitialized;                           // Whether decoder is initialized
    std::string sEmbeddedCodecPath;              // Path to embedded codec data (if needed)
    std::vector<std::string> vsSupportedFormats; // List of supported formats
    class oLogger* m_pLogger;                    // NEW: Logger pointer
    
    #ifdef HAVE_LIBHEIF
    // Libheif context and handle
    struct heif_context* pHeifContext;
    struct heif_image_handle* pHeifHandle;
    struct heif_image* pHeifImage;
    
    // Private functions for libheif
    bool fn_decodeWithLibHeif(const std::vector<unsigned char>& vData, oDecodedImage& oResult);
    void fn_cleanupLibHeif();
    
    // NEW: Panorama handling
    bool fn_handlePanoramaImage(struct heif_image_handle* handle, oDecodedImage& oResult);
    #else
    // Embedded codec context (placeholder)
    void* pDecoderContext;
    
    // Private functions for embedded codecs
    bool fn_initializeEmbeddedCodecs();
    bool fn_createDecoderContext();
    void fn_cleanupDecoderContext();
    #endif
    
    // Fallback dummy decoder
    oDecodedImage fn_decodeDummy(const std::vector<unsigned char>& vData);
}; // End class HeicDecoder

#endif // HEIC_DECODER_H