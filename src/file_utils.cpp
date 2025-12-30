#include "file_utils.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <utime.h>
#include "logger.h"

bool fn_fileExists(const std::string& sPath) // Local Function
 { // Start Function fn_fileExists
 struct stat oStatBuffer;
 return (stat(sPath.c_str(), &oStatBuffer) == 0);
 } // End Function fn_fileExists

bool fn_isDirectory(const std::string& sPath) // Local Function
 { // Start Function fn_isDirectory
 struct stat oStatBuffer;
 if (stat(sPath.c_str(), &oStatBuffer) != 0)
  { // Start if (stat(sPath.c_str(), &oStatBuffer) != 0)
  return false;
  } // End if (stat(sPath.c_str(), &oStatBuffer) != 0)
 return S_ISDIR(oStatBuffer.st_mode);
 } // End Function fn_isDirectory

bool fn_createDirectory(const std::string& sPath) // Local Function
 { // Start Function fn_createDirectory
 if (fn_fileExists(sPath))
  { // Start if (fn_fileExists(sPath))
  return fn_isDirectory(sPath);
  } // End if (fn_fileExists(sPath))
 
 int iResult = mkdir(sPath.c_str(), 0755);
 if (iResult != 0 && errno == ENOENT)
  { // Start if (iResult != 0 && errno == ENOENT)
  size_t iPos = sPath.find_last_of('/');
  if (iPos != std::string::npos)
   { // Start if (iPos != std::string::npos)
   std::string sParent = sPath.substr(0, iPos);
   if (!fn_createDirectory(sParent))
    { // Start if (!fn_createDirectory(sParent))
    return false;
    } // End if (!fn_createDirectory(sParent))
   iResult = mkdir(sPath.c_str(), 0755);
   } // End if (iPos != std::string::npos)
  } // End if (iResult != 0 && errno == ENOENT)
 
 return (iResult == 0);
 } // End Function fn_createDirectory

std::string fn_getFileExtension(const std::string& sFilePath) // Local Function
 { // Start Function fn_getFileExtension
 size_t iPos = sFilePath.find_last_of('.');
 if (iPos == std::string::npos)
  { // Start if (iPos == std::string::npos)
  return "";
  } // End if (iPos == std::string::npos)
 
 std::string sExtension = sFilePath.substr(iPos + 1);
 std::transform(sExtension.begin(), sExtension.end(), sExtension.begin(), ::tolower);
 return sExtension;
 } // End Function fn_getFileExtension

std::string fn_changeFileExtension(const std::string& sFilePath, const std::string& sNewExtension) // Local Function
 { // Start Function fn_changeFileExtension
 size_t iPos = sFilePath.find_last_of('.');
 if (iPos == std::string::npos)
  { // Start if (iPos == std::string::npos)
  return sFilePath + "." + sNewExtension;
  } // End if (iPos == std::string::npos)
 
 std::string sNewPath = sFilePath.substr(0, iPos) + "." + sNewExtension;
 return sNewPath;
 } // End Function fn_changeFileExtension

std::string fn_getFileNameWithoutExtension(const std::string& sFilePath) // Local Function
 { // Start Function fn_getFileNameWithoutExtension
 size_t iSlashPos = sFilePath.find_last_of('/');
 size_t iDotPos = sFilePath.find_last_of('.');
 
 if (iSlashPos == std::string::npos)
  { // Start if (iSlashPos == std::string::npos)
  iSlashPos = 0;
  } // End if (iSlashPos == std::string::npos)
 else
  { // Start else
  iSlashPos++;
  } // End else
 
 if (iDotPos == std::string::npos || iDotPos < iSlashPos)
  { // Start if (iDotPos == std::string::npos || iDotPos < iSlashPos)
  return sFilePath.substr(iSlashPos);
  } // End if (iDotPos == std::string::npos || iDotPos < iSlashPos)
 
 return sFilePath.substr(iSlashPos, iDotPos - iSlashPos);
 } // End Function fn_getFileNameWithoutExtension

std::string fn_getAbsolutePath(const std::string& sRelativePath) // Local Function
 { // Start Function fn_getAbsolutePath
 char sBuffer[PATH_MAX];
 if (realpath(sRelativePath.c_str(), sBuffer) == nullptr)
  { // Start if (realpath(sRelativePath.c_str(), sBuffer) == nullptr)
  return sRelativePath;
  } // End if (realpath(sRelativePath.c_str(), sBuffer) == nullptr)
 return std::string(sBuffer);
 } // End Function fn_getAbsolutePath

