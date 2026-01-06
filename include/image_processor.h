// image_processor.h - Updated version with metadata support
#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <string>
#include <vector>
#include "logger.h"

class ImageProcessor 
{
    public:
        // Constructor and Destructor - Updated
        ImageProcessor(oLogger* pLogger = nullptr);
        ~ImageProcessor();
        
        // Public Methods - Updated with metadata support
        bool fn_convertImage(const std::string& sInputPath, 
                             const std::string& sOutputPath,
                             const std::string& sOutputFormat = "",
                             int iQuality = 85);
        
        // NEW: Convert image with metadata preservation
        bool fn_convertImageWithMetadata(
            const std::string& sInputPath, 
            const std::string& sOutputPath,
            const std::string& sOutputFormat = "",
            int iQuality = 85,
            const std::vector<unsigned char>& vExifData = {},
            const std::vector<unsigned char>& vXmpData = {},
            const std::vector<unsigned char>& vIptcData = {}
        );
        
        bool fn_validateImage(const std::string& sImagePath);
        std::vector<std::string> fn_getSupportedInputFormats();
        std::vector<std::string> fn_getSupportedOutputFormats();
        bool fn_setOutputQuality(int iQuality);
        int fn_getOutputQuality();
        std::string fn_getLastError();
        
    private:
        // Private Variables
        oLogger* m_pLogger;
        std::string m_sLastError;
        int m_iOutputQuality;
        bool m_bCodecsInitialized;
        void* m_pHeifContext;
        void* m_pHeifImage;
        
        // Private Methods
        bool fn_initializeCodecs();
        bool fn_cleanupResources();
        bool fn_decodeHEIC(const std::string& sInputPath, 
                          unsigned char** ppImageData, 
                          int& iWidth, 
                          int& iHeight, 
                          int& iChannels);
        bool fn_encodeImage(const unsigned char* pImageData, 
                           int iWidth, 
                           int iHeight, 
                           int iChannels, 
                           const std::string& sOutputPath, 
                           const std::string& sOutputFormat, 
                           int iQuality);
        
        // NEW: Encode with metadata
        bool fn_encodeImageWithMetadata(
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
        );
        
        bool fn_validateOutputFormat(const std::string& sFormat);
        std::string fn_determineOutputFormat(const std::string& sOutputPath);
}; // End Class ImageProcessor

#endif // IMAGE_PROCESSOR_H