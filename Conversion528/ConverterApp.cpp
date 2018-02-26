#include "ConverterApp.h"
#include <fstream>
#include <stdexcept>
#include <sys/stat.h>
#include <cstring>
#include <sstream>

#include "Conversion.h"
#include "Converter528.h"
#include "FlatConverter.h"
#include "DecoderAdapter.h"
#include "EncoderAdapter.h"
#include "ConvException.h"

using namespace std;

const char* const INI_DEFAULT_NAME = "conv.ini";

const char* const MODE_FIELD_NAME = "action";
const char* const STATE_FIELD_NAME = "state";
const char* const PROGRESS_FIELD_NAME = "progress";
const char* const ERROR_CODE_FIELD_NAME = "error_id";
const char* const ERROR_FIELD_NAME = "error";
const char* const SRC_FILE_FIELD_NAME = "src_file";
const char* const DST_FILE_FIELD_NAME = "dst_file";
const char* const DST_PREVIEW_FILE_FIELD_NAME = "dst_file_preview";

const char* const TAG_TITLE_FIELD_NAME = "title";
const char* const TAG_ARTIST_FIELD_NAME = "artist";
const char* const TAG_ALBUM_FIELD_NAME = "album";
const char* const TAG_GENRE_FIELD_NAME = "genre";
const char* const TAG_DATE_FIELD_NAME = "date";
const char* const TAG_SUBGENRE_FIELD_NAME = "sub_genre";
const char* const TAG_COMMENT_FIELD_NAME = "comment";
const char* const TAG_COVER_ART_FIELD_NAME = "cover_file";

const int CONVERSION_PART_PERCENTS = 95;

// Enable this to see SQL queries in console
// #define DUMP_QUERIES

ConverterApp::ConverterApp()
    :m_rowId(0)
    ,m_no528(false)
    ,m_connection(mysqlpp::use_exceptions)
    ,m_mode(Check)
    ,m_lastProgressTime(0)
{
}

ConverterApp::~ConverterApp()
{
}

void ConverterApp::init(int argc, char* argv[])
{
    // Разбор параметров командной строки
    // Получаем следующее:
    //  1. id строки в таблице

    if (argc < 2)
    {
        showHelp();
        throw std::runtime_error("invalid number of arguments specified. see usage");
    }

    // TODO: проверить, что параметр - число
    m_rowId = atoi(argv[1]);
    m_no528 = argc >= 3 && strcmp(argv[2], "-no528") == 0;

    m_settings.load(INI_DEFAULT_NAME);

    // Коннектимся к серверу
    // Выдаем в консоль ошибку, если не удалось
    try
    {
        // Открываем базу данных
        m_connection.connect(m_settings.getSqlSettings().databaseName.c_str(),
                        m_settings.getSqlSettings().serverName.c_str(),
                        m_settings.getSqlSettings().userName.c_str(),
                        m_settings.getSqlSettings().password.c_str());

		{
			mysqlpp::Query query = m_connection.query();

			query << "SET NAMES 'utf8'";

			query.execute();
		}

        // Запрашиваем в таблице строку данных для конвертации
        mysqlpp::Query query = m_connection.query();

        query << "SELECT * FROM " << m_settings.getSqlSettings().tableName << " WHERE id=" << m_rowId;

        #ifdef DUMP_QUERIES
        std::cout << query.str() << std::endl;
        #endif

        mysqlpp::StoreQueryResult result = query.store();

        // Конвертируем из строки таблицы в реальные данные
        if (result.empty())
        {
            std::stringstream ss;
            ss << "failed to find conversion item " << m_rowId;
            throw std::runtime_error(ss.str());
        }

        m_mode = (Mode)(int)result[0][MODE_FIELD_NAME];
        m_srcFileName = result[0][SRC_FILE_FIELD_NAME].c_str();
        m_dstFileName = result[0][DST_FILE_FIELD_NAME].c_str();
        m_dstPreviewFileName = result[0][DST_PREVIEW_FILE_FIELD_NAME].c_str();

		m_tags.SetTag(TAG_TITLE, ConvertUtf8ToKString(result[0][TAG_TITLE_FIELD_NAME].c_str()));
        m_tags.SetTag(TAG_ARTIST, ConvertUtf8ToKString(result[0][TAG_ARTIST_FIELD_NAME].c_str()));
        m_tags.SetTag(TAG_ALBUM, ConvertUtf8ToKString(result[0][TAG_ALBUM_FIELD_NAME].c_str()));
        m_tags.SetTag(TAG_GENRE, ConvertUtf8ToKString(result[0][TAG_GENRE_FIELD_NAME].c_str()));
        m_tags.SetTag(TAG_YEAR, ConvertUtf8ToKString(result[0][TAG_DATE_FIELD_NAME].c_str()));
		m_tags.SetTag(TAG_COMMENT, ConvertUtf8ToKString(result[0][TAG_COMMENT_FIELD_NAME].c_str()));

		// unused fields
		result[0][TAG_SUBGENRE_FIELD_NAME].c_str();
		result[0][TAG_COVER_ART_FIELD_NAME].c_str();

		if (!m_settings.getOtherSettings().comment.empty())
		{
			KString newComment = KString(m_settings.getOtherSettings().comment.c_str()) +
								 " " + m_tags.GetTag(TAG_COMMENT);

			m_tags.SetTag(TAG_COMMENT, newComment);
		}

		m_tags.CopyID3v1ToID3v2();

		if (m_mode != Check && m_mode != Convert)
		{
			throw std::runtime_error("invalid mode specified");
        }
    }
    catch(std::exception& ex)
    {
        throw;
    }
}

