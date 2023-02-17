#if !defined( TimeProfiler__H )
#define TimeProfiler__H

#include <list>
#include <string>

namespace FrontEnd {
	using namespace std;
	
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
		TimedSection( string name );
		void start( float time );
		void stop( float time );
		
		//send these at start/end of frame to let section
		//know that frame is starting or done
		void startFrame();
		void endFrame();

		float getElapsedTime( ) { return elapsedTime; }

		//is this section still being timed?
		bool isActive() { return activeFrame; }

		string &getName() { return name; }

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
		string name;
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
		void addSection( string name );
		
		//clear all sections in list
		void clearSections();

		void startSection( string name );
		void stopSection( string name );

		float getSectionRunPercent( string name );

		string getSectionReport();

	  private:

		//current sample that we are on
		int sampleNum;

		int rateCounter;

		//do we have a full set of samples yet?
		bool fullSet;

		float frameStartTime;
		float frameStopTime;
		float elapsedTime;

		list< TimedSection* > timedSections;

		TimedSection *getSection( string name );

		string currentReport;
	};
}
#endif