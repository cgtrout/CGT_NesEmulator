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
TimedSection::TimedSection( std::string name )
==============================================
*/
TimedSection::TimedSection( std::string name ) {
	this->name = name;
	startTime = 0;
	stopTime = 0;
	elapsedTime = 0;
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
TimeProfiler::TimeProfiler() {
	timer = Timer::getInstance();
	fullSet = false;	//set to true when a full set of samples is obtained
	sampleNum = 0;
	rateCounter = 0;
	frameStartTime = 0;
	frameStopTime = 0;
	elapsedTime = 0;
}

/* 
==============================================
void TimeProfiler::startFrame( )
==============================================
*/
void TimeProfiler::startFrame( ) {
	frameStartTime = timer->getCurrTime();
	
	std::list< TimedSection* >::iterator i = timedSections.begin();
	for( ; i != timedSections.end(); ++i ) {
		(*i)->endFrame();
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
	std::list< TimedSection* >::iterator i = timedSections.begin();
	for( ; i != timedSections.end(); ++i ) {
		
		//add run percent to percent average array in current timed section
		(*i)->addPercentSample( ( (*i)->getElapsedTime() / elapsedTime ) * 100.0f, sampleNum );
		(*i)->addTimeSample( (*i)->getElapsedTime(), sampleNum );
		(*i)->startFrame();
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
void TimeProfiler::addSection( std::string name )
==============================================
*/
void TimeProfiler::addSection( std::string name ) {
	timedSections.push_back( new TimedSection( name ) );
}

/* 
==============================================
void TimeProfiler::clearSections()
==============================================
*/
void TimeProfiler::clearSections() {
	std::list<TimedSection*>::iterator i = timedSections.begin();
	for( ; i != timedSections.end(); i++ ) {
		if( *i != 0 ) {
			delete ( *i );
		}
	}
	timedSections.clear();
}

/* 
==============================================
TimedSection *TimeProfiler::getSection( std::string name )
==============================================
*/
TimedSection *TimeProfiler::getSection( std::string name ) {
	std::list<TimedSection*>::iterator i = timedSections.begin();
	for( ; i != timedSections.end(); i++ ) {
		if( (*i)->getName() == name ) {
			return (*i);
		}
	}
	//TODO throw exception
	return NULL;
}

/* 
==============================================
void TimeProfiler::startSection( std::string name )
==============================================
*/
void TimeProfiler::startSection( std::string name ) {
	TimedSection *section = getSection( name );	
	
	//see if any other sections are running (very slow)
	/*
	list<TimedSection*>::iterator i = timedSections.begin();
	for( ; i != timedSections.end(); ++i ) {
		if( (*i)->getName() == name ) {
			continue;
		}
		if( (*i)->isActive() ) {
			FrontEnd::SystemMain::getInstance()->consoleSystem.printMessage( 
				"More than one section active - '%s' is already running while starting '%s' ",
				section->getName().c_str(), (*i)->getName().c_str() );
		}
	}*/

	section->start( timer->getCurrTime() );
}

/* 
==============================================
void TimeProfiler::stopSection( std::string name )
==============================================
*/
void TimeProfiler::stopSection( std::string name ) {
	TimedSection *section = getSection( name );	
	section->stop( timer->getCurrTime() );
	section->endFrame();
}

/* 
==============================================
float TimeProfiler::getSectionRunPercent( std::string name )
==============================================
*/
float TimeProfiler::getSectionRunPercent( std::string name ) {
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
		std::list< TimedSection* >::iterator i = timedSections.begin();
		for( ; i != timedSections.end(); i++ ) {
			out << std::setiosflags( std::ios::left )
				<< std::setw(6) << (*i)->getName()
				<< std::resetiosflags( std::ios::left )
				<< std::setw(6) << std::setiosflags( std::ios::fixed ) << std::setprecision(2) << (*i)->calcAvgTime() \
				<< "ms" 
				<< std::setw(6) << (*i)->calcAvgPercent() << "%"
				<< " \n";
		}
		currentReport = out.str();

		rateCounter = 0;
	}
	return currentReport;
}


