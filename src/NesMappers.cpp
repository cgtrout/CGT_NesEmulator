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

https://www.nesdev.org/wiki/MMC1


- Change register value by writing five times with bit 7 clear and desired value in bit 0 
  (starting with low bit)
- First four writes: MMC1 shifts bit 0 into shift register
- Fifth write: MMC1 copies bit 0 and shift register contents into internal register (selected 
  by bits 14 and 13 of address), then clears shift register
- Only fifth write's address matters, specifically bits 14 and 13
- After fifth write, shift register is cleared automatically, no need to write again with 
  bit 7 set
=============================================================================
*/
class FuncObjMapper1 : public FunctionObjectBase {
public:
	void write( uword address, ubyte param ) {
		//Writing a value with bit 7 set( $80 through $FF ) to any address in 
		//$8000 - $FFFF clears the shift register to its initial state
		if( BIT( 7, param ) ) {
			MMC1_SR = 0b10000;
		}
		else {
			MMC1_SR = MMC1_SR >> 1;
		}
	}

	ubyte read( uword address ) {
		return 0;
	}

	ubyte MMC1_SR = 0b10000;	//shift register
	ubyte MMC1_PB = 0;	//program bank selection


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
