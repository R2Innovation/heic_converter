#include "converter.h"
#include "image_processor.h"
#include "format_encoder.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sys/statvfs.h>
#include <algorithm>
#include <memory>
#include <ctime>
#include <cstring>
#include <utime.h>
#include <sys/stat.h>

#ifdef HAVE_LIBHEIF
#include <libheif/heif.h>
#endif

// Constructor
Converter::Converter()
{
    // Initialize members
    m_pLogger = std::make_shared<oLogger>();
    m_pImageProcessor = std::make_shared<ImageProcessor>(m_pLogger.get());
    m_pBatchProcessor = std::make_shared<BatchProcessor>();
} // End Constructor

// Destructor
Converter::~Converter()
{
    // Cleanup
    m_pImageProcessor.reset();
    m_pBatchProcessor.reset();
    m_pLogger.reset();
} // End Destructor

// Function: fn_initialize
int Converter::fn_initialize(const oConfig& oCurrentConfig)
{
    // Simple initialization
    m_pLogger->fn_logInfo("Converter initialized");
    return ERROR_SUCCESS;
} // End Function fn_initialize

// Function: fn_convertFile
int Converter::fn_convertFile(const std::string& sInputPath, 
                              const std::string& sOutputPath)
{
    m_pLogger->fn_logInfo("Converting: " + sInputPath + " to " + sOutputPath);
    
    // Check if input file exists
    if (!std::filesystem::exists(sInputPath)) {
        m_pLogger->fn_logError("Input file does not exist: " + sInputPath);
        return ERROR_FILE_NOT_FOUND;
    }
    
    // Check if output directory exists
    std::filesystem::path outputPath(sOutputPath);
    std::filesystem::path outputDir = outputPath.parent_path();
    
    if (!outputDir.empty() && !std::filesystem::exists(outputDir)) {
        if (!std::filesystem::create_directories(outputDir)) {
            m_pLogger->fn_logError("Failed to create output directory: " + outputDir.string());
            return ERROR_WRITE_PERMISSION;
        }
    }
    
    // Check if output file already exists
    if (std::filesystem::exists(sOutputPath)) {
        m_pLogger->fn_logWarning("Output file already exists: " + sOutputPath);
        // For now, we'll overwrite
    }
    
    // Extract format from output path
    std::string sOutputFormat = outputPath.extension().string();
    if (sOutputFormat.empty()) {
        sOutputFormat = ".jpg"; // Default format
    }
    
    // Remove the dot from format for image processor
    std::string formatWithoutDot = sOutputFormat;
    if (!formatWithoutDot.empty() && formatWithoutDot[0] == '.') {
        formatWithoutDot = formatWithoutDot.substr(1);
    }
    
    // Create default conversion options (preserve metadata by default)
    ConversionOptions oOptions;
    oOptions.sOutputFormat = formatWithoutDot;
    oOptions.iQuality = 85;
    oOptions.bKeepMetadata = true;
    oOptions.bOverwrite = true;
    oOptions.bVerbose = true;
    oOptions.bPreserveTimestamps = true;
    oOptions.bPreserveEXIF = true;
    oOptions.bPreserveXMP = true;
    oOptions.bPreserveIPTC = true;
    oOptions.bPreserveGPS = true;
    
    // Get original file timestamps
    FileTimestamps oOriginalTimestamps;
    if (oOptions.bPreserveTimestamps) {
        oOriginalTimestamps = ::fn_getFileTimestamps(sInputPath); // Use global function
    }
    
    // Extract metadata if needed
    ImageMetadata oMetadata;
    if (oOptions.bKeepMetadata || oOptions.bPreserveEXIF || 
        oOptions.bPreserveXMP || oOptions.bPreserveIPTC || oOptions.bPreserveGPS) {
        oMetadata = fn_extractMetadata(sInputPath);
    }
    
    // Use ImageProcessor to convert the file with metadata
    bool success = m_pImageProcessor->fn_convertImageWithMetadata(
        sInputPath,
        sOutputPath,
        formatWithoutDot,
        85, // Default quality
        oMetadata.vExifData,  // Pass EXIF data
        oMetadata.vXmpData,   // Pass XMP data
        oMetadata.vIptcData   // Pass IPTC data
    );
    
    if (!success) {
        m_pLogger->fn_logError("Conversion failed: " + sInputPath);
        return ERROR_DECODING_FAILED;
    }
    
    // Write metadata to output file if needed
    if (oOptions.bKeepMetadata && !oMetadata.vExifData.empty()) {
        bool metadataSuccess = fn_writeMetadata(sOutputPath, oMetadata);
        if (!metadataSuccess) {
            m_pLogger->fn_logWarning("Failed to write metadata to: " + sOutputPath);
        }
    }
    
    // Copy timestamps if needed
    if (oOptions.bPreserveTimestamps) {
        bool timestampSuccess = fn_setFileTimestamps(sOutputPath, oOriginalTimestamps);
        if (!timestampSuccess) {
            m_pLogger->fn_logWarning("Failed to copy timestamps to: " + sOutputPath);
        }
    }
    
    m_pLogger->fn_logSuccess("Successfully converted: " + sInputPath + " to " + sOutputPath);
    return ERROR_SUCCESS;
} // End Function fn_convertFile

