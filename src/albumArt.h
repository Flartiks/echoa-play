#ifndef ALBUMART_H
#define ALBUMART_H

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <iostream>

// Функция для извлечения обложки альбома и сохранения в файл
void extractCoverArt(const std::string &audioFilePath, const std::string &outputImagePath);

#endif // ALBUMART_H