std::vector<std::string> fn_getFilesInDirectory(const std::string& sDirectory) // Local Function
 { // Start Function fn_getFilesInDirectory
 std::vector<std::string> vsFiles;
 
 if (!fn_isDirectory(sDirectory))
  { // Start if (!fn_isDirectory(sDirectory))
  fn_logError("Directory does not exist: " + sDirectory);
  return vsFiles;
  } // End if (!fn_isDirectory(sDirectory))
 
 DIR* pDir = opendir(sDirectory.c_str());
 if (pDir == nullptr)
  { // Start if (pDir == nullptr)
  fn_logError("Cannot open directory: " + sDirectory);
  return vsFiles;
  } // End if (pDir == nullptr)
 
 struct dirent* pEntry;
 while ((pEntry = readdir(pDir)) != nullptr)
  { // Start while ((pEntry = readdir(pDir)) != nullptr)
  if (pEntry->d_type == DT_REG)
   { // Start if (pEntry->d_type == DT_REG)
   vsFiles.push_back(sDirectory + "/" + std::string(pEntry->d_name));
   } // End if (pEntry->d_type == DT_REG)
  } // End while ((pEntry = readdir(pDir)) != nullptr)
 
 closedir(pDir);
 return vsFiles;
 } // End Function fn_getFilesInDirectory

std::vector<std::string> fn_filterFilesByExtension(const std::vector<std::string>& vsFiles, const std::vector<std::string>& vsExtensions) // Local Function
 { // Start Function fn_filterFilesByExtension
 std::vector<std::string> vsFilteredFiles;
 
 for (const std::string& sFile : vsFiles)
  { // Start for (const std::string& sFile : vsFiles)
  std::string sExtension = fn_getFileExtension(sFile);
  for (const std::string& sTargetExtension : vsExtensions)
   { // Start for (const std::string& sTargetExtension : vsExtensions)
   if (sExtension == sTargetExtension)
    { // Start if (sExtension == sTargetExtension)
    vsFilteredFiles.push_back(sFile);
    break;
    } // End if (sExtension == sTargetExtension)
   } // End for (const std::string& sTargetExtension : vsExtensions)
  } // End for (const std::string& sFile : vsFiles)
 
 return vsFilteredFiles;
 } // End Function fn_filterFilesByExtension

bool fn_validateOutputPath(const std::string& sOutputPath) // Local Function
 { // Start Function fn_validateOutputPath
 size_t iLastSlash = sOutputPath.find_last_of('/');
 if (iLastSlash != std::string::npos)
  { // Start if (iLastSlash != std::string::npos)
  std::string sDirectory = sOutputPath.substr(0, iLastSlash);
  if (!sDirectory.empty() && !fn_fileExists(sDirectory))
   { // Start if (!sDirectory.empty() && !fn_fileExists(sDirectory))
   return fn_createDirectory(sDirectory);
   } // End if (!sDirectory.empty() && !fn_fileExists(sDirectory))
  } // End if (iLastSlash != std::string::npos)
 
 return true;
 } // End Function fn_validateOutputPath

bool fn_copyFile(const std::string& sSource, const std::string& sDestination) // Local Function
 { // Start Function fn_copyFile
 std::ifstream oSourceFile(sSource, std::ios::binary);
 std::ofstream oDestFile(sDestination, std::ios::binary);
 
 if (!oSourceFile.is_open())
  { // Start if (!oSourceFile.is_open())
  fn_logError("Cannot open source file: " + sSource);
  return false;
  } // End if (!oSourceFile.is_open())
 
 if (!oDestFile.is_open())
  { // Start if (!oDestFile.is_open())
  fn_logError("Cannot create destination file: " + sDestination);
  return false;
  } // End if (!oDestFile.is_open())
 
 oDestFile << oSourceFile.rdbuf();
 
 bool bSourceGood = oSourceFile.good();
 bool bDestGood = oDestFile.good();
 
 oSourceFile.close();
 oDestFile.close();
 
 return (bSourceGood && bDestGood);
 } // End Function fn_copyFile

bool fn_deleteFile(const std::string& sFilePath) // Local Function
 { // Start Function fn_deleteFile
 if (remove(sFilePath.c_str()) != 0)
  { // Start if (remove(sFilePath.c_str()) != 0)
  fn_logError("Cannot delete file: " + sFilePath);
  return false;
  } // End if (remove(sFilePath.c_str()) != 0)
 return true;
 } // End Function fn_deleteFile

