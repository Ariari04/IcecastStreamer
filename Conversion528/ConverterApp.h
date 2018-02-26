
#ifndef CONVERTERAPP_H
#define CONVERTERAPP_H

#include <string>

#define MYSQLPP_MYSQL_HEADERS_BURIED
#include <mysql++/mysql++.h>
#include "EncoderAdapter.h"
#include "Settings.h"
#include "Progress.h"
#include "tags/Tags.h"

class ConvException;

class ConverterApp : public IProgressManager
{
public:
    ConverterApp();
    ~ConverterApp();

    void init(int argc, char* argv[]);
    int  run();

    void reportProgress(float fProgress);

private:

    enum State
    {
        Idle = 0,
        Checking = 1,
        Converting = 2,
        Finished = 3,
        Error = 4,
    };

    // Режим работы программы
    enum Mode
    {
        Check = 0,      // Проверка
        Convert = 1,    // Конвертация
    };


private:
    void showHelp();

    bool checkFile();
    void convert();
    void updateTags();

    void updateDb(State state, int progress, int errorCode, const std::string& errorText);
    void reportProgress(State state, int progress);
    void reportError(const ConvException& error);

private:

    int     m_rowId;
    bool	m_no528;

    ConverterAppSettings m_settings;

    mysqlpp::Connection m_connection;

    Mode    m_mode;
    std::string m_srcFileName;
    std::string m_dstFileName;
    std::string m_dstPreviewFileName;
    KTags  	m_tags;

    time_t m_lastProgressTime;
};

#endif
