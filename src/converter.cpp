#include "converter.h"
#include "image_processor.h"
#include "format_encoder.h"
#include "metadata_handler.h"
#include "logger.h"
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
    
    // Extract EXIF metadata from HEIC file
    std::vector<unsigned char> exifData;
    MetadataHandler metadataHandler;
    
    if (fn_isHeicFormat(sInputPath)) {
        m_pLogger->fn_logInfo("Extracting metadata from HEIC file...");
        exifData = metadataHandler.extractExifFromHeic(sInputPath);
        
        if (!exifData.empty()) {
            m_pLogger->fn_logInfo("Extracted " + std::to_string(exifData.size()) + " bytes of EXIF data");
        } else {
            m_pLogger->fn_logInfo("No EXIF metadata found or failed to extract");
        }
    }
    
    // Use ImageProcessor to convert the file
    bool success = m_pImageProcessor->fn_convertImage(
        sInputPath,
        sOutputPath,
        formatWithoutDot,
        85 // Default quality
    );
    
    if (!success) {
        m_pLogger->fn_logError("Conversion failed: " + sInputPath);
        success = fn_fallbackSystemConversion(sInputPath, sOutputPath);
    }
    
    // Write EXIF metadata to output file if it's a JPEG
    std::string outputExt = outputPath.extension().string();
    std::transform(outputExt.begin(), outputExt.end(), outputExt.begin(), ::tolower);
    
    if ((outputExt == ".jpg" || outputExt == ".jpeg") && !exifData.empty()) {
        m_pLogger->fn_logInfo("Writing EXIF metadata to JPEG file...");
        bool metadataWritten = metadataHandler.writeExifToJpeg(sOutputPath, exifData);
        
        if (!metadataWritten) {
            m_pLogger->fn_logWarning("Failed to write EXIF metadata to output file");
        } else {
            m_pLogger->fn_logInfo("EXIF metadata successfully written");
        }
    }
    
    // Copy timestamps from source to destination
    m_pLogger->fn_logInfo("Copying file timestamps...");
    bool timestampsCopied = metadataHandler.copyTimestamps(sInputPath, sOutputPath);
    
    if (!timestampsCopied) {
        m_pLogger->fn_logWarning("Failed to copy file timestamps");
    } else {
        m_pLogger->fn_logInfo("File timestamps successfully copied");
    }
    
    m_pLogger->fn_logSuccess("Successfully converted: " + sInputPath + " to " + sOutputPath);
    return ERROR_SUCCESS;
} // End Function fn_convertFile

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

bool Converter::fn_fallbackSystemConversion(const std::string& sInputPath, 
                                           const std::string& sOutputPath)
{
    if (m_pLogger) {
        m_pLogger->fn_logWarning("System fallback not implemented for: " + sInputPath);
    }
    return false;
}