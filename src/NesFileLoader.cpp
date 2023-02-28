#include "precompiled.h"
using namespace NesEmulator;

/* 
==============================================
NesFile::NesFile() 
==============================================
*/
NesFile::NesFile() {	
	prgRomPages = 0;
	chrRomPages = 0;
}

/* 
==============================================
void NesFile::initialize()
==============================================
*/
void NesFile::initialize() {}

/* 
==============================================
void NesFile::loadFile( std::string filename )


TODO delete old mapper if present
TODO allocated memory not being deleted after exception??
==============================================
*/
void NesFile::loadFile( std::string filename ) {
	char nesStr[ 4 ];		//checks for initial string header
	std::string file = filename;	//local copy of filename
	ubyte numcheck, controlbyte1, controlbyte2;		//temp vars
	NesMemory *nesMemory = &FrontEnd::SystemMain::getInstance()->nesMain.nesMemory;
		
	//TODO make this more generalized (maybe add a console variable)
	file = "./roms/" + file + ".nes";
	std::ifstream is( file.c_str(), std::ios::binary );
		
	if( !is ) throw NesFileException( "NesFile::loadFile error", "Could not find file" );
	
	
	//if memory has already been allocated delete it first
	if( prgRomPages != 0 ) {
		delete[] prgRomPages; 
		prgRomPages = 0;
	}
	if( chrRomPages != 0 ) {
		delete[] chrRomPages; 
		chrRomPages = 0;
	}
	
	//if first 3 bytes of header are not equal to "NES" throw exception
	is.read( reinterpret_cast< char* >( nesStr ), 3 );
	nesStr[ 3 ] = '\0';
	if( strcmp( "NES", nesStr ) != 0 ) throw NesFileException( "NesFile::loadFile error", "File not an .nes file" );
	
	//if next value is not 0x1a throw an exception
	is >> numcheck;
	if( numcheck != 0x1a ) throw new NesFileException( "NesFile::loadFile error", "File not an .nes file" );
	
	//read prgRom and charRom counts
	is >> prgRomPageCount;
	is >> chrRomPageCount;

	//force chrRomPage count to 1 if it is 0
	//if( chrRomPageCount == 0 ) chrRomPageCount = 1;
	
	//read the two control bytes
	is >> controlbyte1 >> controlbyte2;

	//extract information from control bytes
	horizontalMirroring 	= !( controlbyte1 & 1);		   //0001
	verticalMirroring	=  ( controlbyte1 & 1);	
	sramEnabled		=  ( controlbyte1 >> 1 ) & 1;  //0010
	trainer			=  ( controlbyte1 >> 2 ) & 1 ; //0100
	fourScreenVRam		=  ( controlbyte1 >> 3 ) & 1 ; //1000
	
	mapperNum = ( controlbyte1 >> 4 ) & 0x0f;
	mapperNum +=  controlbyte2 & 0x0f;

	//throw exception of trainer setting set - its not supported
	if( trainer ) throw new NesFileException( "NesFile::loadFile error", "Trainer not supported" );

	//initialize prg-rom
	prgRomPages = new ubyte[ prgRomPageCount * 0x4000 ];
	
	if( chrRomPageCount != 0 ) {
		chrRomPages = new ubyte[ chrRomPageCount * 0x2000 ];
	}
	
	//load prg and chr rom from file
	is.seekg( 16, std::ios::beg );
	is.read( reinterpret_cast< char* >( prgRomPages ), prgRomPageCount * 0x4000 );
	
	if( chrRomPageCount != 0 ) {
		is.read( reinterpret_cast< char* >( chrRomPages ), chrRomPageCount * 0x2000 );
	}

	is.close();

	//put in cpu and ppu membanks
	nesMemory->loadPrgRomPages( prgRomPageCount, prgRomPages );
	nesMemory->ppuMemory.loadChrRomPages( chrRomPageCount, chrRomPages );
	
	//TODO load appropriate mapper
	if( mapperNum == 0 ) {
		nesMemory->initializeMemoryMap( new NesMapper0() );
	} else if( mapperNum == 2 ) {
		nesMemory->initializeMemoryMap( new NesMapperUnRom() );
	} else {
		throw new NesFileException( "NesFile::loadFile error", "Mapper not yet supported" );
	}
}
