#include "precompiled.h"
#include "NesMappers.h"
#include "NesMain.h"
#include "CgtException.h"
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
	//add functions
	//
	auto entry = FunctionTableEntry(
		0x8000,	//low
		0xffff,	//high
		
		// Lambda function for write operation
		[this]( uword address, ubyte param ) {
			//Writing a value with bit 7 set( $80 through $FF ) to any address in 
			//$8000 - $FFFF clears the shift register to its initial state
			if( BIT( 7, param ) ) {
				MMC1_SR = 0b10000;
				write_count = 0;	

				//bit 2 - reset to logic 1
				//bit 3 - reset to logic 1
				prgrom_bank_address = 0x8000; 
				prgrom_switch_size = 16;
			}
			else {
				//wait 5 writes...
				ubyte userVal = (param & 0b00000001);
				
				MMC1_SR = MMC1_SR >> 1;
				MMC1_SR += userVal << 4;
				
				//on the 5th write...
				if( write_count == 4 ) {
					//Now handle register logic
					//--------------------------------------------------
					//Register 0 is used to configure the mapper
					//Register 1 and 2 are used to control chr_rom banks
					//Register 3 is used to control prg_rom banks

					MMC1_PB = MMC1_SR;  //assign shift register value now
					// REGISTER 00					
					//   https://www.zophar.net/fileuploads/2/10694edseu/mapper1.txt
					//
					if( address >= 0x8000 && address < 0x9FFF ) {
						ubyte horz_vert = BIT( 0, MMC1_PB );
						ubyte bit1 = BIT( 1, MMC1_PB );
						ubyte bit2 = BIT( 2, MMC1_PB );
						ubyte bit3 = BIT( 3, MMC1_PB );
						ubyte bit4 = BIT( 4, MMC1_PB );

						//bit 0 - toggles between "horizontal" and "vertical" mirroring
						//			0 = vertical, 1 = horizontal
						switch( horz_vert ) {
						case 0:
							nesMain->nesMemory.ppuMemory.switchVerticalMirroring( );
							break;
						case 1://one screen upper bank
							nesMain->nesMemory.ppuMemory.switchHorizontalMirroring( );
							break;
						}

						//bit 1 - toggles between H / V and "one-screen" mirroring
						//			0 = one - screen mirroring, 1 = H / V mirroring
						//TODO handle this
												
						//bit 2 - toggles between low PRGROM area switching and high
						//			0 = high PRGROM switching, 1 = low PRGROM switching
						prgrom_bank_address = ( bit2 == 0 ) ? 0xc000 : 0x8000;

						//bit 3 - toggles between 16KB and 32KB PRGROM bank switching
						//			0 = 32KB PRGROM switching, 1 = 16KB PRGROM switching
						ubyte prgrom_switch_size = (bit3 == 0) ? 32 : 16;

						//bit 4 - sets 8KB or 4KB CHRROM switching mode
						//			0 = 8KB CHRROM banks, 1 = 4KB CHRROM banks
						ubyte chrrom_switch_size = ( bit4 == 0 ) ? 8 : 4;

					}
					//CHR bank 0 ( internal, $A000 - $BFFF )
					//4horz_vert 
					//----- 
					//CCCCC 
					//||||| 
					//+++++- Select 4 KB or 8 KB CHR bank at PPU $0000 (low bit ignored in 8 KB mode) 
					// REGISTER 01 - Assign CHR bank
					else if( address >= 0xA000 && address <= 0xBFFF ) {
						ubyte selected_bank = MMC1_PB & 0b11111;

						//todo may need to mask here?
						switch( char_rom_switch_size ) {
						case 4:
							break;
						case 8:
							break;
						}
						
						nesMain->nesMemory.ppuMemory.fillChrBanks(
							0x0000,								//start address
							selected_bank * CHR_BANKS_PER_PAGE,	//chr rom page
							char_rom_switch_size );				//number of banks
							
					}
					//CHR bank 1 ( internal, $C000 - $DFFF )
					//4horz_vert 
					//----- 
					//CCCCC 
					//||||| 
					//+++++- Select 4 KB CHR bank at PPU $1000 (ignored in 8 KB mode) 
					// REGISTER 02 - Assign CHR bank
					else if( address >= 0xC000 && address <= 0xDFFF && char_rom_switch_size != 8 ) {
						ubyte selected_bank = MMC1_PB & 0b01111;
						//TODO wram

						//incorrect in metroid
						//when run in metroid causes 1st chr to be changes?
						//it isn't this directly, but how it somehow affects something else??
						nesMain->nesMemory.ppuMemory.fillChrBanks(
							0x1000,								//start address
							selected_bank * CHR_BANKS_PER_PAGE,	//chr rom page
							char_rom_switch_size);				//number of banks
					}
					//PRG bank( internal, $E000 - $FFFF )
					//4horz_vert 
					//----- 
					//RPPPP 
					//||||| 
					//|++++- Select 16 KB PRG ROM bank (low bit ignored in 32 KB mode) 
					//+----- PRG RAM chip enable (0: enabled; 1: disabled; ignored on MMC1A) 
					// REGISTER 03 - Assign PRG bank	
					else if( address >= 0xE000 && address <= 0xFFFF ) {
						ubyte selected_bank = MMC1_PB & 0b1111;
						ubyte prg_ram_enable = BIT( 4, selected_bank );

						if( prgrom_switch_size == 32 ) {
							selected_bank = selected_bank & 0b1110;
						}

						//do prg rom swap
						nesMain->nesMemory.fillPrgBanks(
							prgrom_bank_address,				//start address
							selected_bank * PRG_BANKS_PER_PAGE,	//prg rom page
							prgrom_switch_size );				//number of banks
					}
						
					//reset shift register
					write_count = 0;
					MMC1_SR = 0b10000;

					nesMain->enableStepDebugging( "RESET SHIFT REGISTER" );
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

	entry.setNonReadable( );
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
	auto entry = FunctionTableEntry(
		0x8000,	//low
		0xffff,	//high
		
		// Lambda function for write operation
		[this]( uword address, ubyte param ) {
 			nesMain->nesMemory.fillPrgBanks(
				0x8000,						//start pos
				param * PRG_BANKS_PER_PAGE,	//prg rom page
				0x4000 / CPU_BANKSIZE );	//number of banks
		},
		
		// Lambda function for read operation
			[this]( uword address ) -> ubyte {
			return 0;
		} );

	entry.setNonReadable( );
	nesMain->nesMemory.addFunction( entry );
}

void NesMapper2::reset() {
	int prgRomPages = nesMain->nesMemory.getNumPrgPages();
	
	//map first prg rom bank
	nesMain->nesMemory.fillPrgBanks( 0x8000, 0, 0x4000 / CPU_BANKSIZE );

	//map last prg rom bank
	nesMain->nesMemory.fillPrgBanks( 0xC000, (prgRomPages - 1) * PRG_BANKS_PER_PAGE, 0x4000 / CPU_BANKSIZE );
}
