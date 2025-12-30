// image_processor.cpp - Updated for metadata passing
#include "image_processor.h"
#include "heic_decoder.h"
#include "format_encoder.h"
#include "file_utils.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <memory>
#include <stdexcept>

// Constructor
ImageProcessor::ImageProcessor(oLogger* pLogger) 
{
    // Initialize members
    m_pLogger = pLogger;
    m_sLastError = "";
    m_iOutputQuality = 85;
    m_bCodecsInitialized = false;
    m_pHeifContext = nullptr;
    m_pHeifImage = nullptr;
    
    // Initialize codecs
    fn_initializeCodecs();
} // End Constructor

// Destructor
ImageProcessor::~ImageProcessor() 
{
    // Cleanup resources
    fn_cleanupResources();
    m_pLogger = nullptr;
} // End Destructor

// NEW: Convert image with metadata options
bool ImageProcessor::fn_convertImageWithMetadata(
    const std::string& sInputPath, 
    const std::string& sOutputPath,
    const std::string& sOutputFormat,
    int iQuality,
    const std::vector<unsigned char>& vExifData,
    const std::vector<unsigned char>& vXmpData,
    const std::vector<unsigned char>& vIptcData
) 
{
    // Reset last error
    m_sLastError = "";
    
    // Validate parameters
    if (sInputPath.empty()) {
        m_sLastError = "Input path is empty";
        if (m_pLogger) m_pLogger->fn_logError(m_sLastError);
        return false;
    }
    
    if (sOutputPath.empty()) {
        m_sLastError = "Output path is empty";
        if (m_pLogger) m_pLogger->fn_logError(m_sLastError);
        return false;
    }
    
    // Determine output format if not specified
    std::string sFormat = sOutputFormat;
    if (sFormat.empty()) {
        sFormat = fn_determineOutputFormat(sOutputPath);
    }
    
    // Validate output format
    if (!fn_validateOutputFormat(sFormat)) {
        m_sLastError = "Unsupported output format: " + sFormat;
        if (m_pLogger) m_pLogger->fn_logError(m_sLastError);
        return false;
    }
    
    // Set quality if provided
    if (iQuality > 0) {
        m_iOutputQuality = iQuality;
    }
    
    if (m_pLogger) {
        m_pLogger->fn_logInfo("Converting " + sInputPath + " to " + sFormat + " format");
        if (!vExifData.empty()) {
            m_pLogger->fn_logInfo("Preserving EXIF metadata (" + std::to_string(vExifData.size()) + " bytes)");
        }
    }
    
    // Decode HEIC/HEIF image
    unsigned char* pImageData = nullptr;
    int iWidth = 0, iHeight = 0, iChannels = 0;
    
    bool bDecoded = fn_decodeHEIC(sInputPath, &pImageData, iWidth, iHeight, iChannels);
    if (!bDecoded) {
        if (m_pLogger) m_pLogger->fn_logError("Failed to decode image: " + sInputPath);
        return false;
    }
    
    // Encode to output format with metadata
    bool bEncoded = fn_encodeImageWithMetadata(pImageData, iWidth, iHeight, iChannels, 
                                               sOutputPath, sFormat, m_iOutputQuality,
                                               vExifData, vXmpData, vIptcData);
    
    // Cleanup image data
    if (pImageData) {
        delete[] pImageData;
        pImageData = nullptr;
    }
    
    if (!bEncoded) {
        if (m_pLogger) m_pLogger->fn_logError("Failed to encode image: " + sOutputPath);
        return false;
    }
    
    if (m_pLogger) {
        m_pLogger->fn_logSuccess("Successfully converted: " + sInputPath);
    }
    
    return true;
} // End Function fn_convertImageWithMetadata

// Original function for backward compatibility
bool ImageProcessor::fn_convertImage(
    const std::string& sInputPath, 
    const std::string& sOutputPath,
    const std::string& sOutputFormat,
    int iQuality
) 
{
    // Call the new function without metadata
    return fn_convertImageWithMetadata(sInputPath, sOutputPath, sOutputFormat, iQuality,
                                       {}, {}, {});
} // End Function fn_convertImage