// NEW: Extract metadata from HEIC/HEIF file
ImageMetadata Converter::fn_extractMetadata(const std::string& sFilePath)
{
    ImageMetadata oMetadata;
    
    #ifdef HAVE_LIBHEIF
    try {
        struct heif_context* pContext = heif_context_alloc();
        if (!pContext) {
            m_pLogger->fn_logError("Failed to allocate HEIF context");
            return oMetadata;
        }
        
        struct heif_error err = heif_context_read_from_file(pContext, sFilePath.c_str(), nullptr);
        if (err.code != heif_error_Ok) {
            m_pLogger->fn_logError("Failed to read HEIF file: " + std::string(err.message));
            heif_context_free(pContext);
            return oMetadata;
        }
        
        struct heif_image_handle* pHandle = nullptr;
        err = heif_context_get_primary_image_handle(pContext, &pHandle);
        if (err.code != heif_error_Ok) {
            m_pLogger->fn_logError("Failed to get primary image handle: " + std::string(err.message));
            heif_context_free(pContext);
            return oMetadata;
        }
        
        // Get EXIF metadata
        heif_item_id exif_id;
        int count = heif_image_handle_get_list_of_metadata_block_IDs(pHandle, "Exif", &exif_id, 1);
        if (count > 0) {
            size_t exif_size = heif_image_handle_get_metadata_size(pHandle, exif_id);
            oMetadata.vExifData.resize(exif_size);
            err = heif_image_handle_get_metadata(pHandle, exif_id, oMetadata.vExifData.data());
            if (err.code != heif_error_Ok) {
                m_pLogger->fn_logWarning("Failed to read EXIF metadata");
                oMetadata.vExifData.clear();
            }
        }
        
        // Get XMP metadata
        heif_item_id xmp_id;
        count = heif_image_handle_get_list_of_metadata_block_IDs(pHandle, "mime", &xmp_id, 1);
        if (count > 0) {
            const char* mime_type = heif_image_handle_get_metadata_content_type(pHandle, xmp_id);
            if (mime_type && std::strstr(mime_type, "xmp")) {
                size_t xmp_size = heif_image_handle_get_metadata_size(pHandle, xmp_id);
                oMetadata.vXmpData.resize(xmp_size);
                err = heif_image_handle_get_metadata(pHandle, xmp_id, oMetadata.vXmpData.data());
                if (err.code != heif_error_Ok) {
                    m_pLogger->fn_logWarning("Failed to read XMP metadata");
                    oMetadata.vXmpData.clear();
                }
            }
        }
        
        // Get image properties for metadata
        int width = heif_image_handle_get_width(pHandle);
        int height = heif_image_handle_get_height(pHandle);
        bool has_alpha = heif_image_handle_has_alpha_channel(pHandle);
        
        // Get color profile if available
        size_t icc_size = heif_image_handle_get_raw_color_profile_size(pHandle);
        if (icc_size > 0) {
            std::vector<unsigned char> icc_profile(icc_size);
            err = heif_image_handle_get_raw_color_profile(pHandle, icc_profile.data());
            if (err.code == heif_error_Ok) {
                // Store color profile if needed
            }
        }
        
        // Cleanup
        heif_image_handle_release(pHandle);
        heif_context_free(pContext);
        
        m_pLogger->fn_logInfo("Successfully extracted metadata from: " + sFilePath);
        
    } catch (const std::exception& e) {
        m_pLogger->fn_logError("Exception extracting metadata: " + std::string(e.what()));
    }
    #else
    m_pLogger->fn_logWarning("libheif not available for metadata extraction");
    #endif
    
    return oMetadata;
} // End Function fn_extractMetadata

