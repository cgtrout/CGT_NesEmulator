#include "precompiled.h"
#include "TimeProfiler.h"

#include <sstream>
#include <numeric>

using namespace FrontEnd;

/*
================================================================
================================================================
TimedSection class
================================================================
================================================================
*/
/* 
==============================================
Constructors / Deconstructors
==============================================
*/
TimedSection::TimedSection( const std::string &name ) : name(name) {
	initialize( );
}

TimedSection::TimedSection( ) {
	initialize( );
}

TimedSection::~TimedSection( ) {
	_log->Write( "~TimedSection" );
}

/* 
==============================================
TimedSection::initialize
==============================================
*/
void FrontEnd::TimedSection::initialize( )
{
	for( int i = 0; i < TIMED_SECTION_SAMPLES; i++ ) {
		usagePercentAvg[ i ] = 0;
		timeAvg[ i ] = 0;
	}
}

/* 
==============================================
void TimedSection::start
==============================================
*/
void TimedSection::start( std::chrono::steady_clock::time_point time ) {
	activeFrame = true;
	startTime = time;
}

/* 
==============================================
void TimedSection::stop
==============================================
*/
void TimedSection::stop( std::chrono::steady_clock::time_point time ) {
	stopTime = time;
	elapsedTime += stopTime - startTime;
	startTime = std::chrono::steady_clock::time_point();
	activeFrame = false;
	//stopTime = std::chrono::steady_clock::time_point( );
}

/* 
==============================================
void TimedSection::startFrame()
==============================================
*/
void TimedSection::startFrame() {
	elapsedTime = std::chrono::duration<double>( );
}

/* 
==============================================
void TimedSection::endFrame()
==============================================
*/
void TimedSection::endFrame() {
	activeFrame = false;
}

/* 
==============================================
void TimedSection::addPercentSample( double sample, int index )
==============================================
*/
void TimedSection::addPercentSample( double sample, int index ) {
	//TODO add fault tolerance
	usagePercentAvg[ index ] = sample;
}

void TimedSection::addTimeSample( double sample, int index ) {
	//TODO add fault tolerance

	//convert to ms
	timeAvg[ index ] = sample * 1000;
}

/* 
==============================================
TimedSection::calcAvgPercent()
==============================================
*/
double TimedSection::calcAvgPercent() {
	double total = 0;
	for( int i = 0; i < TIMED_SECTION_SAMPLES; i++ ) {
		total += usagePercentAvg[ i ];
	}
	return total / TIMED_SECTION_SAMPLES;
}

/* 
==============================================
TimedSection::calcAvgTime()
==============================================
*/
double TimedSection::calcAvgTime() {
	double total = 0;
	for( int i = 0; i < TIMED_SECTION_SAMPLES; i++ ) {
		total += timeAvg[ i ];
	}
	return total / TIMED_SECTION_SAMPLES;
}

/*
================================================================
================================================================
TimeProfiler class
================================================================
================================================================
*/

/* 
==============================================
TimeProfiler::TimeProfiler()
==============================================
*/
TimeProfiler::TimeProfiler() : timedSections() {
	fullSet = false;	//set to true when a full set of samples is obtained
	sampleNum = 0;
	rateCounter = 0;
	frameTimeBuffer.fill( 0 );
}

TimeProfiler::~TimeProfiler( ) {
	_log->Write( "~TimeProfiler" );
}

/* 
==============================================
void TimeProfiler::startFrame( )
==============================================
*/
void TimeProfiler::startFrame( ) {
	frameStartTime = std::chrono::steady_clock::now( );
	
	auto i = timedSections.begin();
	for( ; i != timedSections.end(); ++i ) {
		(*i).second.endFrame();
	}
}