// Initialize codecs
bool ImageProcessor::fn_initializeCodecs() 
{
    if (m_bCodecsInitialized) 
    {
        return true;
    }
    
    // Codecs are initialized on-demand in the decoder
    m_bCodecsInitialized = true;
    
    if (m_pLogger) {
        m_pLogger->fn_logInfo("ImageProcessor codecs ready");
    }
    
    return true;
} // End Function fn_initializeCodecs

// Cleanup resources
bool ImageProcessor::fn_cleanupResources() 
{
    // Cleanup any allocated resources
    m_bCodecsInitialized = false;
    return true;
} // End Function fn_cleanupResources

// Decode HEIC/HEIF file
bool ImageProcessor::fn_decodeHEIC(
    const std::string& sInputPath, 
    unsigned char** ppImageData, 
    int& iWidth, 
    int& iHeight, 
    int& iChannels
) 
{
    // Create decoder instance
    HeicDecoder oDecoder;
    
    // Decode the image
    oDecodedImage oResult = oDecoder.fn_decodeFile(sInputPath);
    
    // Check for errors
    if (!oResult.sError.empty()) {
        m_sLastError = oResult.sError;
        if (m_pLogger) m_pLogger->fn_logError("Decode error: " + m_sLastError);
        return false;
    }
    
    // Allocate memory for image data
    size_t dataSize = oResult.vData.size();
    *ppImageData = new unsigned char[dataSize];
    if (!*ppImageData) {
        m_sLastError = "Failed to allocate memory for image data";
        if (m_pLogger) m_pLogger->fn_logError(m_sLastError);
        return false;
    }
    
    // Copy data
    std::memcpy(*ppImageData, oResult.vData.data(), dataSize);
    
    // Set dimensions
    iWidth = oResult.iWidth;
    iHeight = oResult.iHeight;
    iChannels = oResult.iChannels;
    
    if (m_pLogger) {
        m_pLogger->fn_logInfo("Decoded image: " + std::to_string(iWidth) + "x" + 
                             std::to_string(iHeight) + " with " + 
                             std::to_string(iChannels) + " channels");
    }
    
    return true;
} // End Function fn_decodeHEIC

// NEW: Encode image to output format with metadata
bool ImageProcessor::fn_encodeImageWithMetadata(
    const unsigned char* pImageData, 
    int iWidth, 
    int iHeight, 
    int iChannels, 
    const std::string& sOutputPath, 
    const std::string& sOutputFormat, 
    int iQuality,
    const std::vector<unsigned char>& vExifData,
    const std::vector<unsigned char>& vXmpData,
    const std::vector<unsigned char>& vIptcData
) 
{
    // Create encoder instance
    FormatEncoder oEncoder;
    
    // Prepare image data structure
    sImageData oImageData;
    oImageData.pData = const_cast<unsigned char*>(pImageData);
    oImageData.iWidth = iWidth;
    oImageData.iHeight = iHeight;
    oImageData.iChannels = iChannels;
    oImageData.iBitDepth = 8;
    
    // Prepare encoding options
    sEncodeOptions oOptions;
    oOptions.sFormat = sOutputFormat;
    oOptions.iQuality = iQuality;
    
    // Set metadata
    oOptions.vExifData = vExifData;
    oOptions.vXmpData = vXmpData;
    oOptions.vIptcData = vIptcData;
    oOptions.bPreserveMetadata = !vExifData.empty() || !vXmpData.empty() || !vIptcData.empty();
    
    // Set format-specific options
    std::string sLowerFormat = sOutputFormat;
    std::transform(sLowerFormat.begin(), sLowerFormat.end(), sLowerFormat.begin(), ::tolower);
    
    if (sLowerFormat == "png") {
        oOptions.iCompressionLevel = 6;
        oOptions.bInterlace = false;
    } else if (sLowerFormat == "jpg" || sLowerFormat == "jpeg") {
        oOptions.bProgressive = false;
    } else if (sLowerFormat == "webp") {
        oOptions.bLossless = false;
    } else if (sLowerFormat == "tiff" || sLowerFormat == "tif") {
        oOptions.iCompressionLevel = 0;
    }
    
    // Encode the image
    bool bResult = oEncoder.fn_encodeImage(oImageData, sOutputPath, oOptions);
    
    if (!bResult) {
        m_sLastError = "Failed to encode image to " + sOutputFormat;
    }
    
    return bResult;
} // End Function fn_encodeImageWithMetadata

