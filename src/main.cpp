// main.cpp - Main entry point for HEIC Converter
// Author: R Square Innovation Software
// Version: v1.1  // CHANGED: v1.0 to v1.1

#include "converter.h"
#include "batch_processor.h"
#include "config.h"
#include "logger.h"
#include "file_utils.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

// Function Declarations (Local Functions)
void fn_showHelp(); // Local Function
void fn_showVersion(); // Local Function
int fn_parseArguments(int argc, char* argv[], oConfig& oCurrentConfig); // Local Function
int fn_processConversion(const oConfig& oCurrentConfig); // Local Function
void fn_printWelcome(); // Local Function

// Local Function
int main(int argc, char* argv[]) 
{ // Begin main
    fn_printWelcome(); // Local Function
    
    oConfig oCurrentConfig = fn_getDefaultConfig(); // In config.cpp
    oLogger oMainLogger; // In logger.h
    
    // Parse command line arguments
    int iParseResult = fn_parseArguments(argc, argv, oCurrentConfig); // Local Function
    
    if (iParseResult != ERROR_SUCCESS) 
    { // Begin if
        if (iParseResult == ERROR_INVALID_ARGUMENTS) 
        { // Begin if
            fn_showHelp(); // Local Function
        } // End if(iParseResult == ERROR_INVALID_ARGUMENTS)
        return iParseResult; // Return error code
    } // End if(iParseResult != ERROR_SUCCESS)
    
    // Setup logger based on verbose flag
    oMainLogger.fn_setVerbose(oCurrentConfig.bVerbose); // In logger.cpp
    
    // Log configuration if verbose
    if (oCurrentConfig.bVerbose) 
    { // Begin if
        fn_printConfig(oCurrentConfig); // In config.cpp
    } // End if(oCurrentConfig.bVerbose)
    
    // Process conversion
    int iConversionResult = fn_processConversion(oCurrentConfig); // Local Function
    
    if (iConversionResult == ERROR_SUCCESS) 
    { // Begin if
        oMainLogger.fn_logInfo("Conversion completed successfully"); // In logger.cpp
    } 
    else 
    { // Begin else
        oMainLogger.fn_logError("Conversion failed with error code: " + std::to_string(iConversionResult)); // In logger.cpp
    } // End else
    
    return iConversionResult; // Return conversion result
} // End Function main

