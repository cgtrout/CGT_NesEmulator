#include "Precompiled.h"
#include "NesSound.h"

//#include <boost/thread/mutex.hpp>

using namespace NesApu;

//number of clocks in 240hz
static const int clocksPer240Hz = 7457;

#include "graph.h"

GraphGen<uword> graph;
//uword data[22050];

//TODO set fracComp properly 
NesSoundBuffer::NesSoundBuffer( int bufLength ):  
 bufferLength( bufLength),
 fracComp( 0, 77 ),
 buffer( 0 ),
 playPos( 0 ) {
	buffer.resize( bufferLength, 0 );

	graph.setWindow_x( 0, 256 );
	graph.setWindow_y( 0, 20000 );
	graph.setDrawGrid_x( false );
	graph.setDrawGrid_y( false );
	graph.setGridSize_x( 1 );
	graph.setGridSize_y( 1000 );
	graph.setPixelHeight( 256 );
	graph.setPixelWidth( 512 );
	graph.setDataBuffer( &buffer, bufferLength );
	graph.setShadeGraph( true );
	graph.setAxisColor( &Pixel3Byte( 255, 255, 255 ) );
	graph.setGridColor( &Pixel3Byte( 100, 100, 100 ) );
	graph.setLineColor( &Pixel3Byte( 255, 255, 0 ) );
	graph.setShadeColor( &Pixel3Byte( 200, 0, 0 ) );
}

NesSoundBuffer::~NesSoundBuffer() {
}

//boost::mutex mutex;
//size is number of samples, and not actual byte size of copy
void NesSoundBuffer::fillExternalBuffer( Uint16* ptr, int size ) {
	_ASSERTE( size <= bufferLength );
	if( this == NULL ) { 
		return;
	}
	
	//needed??
	//boost::mutex::scoped_lock lock(mutex);

	//may need to do in two steps
	bool twoStep = ( playPos + size ) >= bufferLength;

	/*
	_log->Write( "\n\nfilling external buffer" );
	_log->Write( "playPos at start = %d ", playPos );
	_log->Write( "bufferPos = %d", bufferPos  );
	_log->Write( "size request = %d", size );
	*/

	try {
		if( !twoStep ) {
			//copy in one step
			memcpy( ptr, &buffer[ playPos ], size );
			playPos += size;
			//_log->Write( "one step copy" );
			//_log->Write( "playPos at end = %d ", playPos );
			
			return;
		} else {
			//_log->Write( "two step copy" );

			//first chunck
			int chunk1Size = bufferLength - playPos;
			memcpy( ptr, &buffer[ playPos ], chunk1Size * 2 );

			//_log->Write( "copying chunk 1 - chunk1Size = %d", chunk1Size );
			//playPos = 0;
			
			//second chunk
			int chunk2Size = size - chunk1Size;
			memcpy( &ptr[ chunk1Size ] , &buffer[0], chunk2Size * 2 );
			//_log->Write( "copying chunk 2 - chunk2Size = %d", chunk2Size );
			playPos = chunk2Size;
			
			//_log->Write( "playPos at end = %d ", playPos );			
		}
	}
	catch( std::exception ) {
		//TODO handle this better
		_log->Write( "Exception generated in NesSoundBuffer::fillExternalBuffer" );
	}
}

void NesSoundBuffer::addSample( Uint16 sample ) {
	sampleTotal += sample;
	if( ++sampleNum == 40 ) {
		sampleNum = 0;
		
		//was a whole number generated?
		int whole = fracComp.add( 45 );
		sampleNum += whole;
		
		//add sample to buffer	
		buffer[ bufferPos++ ] = sampleTotal / ( 40 + whole );
		if( bufferPos == bufferLength ) {
			bufferPos = 0;
		}
		sampleTotal = 0;
	}
}

NesSound::NesSound():  
	curr240Clock(0),
	fracCC(0, 3),
	frac240Clock( 0, 2 ),
	square0( 0 ),
	square1( 1 ), 
	
	buffer( 44100 / 2 )  {
}

//descc desired clock cycle
void NesSound::runTo( PpuClockCycles desPpucc ) {
	CpuClockCycles descc = PpuToMaster( desPpucc );
	
	//descc must be greater than cc
	_ASSERTE( descc > cc );

	//currc = descc;
	
	//if fraction adds up to whole numbers add it to current cc;
	cc += fracCC.add( PpuToMasterMod( desPpucc ) );

	for( ; cc < descc; ++cc ) {
		clock();
		makeSample();
	}

	//_log->Write( "nesSound::runTo() bufferPos @ end = %d", buffer.getBufferPos() );
}

void NesSound::resetCC() {
	cc = 0;
}

Image *NesSound::getGraph() {
	return graph.getImage();
}

void NesSound::makeSample() {
	float squareOut = 0;
	if( square0.getDacValue() == 0 && square1.getDacValue() == 0 ) {
		squareOut = 0;
	} else {
		//TODO slow - use tables
		squareOut = (float)95.88 / ( ( 8128 / ( square0.getDacValue() + square1.getDacValue() ) ) + 100 );
		//if( square1.getDacValue() == 0 ) squareOut = 0;
		//else squareOut = (float)95.88 / ( ( 8128 / ( square1.getDacValue() ) ) + 100 );
	}
	float output = squareOut;
	
	//convert and scale to uword
	buffer.addSample( (uword)( output * 0xffff ) ); 
}

inline void NesSound::clock() {
	//increment clocks
	square0.clock();
	square1.clock();
	//triangle.clock();
	//noise.clock();
	//dmc.clock();
	
	//check for 240hz clock
	if( ++curr240Clock == clocksPer240Hz ) {
		clock240Hz();
		curr240Clock = 0;
		
		//add 1 to clock if one whole number has been produced
		curr240Clock += frac240Clock.add( 1 );	
	}
}

//TODO handle 5 step
void NesSound::clock240Hz( ) {
	square0.clock240Hz( curr240Clock );
	square1.clock240Hz( curr240Clock );
	//triangle.clock240Hz( curr240Clock );
	//noise.clock240Hz( curr240Clock );
	//dmc.clock240Hz( curr240Clock );
	
	//increment 240 clock and reset it if necessary
	if( ++curr240Clock == 4 ) {
		curr240Clock = 0;
	}
}

//
//
//Memory registers
//
//

//square 0 channel

