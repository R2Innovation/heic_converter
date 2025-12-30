#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <ctime>

// Include actual headers instead of forward declarations
#include "logger.h"      // Contains oLogger class
#include "image_processor.h"
#include "batch_processor.h"
#include "file_utils.h"  // Already includes FileTimestamps
#include "config.h"      // Add this for oConfig

// Metadata structures
struct ImageMetadata
{
    std::string sMake;                // Camera manufacturer
    std::string sModel;               // Camera model
    std::string sSoftware;            // Software used
    std::time_t tDateTime;            // Original date/time
    std::time_t tDateTimeOriginal;    // Original date/time (EXIF)
    std::time_t tDateTimeDigitized;   // Digitized date/time
    std::string sOrientation;         // Image orientation
    float fGPSLatitude;               // GPS latitude
    float fGPSLongitude;              // GPS longitude
    float fGPSAltitude;               // GPS altitude
    std::string sLensMake;            // Lens manufacturer
    std::string sLensModel;           // Lens model
    int iISO;                         // ISO speed
    float fExposureTime;              // Exposure time
    float fFNumber;                   // F-stop
    float fFocalLength;               // Focal length
    bool bFlash;                      // Flash fired
    std::string sColorSpace;          // Color space
    std::vector<unsigned char> vExifData;  // Raw EXIF data
    std::vector<unsigned char> vXmpData;   // Raw XMP data
    std::vector<unsigned char> vIptcData;  // Raw IPTC data
};

// REMOVED duplicate FileTimestamps struct - using the one from file_utils.h

struct ConversionOptions
{
    std::string sOutputFormat;
    int iQuality;
    bool bKeepMetadata;
    bool bOverwrite;
    std::string sOutputDirectory;
    int iThreadCount;
    bool bVerbose;
    float fScaleFactor;
    bool bPreserveTimestamps;     // NEW: Preserve file timestamps
    bool bPreserveEXIF;           // NEW: Preserve EXIF metadata
    bool bPreserveXMP;            // NEW: Preserve XMP metadata
    bool bPreserveIPTC;           // NEW: Preserve IPTC metadata
    bool bPreserveGPS;            // NEW: Preserve GPS data
};

class Converter
{
public:
    Converter();
    ~Converter();

    // Main conversion functions
    int fn_initialize(const oConfig& oCurrentConfig);
    int fn_convertFile(const std::string& sInputPath,  
                       const std::string& sOutputPath);
    
    // Original functions (keep these for compatibility)
    bool fn_convertSingleFile(const std::string& sInputPath, 
                              const std::string& sOutputPath, 
                              const ConversionOptions& oOptions);
                              
    bool fn_convertBatch(const std::vector<std::string>& vsInputPaths, 
                         const std::string& sOutputDir, 
                         const ConversionOptions& oOptions);
                         
    bool fn_convertDirectory(const std::string& sInputDir, 
                             const std::string& sOutputDir, 
                             const ConversionOptions& oOptions);

    // Utility functions
    bool fn_validateInputFile(const std::string& sFilePath);
    bool fn_validateOutputFormat(const std::string& sFormat);
    std::string fn_generateOutputPath(const std::string& sInputPath, 
                                      const std::string& sOutputDir, 
                                      const std::string& sFormat);
                                      
    // NEW: Metadata functions
    ImageMetadata fn_extractMetadata(const std::string& sFilePath);
    bool fn_writeMetadata(const std::string& sFilePath, 
                         const ImageMetadata& oMetadata);
    // FileTimestamps fn_getFileTimestamps(const std::string& sFilePath); // Moved to file_utils
    bool fn_setFileTimestamps(const std::string& sFilePath,
                             const FileTimestamps& oTimestamps);
    
    // Getters and setters
    void fn_setLogger(std::shared_ptr<oLogger> pLogger);
    std::shared_ptr<oLogger> fn_getLogger() const;
    
    void fn_setImageProcessor(std::shared_ptr<ImageProcessor> pProcessor);
    void fn_setBatchProcessor(std::shared_ptr<BatchProcessor> pProcessor);

private:
    // Private member variables
    std::shared_ptr<ImageProcessor> m_pImageProcessor;
    std::shared_ptr<BatchProcessor> m_pBatchProcessor;
    std::shared_ptr<oLogger> m_pLogger;
    
    // Private helper functions
    bool fn_initializeCodecs();
    bool fn_cleanupTempFiles();
    bool fn_checkDiskSpace(const std::string& sPath, long long iRequiredBytes);
    bool fn_createDirectory(const std::string& sPath);
    
    bool fn_isHeicFormat(const std::string& sFilePath);
    
    bool fn_prepareConversion(const std::string& sInputPath, 
                              const std::string& sOutputPath, 
                              const ConversionOptions& oOptions);
                              
    bool fn_executeConversion(const std::string& sInputPath, 
                              const std::string& sOutputPath, 
                              const ConversionOptions& oOptions);
                              
    bool fn_finalizeConversion(const std::string& sInputPath, 
                               const std::string& sOutputPath, 
                               const ConversionOptions& oOptions, 
                               bool bSuccess);
                               
    // NEW: Private metadata helpers
    bool fn_copyRawMetadata(const std::string& sInputPath,
                           const std::string& sOutputPath);
    bool fn_processMetadata(const ImageMetadata& oMetadata,
                           const std::string& sOutputPath);
    
    // Helper functions for metadata writing (declared here)
    bool fn_writeJpegMetadata(const std::string& sFilePath, const ImageMetadata& oMetadata);
    bool fn_writePngMetadata(const std::string& sFilePath, const ImageMetadata& oMetadata);
    bool fn_writeTiffMetadata(const std::string& sFilePath, const ImageMetadata& oMetadata);
};

#endif // CONVERTER_H