// Local Function
void fn_showHelp() 
{ // Begin fn_showHelp
    std::cout << "HEIC/HEIF Converter " << sVERSION << " by " << sAUTHOR << std::endl; // In iostream
    std::cout << std::endl; // In iostream
    std::cout << "Usage: " << sPROGRAM_NAME << " [options] <input> [output]" << std::endl; // In iostream
    std::cout << std::endl; // In iostream
    std::cout << "Arguments:" << std::endl; // In iostream
    std::cout << "  <input>              Input file or directory" << std::endl; // In iostream
    std::cout << "  [output]             Output file or directory (optional)" << std::endl; // In iostream
    std::cout << std::endl; // In iostream
    std::cout << "Options:" << std::endl; // In iostream
    std::cout << "  -f, --format FORMAT  Output format (jpg, png, bmp, tiff, webp)" << std::endl; // In iostream
    std::cout << "                       Default: jpg" << std::endl; // In iostream
    std::cout << "  -q, --quality N      JPEG quality (1-100)" << std::endl; // In iostream
    std::cout << "                       Default: " << iDEFAULT_JPEG_QUALITY << std::endl; // In iostream
    std::cout << "  -c, --compression N  PNG compression level (0-9)" << std::endl; // In iostream
    std::cout << "                       Default: " << iDEFAULT_PNG_COMPRESSION << std::endl; // In iostream
    std::cout << "  -s, --scale FACTOR   Scale factor (0.1 to 10.0)" << std::endl; // In iostream
    std::cout << "                       Default: " << fDEFAULT_SCALE_FACTOR << std::endl; // In iostream
    std::cout << "  -t, --threads N      Number of threads for batch processing" << std::endl; // In iostream
    std::cout << "                       Default: " << iDEFAULT_THREAD_COUNT << " (max: " << iMAX_THREAD_COUNT << ")" << std::endl; // In iostream
    std::cout << "  -r, --recursive      Process directories recursively" << std::endl; // In iostream
    std::cout << "  -o, --overwrite      Overwrite existing files" << std::endl; // In iostream
    std::cout << "  -v, --verbose        Enable verbose output" << std::endl; // In iostream
    std::cout << "  --no-metadata        Strip metadata from output" << std::endl; // In iostream
    std::cout << "  --no-timestamps      Do not preserve file timestamps" << std::endl; // NEW
    std::cout << "  --no-exif            Strip EXIF metadata" << std::endl; // NEW
    std::cout << "  --no-xmp             Strip XMP metadata" << std::endl; // NEW
    std::cout << "  --no-iptc            Strip IPTC metadata" << std::endl; // NEW
    std::cout << "  --no-gps             Strip GPS location data" << std::endl; // NEW
    std::cout << "  --no-color-profile   Strip color profile from output" << std::endl; // In iostream
    std::cout << "  -h, --help           Show this help message" << std::endl; // In iostream
    std::cout << "  --version            Show version information" << std::endl; // In iostream
    std::cout << std::endl; // In iostream
    std::cout << "Examples:" << std::endl; // In iostream
    std::cout << "  " << sPROGRAM_NAME << " image.heic" << std::endl; // In iostream
    std::cout << "  " << sPROGRAM_NAME << " image.heic image.jpg" << std::endl; // In iostream
    std::cout << "  " << sPROGRAM_NAME << " -f png -q 90 image.heic" << std::endl; // In iostream
    std::cout << "  " << sPROGRAM_NAME << " -r -f jpg --no-gps ./input_dir ./output_dir" << std::endl; // NEW example
    std::cout << "  " << sPROGRAM_NAME << " -t 8 -o -v ./photos ./converted" << std::endl; // In iostream
    std::cout << std::endl; // In iostream
    std::cout << "Supported input formats: .heic, .heif" << std::endl; // In iostream
    std::cout << "Supported output formats: .jpg, .jpeg, .png, .bmp, .tiff, .webp" << std::endl; // In iostream
    std::cout << "Version 1.1 features: Metadata preservation, timestamp copying" << std::endl; // NEW
} // End Function fn_showHelp

// Local Function
void fn_showVersion() 
{ // Begin fn_showVersion
    std::cout << sPROGRAM_NAME << " " << sVERSION << std::endl; // In iostream
    std::cout << "Build type: " << sBUILD_TYPE << std::endl; // In iostream
    std::cout << "Author: " << sAUTHOR << std::endl; // In iostream
    std::cout << "Embedded codecs: Enabled" << std::endl; // In iostream
    std::cout << "Metadata preservation: Enabled" << std::endl; // NEW
    std::cout << "Timestamp preservation: Enabled" << std::endl; // NEW
} // End Function fn_showVersion

// Local Function
void fn_printWelcome() 
{ // Begin fn_printWelcome
    std::cout << "========================================" << std::endl; // In iostream
    std::cout << "HEIC/HEIF Converter " << sVERSION << std::endl; // In iostream
    std::cout << "by " << sAUTHOR << std::endl; // In iostream
    std::cout << "Build: " << sBUILD_TYPE << std::endl; // In iostream
    std::cout << "Features: Metadata and timestamp preservation" << std::endl; // NEW
    std::cout << "========================================" << std::endl; // In iostream
    std::cout << std::endl; // In iostream
} // End Function fn_printWelcome

