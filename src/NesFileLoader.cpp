#include "precompiled.h"
#include "SystemMain.h"
#include <memory>

using namespace NesEmulator;

/* 
==============================================
NesFile::NesFile() 
==============================================
*/
NesFile::NesFile( NesMain* nesMain ) :
	nesMain( nesMain )
{	
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
void NesFile::loadFile( std::string_view filename ) {
	char nesStr[ 4 ];		//checks for initial string header
	std::string file{ filename };//local copy of filename
	ubyte numcheck, controlbyte1, controlbyte2;		//temp vars
	NesMemory *nesMemory = &FrontEnd::SystemMain::getInstance()->nesMain.nesMemory;
		
	//TODO make this more generalized (maybe add a console variable)
	//file = "./roms/" + file + ".nes";
	file = file + ".nes";
	std::ifstream is( file.c_str(), std::ios::binary );
	if( !is.is_open( ) ) {
		std::error_code ec( errno, std::generic_category( ) );
		std::string error_msg = ec.message( );
		std::cerr << "Failed to open file: " << error_msg << std::endl;
		throw NesFileException( "NesFile::loadFile error", error_msg );
	}
	
	//if memory has already been allocated delete it first
	if( prgRomPages.empty() == false ) {
		prgRomPages.clear( );
	}
	if( chrRomPages.empty() == false ) {
		chrRomPages.clear( );
	}
	
	//if first 3 bytes of header are not equal to "NES" throw exception
	is.read( reinterpret_cast< char* >( nesStr ), 3 );
	nesStr[ 3 ] = '\0';
	if( strcmp( "NES", nesStr ) != 0 ) throw NesFileException( "NesFile::loadFile error", "File not an .nes file" );
	
	//if next value is not 0x1a throw an exception
	//byte 3
	is >> numcheck;
	if( numcheck != 0x1a ) throw NesFileException( "NesFile::loadFile error", "File not an .nes file" );
	
	//read prgRom and charRom counts
	//byte 4/5
	is >> prgRomPageCount;
	is >> chrRomPageCount;

	//force chrRomPage count to 1 if it is 0
	//if( chrRomPageCount == 0 ) chrRomPageCount = 1;
	
	//read the two control bytes
	//byte 6/7
	is >> controlbyte1 >> controlbyte2;

	//extract information from control bytes (also known as flag 6)
	horizontalMirroring 	= !( controlbyte1 & 1);		   //0001
	verticalMirroring		=  ( controlbyte1 & 1);	
	sramEnabled				=  ( controlbyte1 >> 1 ) & 1;  //0010
	trainer					=  ( controlbyte1 >> 2 ) & 1 ; //0100
	fourScreenVRam			=  ( controlbyte1 >> 3 ) & 1 ; //1000
	
	mapperNum = ( controlbyte1 >> 4 ) & 0x0f;
	
	//flag 7 (byte 7)
	mapperNum +=  controlbyte2 & 0x0f;

	//detect if this is an NES 2.0 file format
	bool isNes2 = ( ( 0b00001100 & controlbyte2 ) >> 2 ) == 2;

	//throw exception of trainer setting set - its not supported
	if( trainer ) throw NesFileException( "NesFile::loadFile error", "Trainer not supported" );

	//initialize prg-rom
	prgRomPages = std::vector<ubyte>( prgRomPageCount * PRG_ROM_PAGESIZE );
	
	if( chrRomPageCount != 0 ) {
		chrRomPages = std::vector<ubyte>( chrRomPageCount * CHR_ROM_PAGESIZE );
	}
	
	//load prg and chr rom from file
	//jump to byte 16, which is where data begins
	is.seekg( 16, std::ios::beg );
	is.read( reinterpret_cast< char* >( prgRomPages.data() ), prgRomPageCount * PRG_ROM_PAGESIZE );
	
	if( chrRomPageCount != 0 ) {
		is.read( reinterpret_cast< char* >( chrRomPages.data() ), chrRomPageCount * CHR_ROM_PAGESIZE );
	}

	is.close();

	//put in cpu and ppu membanks
	nesMemory->loadPrgRomPages( prgRomPageCount, prgRomPages );
	nesMemory->ppuMemory.loadChrRomPages( chrRomPageCount, chrRomPages );
	
	//load appropriate mapper
	switch( mapperNum ) {
	case 0:
		nesMemory->initializeMemoryMap( std::make_unique<NesMapper0>() );
		break;															 			   
	case 1:																 			   
		nesMemory->initializeMemoryMap( std::make_unique<NesMapper1>() );
		break;															 			   
	case 2:																 			   
		nesMemory->initializeMemoryMap( std::make_unique<NesMapper2>() );
		break;
	default:
		throw NesFileException( "NesFile::loadFile error", "Mapper not yet supported" );
	}

}
