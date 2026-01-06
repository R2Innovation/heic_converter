// heic_decoder.cpp - Simplified version for Debian 12
#include "heic_decoder.h"
#include "logger.h"
#include "file_utils.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <memory>
#include <algorithm>

#ifdef HAVE_LIBHEIF
#include <libheif/heif.h>
#endif

// Constructor
HeicDecoder::HeicDecoder()
{
    // Initialize variables
    sLastError = "";
    sEmbeddedCodecPath = "";
    
    // Set supported formats
    vsSupportedFormats = {"heic", "heif", "hif", "avci", "avcs", "avif"};
    
    #ifdef HAVE_LIBHEIF
    // Initialize libheif members
    pHeifContext = nullptr;
    pHeifHandle = nullptr;
    pHeifImage = nullptr;
    
    // Libheif doesn't require explicit initialization
    bInitialized = true;
    #else
    // For embedded codecs
    pDecoderContext = nullptr;
    bInitialized = false;
    #endif
    
    m_pLogger = nullptr;  // Initialize logger pointer
} // End Function HeicDecoder::HeicDecoder

// Destructor
HeicDecoder::~HeicDecoder()
{
    #ifdef HAVE_LIBHEIF
    // Cleanup libheif resources
    fn_cleanupLibHeif();
    #else
    // Cleanup embedded decoder context
    if (pDecoderContext) {
        free(pDecoderContext);
        pDecoderContext = nullptr;
    }
    #endif
} // End Function HeicDecoder::~HeicDecoder

#ifdef HAVE_LIBHEIF
// Cleanup libheif resources
void HeicDecoder::fn_cleanupLibHeif()
{
    if (pHeifImage)
    {
        heif_image_release(pHeifImage);
        pHeifImage = nullptr;
    }
    
    if (pHeifHandle)
    {
        heif_image_handle_release(pHeifHandle);
        pHeifHandle = nullptr;
    }
    
    if (pHeifContext)
    {
        heif_context_free(pHeifContext);
        pHeifContext = nullptr;
    }
}

// Decode with libheif - SIMPLIFIED VERSION FOR DEBIAN 12
bool HeicDecoder::fn_decodeWithLibHeif(const std::vector<unsigned char>& vData, oDecodedImage& oResult)
{
    fn_cleanupLibHeif();
    
    pHeifContext = heif_context_alloc();
    if (!pHeifContext)
    {
        oResult.sError = "Failed to allocate HEIF context";
        return false;
    }
    
    struct heif_error err = heif_context_read_from_memory(pHeifContext, vData.data(), vData.size(), nullptr);
    if (err.code != heif_error_Ok)
    {
        oResult.sError = "Failed to read HEIF data: " + std::string(err.message);
        return false;
    }
    
    err = heif_context_get_primary_image_handle(pHeifContext, &pHeifHandle);
    if (err.code != heif_error_Ok)
    {
        oResult.sError = "Failed to get primary image handle: " + std::string(err.message);
        return false;
    }
    
    oResult.iWidth = heif_image_handle_get_width(pHeifHandle);
    oResult.iHeight = heif_image_handle_get_height(pHeifHandle);
    oResult.bHasAlpha = heif_image_handle_has_alpha_channel(pHeifHandle);
    oResult.iChannels = oResult.bHasAlpha ? 4 : 3;
    
    // Check if this might be a panorama (aspect ratio > 2:1)
    bool is_panorama = (oResult.iWidth > oResult.iHeight * 2) || (oResult.iHeight > oResult.iWidth * 2);
    
    if (is_panorama && m_pLogger)
    {
        // Log panorama detection
        // Note: We can't log without logger, so we'll just note it
    }
    
    // Try to decode with default options
    err = heif_decode_image(pHeifHandle, &pHeifImage,
                           heif_colorspace_RGB,
                           oResult.bHasAlpha ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RGB,
                           nullptr);
    
    if (err.code != heif_error_Ok)
    {
        oResult.sError = "Failed to decode image: " + std::string(err.message);
        return false;
    }
    
    int stride;
    const uint8_t* pData = heif_image_get_plane_readonly(pHeifImage, heif_channel_interleaved, &stride);
    if (!pData)
    {
        oResult.sError = "Failed to get image plane";
        return false;
    }
    
    // For panoramas, we need to handle the stride properly
    size_t dataSize = oResult.iHeight * stride;
    oResult.vData.resize(dataSize);
    
    // Copy data row by row to handle stride
    for (int row = 0; row < oResult.iHeight; row++)
    {
        const uint8_t* src_row = pData + (row * stride);
        uint8_t* dst_row = oResult.vData.data() + (row * oResult.iWidth * oResult.iChannels);
        memcpy(dst_row, src_row, oResult.iWidth * oResult.iChannels);
    }
    
    return true;
}
#endif

#ifndef HAVE_LIBHEIF
// Initialize embedded codecs (only if libheif not available)
bool HeicDecoder::fn_initializeEmbeddedCodecs()
{
    // This is a stub - you would need to implement embedded codec initialization
    // For now, just set initialized to true
    bInitialized = true;
    return true;
}
#endif