// NEW: Write metadata to output file
bool Converter::fn_writeMetadata(const std::string& sFilePath, 
                                 const ImageMetadata& oMetadata)
{
    // Implementation depends on output format
    std::string sExtension = std::filesystem::path(sFilePath).extension().string();
    std::transform(sExtension.begin(), sExtension.end(), sExtension.begin(), ::tolower);
    
    if (sExtension == ".jpg" || sExtension == ".jpeg") {
        // Write EXIF to JPEG
        return fn_writeJpegMetadata(sFilePath, oMetadata);
    } else if (sExtension == ".png") {
        // Write metadata to PNG
        return fn_writePngMetadata(sFilePath, oMetadata);
    } else if (sExtension == ".tiff" || sExtension == ".tif") {
        // Write metadata to TIFF
        return fn_writeTiffMetadata(sFilePath, oMetadata);
    }
    
    // Other formats might not support metadata
    m_pLogger->fn_logWarning("Metadata not supported for format: " + sExtension);
    return false;
} // End Function fn_writeMetadata

// Helper: Write metadata to JPEG
bool Converter::fn_writeJpegMetadata(const std::string& sFilePath, 
                                     const ImageMetadata& oMetadata)
{
    #ifdef HAVE_JPEG
    // For simplicity, we'll use a basic approach
    // In production, you'd want to use a library like libexif or write proper APP1 marker
    m_pLogger->fn_logInfo("JPG metadata writing simplified - EXIF data size: " + 
                         std::to_string(oMetadata.vExifData.size()) + " bytes");
    
    // Check if we can just append the EXIF data
    if (!oMetadata.vExifData.empty() && oMetadata.vExifData.size() < 65535) {
        // Simple approach: read the JPEG, find where to insert EXIF
        std::ifstream inFile(sFilePath, std::ios::binary);
        if (!inFile) {
            m_pLogger->fn_logError("Cannot open JPEG file: " + sFilePath);
            return false;
        }
        
        // Read the entire file
        std::vector<unsigned char> fileData((std::istreambuf_iterator<char>(inFile)),
                                           std::istreambuf_iterator<char>());
        inFile.close();
        
        // Create new file with EXIF
        std::ofstream outFile(sFilePath, std::ios::binary);
        if (!outFile) {
            m_pLogger->fn_logError("Cannot recreate JPEG file: " + sFilePath);
            return false;
        }
        
        // Find SOI marker (0xFF 0xD8)
        size_t soiPos = 0;
        for (size_t i = 0; i < fileData.size() - 1; i++) {
            if (fileData[i] == 0xFF && fileData[i+1] == 0xD8) {
                soiPos = i;
                break;
            }
        }
        
        if (soiPos != 0) {
            m_pLogger->fn_logError("Invalid JPEG file: " + sFilePath);
            return false;
        }
        
        // Write SOI
        outFile.write(reinterpret_cast<const char*>(fileData.data()), 2);
        
        // Write EXIF APP1 marker
        // APP1 marker: 0xFF 0xE1
        outFile.put(0xFF);
        outFile.put(0xE1);
        
        // Length (including 2 bytes for length)
        unsigned short length = oMetadata.vExifData.size() + 2 + 6; // +6 for "Exif\0\0"
        outFile.put((length >> 8) & 0xFF);
        outFile.put(length & 0xFF);
        
        // EXIF identifier
        outFile.write("Exif\0\0", 6);
        
        // EXIF data
        outFile.write(reinterpret_cast<const char*>(oMetadata.vExifData.data()), 
                     oMetadata.vExifData.size());
        
        // Write rest of file
        outFile.write(reinterpret_cast<const char*>(fileData.data() + 2), 
                     fileData.size() - 2);
        
        outFile.close();
        m_pLogger->fn_logInfo("Successfully wrote EXIF metadata to JPEG: " + sFilePath);
        return true;
    }
    
    return false;
    #else
    m_pLogger->fn_logWarning("JPEG support not available for metadata writing");
    return false;
    #endif
} // End Function fn_writeJpegMetadata

