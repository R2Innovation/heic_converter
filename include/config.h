// config.h - Simplified version without PROJECT_SOURCE_DIR
#ifndef HEIC_CONVERTER_CONFIG_H
#define HEIC_CONVERTER_CONFIG_H

#include <string>
#include <vector>

// Program Information
const std::string sPROGRAM_NAME = "heic_converter";
const std::string sVERSION = "v1.1";
const std::string sAUTHOR = "R Square Innovation Software";

// Build Configuration
#ifdef DEBIAN9_BUILD
const std::string sBUILD_TYPE = "debian9";
#elif defined(DEBIAN12_BUILD)
const std::string sBUILD_TYPE = "debian12";
#else
const std::string sBUILD_TYPE = "generic";
#endif

// Default Settings
const int iDEFAULT_JPEG_QUALITY = 85;
const int iDEFAULT_PNG_COMPRESSION = 6;
const int iDEFAULT_THREAD_COUNT = 4;
const int iMAX_THREAD_COUNT = 16;
const float fDEFAULT_SCALE_FACTOR = 1.0f;
const bool bDEFAULT_OVERWRITE = false;
const bool bDEFAULT_VERBOSE = false;
const bool bDEFAULT_RECURSIVE = false;
const bool bDEFAULT_PRESERVE_METADATA = true;      // NEW: Default preserve metadata
const bool bDEFAULT_PRESERVE_TIMESTAMPS = true;    // NEW: Default preserve timestamps
const bool bDEFAULT_PRESERVE_EXIF = true;          // NEW: Default preserve EXIF
const bool bDEFAULT_PRESERVE_XMP = true;           // NEW: Default preserve XMP
const bool bDEFAULT_PRESERVE_IPTC = true;          // NEW: Default preserve IPTC
const bool bDEFAULT_PRESERVE_GPS = true;           // NEW: Default preserve GPS

// Supported Input Formats
const std::vector<std::string> vsSUPPORTED_INPUT_FORMATS = {
    ".heic",
    ".heif",
    ".HEIC",
    ".HEIF"
};

// Supported Output Formats
const std::vector<std::string> vsSUPPORTED_OUTPUT_FORMATS = {
    ".jpg",
    ".jpeg",
    ".png",
    ".bmp",
    ".tiff",
    ".webp",
    ".JPG",
    ".JPEG",
    ".PNG",
    ".BMP",
    ".TIFF",
    ".WEBP"
};

// Libheif availability
#ifdef HAVE_LIBHEIF
const bool bUSE_SYSTEM_LIBHEIF = true;
#else
const bool bUSE_SYSTEM_LIBHEIF = false;
#endif

// Error Codes
enum eErrorCode {
    ERROR_SUCCESS = 0,
    ERROR_INVALID_ARGUMENTS = 1,
    ERROR_UNSUPPORTED_FORMAT = 2,
    ERROR_FILE_NOT_FOUND = 3,
    ERROR_READ_PERMISSION = 4,
    ERROR_WRITE_PERMISSION = 5,
    ERROR_DECODING_FAILED = 6,
    ERROR_ENCODING_FAILED = 7,
    ERROR_MEMORY_ALLOCATION = 8,
    ERROR_CODEC_INITIALIZATION = 9,
    ERROR_BATCH_PROCESSING = 10,
    ERROR_METADATA_EXTRACTION = 11,     // NEW: Metadata error
    ERROR_METADATA_WRITING = 12,        // NEW: Metadata error
    ERROR_TIMESTAMP_COPY = 13,          // NEW: Timestamp error
    ERROR_UNKNOWN = 255
};

// Configuration Structure
struct oConfig {
    std::string sInputPath;
    std::string sOutputPath;
    std::string sOutputFormat;
    int iJpegQuality;
    int iPngCompression;
    int iThreadCount;
    float fScaleFactor;
    bool bOverwrite;
    bool bVerbose;
    bool bRecursive;
    bool bKeepMetadata;
    bool bStripColorProfile;
    bool bPreserveTimestamps;     // NEW: Preserve file timestamps
    bool bPreserveEXIF;           // NEW: Preserve EXIF metadata
    bool bPreserveXMP;            // NEW: Preserve XMP metadata
    bool bPreserveIPTC;           // NEW: Preserve IPTC metadata
    bool bPreserveGPS;            // NEW: Preserve GPS data
};

// Function Declarations - KEEP THESE
std::string fn_getDefaultOutputFormat();
std::string fn_getDefaultOutputPath(const std::string& sInputPath);
bool fn_isSupportedInputFormat(const std::string& sExtension);
bool fn_isSupportedOutputFormat(const std::string& sExtension);
std::string fn_normalizeExtension(const std::string& sExtension);
std::string fn_getMimeTypeForExtension(const std::string& sExtension);
std::string fn_getExtensionForMimeType(const std::string& sMimeType);
oConfig fn_getDefaultConfig();
void fn_printConfig(const oConfig& oCurrentConfig);

#endif // HEIC_CONVERTER_CONFIG_H