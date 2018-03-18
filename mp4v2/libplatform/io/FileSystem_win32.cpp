#include "libplatform/impl.h"

namespace mp4v2 { namespace platform { namespace io {

///////////////////////////////////////////////////////////////////////////////

bool
FileSystem::exists( string path_ )
{
    WIN32_FIND_DATAA buf;
    HANDLE h = FindFirstFileA( path_.c_str(), &buf );
    if( h == INVALID_HANDLE_VALUE )
        return false;
    FindClose( h );
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
FileSystem::isDirectory( string path_ )
{
    WIN32_FIND_DATAA buf;
    HANDLE h = FindFirstFileA( path_.c_str(), &buf );
    if( h == INVALID_HANDLE_VALUE )
        return false;
    FindClose( h );
    return (buf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

///////////////////////////////////////////////////////////////////////////////

bool
FileSystem::isFile( string path_ )
{
    WIN32_FIND_DATAA buf;
    HANDLE h = FindFirstFileA( path_.c_str(), &buf );
    if( h == INVALID_HANDLE_VALUE )
        return false;
    FindClose( h );
    return (buf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

///////////////////////////////////////////////////////////////////////////////

bool
FileSystem::getFileSize( string path_, File::Size& size_ )
{
    size_ = 0;
    WIN32_FILE_ATTRIBUTE_DATA data;

#ifdef AMBIENT_CHAR_INSTEAD_OF_UTF8
	if( !GetFileAttributesExA( path_.c_str(), GetFileExInfoStandard, (LPVOID)&data ))
        return true;
#else
	wchar_t buffer[MAX_PATH + 1];
    MultiByteToWideChar(CP_UTF8, 0, path_.c_str(), -1, buffer, (int) MAX_PATH);
	if( !GetFileAttributesExW(buffer, GetFileExInfoStandard, (LPVOID)&data ))
        return true;
#endif

    size_ = ((File::Size)data.nFileSizeHigh << 32) | data.nFileSizeLow;
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
FileSystem::rename( string from, string to )
{
    if( exists( to )) {
        if( ::remove( to.c_str() ))
            return true;
    }
    return ::rename( from.c_str(), to.c_str() ) != 0;
}

///////////////////////////////////////////////////////////////////////////////

string FileSystem::DIR_SEPARATOR  = "\\";
string FileSystem::PATH_SEPARATOR = ";";

///////////////////////////////////////////////////////////////////////////////

}}} // namespace mp4v2::platform::io