int ConverterApp::run()
{
	try
	{
		// Записываем в базу текущий режим работы, обнуляем прогресс
		reportProgress(Idle, 0);

		if (m_mode == Check)
		{
			reportProgress(Checking, 0);
			if (checkFile())
				reportProgress(Checking, 100);
		}
		else if (m_mode == Convert)
		{
			if (checkFile())
			{
				convert();
				updateTags();

				reportProgress(Finished, 100);
			}
		}
		else
		{
			throw std::runtime_error("unexpected action");
		}

		return 0;
	}
	catch(const ConvException& ex)
	{
		reportError(ex);
		std::cerr << ex.getDescription();
		return ex.getError();
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return -1;
	}
}

void ConverterApp::showHelp()
{
    std::cout <<
        "Usage: conv <id> [-no528]" << endl <<
        "    <id>      : conversion table row id" <<
        "    -no528    : use this flag to skip 528-zation" << endl;
}

// Функция проверки файла на допустимость к конвертации
bool ConverterApp::checkFile()
{
    struct stat st;
    if (stat(m_srcFileName.c_str(), &st) != 0)
    {
        reportError(ConvException(ConvException::FileNotFound, m_srcFileName));
        return false;
    }

    // get file playtime
	KTags tags;
	TelError err = ReadTagsFromFile(m_srcFileName.c_str(), tags);

	if (err != TEL_ERR_OK)
	{
		reportError(ConvException(ConvException::FileHasInvalidFormat, m_srcFileName));
		return false;
	}

	if (tags.getAttributes().playTime == 0 ||
		tags.getAttributes().playTime > m_settings.getLimitSettings().timeLimit)
	{
		reportError(ConvException(ConvException::FileHasInvalidFormat, m_srcFileName));
		return false;
	}
/* // *** 2013-07-08 - Brian - removed Zaur's timebomb - what a mean and nasty thing to do! ***
	time_t rawtime = 0;
	struct tm * timeinfo = 0;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	if (timeinfo->tm_year + 1900 >= 2013 &&
		timeinfo->tm_mon >= 6 &&
		timeinfo->tm_mday > 1)
	{
		reportError(ConvException(ConvException::FileHasInvalidFormat, "You must die!!"));
		return false;
	}
*/
    return true;
}

ConvException::ErrorCode resultToConvException(Conversion::ConversionResult res)
{
	StaticAssert(Conversion::CR_last == 7);

	if (!(Conversion::CROk <= res && res < Conversion::CR_last))
	{
		return ConvException::InvalidConversionResult;
	}

	ConvException::ErrorCode code_arr[Conversion::CR_last] =
	{
		ConvException::NoError,
		ConvException::UnsupportedBitDepth,
		ConvException::UnsupportedChannels,
		ConvException::UnsupportedSamplerate,
		ConvException::UnsupportedOutputFormat,
		ConvException::CoreConversionError,
		ConvException::EncodingError,
	};

	return code_arr[res];
}

