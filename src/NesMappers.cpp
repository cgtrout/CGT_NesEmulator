#include "precompiled.h"
#include "NesMappers.h"

#include <cassert>


void NesMapper0::initializeMap() {
	int prgRomPages = systemMain->nesMain.nesMemory.getNumPrgPages();
	
	//map prg rom
	if( prgRomPages == 1 ) {
		systemMain->nesMain.nesMemory.fillPrgBanks( 0x8000, 0, 0x4000 / CPU_BANKSIZE );
		systemMain->nesMain.nesMemory.fillPrgBanks( 0xC000, 0, 0x4000 / CPU_BANKSIZE );
	} else if( prgRomPages == 2 ) {
		systemMain->nesMain.nesMemory.fillPrgBanks( 0x8000, 0, 0x8000 / CPU_BANKSIZE );
	} else {
		assert(true);
	}
	
	//map chr rom
	systemMain->nesMain.nesMemory.ppuMemory.fillChrBanks( 0, 0, 0x2000 / PPU_BANKSIZE );
}

void NesMapper0::reset() {

}

/*
=============================================================================
Mapper 1 - MMC1
=============================================================================
*/
class FuncObjMapper1 : public FunctionObjectBase {
public:
	void write( uword address, ubyte param ) {
		/*
		systemMain->nesMain.nesMemory.fillPrgBanks(
			0x8000,						//start pos
			param * PRG_BANKS_PER_PAGE,	//prg rom page
			0x4000 / CPU_BANKSIZE );	//size of transfer
			*/
	}

	ubyte read( uword address ) {
		return 0;
	}


} funcObjMapper1;

void NesMapper1::initializeMap( ) {
	//map chr rom
	systemMain->nesMain.nesMemory.ppuMemory.fillChrBanks( 0, 0, 0x2000 / PPU_BANKSIZE );

	//add functions
	FunctionTableEntry entry( 0x8000, 0xffff, &funcObjMapper1 );
	entry.setNonReadable( );

	systemMain->nesMain.nesMemory.addFunction( entry );
}

void NesMapper1::reset( ) {
	int prgRomPages = systemMain->nesMain.nesMemory.getNumPrgPages( );

	//map first prg rom bank
	systemMain->nesMain.nesMemory.fillPrgBanks( 0x8000, 0, 0x4000 / CPU_BANKSIZE );

	//map last prg rom bank
	systemMain->nesMain.nesMemory.fillPrgBanks( 0xC000, ( prgRomPages - 1 ) * PRG_BANKS_PER_PAGE, 0x4000 / CPU_BANKSIZE );
}

/*
=============================================================================
Mapper 2 - UnRom
=============================================================================
*/

class FuncObjMapper2 : public FunctionObjectBase {
public:
	void write( uword address, ubyte param ) {
		systemMain->nesMain.nesMemory.fillPrgBanks( 
			0x8000,						//start pos
			param * PRG_BANKS_PER_PAGE,	//prg rom page
			0x4000 / CPU_BANKSIZE );	//size of transfer
	}
	
	ubyte read( uword address ) {
		return 0;
	}
	
} funcObjMapper2;

void NesMapper2::initializeMap() {
	//map chr rom
	systemMain->nesMain.nesMemory.ppuMemory.fillChrBanks( 0, 0, 0x2000 / PPU_BANKSIZE );

	//add functions
	FunctionTableEntry entry( 0x8000, 0xffff, &funcObjMapper2 );
	entry.setNonReadable();

	systemMain->nesMain.nesMemory.addFunction( entry );
}

void NesMapper2::reset() {
	int prgRomPages = systemMain->nesMain.nesMemory.getNumPrgPages();
	
	//map first prg rom bank
	systemMain->nesMain.nesMemory.fillPrgBanks( 0x8000, 0, 0x4000 / CPU_BANKSIZE );

	//map last prg rom bank
	systemMain->nesMain.nesMemory.fillPrgBanks( 0xC000, (prgRomPages - 1) * PRG_BANKS_PER_PAGE, 0x4000 / CPU_BANKSIZE );
}
