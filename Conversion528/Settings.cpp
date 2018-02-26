#include "Settings.h"
#include "SimpleIni.h"
#include <stdexcept>

const char* const MYSQL_INI_SECTION = "mysql";
const char* const ENCODING_SECTION = "encoding";
const char* const PREVIEW_SECTION = "preview";
const char* const LIMITS_SECTION = "limits";
const char* const OTHER_SECTION = "other";

const char* const ENCODING_FORMAT_FIELD_NAME = "format";
const char* const BITRATE_TYPE_FIELD_NAME = "bitratetype";
const char* const MIN_BITRATE_FIELD_NAME = "minbitrate";
const char* const MAX_BITRATE_FIELD_NAME = "maxbitrate";

const char* const TIME_LIMIT_FIELD_NAME = "timelimit";
const char* const PREVIEW_LIMIT_FIELD_NAME = "previewlimit";

const char* const COMMENT_FIELD_NAME = "comment";


const ConverterAppSettings::SqlSettings& ConverterAppSettings::getSqlSettings() const
{
    return m_sqlSettings;
}

const ConverterAppSettings::EncodingSettings& ConverterAppSettings::getEncodingSettings() const
{
    return m_encodingSettings;
}

const ConverterAppSettings::EncodingSettings& ConverterAppSettings::getPreviewSettings() const
{
    return m_previewSettings;
}

const ConverterAppSettings::LimitSettings& ConverterAppSettings::getLimitSettings() const
{
    return m_limitSettings;
}

const ConverterAppSettings::OtherSettings& ConverterAppSettings::getOtherSettings() const
{
	return m_otherSettings;
}

static ConverterAppSettings::EncodingSettings parseEncodingSettings(CSimpleIniA& ini, const char* sectionName)
{
    ConverterAppSettings::EncodingSettings settings;

	/*
    const std::string strEncoderType = ini.GetValue(sectionName, ENCODING_FORMAT_FIELD_NAME, "mp3");
    if (strcasecmp(strEncoderType.c_str(), "mp3") == 0)
        settings.encoderType = Encoding::Mp3;
    else
    {
        throw std::runtime_error("Unexpected encoder type in settings");
    }
    */

    const std::string strBitrateType = ini.GetValue(sectionName, BITRATE_TYPE_FIELD_NAME, "cbr");
    if (strcasecmp(strBitrateType.c_str(), "cbr") == 0)
        settings.bitrateType = Encoding::EncoderSettings::ConstantBitrate;
    else if (strcasecmp(strBitrateType.c_str(), "vbr") == 0)
        settings.bitrateType = Encoding::EncoderSettings::VariableBitrate;
    else
    {
        throw std::runtime_error("Unexpected bitrate type in settings");
    }

    settings.minBitrate = ini.GetLongValue(sectionName, MIN_BITRATE_FIELD_NAME, 128);
    settings.maxBitrate = ini.GetLongValue(sectionName, MAX_BITRATE_FIELD_NAME, 128);

    return settings;
}

void ConverterAppSettings::load(const char* iniFileName)
{
    CSimpleIniA ini(true); // allow utf-8

    // TODO: добавить возможность указывать файл конфигурации в командной строке
    if (ini.LoadFile(iniFileName) < 0)
    {
        throw std::runtime_error("failed to open configuration file");
    }

    // Распарсить файл конфигурации и вытащить параметры сервера:
    //  3. Адрес сервера
    //  4. Имя и пароль
    //  5. Имя базы данных
    //  6. Имя таблицы с данными о конвертациях
    // FIXME: добавить обработку ошибок!
    m_sqlSettings.serverName = ini.GetValue(MYSQL_INI_SECTION, "server");
    m_sqlSettings.userName = ini.GetValue(MYSQL_INI_SECTION, "user");
    m_sqlSettings.password = ini.GetValue(MYSQL_INI_SECTION, "password");
    m_sqlSettings.databaseName = ini.GetValue(MYSQL_INI_SECTION, "database");
    m_sqlSettings.tableName = ini.GetValue(MYSQL_INI_SECTION, "table");

    // Вытащить из файла конфигурации параметры энкодинга
    m_encodingSettings = parseEncodingSettings(ini, ENCODING_SECTION);
    m_previewSettings = parseEncodingSettings(ini, PREVIEW_SECTION);

    m_limitSettings.timeLimit = ini.GetLongValue(LIMITS_SECTION, TIME_LIMIT_FIELD_NAME, 300);
    m_limitSettings.previewLimit = ini.GetLongValue(LIMITS_SECTION, PREVIEW_LIMIT_FIELD_NAME, 25);

    m_otherSettings.comment = ini.GetValue(OTHER_SECTION, COMMENT_FIELD_NAME);
}
