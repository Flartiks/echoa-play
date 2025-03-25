#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <fstream>
#include <iostream>
#include "albumArt.h"

void extractCoverArt(const std::string &audioFilePath, const std::string &outputImagePath) {
    TagLib::MPEG::File mpegFile(audioFilePath.c_str());
    if (!mpegFile.isValid()) {
        std::cerr << "Invalid audio file: " << audioFilePath << std::endl;
        return;
    }

    TagLib::ID3v2::Tag *id3v2Tag = mpegFile.ID3v2Tag();
    if (!id3v2Tag) {
        std::cerr << "No ID3v2 tag found in file: " << audioFilePath << std::endl;
        return;
    }

    TagLib::ID3v2::FrameList frames = id3v2Tag->frameListMap()["APIC"];
    if (frames.isEmpty()) {
        std::cerr << "No cover art found in file: " << audioFilePath << std::endl;
        return;
    }

    TagLib::ID3v2::AttachedPictureFrame *coverArtFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(frames.front());
    std::ofstream outFile(outputImagePath, std::ios::binary);
    outFile.write(coverArtFrame->picture().data(), coverArtFrame->picture().size());
    outFile.close();

    std::cout << "Cover art extracted to: " << outputImagePath << std::endl;
}

