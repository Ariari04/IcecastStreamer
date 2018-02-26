#pragma once

#include <stdexcept>

class ConvException : public std::exception
{
public:
    enum ErrorCode
    {
        NoError = 0,
        FileNotFound,
        FileHasInvalidFormat,
        FileFormatNotSupported,
        InvalidConversionResult,
		UnsupportedBitDepth,
		UnsupportedChannels,
		UnsupportedSamplerate,
		UnsupportedOutputFormat,
		CoreConversionError,
		EncodingError,

		LastError,
    };

    static std::string errorCodeToString(ErrorCode error);

    ConvException(ErrorCode error, const std::string& parameter = std::string()) throw()
        :m_error(error)
        ,m_parameter(parameter)
    {
    	m_description = errorCodeToString(m_error);
    }

    ~ConvException() throw() {}

    ErrorCode getError() const
    {
    	return m_error;
    }

    const std::string& getParameter() const
    {
        return m_parameter;
    }

    std::string getDescription() const
    {
    	return std::string(what()) + "(" + m_parameter + ")";
    }

    virtual const char* what() const throw()
    {
        return m_description.c_str();
    }

private:
    ErrorCode   m_error;
    std::string m_description;
    std::string m_parameter;
};