// Helper: Write metadata to PNG
bool Converter::fn_writePngMetadata(const std::string& sFilePath,
                                    const ImageMetadata& oMetadata)
{
    #ifdef HAVE_PNG
    // PNG metadata is complex - we'll just log for now
    m_pLogger->fn_logInfo("PNG metadata writing not fully implemented - EXIF data size: " + 
                         std::to_string(oMetadata.vExifData.size()) + " bytes");
    
    // For now, we'll create a simple tEXt chunk with basic info
    if (!oMetadata.vExifData.empty() && oMetadata.vExifData.size() < 1000) {
        // Simple approach: convert EXIF to base64 and store in tEXt chunk
        std::string exifBase64 = "EXIF data present (" + 
                                std::to_string(oMetadata.vExifData.size()) + 
                                " bytes)";
        
        // Read PNG file
        std::ifstream inFile(sFilePath, std::ios::binary);
        if (!inFile) {
            m_pLogger->fn_logError("Cannot open PNG file: " + sFilePath);
            return false;
        }
        
        std::vector<unsigned char> fileData((std::istreambuf_iterator<char>(inFile)),
                                           std::istreambuf_iterator<char>());
        inFile.close();
        
        // Find PNG signature
        if (fileData.size() < 8 || 
            fileData[0] != 0x89 || fileData[1] != 'P' || 
            fileData[2] != 'N' || fileData[3] != 'G') {
            m_pLogger->fn_logError("Invalid PNG file: " + sFilePath);
            return false;
        }
        
        // For simplicity, we'll just note that metadata is available
        m_pLogger->fn_logInfo("PNG metadata preservation available (requires libpng)");
        return false; // Not fully implemented
    }
    
    return false;
    #else
    m_pLogger->fn_logWarning("PNG support not available for metadata writing");
    return false;
    #endif
} // End Function fn_writePngMetadata

// Helper: Write metadata to TIFF
bool Converter::fn_writeTiffMetadata(const std::string& sFilePath,
                                     const ImageMetadata& oMetadata)
{
    #ifdef HAVE_TIFF
    // TIFF metadata writing would use libtiff
    m_pLogger->fn_logInfo("TIFF metadata writing not implemented - EXIF data size: " + 
                         std::to_string(oMetadata.vExifData.size()) + " bytes");
    
    // For now, just log that we can't do it easily
    m_pLogger->fn_logInfo("TIFF metadata writing requires libtiff with EXIF support");
    return false;
    #else
    m_pLogger->fn_logWarning("TIFF support not available for metadata writing");
    return false;
    #endif
} // End Function fn_writeTiffMetadata

// NEW: Get file timestamps (now using file_utils function)
//FileTimestamps Converter::fn_getFileTimestamps(const std::string& sFilePath)
//{
//    return ::fn_getFileTimestamps(sFilePath); // Use global function from file_utils
//} // End Function fn_getFileTimestamps