// Original encode function for backward compatibility
bool ImageProcessor::fn_encodeImage(
    const unsigned char* pImageData, 
    int iWidth, 
    int iHeight, 
    int iChannels, 
    const std::string& sOutputPath, 
    const std::string& sOutputFormat, 
    int iQuality
) 
{
    return fn_encodeImageWithMetadata(pImageData, iWidth, iHeight, iChannels,
                                      sOutputPath, sOutputFormat, iQuality,
                                      {}, {}, {});
} // End Function fn_encodeImage

// Validate image file
bool ImageProcessor::fn_validateImage(const std::string& sImagePath) 
{
    // Check if file exists
    if (!fn_fileExists(sImagePath)) {
        m_sLastError = "File does not exist: " + sImagePath;
        return false;
    }
    
    // Check file extension
    std::string sExtension = fn_getFileExtension(sImagePath);
    std::transform(sExtension.begin(), sExtension.end(), sExtension.begin(), ::tolower);
    
    // Check if it's a supported format
    std::vector<std::string> vsSupported = fn_getSupportedInputFormats();
    for (const auto& format : vsSupported) {
        if (sExtension == format) {
            return true;
        }
    }
    
    m_sLastError = "Unsupported image format: " + sExtension;
    return false;
} // End Function fn_validateImage

// Get supported input formats
std::vector<std::string> ImageProcessor::fn_getSupportedInputFormats() 
{
    return {"heic", "heif", "hif", "avci", "avcs", "avif"};
} // End Function fn_getSupportedInputFormats

// Get supported output formats
std::vector<std::string> ImageProcessor::fn_getSupportedOutputFormats() 
{
    // Return formats supported by FormatEncoder
    FormatEncoder oEncoder;
    return oEncoder.fn_getSupportedFormats();
} // End Function fn_getSupportedOutputFormats

// Set output quality
bool ImageProcessor::fn_setOutputQuality(int iQuality) 
{
    if (iQuality < 1 || iQuality > 100) {
        m_sLastError = "Quality must be between 1 and 100";
        return false;
    }
    
    m_iOutputQuality = iQuality;
    return true;
} // End Function fn_setOutputQuality

// Get output quality
int ImageProcessor::fn_getOutputQuality() 
{
    return m_iOutputQuality;
} // End Function fn_getOutputQuality

// Get last error
std::string ImageProcessor::fn_getLastError() 
{
    return m_sLastError;
} // End Function fn_getLastError

// Validate output format
bool ImageProcessor::fn_validateOutputFormat(const std::string& sFormat) 
{
    FormatEncoder oEncoder;
    return oEncoder.fn_validateFormat(sFormat);
} // End Function fn_validateOutputFormat

// Determine output format from file extension
std::string ImageProcessor::fn_determineOutputFormat(const std::string& sOutputPath) 
{
    std::string sExtension = fn_getFileExtension(sOutputPath);
    if (sExtension.empty()) {
        return "jpg"; // Default format
    }
    
    // Normalize format
    std::transform(sExtension.begin(), sExtension.end(), sExtension.begin(), ::tolower);
    
    // Handle common aliases
    if (sExtension == "jpeg") {
        return "jpg";
    } else if (sExtension == "tif") {
        return "tiff";
    }
    
    return sExtension;
} // End Function fn_determineOutputFormat