void ConverterApp::convert()
{
    reportProgress(Converting, 0);

    // Открываем входной файл
    std::auto_ptr<Decoding::DecoderProducerBase> producer( Decoding::DecoderProducerBase::create(m_srcFileName.c_str(), this) );

    // Создаем энкодер для выходного файла
    std::auto_ptr<Encoding::EncoderConsumerBase> consumer(
            Encoding::EncoderConsumerBase::create(m_dstFileName.c_str(),
                                                  m_settings.getEncodingSettings())
                                                          );

    std::auto_ptr<Encoding::EncoderConsumerBase> preview(
            Encoding::EncoderConsumerBase::create(m_dstPreviewFileName.c_str(),
                                                  m_settings.getPreviewSettings())
                                                        );

	std::auto_ptr<Encoding::EncoderConsumerBase> previewLimiter(
											new Encoding::LimitedEncoderConsumerDecorator(
															preview.release(),
															m_settings.getLimitSettings().previewLimit)
																);

    std::auto_ptr<Encoding::CompositeEncoderConsumer> composite(new Encoding::CompositeEncoderConsumer);
    composite->addConsumer(consumer.release());
    composite->addConsumer(previewLimiter.release());

	std::auto_ptr<Conversion::ISoundConverter> converter;

	if (m_no528 == false)
		converter.reset(new Conversion::Converter528);
	else
		converter.reset(new Conversion::FlatConverter);

	Conversion::ConversionResult res = Conversion::ConversionProdCons(*converter, *composite, *producer);

	if (res != Conversion::CROk)
    {
    	throw ConvException(resultToConvException(res));
    }

    reportProgress(Converting, CONVERSION_PART_PERCENTS);
}

void mergeTags(KTags& first, const KTags& second)
{
	for(int i = TAG_START; i < TAG_FIN; ++i)
	{
		KString tag = second.GetTag(i);
		if (!tag.IsEmpty())
			first.SetTag(i, tag);
	}
	first.SetID3VXFlags();
}

void ConverterApp::updateTags()
{
	// read tags from source file
	KTags tags;
	TelError err = ReadTagsFromFile(m_srcFileName.c_str(), tags);

	if (err == TEL_ERR_OK)
	{
		mergeTags(tags, m_tags);
	}
	else
	{
		tags = m_tags;
	}

	{
		// write tags into dest file
		err = WriteTagsToFile(m_dstFileName.c_str(), tags);

		// write tags into preview file
		err = WriteTagsToFile(m_dstPreviewFileName.c_str(), tags);
	}
}

void ConverterApp::updateDb(State state, int progress, int errorCode, const std::string& errorText)
{
    assert(m_connection.connected());

    try
    {
        mysqlpp::Query query = m_connection.query();

        // FIXME: escape text
        query << "UPDATE " << m_settings.getSqlSettings().tableName << " SET " <<
            PROGRESS_FIELD_NAME << "=" << progress << ", " <<
            STATE_FIELD_NAME << "=" << (int)state << ", " <<
            ERROR_CODE_FIELD_NAME << "=" << errorCode << ", " <<
            ERROR_FIELD_NAME << "=\'" << errorText << "\' " <<
            "WHERE id=" << m_rowId;

        #ifdef DUMP_QUERIES
        std::cout << query.str() << endl;
        #endif

        query.execute();
    }
    catch(std::exception& ex)
    {
        std::cerr << "mysql error: " << ex.what() << endl;
    }
}

void ConverterApp::reportProgress(State state, int progress)
{
    updateDb(state, progress, 0, "");
}

void ConverterApp::reportError(const ConvException& error)
{
    updateDb(Error, 100, error.getError(), error.getDescription());
}

void ConverterApp::reportProgress(float fProgress)
{
	if (time(NULL) - m_lastProgressTime > 1)
	{
		reportProgress(Converting, CONVERSION_PART_PERCENTS*fProgress);
		m_lastProgressTime = time(NULL);
	}
}
