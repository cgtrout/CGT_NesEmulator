// Log.cpp: implementation of the CLog class.
//
// Ripped from enginuity articles ( gamedev.com )
//TODO get rid of this and replace
//
//////////////////////////////////////////////////////////////////////

#include "precompiled.h"
#include <cstdarg>

//#include "engine.h"
//#include "Log.h"

using namespace FrontEnd;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//CMMPointer< CSettingVariable< std::string >> CLog::appLogName=0;
//CMMPointer< CSettingVariable< std::string >> CLog::clientLogName=0;
//CMMPointer< CSettingVariable< std::string >> CLog::serverLogName=0;

/*
==============================================
CLog()
==============================================
*/
CLog::CLog()
{

}

/*
==============================================
CLog::Init()
==============================================
*/
bool CLog::Init()
 {
	appLog.open( "applog.txt" );
	//clientLog.open( "clntlog.txt" );
	//serverLog.open( "srvrlog.txt" );
	//user errors get logged to client

	return true;
}

/*
==============================================
CLog::Write()
==============================================
*/
//void CLog::Write( int target, const char *msg, ... )
void CLog::Write( const char* msg, ... )
{
    constexpr size_t kMaxBufSize = 1024;
    char szBuf[ kMaxBufSize ];

    va_list args;
    va_start( args, msg );
    int len = std::vsnprintf( szBuf, kMaxBufSize, msg, args );
    va_end( args );

    if( len >= 0 && static_cast<size_t>( len ) < kMaxBufSize ) {
        appLog << std::string( szBuf, len ) << '\n';
        appLog.flush();
    }
    else {
        // The output was truncated or the formatting failed
        // Handle the error accordingly
        throw std::runtime_error( "CLog::Write: Formatting error" );
    }
}


