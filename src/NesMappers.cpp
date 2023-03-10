#include "precompiled.h"
#include "NesMappers.h"

void NesMapper0::initializeMap() {
	int prgRomPages = systemMain->nesMain.nesMemory.getNumPrgPages();
	
	//map prg rom
	if( prgRomPages == 1 ) {
		systemMain->nesMain.nesMemory.fillPrgBanks( 0x8000, 0, 0x4000 / CPU_BANKSIZE );
		systemMain->nesMain.nesMemory.fillPrgBanks( 0xC000, 0, 0x4000 / CPU_BANKSIZE );
	} else if( prgRomPages == 2 ) {
		systemMain->nesMain.nesMemory.fillPrgBanks( 0x8000, 0, 0x8000 / CPU_BANKSIZE );
	} else {
		_ASSERTE(true);
	}
	
	//map chr rom
	systemMain->nesMain.nesMemory.ppuMemory.fillChrBanks( 0, 0, 0x2000 / PPU_BANKSIZE );
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

class FuncObjUnRom : public FunctionObjectBase {
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

	
} funcObjUnRom;

void NesMapperUnRom::initializeMap() {
	//map chr rom
	systemMain->nesMain.nesMemory.ppuMemory.fillChrBanks( 0, 0, 0x2000 / PPU_BANKSIZE );

	//add functions
	FunctionTableEntry entry( 0x8000, 0xffff, &funcObjUnRom );
	entry.setNonReadable();

	systemMain->nesMain.nesMemory.addFunction( &entry );
}

void NesMapperUnRom::reset() {
	int prgRomPages = systemMain->nesMain.nesMemory.getNumPrgPages();
	
	//map first prg rom bank
	systemMain->nesMain.nesMemory.fillPrgBanks( 0x8000, 0, 0x4000 / CPU_BANKSIZE );

	//map last prg rom bank
	systemMain->nesMain.nesMemory.fillPrgBanks( 0xC000, (prgRomPages - 1) * PRG_BANKS_PER_PAGE, 0x4000 / CPU_BANKSIZE );
}
