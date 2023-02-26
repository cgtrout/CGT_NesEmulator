#include "precompiled.h"

#include "TimeProfiler.h"

#include <sstream>
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
TimedSection::TimedSection( const std::string &name )
==============================================
*/
TimedSection::TimedSection( const std::string &name ) : name(name) {
	startTime = 0;
	stopTime = 0;
	elapsedTime = 0;

	for ( int i = 0; i < TIMED_SECTION_SAMPLES; i++ ) {
		usagePercentAvg[ i ] = 0;
		timeAvg[ i ] = 0;
	}
}

TimedSection::~TimedSection( ) {
	_log->Write( "~TimedSection" );
}

/* 
==============================================
void TimedSection::start( float time )

  TODO what if it starts and stops more than once a frame??
==============================================
*/
void TimedSection::start( float time ) {
	startTime = time;
}

/* 
==============================================
void TimedSection::stop( float time )
==============================================
*/
void TimedSection::stop( float time ) {
	stopTime = time;
	elapsedTime += stopTime - startTime;
	startTime = 0;
	stopTime = 0;
}

/* 
==============================================
void TimedSection::startFrame()
==============================================
*/
void TimedSection::startFrame() {
	activeFrame = true;
	elapsedTime = 0;
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
void TimedSection::addPercentSample( float sample, int index )
==============================================
*/
void TimedSection::addPercentSample( float sample, int index ) {
	//TODO add fault tolerance
	usagePercentAvg[ index ] = sample;
}

void TimedSection::addTimeSample( float sample, int index ) {
	//TODO add fault tolerance

	//convert to ms
	timeAvg[ index ] = sample * 1000;
}

/* 
==============================================
float TimedSection::calcAvgPercent()
==============================================
*/
float TimedSection::calcAvgPercent() {
	float total = 0;
	for( int i = 0; i < TIMED_SECTION_SAMPLES; i++ ) {
		total += usagePercentAvg[ i ];
	}
	return total / TIMED_SECTION_SAMPLES;
}

float TimedSection::calcAvgTime() {
	float total = 0;
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
	timer = Timer::getInstance();
	fullSet = false;	//set to true when a full set of samples is obtained
	sampleNum = 0;
	rateCounter = 0;
	frameStartTime = 0;
	frameStopTime = 0;
	elapsedTime = 0;
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
	frameStartTime = timer->getCurrTime();
	
	auto i = timedSections.begin();
	for( ; i != timedSections.end(); ++i ) {
		(*i).endFrame();
	}
}

/* 
==============================================
void TimeProfiler::stopFrame( )
==============================================
*/
void TimeProfiler::stopFrame( ) {
	frameStopTime = timer->getCurrTime();
	elapsedTime = frameStopTime - frameStartTime;

	//iterate through all time sections to add run percentages
	auto i = timedSections.begin();
	for( ; i != timedSections.end(); ++i ) {
		
		//add run percent to percent average array in current timed section
		(*i).addPercentSample( ( (*i).getElapsedTime() / elapsedTime ) * 100.0f, sampleNum );
		(*i).addTimeSample( (*i).getElapsedTime(), sampleNum );
		(*i).startFrame();
	}

	//handle sampleNum wraparound logic
	if( sampleNum++ == TIMED_SECTION_SAMPLES ) {
		sampleNum = 0;
		
		if( !fullSet ) {
			fullSet = true;
		}
	}
}

/* 
==============================================
void TimeProfiler::addSection( const std::string &name )
==============================================
*/
void TimeProfiler::addSection( const std::string &name ) {
	timedSections.push_back( TimedSection( name ) );
}


/* 
==============================================
TimedSection *TimeProfiler::getSection( const std::string const name )
==============================================
*/
TimedSection *TimeProfiler::getSection( const std::string &name ) {
	_log->Write( "input name=%s, %p", name.c_str( ), this );

	auto i = timedSections.begin();
	for( ; i != timedSections.end(); i++ ) {
		_log->Write("iter=%s %p", ( *i ).getName( ).c_str( ), i );
		if( (*i).getName() == name ) {
			return &(*i);
		}
	}
	
	//TODO throw exception
	return NULL;
}

/* 
==============================================
void TimeProfiler::startSection( const std::string &name )
==============================================
*/
void TimeProfiler::startSection( const std::string &name ) {
	TimedSection *section = getSection( name );	
	
	_log->Write( "Start section %s: ptr=%p", name.c_str(), this );
	
	if ( section == nullptr ) {
		_log->Write( "TimeProfiler::startsection - not found! %s", name.c_str() );
		return;
	}
	_log->Write( "section start" );
	section->start( timer->getCurrTime() );
}

/* 
==============================================
void TimeProfiler::stopSection( const std::string &name )
==============================================
*/
void TimeProfiler::stopSection( const std::string &name ) {
	TimedSection *section = getSection( name );	
	
	if ( section == nullptr ) {
		_log->Write( "stopSection: section not found: %s", name.c_str( ) );
		return;
	}

	section->stop( timer->getCurrTime() );
	section->endFrame();
}

/* 
==============================================
float TimeProfiler::getSectionRunPercent( const std::string &name )
==============================================
*/
float TimeProfiler::getSectionRunPercent( const std::string &name ) {
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
		auto i = timedSections.begin();
		for( ; i != timedSections.end(); i++ ) {
			out << std::setiosflags( std::ios::left )
				<< std::setw(6) << (*i).getName()
				<< std::resetiosflags( std::ios::left )
				<< std::setw(6) << std::setiosflags( std::ios::fixed ) << std::setprecision(2) << (*i).calcAvgTime() \
				<< "ms" 
				<< std::setw(6) << (*i).calcAvgPercent() << "%"
				<< " \n";
		}
		currentReport = out.str();

		rateCounter = 0;
	}
	return currentReport;
}


