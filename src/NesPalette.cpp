#include "precompiled.h"

#include "NesPalette.h"
#include < fstream >

using namespace PpuSystem;
/*
==============================================
NesPalette::loadFromFile( std::string fileName )
==============================================
*/
void NesPalette::loadFromFile( std::string fileName ) {
	//initialize file
	std::ifstream is( fileName.c_str(), std::ios::binary );
	std::string error;	
	
	if( is.fail() ) {
		error = "Error opening file: ";
		error += fileName;
		throw NesFileException( error );
		//TODO present error message to user
	}
	//loop through all elements
	for( int x = 0; x < NUM_PALETTE_ENTRYS; x++ ) {
		////load 3 bytes into palette
		is.read( ( char* )&palette[ x ].color[ COLOR_RED ], 1 );
		is.read( ( char* )&palette[ x ].color[ COLOR_GREEN ], 1 );
		is.read( ( char* )&palette[ x ].color[ COLOR_BLUE ], 1 );
	}

	is.close();
}