uint64_t fn_getFileSize(const std::string& sFilePath) // Local Function
 { // Start Function fn_getFileSize
 struct stat oStatBuffer;
 if (stat(sFilePath.c_str(), &oStatBuffer) != 0)
  { // Start if (stat(sFilePath.c_str(), &oStatBuffer) != 0)
  return 0;
  } // End if (stat(sFilePath.c_str(), &oStatBuffer) != 0)
 return static_cast<uint64_t>(oStatBuffer.st_size);
 } // End Function fn_getFileSize

std::string fn_generateUniqueFileName(const std::string& sDirectory, const std::string& sBaseName, const std::string& sExtension) // Local Function
 { // Start Function fn_generateUniqueFileName
 std::string sFullPath;
 int iCounter = 1;
 
 do
  { // Start do
  std::ostringstream oss;
  if (iCounter == 1)
   { // Start if (iCounter == 1)
   oss << sDirectory << "/" << sBaseName << "." << sExtension;
   } // End if (iCounter == 1)
  else
   { // Start else
   oss << sDirectory << "/" << sBaseName << "_" << iCounter << "." << sExtension;
   } // End else
  sFullPath = oss.str();
  iCounter++;
  } // End do
 while (fn_fileExists(sFullPath));
 
 return sFullPath;
 } // End Function fn_generateUniqueFileName

void fn_normalizePath(std::string& sPath) // Local Function
 { // Start Function fn_normalizePath
 if (sPath.empty())
  { // Start if (sPath.empty())
  return;
  } // End if (sPath.empty())
 
 // Replace backslashes with forward slashes
 std::replace(sPath.begin(), sPath.end(), '\\', '/');
 
 // Remove trailing slash
 if (sPath.back() == '/')
  { // Start if (sPath.back() == '/')
  sPath.pop_back();
  } // End if (sPath.back() == '/')
 
 // Remove double slashes
 size_t iPos;
 while ((iPos = sPath.find("//")) != std::string::npos)
  { // Start while ((iPos = sPath.find("//")) != std::string::npos)
  sPath.replace(iPos, 2, "/");
  } // End while ((iPos = sPath.find("//")) != std::string::npos)
 } // End Function fn_normalizePath

bool fn_hasWritePermission(const std::string& sPath) // Local Function
 { // Start Function fn_hasWritePermission
 if (access(sPath.c_str(), W_OK) == 0)
  { // Start if (access(sPath.c_str(), W_OK) == 0)
  return true;
  } // End if (access(sPath.c_str(), W_OK) == 0)
 
 // Check parent directory if file doesn't exist
 if (!fn_fileExists(sPath))
  { // Start if (!fn_fileExists(sPath))
  size_t iPos = sPath.find_last_of('/');
  if (iPos != std::string::npos)
   { // Start if (iPos != std::string::npos)
   std::string sParent = sPath.substr(0, iPos);
   if (sParent.empty())
    { // Start if (sParent.empty())
    sParent = ".";
    } // End if (sParent.empty())
   return (access(sParent.c_str(), W_OK) == 0);
   } // End if (iPos != std::string::npos)
  } // End if (!fn_fileExists(sPath))
 
 return false;
 } // End Function fn_hasWritePermission

 // Function: fn_readBinaryFile
std::vector<unsigned char> fn_readBinaryFile(const std::string& sFilePath)
{
    std::vector<unsigned char> vData;
    
    std::ifstream oFile(sFilePath, std::ios::binary | std::ios::ate);
    if (!oFile.is_open())
    {
        fn_logError("Cannot open file for reading: " + sFilePath);
        return vData;
    }
    
    std::streamsize iSize = oFile.tellg();
    oFile.seekg(0, std::ios::beg);
    
    vData.resize(static_cast<size_t>(iSize));
    if (!oFile.read(reinterpret_cast<char*>(vData.data()), iSize))
    {
        fn_logError("Failed to read file: " + sFilePath);
        vData.clear();
    }
    
    oFile.close();
    return vData;
} // End Function fn_readBinaryFile

// Function: fn_getDirectory
std::string fn_getDirectory(const std::string& sPath)
{
    size_t iPos = sPath.find_last_of('/');
    if (iPos == std::string::npos)
    {
        return "."; // Current directory
    }
    
    return sPath.substr(0, iPos);
} // End Function fn_getDirectory

// Function: fn_directoryExists
bool fn_directoryExists(const std::string& sPath)
{
    return fn_isDirectory(sPath);
} // End Function fn_directoryExists

