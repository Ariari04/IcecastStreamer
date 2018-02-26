#pragma once

#include <string>
#include "EncoderAdapter.h"

class ConverterAppSettings
{
public:

    struct SqlSettings
    {
        std::string serverName;
        std::string userName;
        std::string password;
        std::string databaseName;
        std::string tableName;
    };

    typedef Encoding::EncoderSettings EncodingSettings;

    struct LimitSettings
    {
        int timeLimit;
        int previewLimit;
    };

    struct OtherSettings
    {
    	std::string comment;

    };

    const SqlSettings& getSqlSettings() const;

    const EncodingSettings& getEncodingSettings() const;

    const EncodingSettings& getPreviewSettings() const;

    const LimitSettings& getLimitSettings() const;

    const OtherSettings& getOtherSettings() const;

    void load(const char* iniFileName);

private:

    SqlSettings         m_sqlSettings;
    EncodingSettings    m_encodingSettings;
    EncodingSettings    m_previewSettings;
    LimitSettings       m_limitSettings;
    OtherSettings		m_otherSettings;
};

