#include "precompiled.h"
#include "NesMappers.h"
#include "NesMain.h"
#include <cassert>

using namespace NesEmulator;
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
=============================================================================
Mapper 1 - MMC1

https://www.nesdev.org/wiki/MMC1
https://nerdy-nights.nes.science/#advanced_tutorial-0

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
void NesMapper1::initializeMap( ) {
	//TODO do proper memory init
	nesMain->nesMemory.ppuMemory.fillChrBanks( 0, 0, 0x2000 / PPU_BANKSIZE );

	//add functions
	//
	auto *entry = new FunctionTableEntry(
		0x8000,	//low
		0xffff,	//high
		
		// Lambda function for write operation
		[this]( uword address, ubyte param ) {
			//Writing a value with bit 7 set( $80 through $FF ) to any address in 
			//$8000 - $FFFF clears the shift register to its initial state
			if( BIT( 7, param ) ) {
				MMC1_SR = 0b10000;
				write_count = 0;		
			}
			else {
				ubyte userVal = (param & 0b00000001);
				
				MMC1_SR = MMC1_SR >> 1;
				MMC1_SR += userVal << 4;
				
				//on the 5th write
				if( write_count == 4 ) {
					//register logic
					MMC1_PB = MMC1_SR;
					//  43210
					//  -----
					//  CPRMM
					//  |||||
					//  |||++- Mirroring (0: one-screen, lower bank; 1: one-screen, upper bank;
					//  |||               2: vertical; 3: horizontal)
					//  ||+--- PRG swap range (0: switch 16 KB bank at $C000; 1: switch 16 KB bank at $8000;
					//  ||                            only used when PRG bank mode bit below is set to 1)
					//  |+---- PRG size (0: switch 32 KB at $8000, ignoring low bit of bank number;
					//  |                         1: switch 16 KB at address specified by location bit above)
					//  +----- CHR size (0: switch 8 KB at a time; 1: switch two separate 4 KB banks)
					if( address >= 0x8000 && address < 0x9FFF ) {
						ubyte mirror				= 0b00011;
						ubyte prg_swap				= 0b00100 >> 2;
						ubyte prg_size				= 0b01000 >> 3;
						ubyte chr_size				= 0b10000 >> 4;

						switch( mirror ) {
						case 0://one screen lower bank
							//TODO implement
							break;
						case 1://one screen upper bank
							//TODO implement
							break;
						case 2://vertical
							nesMain->nesMemory.ppuMemory.switchVerticalMirroring( );
							break;
						case 3://horizontal
							nesMain->nesMemory.ppuMemory.switchHorizontalMirroring( );
							break;
						}

						//TODO 

					}
					//CHR bank 0 ( internal, $A000 - $BFFF )
					else if( address >= 0xA000 && address < 0xBFFF ) {

					}
					//CHR bank 1 ( internal, $C000 - $DFFF )
					else if( address >= 0xC000 && address < 0xDFFF ) {

					}
					//PRG bank( internal, $E000 - $FFFF )
					else if( address >= 0xE000 && address < 0xFFFF ) {

					}
						
					//reset shift register
					write_count = 0;
					MMC1_SR = 0b10000;
				}
				else {
					write_count++;
				}
			}
		},
		
		// Lambda function for read operation
		[this]( uword address ) -> ubyte {
			return 0;
		} );

	entry->setNonReadable( );
	nesMain->nesMemory.addFunction( entry );
}

void NesMapper1::reset( ) {
	//TODO - need to verify proper procedure on this
	int prgRomPages = nesMain->nesMemory.getNumPrgPages( );
	//map first prg rom bank
	nesMain->nesMemory.fillPrgBanks( 0x8000, 0, 0x4000 / CPU_BANKSIZE );
	//map last prg rom bank
	nesMain->nesMemory.fillPrgBanks( 0xC000, ( prgRomPages - 1 ) * PRG_BANKS_PER_PAGE, 0x4000 / CPU_BANKSIZE );

	write_count = 0;
}

/*
=============================================================================
Mapper 2 - UnRom
=============================================================================
*/

void NesMapper2::initializeMap() {
	//map chr rom
	nesMain->nesMemory.ppuMemory.fillChrBanks( 0, 0, 0x2000 / PPU_BANKSIZE );

	//add functions
	//
	auto *entry = new FunctionTableEntry(
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
		} );

	entry->setNonReadable( );
	nesMain->nesMemory.addFunction( entry );
}

void NesMapper2::reset() {
	int prgRomPages = nesMain->nesMemory.getNumPrgPages();
	
	//map first prg rom bank
	nesMain->nesMemory.fillPrgBanks( 0x8000, 0, 0x4000 / CPU_BANKSIZE );

	//map last prg rom bank
	nesMain->nesMemory.fillPrgBanks( 0xC000, (prgRomPages - 1) * PRG_BANKS_PER_PAGE, 0x4000 / CPU_BANKSIZE );
}
