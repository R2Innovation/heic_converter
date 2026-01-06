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
#include "file_utils.h"
#include "config.h"      // Add this for oConfig

// Simplified ConversionOptions
struct ConversionOptions
{
    std::string sOutputFormat;
    int iQuality;
    bool bKeepMetadata;      // Simple flag
    bool bOverwrite;
    std::string sOutputDirectory;
    int iThreadCount;
    bool bVerbose;
    float fScaleFactor;
    bool bPreserveTimestamps; // Simple timestamp flag
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
                                      
    // Simple metadata check
    bool fn_isHeicFormat(const std::string& sFilePath);
    
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
    
    // NEW: Add the missing function declaration
    bool fn_fallbackSystemConversion(const std::string& sInputPath, 
                                     const std::string& sOutputPath);
};

#endif // CONVERTER_H