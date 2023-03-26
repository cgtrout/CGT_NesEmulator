// Log.h: interface for the CLog class.
//////////////////////////////////////////////////////////////////////
#pragma once

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

	public:
		friend Singleton< CLog >;

		bool Init();

		void Write( /*int target,*/ const char *msg, ... );
	};
}
