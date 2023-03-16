// Log.h: interface for the CLog class.
// TODO revise this - take out all unneeded junk
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_LOG_H__78348862_06C2_4FAA_B192_13F09596E2FD__INCLUDED_ )
#define AFX_LOG_H__78348862_06C2_4FAA_B192_13F09596E2FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <iostream>
#include <ostream>
#include <istream>
#include <fstream>
#include <list>
#include <string>

#include "CGTSingleton.h"
using namespace CGTSingleton;

namespace FrontEnd {
	const int LOG_APP=1;
	const int LOG_CLIENT=2;
	const int LOG_SERVER=4;
	const int LOG_USER=8;

	#define MAX_LOG_STRINGS 256

	class CLog : public Singleton< CLog > {
	protected:
		CLog();
		
		std::ofstream appLog;
		//ofstream clientLog;
		//ofstream serverLog;

		std::string logStrings[ MAX_LOG_STRINGS ];
		bool LoadStrings();

	public:
		friend Singleton< CLog >;

		bool Init();

		void Write( /*int target,*/ const char *msg, ... );
		void Write( /*int target,*/ unsigned long msgID, ... );
	};
}
#endif // !defined( AFX_LOG_H__78348862_06C2_4FAA_B192_13F09596E2FD__INCLUDED_ )
