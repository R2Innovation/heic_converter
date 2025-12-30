#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <vector>
#include <cstdint>
#include <ctime>

// File timestamp structure
struct FileTimestamps
{
    std::time_t tCreationTime;
    std::time_t tModificationTime;
    std::time_t tAccessTime;
};

// Function Declarations
bool fn_fileExists(const std::string& sPath);
bool fn_isDirectory(const std::string& sPath);
bool fn_createDirectory(const std::string& sPath);
std::string fn_getFileExtension(const std::string& sFilePath);
std::string fn_changeFileExtension(const std::string& sFilePath, const std::string& sNewExtension);
std::string fn_getFileNameWithoutExtension(const std::string& sFilePath);
std::string fn_getAbsolutePath(const std::string& sRelativePath);
std::vector<std::string> fn_getFilesInDirectory(const std::string& sDirectory);
std::vector<std::string> fn_filterFilesByExtension(const std::vector<std::string>& vsFiles, const std::vector<std::string>& vsExtensions);
bool fn_validateOutputPath(const std::string& sOutputPath);
bool fn_copyFile(const std::string& sSource, const std::string& sDestination);
bool fn_deleteFile(const std::string& sFilePath);
uint64_t fn_getFileSize(const std::string& sFilePath);
bool fn_isHeicFile(const std::string& sFilePath);
std::string fn_generateUniqueFileName(const std::string& sDirectory, const std::string& sBaseName, const std::string& sExtension);
void fn_normalizePath(std::string& sPath);
bool fn_hasWritePermission(const std::string& sPath);
std::vector<unsigned char> fn_readBinaryFile(const std::string& sFilePath);
std::string fn_getDirectory(const std::string& sPath);
bool fn_directoryExists(const std::string& sPath);
bool fn_createDirectoryIfNeeded(const std::string& sPath);
std::vector<std::string> fn_collectDirectoryFiles(const std::string& sDirectory, bool bRecursive);

// NEW: Timestamp functions
FileTimestamps fn_getFileTimestamps(const std::string& sFilePath);
bool fn_setFileTimestamps(const std::string& sFilePath, const FileTimestamps& oTimestamps);
bool fn_copyFileTimestamps(const std::string& sSource, const std::string& sDestination);

#endif // FILE_UTILS_H