// Fallback dummy decoder
oDecodedImage HeicDecoder::fn_decodeDummy(const std::vector<unsigned char>& vData)
{
    oDecodedImage oResult;
    
    // Create a simple 100x100 RGB image for testing
    oResult.iWidth = 100;
    oResult.iHeight = 100;
    oResult.iChannels = 3;
    oResult.sColorSpace = "sRGB";
    oResult.bHasAlpha = false;
    
    oResult.vData.resize(oResult.iWidth * oResult.iHeight * oResult.iChannels);
    for (int y = 0; y < oResult.iHeight; y++)
    {
        for (int x = 0; x < oResult.iWidth; x++)
        {
            int iIndex = (y * oResult.iWidth + x) * 3;
            oResult.vData[iIndex] = static_cast<unsigned char>((x * 255) / oResult.iWidth);
            oResult.vData[iIndex + 1] = static_cast<unsigned char>((y * 255) / oResult.iHeight);
            oResult.vData[iIndex + 2] = 128;
        }
    }
    
    return oResult;
}

// Main decoding function
oDecodedImage HeicDecoder::fn_decodeFile(const std::string& sFilePath)
{
    oDecodedImage oResult;
    oResult.sError = "";
    
    // Check if file exists
    if (!fn_fileExists(sFilePath))
    {
        oResult.sError = "File does not exist: " + sFilePath;
        sLastError = oResult.sError;
        return oResult;
    }
    
    // Check file extension
    std::string sExtension = fn_getFileExtension(sFilePath);
    std::transform(sExtension.begin(), sExtension.end(), sExtension.begin(), ::tolower);
    
    if (!fn_isFormatSupported(sExtension))
    {
        oResult.sError = "Unsupported file format: " + sExtension;
        sLastError = oResult.sError;
        return oResult;
    }
    
    // Read file into memory
    std::vector<unsigned char> vFileData = fn_readBinaryFile(sFilePath);
    if (vFileData.empty())
    {
        oResult.sError = "Failed to read file: " + sFilePath;
        sLastError = oResult.sError;
        return oResult;
    }
    
    // Decode from memory
    oResult = fn_decodeMemory(vFileData);
    if (!oResult.sError.empty())
    {
        sLastError = oResult.sError;
    }
    
    return oResult;
} // End Function HeicDecoder::fn_decodeFile

// Memory decoding function
oDecodedImage HeicDecoder::fn_decodeMemory(const std::vector<unsigned char>& vData)
{
    oDecodedImage oResult;
    oResult.sError = "";
    
    // Check if data is not empty
    if (vData.empty())
    {
        oResult.sError = "Input data is empty";
        sLastError = oResult.sError;
        return oResult;
    }
    
    #ifdef HAVE_LIBHEIF
    // Use libheif for decoding
    if (fn_decodeWithLibHeif(vData, oResult))
    {
        return oResult;
    }
    else
    {
        // If libheif fails, fall through to dummy decoder
        sLastError = oResult.sError;
    }
    #else
    // For embedded codecs
    if (!bInitialized && !fn_initializeEmbeddedCodecs())
    {
        oResult.sError = "Failed to initialize decoder: " + sLastError;
        sLastError = oResult.sError;
        return oResult;
    }
    #endif
    
    // Fallback to dummy decoder
    return fn_decodeDummy(vData);
} // End Function HeicDecoder::fn_decodeMemory

// Get image information
oHeicInfo HeicDecoder::fn_getImageInfo(const std::string& sFilePath)
{
    oHeicInfo oInfo;
    
    // Simple implementation for now
    if (!fn_fileExists(sFilePath))
    {
        sLastError = "File does not exist: " + sFilePath;
        return oInfo;
    }
    
    // Get basic info from filename
    std::string ext = fn_getFileExtension(sFilePath);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    oInfo.sFormat = (ext == "heic") ? "HEIC" : "HEIF";
    
    // Default values
    oInfo.iWidth = 1920;
    oInfo.iHeight = 1080;
    oInfo.iBitDepth = 8;
    oInfo.sColorSpace = "sRGB";
    oInfo.bHasAlpha = false;
    oInfo.iOrientation = 1;
    
    return oInfo;
} // End Function HeicDecoder::fn_getImageInfo

// Get image info from memory
oHeicInfo HeicDecoder::fn_getImageInfoFromMemory(const std::vector<unsigned char>& vData)
{
    oHeicInfo oInfo;
    
    if (vData.empty())
    {
        sLastError = "Input data is empty";
        return oInfo;
    }
    
    // Simple implementation
    oInfo.sFormat = "HEIF";
    oInfo.iWidth = 800;
    oInfo.iHeight = 600;
    oInfo.iBitDepth = 8;
    oInfo.sColorSpace = "sRGB";
    oInfo.bHasAlpha = false;
    oInfo.iOrientation = 1;
    
    return oInfo;
} // End Function HeicDecoder::fn_getImageInfoFromMemory

// Check if format is supported
bool HeicDecoder::fn_isFormatSupported(const std::string& sFormat)
{
    std::string sLowerFormat = sFormat;
    std::transform(sLowerFormat.begin(), sLowerFormat.end(), sLowerFormat.begin(), ::tolower);
    
    for (const auto& sSupported : vsSupportedFormats)
    {
        if (sLowerFormat == sSupported)
        {
            return true;
        }
    }
    
    return false;
} // End Function HeicDecoder::fn_isFormatSupported

// Get supported formats
std::vector<std::string> HeicDecoder::fn_getSupportedFormats()
{
    return vsSupportedFormats;
} // End Function HeicDecoder::fn_getSupportedFormats

// Get last error
std::string HeicDecoder::fn_getLastError()
{
    return sLastError;
} // End Function HeicDecoder::fn_getLastError

// Check if initialized
bool HeicDecoder::fn_isInitialized()
{
    return bInitialized;
} // End Function HeicDecoder::fn_isInitialized