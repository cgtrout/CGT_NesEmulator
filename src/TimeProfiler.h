#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <map>

namespace FrontEnd {

	const int TIMED_SECTION_SAMPLES = 60;
	const int TIMED_SECTION_RATE 	= 20;
	
	/*
	================================================================
	===============================================================
	Class TimedSection
	 
	  represents the time taken to run a particular "section" marked
	  by start and stops
	===============================================================
	===============================================================
	*/
	class TimedSection {
	  public:
		
		TimedSection( const std::string &name );
		TimedSection( );
		void initialize( );
		~TimedSection( );

		bool isActive( ) { return activeFrame; }
		
		void start( std::chrono::steady_clock::time_point );
		void stop( std::chrono::steady_clock::time_point );
		
		//send these at start/end of frame to let section
		//know that frame is starting or done
		void startFrame();
		void endFrame();

		double  getElapsedTime( ) { return elapsedTime.count(); }

		//is this section still being timed?
		//bool isActive() { return activeFrame; }

		const std::string& getName( ) { return name; } 

		void addPercentSample( double  sample, int index );
		void addTimeSample( double  sample, int index );

		double  calcAvgPercent( );
		double  calcAvgTime();
	
	  private:	
		bool activeFrame;
		  
		std::chrono::steady_clock::time_point startTime;
		std::chrono::steady_clock::time_point stopTime;
		std::chrono::duration<double>  elapsedTime;
		std::array<double , TIMED_SECTION_SAMPLES> usagePercentAvg;
		std::array<double , TIMED_SECTION_SAMPLES> timeAvg;
		std::string name;

		bool running = false;
	};

	/*
	================================================================
	================================================================
	Class TimeProfiler

	  contains one or more timedSection
	================================================================
	================================================================
	*/
	class TimeProfiler {
	  public:
		TimeProfiler();
		~TimeProfiler( );
		
		void startFrame();
		void stopFrame();
		
		void stopActive( );

		//Start the profile for the given section
		//Will automatically stop other sections that may be running
		void startSection( const std::string &name );
		
		double  getSectionRunPercent( const std::string &name );

		std::string getSectionReport();

	  private:

		//current sample that we are on
		int sampleNum;

		int rateCounter;

		//do we have a full set of samples yet?
		bool fullSet;

		std::chrono::steady_clock::time_point frameStartTime;
		std::chrono::steady_clock::time_point frameStopTime;
		std::chrono::duration<double>  elapsedTime;

		std::map<std::string, TimedSection > timedSections;

		//frame buffer - used to calculate average framerate
		static const int frameTimeBufferSize = 60;
		std::array<double, frameTimeBufferSize> frameTimeBuffer;
		int frameTimeBufferIndex = 0;

		TimedSection *getSection( const std::string &name );

		std::string currentReport;

		void stopAll( );
	};
}