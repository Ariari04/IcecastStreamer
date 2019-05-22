#pragma once

#include "Tags.h"

int ReadTagsFromFileID3(const char* strFileName, KTags& rTags);
int WriteTagsToFileID3(const char* strFileName, const KTags& rTags);
