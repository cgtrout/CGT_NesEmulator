#include "precompiled.h"
#include "NesMappers.h"
#include "NesMain.h"

#include <cassert>

using namespace NesEmulator;

void NesMapHandler::setMapHandler( NesMain* nesMain ) {
	this->nesMain = nesMain;
}
void NesMapper0::initializeMap() {
	int prgRomPages = nesMain->nesMemory.getNumPrgPages();
	
	//map prg rom
	if( prgRomPages == 1 ) {
		nesMain->nesMemory.fillPrgBanks( 0x8000, 0, 0x4000 / CPU_BANKSIZE );
		nesMain->nesMemory.fillPrgBanks( 0xC000, 0, 0x4000 / CPU_BANKSIZE );
	} else if( prgRomPages == 2 ) {
		nesMain->nesMemory.fillPrgBanks( 0x8000, 0, 0x8000 / CPU_BANKSIZE );
	} else {
		assert(true);
	}
	
	//map chr rom
	nesMain->nesMemory.ppuMemory.fillChrBanks( 0, 0, 0x2000 / PPU_BANKSIZE );
}

void NesMapper0::reset() {

}


/*

Mapper 2 - UNROM
 ┌───────────────┐         ┌──────────────────────────────────────────┐
 │ $8000 ─ $FFFF ├─────────┤ PPPPPPPP                                 │
 └───────────────┘         │ └──┬───┘                                 │
                           │    │                                     │
                           │    │                                     │
                           │    └─────── Select 16K ROM bank at $8000 │
                           └──────────────────────────────────────────┘

 Notes: - When the cart is first started, the first 16K ROM bank in the cart
           is loaded into $8000, and the LAST 16K ROM bank is loaded into
           $C000. This last 16K bank is permanently "hard-wired" to $C000,
           and it cannot be swapped.
        - This mapper has no provisions for VROM; therefore, all carts
           using it have 8K of VRAM at PPU $0000.
        - Most carts with this mapper are 128K. A few, mostly Japanese
           carts, such as Final Fantasy 2 and Dragon Quest 3, are 256K.
        - Overall, this is one of the easiest mappers to implement in
           a NES emulator.

****************************************************************************
*/

void NesMapperUnRom::initializeMap() {
	//map chr rom
	nesMain->nesMemory.ppuMemory.fillChrBanks( 0, 0, 0x2000 / PPU_BANKSIZE );

	auto entry = new FunctionTableEntry(
		0x8000,	//low
		0xffff,	//high
		// Lambda function for write operation
		[this]( uword address, ubyte param ) {
			nesMain->nesMemory.fillPrgBanks(
				0x8000,						//start pos
				param * PRG_BANKS_PER_PAGE,	//prg rom page
				0x4000 / CPU_BANKSIZE );	//size of transfer
		},
		// Lambda function for read operation
		[this]( uword address ) -> ubyte {
			return 0;
		}
		);

	entry->setNonReadable();
	nesMain->nesMemory.addFunction( entry );
}

void NesMapperUnRom::reset() {
	int prgRomPages = nesMain->nesMemory.getNumPrgPages();
	
	//map first prg rom bank
	nesMain->nesMemory.fillPrgBanks( 0x8000, 0, 0x4000 / CPU_BANKSIZE );

	//map last prg rom bank
	nesMain->nesMemory.fillPrgBanks( 0xC000, (prgRomPages - 1) * PRG_BANKS_PER_PAGE, 0x4000 / CPU_BANKSIZE );
}