// NEW: Set file timestamps
bool Converter::fn_setFileTimestamps(const std::string& sFilePath,
                                     const FileTimestamps& oTimestamps)
{
    struct utimbuf new_times;
    new_times.actime = oTimestamps.tAccessTime;
    new_times.modtime = oTimestamps.tModificationTime;
    
    if (utime(sFilePath.c_str(), &new_times) == 0) {
        m_pLogger->fn_logInfo("Successfully copied timestamps to: " + sFilePath);
        return true;
    } else {
        m_pLogger->fn_logWarning("Failed to set timestamps for: " + sFilePath);
        return false;
    }
} // End Function fn_setFileTimestamps

// Set logger
void Converter::fn_setLogger(std::shared_ptr<oLogger> pLogger)
{
    m_pLogger = pLogger;
} // End Function fn_setLogger

// Get logger
std::shared_ptr<oLogger> Converter::fn_getLogger() const
{
    return m_pLogger;
} // End Function fn_getLogger

// Set image processor
void Converter::fn_setImageProcessor(std::shared_ptr<ImageProcessor> pProcessor)
{
    m_pImageProcessor = pProcessor;
} // End Function fn_setImageProcessor

// Set batch processor
void Converter::fn_setBatchProcessor(std::shared_ptr<BatchProcessor> pProcessor)
{
    m_pBatchProcessor = pProcessor;
} // End Function fn_setBatchProcessor

// Local Function: fn_isHeicFormat
bool Converter::fn_isHeicFormat(const std::string& sFilePath)
{
    std::string sExtension = std::filesystem::path(sFilePath).extension().string();
    std::transform(sExtension.begin(), sExtension.end(), sExtension.begin(), ::tolower);
    
    return (sExtension == ".heic" || sExtension == ".heif");
} // End Function fn_isHeicFormat

// Placeholder implementations for other functions
bool Converter::fn_convertSingleFile(const std::string& sInputPath, 
                                     const std::string& sOutputPath, 
                                     const ConversionOptions& oOptions)
{
    return fn_convertFile(sInputPath, sOutputPath) == ERROR_SUCCESS;
}

bool Converter::fn_convertBatch(const std::vector<std::string>& vsInputPaths, 
                               const std::string& sOutputDir, 
                               const ConversionOptions& oOptions)
{
    // TODO: Implement batch conversion
    return false;
}

bool Converter::fn_convertDirectory(const std::string& sInputDir, 
                                   const std::string& sOutputDir, 
                                   const ConversionOptions& oOptions)
{
    // TODO: Implement directory conversion
    return false;
}

bool Converter::fn_validateInputFile(const std::string& sFilePath)
{
    return std::filesystem::exists(sFilePath);
}

bool Converter::fn_validateOutputFormat(const std::string& sFormat)
{
    return true;
}

std::string Converter::fn_generateOutputPath(const std::string& sInputPath, 
                                            const std::string& sOutputDir, 
                                            const std::string& sFormat)
{
    std::filesystem::path inputPath(sInputPath);
    std::string stem = inputPath.stem().string();
    std::filesystem::path outputPath(sOutputDir);
    outputPath /= (stem + "." + sFormat);
    return outputPath.string();
}

bool Converter::fn_initializeCodecs()
{
    return true;
}

bool Converter::fn_cleanupTempFiles()
{
    return true;
}

bool Converter::fn_checkDiskSpace(const std::string& sPath, long long iRequiredBytes)
{
    return true;
}

bool Converter::fn_createDirectory(const std::string& sPath)
{
    return std::filesystem::create_directories(sPath);
}

// NEW: Copy raw metadata from input to output
bool Converter::fn_copyRawMetadata(const std::string& sInputPath,
                                   const std::string& sOutputPath)
{
    // This is a simplified approach - in reality, metadata copying
    // is format-specific and complex
    m_pLogger->fn_logInfo("Copying metadata from " + sInputPath + " to " + sOutputPath);
    return true;
}

// NEW: Process and write metadata
bool Converter::fn_processMetadata(const ImageMetadata& oMetadata,
                                   const std::string& sOutputPath)
{
    return fn_writeMetadata(sOutputPath, oMetadata);
}