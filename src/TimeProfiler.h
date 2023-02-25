#if !defined( TimeProfiler__H )
#define TimeProfiler__H

#include <list>
#include <string>

namespace FrontEnd {
	
	
	const int TIMED_SECTION_SAMPLES = 60;
	
	//update rate
	const int TIMED_SECTION_RATE = 15;
	
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
		TimedSection( std::string name );
		void start( float time );
		void stop( float time );
		
		//send these at start/end of frame to let section
		//know that frame is starting or done
		void startFrame();
		void endFrame();

		float getElapsedTime( ) { return elapsedTime; }

		//is this section still being timed?
		bool isActive() { return activeFrame; }

		std::string &getName() { return name; }

		void addPercentSample( float sample, int index );
		void addTimeSample( float sample, int index );

		float calcAvgPercent( );
		float calcAvgTime();
	
	  private:	
	
		TimedSection();
		bool activeFrame;
		  
		float startTime;
		float stopTime;
		float elapsedTime;
		float usagePercentAvg[ TIMED_SECTION_SAMPLES ];
		float timeAvg[ TIMED_SECTION_SAMPLES ];
		std::string name;
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
		
		void startFrame();
		void stopFrame();
		
		//add section to main timedSections list
		void addSection( std::string name );
		
		//clear all sections in list
		void clearSections();

		void startSection( std::string name );
		void stopSection( std::string name );

		float getSectionRunPercent( std::string name );

		std::string getSectionReport();

	  private:

		//current sample that we are on
		int sampleNum;

		int rateCounter;

		//do we have a full set of samples yet?
		bool fullSet;

		float frameStartTime;
		float frameStopTime;
		float elapsedTime;

		std::list< TimedSection* > timedSections;

		TimedSection *getSection( std::string name );

		std::string currentReport;
	};
}
#endif