// Local Function
int fn_parseArguments(int argc, char* argv[], oConfig& oCurrentConfig) 
{ // Begin fn_parseArguments
    if (argc < 2) 
    { // Begin if
        return ERROR_INVALID_ARGUMENTS; // Not enough arguments
    } // End if(argc < 2)
    
    std::vector<std::string> vsArguments; // Local Function
    for (int i = 1; i < argc; i++) 
    { // Begin for
        vsArguments.push_back(argv[i]); // In vector
    } // End for(int i = 1; i < argc; i++)
    
    size_t iCurrentIndex = 0; // Local Function
    bool bInputFound = false; // Local Function
    bool bOutputFound = false; // Local Function
    
    while (iCurrentIndex < vsArguments.size()) 
    { // Begin while
        std::string sCurrentArg = vsArguments[iCurrentIndex]; // Local Function
        
        // Check for help flag
        if (sCurrentArg == "-h" || sCurrentArg == "--help") 
        { // Begin if
            fn_showHelp(); // Local Function
            return ERROR_SUCCESS; // Early exit with success
        } // End if(sCurrentArg == "-h" || sCurrentArg == "--help")
        
        // Check for version flag
        if (sCurrentArg == "--version") 
        { // Begin if
            fn_showVersion(); // Local Function
            return ERROR_SUCCESS; // Early exit with success
        } // End if(sCurrentArg == "--version")
        
        // Check for format flag
        if (sCurrentArg == "-f" || sCurrentArg == "--format") 
        { // Begin if
            if (iCurrentIndex + 1 >= vsArguments.size()) 
            { // Begin if
                std::cerr << "Error: Missing argument for format" << std::endl; // In iostream
                return ERROR_INVALID_ARGUMENTS; // Missing argument
            } // End if(iCurrentIndex + 1 >= vsArguments.size())
            
            std::string sFormat = vsArguments[iCurrentIndex + 1]; // Local Function
            std::string sNormalizedFormat = fn_normalizeExtension(sFormat); // In config.cpp
            
            if (!fn_isSupportedOutputFormat(sNormalizedFormat)) 
            { // Begin if
                std::cerr << "Error: Unsupported output format: " << sFormat << std::endl; // In iostream
                return ERROR_UNSUPPORTED_FORMAT; // Unsupported format
            } // End if(!fn_isSupportedOutputFormat(sNormalizedFormat))
            
            oCurrentConfig.sOutputFormat = sNormalizedFormat; // Local Function
            iCurrentIndex += 2; // Skip format and its argument
            continue; // Continue to next argument
        } // End if(sCurrentArg == "-f" || sCurrentArg == "--format")
        
        // Check for quality flag
        if (sCurrentArg == "-q" || sCurrentArg == "--quality") 
        { // Begin if
            if (iCurrentIndex + 1 >= vsArguments.size()) 
            { // Begin if
                std::cerr << "Error: Missing argument for quality" << std::endl; // In iostream
                return ERROR_INVALID_ARGUMENTS; // Missing argument
            } // End if(iCurrentIndex + 1 >= vsArguments.size())
            
            std::string sQuality = vsArguments[iCurrentIndex + 1]; // Local Function
            try 
            { // Begin try
                int iQuality = std::stoi(sQuality); // Local Function
                if (iQuality < 1 || iQuality > 100) 
                { // Begin if
                    std::cerr << "Error: Quality must be between 1 and 100" << std::endl; // In iostream
                    return ERROR_INVALID_ARGUMENTS; // Invalid range
                } // End if(iQuality < 1 || iQuality > 100)
                
                oCurrentConfig.iJpegQuality = iQuality; // Local Function
            } 
            catch (const std::exception& e) 
            { // Begin catch
                std::cerr << "Error: Invalid quality value: " << sQuality << std::endl; // In iostream
                return ERROR_INVALID_ARGUMENTS; // Invalid value
            } // End catch(const std::exception& e)
            
            iCurrentIndex += 2; // Skip quality and its argument
            continue; // Continue to next argument
        } // End if(sCurrentArg == "-q" || sCurrentArg == "--quality")
        
        // Check for compression flag
        if (sCurrentArg == "-c" || sCurrentArg == "--compression") 
        { // Begin if
            if (iCurrentIndex + 1 >= vsArguments.size()) 
            { // Begin if
                std::cerr << "Error: Missing argument for compression" << std::endl; // In iostream
                return ERROR_INVALID_ARGUMENTS; // Missing argument
            } // End if(iCurrentIndex + 1 >= vsArguments.size())
            
            std::string sCompression = vsArguments[iCurrentIndex + 1]; // Local Function
            try 
            { // Begin try
                int iCompression = std::stoi(sCompression); // Local Function
                if (iCompression < 0 || iCompression > 9) 
                { // Begin if
                    std::cerr << "Error: Compression must be between 0 and 9" << std::endl; // In iostream
                    return ERROR_INVALID_ARGUMENTS; // Invalid range
                } // End if(iCompression < 0 || iCompression > 9)
                
                oCurrentConfig.iPngCompression = iCompression; // Local Function
            } 
            catch (const std::exception& e) 
            { // Begin catch
                std::cerr << "Error: Invalid compression value: " + sCompression << std::endl; // In iostream
                return ERROR_INVALID_ARGUMENTS; // Invalid value
            } // End catch(const std::exception& e)
            
            iCurrentIndex += 2; // Skip compression and its argument
            continue; // Continue to next argument
        } // End if(sCurrentArg == "-c" || sCurrentArg == "--compression")
        
        // Check for scale flag
        if (sCurrentArg == "-s" || sCurrentArg == "--scale") 
        { // Begin if
            if (iCurrentIndex + 1 >= vsArguments.size()) 
            { // Begin if
                std::cerr << "Error: Missing argument for scale" << std::endl; // In iostream
                return ERROR_INVALID_ARGUMENTS; // Missing argument
            } // End if(iCurrentIndex + 1 >= vsArguments.size())
            
            std::string sScale = vsArguments[iCurrentIndex + 1]; // Local Function
            try 
            { // Begin try
                float fScale = std::stof(sScale); // Local Function
                if (fScale < 0.1f || fScale > 10.0f) 
                { // Begin if
                    std::cerr << "Error: Scale factor must be between 0.1 and 10.0" << std::endl; // In iostream
                    return ERROR_INVALID_ARGUMENTS; // Invalid range
                } // End if(fScale < 0.1f || fScale > 10.0f)
                
                oCurrentConfig.fScaleFactor = fScale; // Local Function
            } 
            catch (const std::exception& e) 
            { // Begin catch
                std::cerr << "Error: Invalid scale factor: " << sScale << std::endl; // In iostream
                return ERROR_INVALID_ARGUMENTS; // Invalid value
            } // End catch(const std::exception& e)
            
            iCurrentIndex += 2; // Skip scale and its argument
            continue; // Continue to next argument
        } // End if(sCurrentArg == "-s" || sCurrentArg == "--scale")
        
        // Check for threads flag
        if (sCurrentArg == "-t" || sCurrentArg == "--threads") 
        { // Begin if
            if (iCurrentIndex + 1 >= vsArguments.size()) 
            { // Begin if
                std::cerr << "Error: Missing argument for threads" << std::endl; // In iostream
                return ERROR_INVALID_ARGUMENTS; // Missing argument
            } // End if(iCurrentIndex + 1 >= vsArguments.size())
            
            std::string sThreads = vsArguments[iCurrentIndex + 1]; // Local Function
            try 
            { // Begin try
                int iThreads = std::stoi(sThreads); // Local Function
                if (iThreads < 1 || iThreads > iMAX_THREAD_COUNT) 
                { // Begin if
                    std::cerr << "Error: Thread count must be between 1 and " << iMAX_THREAD_COUNT << std::endl; // In iostream
                    return ERROR_INVALID_ARGUMENTS; // Invalid range
                } // End if(iThreads < 1 || iThreads > iMAX_THREAD_COUNT)
                
                oCurrentConfig.iThreadCount = iThreads; // Local Function
            } 
            catch (const std::exception& e) 
            { // Begin catch
                std::cerr << "Error: Invalid thread count: " << sThreads << std::endl; // In iostream
                return ERROR_INVALID_ARGUMENTS; // Invalid value
            } // End catch(const std::exception& e)
            
            iCurrentIndex += 2; // Skip threads and its argument
            continue; // Continue to next argument
        } // End if(sCurrentArg == "-t" || sCurrentArg == "--threads")
        
        // Check for boolean flags
        if (sCurrentArg == "-r" || sCurrentArg == "--recursive") 
        { // Begin if
            oCurrentConfig.bRecursive = true; // Local Function
            iCurrentIndex++; // Skip flag
            continue; // Continue to next argument
        } // End if(sCurrentArg == "-r" || sCurrentArg == "--recursive")
        
        if (sCurrentArg == "-o" || sCurrentArg == "--overwrite") 
        { // Begin if
            oCurrentConfig.bOverwrite = true; // Local Function
            iCurrentIndex++; // Skip flag
            continue; // Continue to next argument
        } // End if(sCurrentArg == "-o" || sCurrentArg == "--overwrite")
        
        if (sCurrentArg == "-v" || sCurrentArg == "--verbose") 
        { // Begin if
            oCurrentConfig.bVerbose = true; // Local Function
            iCurrentIndex++; // Skip flag
            continue; // Continue to next argument
        } // End if(sCurrentArg == "-v" || sCurrentArg == "--verbose")
        
        // NEW: Metadata-related flags
        if (sCurrentArg == "--no-metadata") 
        { // Begin if
            oCurrentConfig.bKeepMetadata = false; // Local Function
            oCurrentConfig.bPreserveEXIF = false; // Also disable EXIF
            oCurrentConfig.bPreserveXMP = false;  // Also disable XMP
            oCurrentConfig.bPreserveIPTC = false; // Also disable IPTC
            oCurrentConfig.bPreserveGPS = false;  // Also disable GPS
            iCurrentIndex++; // Skip flag
            continue; // Continue to next argument
        } // End if(sCurrentArg == "--no-metadata")
        
        if (sCurrentArg == "--no-timestamps") 
        { // Begin if
            oCurrentConfig.bPreserveTimestamps = false; // Local Function
            iCurrentIndex++; // Skip flag
            continue; // Continue to next argument
        } // End if(sCurrentArg == "--no-timestamps")
        
        if (sCurrentArg == "--no-exif") 
        { // Begin if
            oCurrentConfig.bPreserveEXIF = false; // Local Function
            iCurrentIndex++; // Skip flag
            continue; // Continue to next argument
        } // End if(sCurrentArg == "--no-exif")
        
        if (sCurrentArg == "--no-xmp") 
        { // Begin if
            oCurrentConfig.bPreserveXMP = false; // Local Function
            iCurrentIndex++; // Skip flag
            continue; // Continue to next argument
        } // End if(sCurrentArg == "--no-xmp")
        
        if (sCurrentArg == "--no-iptc") 
        { // Begin if
            oCurrentConfig.bPreserveIPTC = false; // Local Function
            iCurrentIndex++; // Skip flag
            continue; // Continue to next argument
        } // End if(sCurrentArg == "--no-iptc")
        
        if (sCurrentArg == "--no-gps") 
        { // Begin if
            oCurrentConfig.bPreserveGPS = false; // Local Function
            iCurrentIndex++; // Skip flag
            continue; // Continue to next argument
        } // End if(sCurrentArg == "--no-gps")
        
        if (sCurrentArg == "--no-color-profile") 
        { // Begin if
            oCurrentConfig.bStripColorProfile = true; // Local Function
            iCurrentIndex++; // Skip flag
            continue; // Continue to next argument
        } // End if(sCurrentArg == "--no-color-profile")
        
        // If not a flag, treat as input/output path
        if (!bInputFound) 
        { // Begin if
            oCurrentConfig.sInputPath = sCurrentArg; // Local Function
            bInputFound = true; // Local Function
        } 
        else if (!bOutputFound) 
        { // Begin else if
            oCurrentConfig.sOutputPath = sCurrentArg; // Local Function
            bOutputFound = true; // Local Function
        } 
        else 
        { // Begin else
            std::cerr << "Error: Too many arguments: " << sCurrentArg << std::endl; // In iostream
            return ERROR_INVALID_ARGUMENTS; // Too many arguments
        } // End else
        
        iCurrentIndex++; // Move to next argument
    } // End while(iCurrentIndex < vsArguments.size())
    
    // Validate input path
    if (oCurrentConfig.sInputPath.empty()) 
    { // Begin if
        std::cerr << "Error: No input path specified" << std::endl; // In iostream
        return ERROR_INVALID_ARGUMENTS; // No input path
    } // End if(oCurrentConfig.sInputPath.empty())
    
    // Set default output path if not specified
    if (oCurrentConfig.sOutputPath.empty()) 
    { // Begin if
        oCurrentConfig.sOutputPath = fn_getDefaultOutputPath(oCurrentConfig.sInputPath); // In config.cpp
    } // End if(oCurrentConfig.sOutputPath.empty())
    
    return ERROR_SUCCESS; // Successfully parsed arguments
} // End Function fn_parseArguments