/* 
==============================================
void TimeProfiler::stopFrame( )
==============================================
*/
void TimeProfiler::stopFrame( ) {
	frameStopTime = std::chrono::steady_clock::now( );
	elapsedTime = frameStopTime - frameStartTime;
	
	//_log->Write( "TimeProfiler: %f", elapsedTime );

	//iterate through all time sections to add run percentages
	for( auto &i : timedSections ) {
		auto& timedSection = i.second;

		//add run percent to percent average array in current timed section
		timedSection.addPercentSample( ( timedSection.getElapsedTime() / elapsedTime.count() ) * 100.0f, sampleNum );
		timedSection.addTimeSample( timedSection.getElapsedTime(), sampleNum );
		timedSection.startFrame();
	}

	//handle sampleNum wraparound logic
	if( ++sampleNum == TIMED_SECTION_SAMPLES ) {
		sampleNum = 0;
		
		if( !fullSet ) {
			fullSet = true;
		}
	}

	frameTimeBuffer[ frameTimeBufferIndex++ ] = elapsedTime.count( );
	if ( frameTimeBufferIndex == frameTimeBufferSize ) {
		frameTimeBufferIndex = 0;
	}
}

/*
==============================================
TimeProfiler::stopActive
==============================================
*/
void TimeProfiler::stopActive( ) {
	for( auto& i : timedSections ) {
		auto& timedSection = i.second;
		if( timedSection.isActive( ) ) {
			timedSection.stop( std::chrono::steady_clock::now( ) );
		}
	}
}

/* 
==============================================
TimedSection *TimeProfiler::getSection( const std::string const name )
==============================================
*/
TimedSection *TimeProfiler::getSection( const std::string &name ) {
	//_log->Write( "input name=%s, %p", name.c_str( ), this );

	auto it = timedSections.find( name );
	if( it != timedSections.end( ) ) {
		return &timedSections[ name ];
	}
	else {
		timedSections[ name ] = TimedSection( name );
		return &timedSections[ name ];
	}
}

/*
==============================================
TimeProfiler::stopAll
==============================================
*/
void FrontEnd::TimeProfiler::stopAll( ) {
	//iterate through all time sections to add run percentages
	for( auto& i : timedSections ) {
		auto& timedSection = i.second;

		if( timedSection.isActive() ) {
			timedSection.stop( std::chrono::steady_clock::now( ) );
		}
	}
}

/* 
==============================================
void TimeProfiler::startSection( const std::string &name )
==============================================
*/
void TimeProfiler::startSection( const std::string &name ) {
	TimedSection *section = getSection( name );	
	stopAll( );
	
	section->start( std::chrono::steady_clock::now( ) );
}

/* 
==============================================
double TimeProfiler::getSectionRunPercent( const std::string &name )
==============================================
*/
double TimeProfiler::getSectionRunPercent( const std::string &name ) {
	return getSection( name )->calcAvgPercent();
}

/* 
==============================================
string TimeProfiler::getSectionReport( )
  assumes it will be called every frame
==============================================
*/
std::string TimeProfiler::getSectionReport( ) {
	std::stringstream out;

	rateCounter++;

	//only update every TIMED_SECTION_RATE frames
	if( rateCounter == TIMED_SECTION_RATE ) {
		//iterate through all sections and create output line
		
		for( auto& s : timedSections ) {
			auto& section = s.second;
			out << std::setiosflags( std::ios::left )
				<< std::setw(20) << section.getName()
				<< std::resetiosflags( std::ios::left )
				<< std::setw(6) << std::setiosflags( std::ios::fixed ) << std::setprecision(2) << section.calcAvgTime() \
				<< "ms" 
				<< std::setw(6) << section.calcAvgPercent() << "%"
				<< " \n";
		}
		
		out << "\n";
		
		auto average = std::accumulate( frameTimeBuffer.begin( ), frameTimeBuffer.end( ), 0.0 ) / frameTimeBuffer.size( );
		out << "FPS: " << 1.0f/average;
		currentReport = out.str();

		rateCounter = 0;
	}
	return currentReport;
}


