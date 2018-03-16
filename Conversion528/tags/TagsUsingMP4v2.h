#pragma once

#include "Tags.h"

TelError ReadTagsFromFileMP4(const char* strFileName, KTags& rTags);
TelError WriteTagsToFileMP4(const char* strFileName, const KTags& rTags);