// Local Function
int fn_processConversion(const oConfig& oCurrentConfig) 
{ // Begin fn_processConversion
    oLogger oProcessLogger; // In logger.h
    oProcessLogger.fn_setVerbose(oCurrentConfig.bVerbose); // In logger.cpp
    
    // Check if input exists
    if (!fn_fileExists(oCurrentConfig.sInputPath)) 
    { // Begin if
        oProcessLogger.fn_logError("Input path does not exist: " + oCurrentConfig.sInputPath); // In logger.cpp
        return ERROR_FILE_NOT_FOUND; // File not found
    } // End if(!fn_fileExists(oCurrentConfig.sInputPath))
    
    // Create converter instance
    Converter oConverter; // Fixed: Use oConverter consistently
    int iInitResult = oConverter.fn_initialize(oCurrentConfig); // In converter.cpp
    
    if (iInitResult != ERROR_SUCCESS) 
    { // Begin if
        oProcessLogger.fn_logError("Failed to initialize converter"); // In logger.cpp
        return iInitResult; // Initialization failed
    } // End if(iInitResult != ERROR_SUCCESS)
    
    // Check if input is a directory (batch processing)
    if (fn_isDirectory(oCurrentConfig.sInputPath)) 
    { // Begin if
        oProcessLogger.fn_logInfo("Processing directory: " + oCurrentConfig.sInputPath); // In logger.cpp
        
        // Create batch processor
        BatchProcessor oBatch; // Fixed: Use oBatch consistently
        int iBatchResult = oBatch.fn_processDirectory(
           oCurrentConfig.sInputPath,
           oCurrentConfig.sOutputFormat.substr(1),  // Remove the dot from extension
           oCurrentConfig.sOutputPath,
           oCurrentConfig.bRecursive,
           oCurrentConfig.iJpegQuality,
           oCurrentConfig.bKeepMetadata,
           oCurrentConfig.bVerbose
        );
        
        return iBatchResult ? ERROR_SUCCESS : ERROR_BATCH_PROCESSING;
    } 
    else 
    { // Begin else (single file)
        std::string sInputExtension = fn_getFileExtension(oCurrentConfig.sInputPath); // In file_utils.cpp
        std::string sInputExtensionWithDot = "." + sInputExtension; // Add dot for comparison
        
        if (!fn_isSupportedInputFormat(sInputExtensionWithDot)) 
        { // Begin if
            oProcessLogger.fn_logError("Unsupported input format: " + sInputExtensionWithDot); // In logger.cpp
            return ERROR_UNSUPPORTED_FORMAT; // Unsupported format
        } // End if(!fn_isSupportedInputFormat(sInputExtension))
        
        oProcessLogger.fn_logInfo("Processing file: " + oCurrentConfig.sInputPath); // In logger.cpp
        
        // Convert single file
        int iConvertResult = oConverter.fn_convertFile( // In converter.cpp
            oCurrentConfig.sInputPath, // Local Function
            oCurrentConfig.sOutputPath // Local Function
        );
        
        return iConvertResult; // Return conversion result
    } // End else
} // End Function fn_processConversion