// Also add these implementations that were referenced in batch_processor.cpp:

// Function: fn_createDirectoryIfNeeded (helper function)
bool fn_createDirectoryIfNeeded(const std::string& sPath)
{
    if (fn_fileExists(sPath))
    {
        return fn_isDirectory(sPath);
    }
    
    return fn_createDirectory(sPath);
} // End Function fn_createDirectoryIfNeeded

// Function: fn_isHeicFile
bool fn_isHeicFile(const std::string& sFilePath)
{
    std::string sExtension = fn_getFileExtension(sFilePath);
    return (sExtension == "heic" || sExtension == "heif" || 
            sExtension == "HEIC" || sExtension == "HEIF");
} // End Function fn_isHeicFile

// Function: fn_collectDirectoryFiles
std::vector<std::string> fn_collectDirectoryFiles(const std::string& sDirectory, bool bRecursive)
{
    std::vector<std::string> vsFiles;
    
    if (!fn_isDirectory(sDirectory))
    {
        fn_logError("Directory does not exist: " + sDirectory);
        return vsFiles;
    }
    
    if (!bRecursive)
    {
        // Non-recursive: just get files in this directory
        vsFiles = fn_getFilesInDirectory(sDirectory);
    }
    else
    {
        // Recursive: walk through directory tree
        // We need to implement this manually since we're using dirent
        std::vector<std::string> vsDirectories;
        vsDirectories.push_back(sDirectory);
        
        for (size_t i = 0; i < vsDirectories.size(); ++i)
        {
            std::string sCurrentDir = vsDirectories[i];
            
            // Get files in current directory
            std::vector<std::string> vsCurrentFiles = fn_getFilesInDirectory(sCurrentDir);
            vsFiles.insert(vsFiles.end(), vsCurrentFiles.begin(), vsCurrentFiles.end());
            
            // Get subdirectories
            DIR* pDir = opendir(sCurrentDir.c_str());
            if (pDir)
            {
                struct dirent* pEntry;
                while ((pEntry = readdir(pDir)) != nullptr)
                {
                    if (pEntry->d_type == DT_DIR)
                    {
                        std::string sName = pEntry->d_name;
                        if (sName != "." && sName != "..")
                        {
                            std::string sSubDir = sCurrentDir + "/" + sName;
                            if (fn_isDirectory(sSubDir))
                            {
                                vsDirectories.push_back(sSubDir);
                            }
                        }
                    }
                }
                closedir(pDir);
            }
        }
    }
    
    return vsFiles;
} // End Function fn_collectDirectoryFiles

// NEW: Get file timestamps
FileTimestamps fn_getFileTimestamps(const std::string& sFilePath)
{
    FileTimestamps oTimestamps;
    
    struct stat file_stat;
    if (stat(sFilePath.c_str(), &file_stat) == 0) {
        #ifdef __APPLE__
        // macOS: st_birthtime is creation time
        oTimestamps.tCreationTime = file_stat.st_birthtime;
        #elif defined(__linux__)
        // Linux: Try to get birth time from statx if available, otherwise use st_mtime
        oTimestamps.tCreationTime = file_stat.st_mtime;
        #else
        // Windows or other: st_ctime might be creation time or change time
        oTimestamps.tCreationTime = file_stat.st_ctime;
        #endif
        
        oTimestamps.tModificationTime = file_stat.st_mtime;
        oTimestamps.tAccessTime = file_stat.st_atime;
    }
    
    return oTimestamps;
} // End Function fn_getFileTimestamps

// NEW: Set file timestamps
bool fn_setFileTimestamps(const std::string& sFilePath, const FileTimestamps& oTimestamps)
{
    struct utimbuf new_times;
    new_times.actime = oTimestamps.tAccessTime;
    new_times.modtime = oTimestamps.tModificationTime;
    
    if (utime(sFilePath.c_str(), &new_times) == 0) {
        fn_logInfo("Successfully set timestamps for: " + sFilePath);
        return true;
    } else {
        fn_logWarning("Failed to set timestamps for: " + sFilePath);
        return false;
    }
} // End Function fn_setFileTimestamps

// NEW: Copy timestamps from source to destination
bool fn_copyFileTimestamps(const std::string& sSource, const std::string& sDestination)
{
    FileTimestamps oTimestamps = fn_getFileTimestamps(sSource);
    return fn_setFileTimestamps(sDestination, oTimestamps);
} // End Function fn_copyFileTimestamps