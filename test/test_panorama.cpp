// test/test_panorama.cpp
#include "heic_decoder.h"
#include "logger.h"
#include "file_utils.h"
#include <iostream>
#include <fstream>

void testPanoramaDecoding(const std::string& filename)
{
    std::cout << "Testing panorama decoding: " << filename << std::endl;
    
    if (!fn_fileExists(filename))
    {
        std::cout << "File does not exist!" << std::endl;
        return;
    }
    
    // Get file size
    uint64_t fileSize = fn_getFileSize(filename);
    std::cout << "File size: " << fileSize << " bytes" << std::endl;
    
    // Try to decode
    HeicDecoder decoder;
    oHeicInfo info = decoder.fn_getImageInfo(filename);
    
    std::cout << "\nImage Info:" << std::endl;
    std::cout << "  Format: " << info.sFormat << std::endl;
    std::cout << "  Dimensions: " << info.iWidth << "x" << info.iHeight << std::endl;
    std::cout << "  Bit Depth: " << info.iBitDepth << std::endl;
    std::cout << "  Has Alpha: " << (info.bHasAlpha ? "Yes" : "No") << std::endl;
    
    // Try to decode the image
    std::cout << "\nAttempting to decode..." << std::endl;
    oDecodedImage decoded = decoder.fn_decodeFile(filename);
    
    if (!decoded.sError.empty())
    {
        std::cout << "Error: " << decoded.sError << std::endl;
    }
    else
    {
        std::cout << "Decoded successfully!" << std::endl;
        std::cout << "  Actual dimensions: " << decoded.iWidth << "x" << decoded.iHeight << std::endl;
        std::cout << "  Channels: " << decoded.iChannels << std::endl;
        std::cout << "  Data size: " << decoded.vData.size() << " bytes" << std::endl;
        
        // Save a preview (first 1KB)
        std::string preview_name = filename + ".preview.raw";
        std::ofstream out(preview_name, std::ios::binary);
        size_t preview_size = std::min((size_t)1024, decoded.vData.size());
        out.write(reinterpret_cast<const char*>(decoded.vData.data()), preview_size);
        out.close();
        
        std::cout << "First " << preview_size << " bytes saved to: " << preview_name << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <heic_file>" << std::endl;
        return 1;
    }
    
    testPanoramaDecoding(argv[1]);
    return 0;
}