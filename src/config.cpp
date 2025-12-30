// config.cpp - Fix all missing function implementations
#include "config.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>

// Local Function
std::string fn_getDefaultOutputFormat() 
{ // Begin fn_getDefaultOutputFormat
    return ".jpg"; // Default to JPEG
} // End Function fn_getDefaultOutputFormat

// Local Function
std::string fn_getDefaultOutputPath(const std::string& sInputPath) 
{ // Begin fn_getDefaultOutputPath
    std::filesystem::path oInputPath(sInputPath);
    
    if (std::filesystem::is_directory(oInputPath)) 
    { // Begin if
        return sInputPath; // Return the same directory
    } // End if(std::filesystem::is_directory(oInputPath))
    
    if (oInputPath.has_extension()) 
    { // Begin if
        std::string sStem = oInputPath.stem().string();
        return oInputPath.parent_path().string() + "/" + sStem + fn_getDefaultOutputFormat();
    } // End if(oInputPath.has_extension())
    
    return sInputPath + fn_getDefaultOutputFormat();
} // End Function fn_getDefaultOutputPath

// Local Function
bool fn_isSupportedInputFormat(const std::string& sExtension) 
{ // Begin fn_isSupportedInputFormat
    if (sExtension.empty()) {
        return false;
    }
    
    std::string sLowerExtension = sExtension;
    std::transform(sLowerExtension.begin(), sLowerExtension.end(), sLowerExtension.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    for (const auto& sSupportedFormat : vsSUPPORTED_INPUT_FORMATS) 
    { // Begin for
        if (sLowerExtension == sSupportedFormat) 
        { // Begin if
            return true;
        } // End if(sLowerExtension == sSupportedFormat)
    } // End for(const auto& sSupportedFormat : vsSUPPORTED_INPUT_FORMATS)
    
    return false;
} // End Function fn_isSupportedInputFormat

// Local Function
bool fn_isSupportedOutputFormat(const std::string& sExtension) 
{ // Begin fn_isSupportedOutputFormat
    if (sExtension.empty()) {
        return false;
    }
    
    std::string sLowerExtension = sExtension;
    std::transform(sLowerExtension.begin(), sLowerExtension.end(), sLowerExtension.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    for (const auto& sSupportedFormat : vsSUPPORTED_OUTPUT_FORMATS) 
    { // Begin for
        if (sLowerExtension == sSupportedFormat) 
        { // Begin if
            return true;
        } // End if(sLowerExtension == sSupportedFormat)
    } // End for(const auto& sSupportedFormat : vsSUPPORTED_OUTPUT_FORMATS)
    
    return false;
} // End Function fn_isSupportedOutputFormat

// Local Function
std::string fn_normalizeExtension(const std::string& sExtension) 
{ // Begin fn_normalizeExtension
    std::string sNormalized = sExtension;
    
    if (!sNormalized.empty() && sNormalized[0] != '.') 
    { // Begin if
        sNormalized = "." + sNormalized;
    } // End if(!sNormalized.empty() && sNormalized[0] != '.')
    
    std::transform(sNormalized.begin(), sNormalized.end(), sNormalized.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    return sNormalized;
} // End Function fn_normalizeExtension

// Local Function
std::string fn_getMimeTypeForExtension(const std::string& sExtension) 
{ // Begin fn_getMimeTypeForExtension
    std::string sNormalizedExtension = fn_normalizeExtension(sExtension);
    
    // Simple mapping
    if (sNormalizedExtension == ".jpg" || sNormalizedExtension == ".jpeg") {
        return "image/jpeg";
    } else if (sNormalizedExtension == ".png") {
        return "image/png";
    } else if (sNormalizedExtension == ".bmp") {
        return "image/bmp";
    } else if (sNormalizedExtension == ".tiff" || sNormalizedExtension == ".tif") {
        return "image/tiff";
    } else if (sNormalizedExtension == ".webp") {
        return "image/webp";
    } else if (sNormalizedExtension == ".heic") {
        return "image/heic";
    } else if (sNormalizedExtension == ".heif") {
        return "image/heif";
    }
    
    return "application/octet-stream";
} // End Function fn_getMimeTypeForExtension

// Local Function
std::string fn_getExtensionForMimeType(const std::string& sMimeType) 
{ // Begin fn_getExtensionForMimeType
    if (sMimeType == "image/jpeg") {
        return ".jpg";
    } else if (sMimeType == "image/png") {
        return ".png";
    } else if (sMimeType == "image/bmp") {
        return ".bmp";
    } else if (sMimeType == "image/tiff") {
        return ".tiff";
    } else if (sMimeType == "image/webp") {
        return ".webp";
    } else if (sMimeType == "image/heic") {
        return ".heic";
    } else if (sMimeType == "image/heif") {
        return ".heif";
    }
    
    return "";
} // End Function fn_getExtensionForMimeType

// Local Function
oConfig fn_getDefaultConfig() 
{ // Begin fn_getDefaultConfig
    oConfig oDefaultConfig;
    
    oDefaultConfig.sInputPath = "";
    oDefaultConfig.sOutputPath = "";
    oDefaultConfig.sOutputFormat = fn_getDefaultOutputFormat();
    oDefaultConfig.iJpegQuality = iDEFAULT_JPEG_QUALITY;
    oDefaultConfig.iPngCompression = iDEFAULT_PNG_COMPRESSION;
    oDefaultConfig.iThreadCount = iDEFAULT_THREAD_COUNT;
    oDefaultConfig.fScaleFactor = fDEFAULT_SCALE_FACTOR;
    oDefaultConfig.bOverwrite = bDEFAULT_OVERWRITE;
    oDefaultConfig.bVerbose = bDEFAULT_VERBOSE;
    oDefaultConfig.bRecursive = bDEFAULT_RECURSIVE;
    oDefaultConfig.bKeepMetadata = bDEFAULT_PRESERVE_METADATA;
    oDefaultConfig.bStripColorProfile = false;
    oDefaultConfig.bPreserveTimestamps = bDEFAULT_PRESERVE_TIMESTAMPS;  // NEW
    oDefaultConfig.bPreserveEXIF = bDEFAULT_PRESERVE_EXIF;              // NEW
    oDefaultConfig.bPreserveXMP = bDEFAULT_PRESERVE_XMP;                // NEW
    oDefaultConfig.bPreserveIPTC = bDEFAULT_PRESERVE_IPTC;              // NEW
    oDefaultConfig.bPreserveGPS = bDEFAULT_PRESERVE_GPS;                // NEW
    
    return oDefaultConfig;
} // End Function fn_getDefaultConfig

// Local Function
void fn_printConfig(const oConfig& oCurrentConfig) 
{ // Begin fn_printConfig
    std::cout << "Current Configuration:" << std::endl;
    std::cout << "  Input Path: " << oCurrentConfig.sInputPath << std::endl;
    std::cout << "  Output Path: " << oCurrentConfig.sOutputPath << std::endl;
    std::cout << "  Output Format: " << oCurrentConfig.sOutputFormat << std::endl;
    std::cout << "  JPEG Quality: " << oCurrentConfig.iJpegQuality << std::endl;
    std::cout << "  PNG Compression: " << oCurrentConfig.iPngCompression << std::endl;
    std::cout << "  Thread Count: " << oCurrentConfig.iThreadCount << std::endl;
    std::cout << "  Scale Factor: " << oCurrentConfig.fScaleFactor << std::endl;
    std::cout << "  Overwrite: " << (oCurrentConfig.bOverwrite ? "true" : "false") << std::endl;
    std::cout << "  Verbose: " << (oCurrentConfig.bVerbose ? "true" : "false") << std::endl;
    std::cout << "  Recursive: " << (oCurrentConfig.bRecursive ? "true" : "false") << std::endl;
    std::cout << "  Keep Metadata: " << (oCurrentConfig.bKeepMetadata ? "true" : "false") << std::endl;
    std::cout << "  Strip Color Profile: " << (oCurrentConfig.bStripColorProfile ? "true" : "false") << std::endl;
    std::cout << "  Preserve Timestamps: " << (oCurrentConfig.bPreserveTimestamps ? "true" : "false") << std::endl;  // NEW
    std::cout << "  Preserve EXIF: " << (oCurrentConfig.bPreserveEXIF ? "true" : "false") << std::endl;              // NEW
    std::cout << "  Preserve XMP: " << (oCurrentConfig.bPreserveXMP ? "true" : "false") << std::endl;                // NEW
    std::cout << "  Preserve IPTC: " << (oCurrentConfig.bPreserveIPTC ? "true" : "false") << std::endl;              // NEW
    std::cout << "  Preserve GPS: " << (oCurrentConfig.bPreserveGPS ? "true" : "false") << std::endl;                // NEW
} // End Function fn_printConfig