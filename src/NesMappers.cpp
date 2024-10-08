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

				//fix last prg bank at $C000
				prgrom_bank_mode = 3;
				remapMemory( );
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
					if( address >= 0x8000 && address <= 0x9FFF ) {
						ubyte horz_vert = BIT( 0, MMC1_PB );
						ubyte bit1 = BIT( 1, MMC1_PB );
						ubyte bit2 = BIT( 2, MMC1_PB );
						ubyte bit3 = BIT( 3, MMC1_PB );
						ubyte bit4 = BIT( 4, MMC1_PB );

						auto mirroring = 0b00011 & MMC1_PB;

						switch( mirroring ) {
						case 0:
							nesMain->nesMemory.ppuMemory.switchSingleLowMirroring( );
							break;
						case 1:
							nesMain->nesMemory.ppuMemory.switchSingleHighMirroring( );
							break;
						case 2:
							nesMain->nesMemory.ppuMemory.switchVerticalMirroring( );
							break;
						case 3:
							nesMain->nesMemory.ppuMemory.switchHorizontalMirroring( );
							break;
						}
												
						//bit 2, 3 prgrom_bank_mode
						prgrom_bank_mode = (MMC1_PB & 0b1100) >> 2;

						//bit 4 - sets 8KB or 4KB CHRROM switching mode
						//			0 = 8KB CHRROM banks, 1 = 4KB CHRROM banks
						char_rom_switch_size = ( bit4 == 0 ) ? 8 : 4;

						//TODO we may need to reassign the locations here if any of these bits are different
						remapMemory( );

					}
					//CHR bank 0 ( internal, $A000 - $BFFF )
					//4horz_vert 
					//----- 
					//CCCCC 
					//||||| 
					//+++++- Select 4 KB or 8 KB CHR bank at PPU $0000 (low bit ignored in 8 KB mode) 
					// REGISTER 01 - Assign CHR bank
					else if( address >= 0xA000 && address <= 0xBFFF ) {
						chr_selected_bank_0000 = MMC1_PB & 0b11111;

						if( char_rom_switch_size == 8 ) {
							//bottom bit is ignored (nerdy nights) so there are 16 possible banks
							chr_selected_bank_0000 &= 0b11110;
							// sets bank for full PPU 0000-1FFF(8kb region)
						}

						remapMemory( );
							
					}
					//CHR bank 1 ( internal, $C000 - $DFFF )
					//4horz_vert 
					//----- 
					//CCCCC 
					//||||| 
					//+++++- Select 4 KB CHR bank at PPU $1000 (ignored in 8 KB mode) 
					// REGISTER 02 - Assign CHR bank
					else if( address >= 0xC000 && address <= 0xDFFF ) {
						chr_selected_bank_1000 = MMC1_PB & 0b01111;
						//TODO wram
						remapMemory( );
						
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
						prg_selected_bank = MMC1_PB & 0b1111;
						prg_ram_enable = BIT( 4, MMC1_PB );

						
						remapMemory( );
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

	entry.setNonReadable( );
	nesMain->nesMemory.addFunction( entry );
}

void NesMapper1::remapMemory( ) {
	//nesMain->nesCpu.cpuTrace.addTraceText( "\nRemap memory..... char_rom_switch_size=%d chr_selected_bank_0000=%d  chr_selected_bank_1000=%d", char_rom_switch_size, chr_selected_bank_0000, chr_selected_bank_1000 );

	//reg 1: A000
	//nesMain->nesCpu.cpuTrace.addTraceText( "reg1: A000   Before fillchrBanks" );
	nesMain->nesMemory.ppuMemory.fillChrBanks(
		0x0000,															
		chr_selected_bank_0000 * char_rom_switch_size * PPU_BANKSIZE,	
		char_rom_switch_size );							

	//reg 2: C000
	//nesMain->nesCpu.cpuTrace.addTraceText( "reg2: C000   Before fillchrBanks" );
	if( char_rom_switch_size != 8 ) {
		nesMain->nesMemory.ppuMemory.fillChrBanks(
			0x1000,											
			chr_selected_bank_1000 * char_rom_switch_size * PPU_BANKSIZE,	
			char_rom_switch_size );							
	}

	//reg 3: E000
	//nesMain->nesCpu.cpuTrace.addTraceText( "reg3: E000   Before fillprgBanks" );

	switch( prgrom_bank_mode ) {
	case 0:
	case 1: {
		//switch 32kb at $8000, ignoring low bit
		ubyte selectedBank = prg_selected_bank;
		selectedBank &= 0b11111110;
		nesMain->nesMemory.fillPrgBanks(
			0x8000,
			selectedBank * PRG_ROM_PAGESIZE,
			32 );
		break;
	}
	case 2:
		//fix first bank at $8000
		nesMain->nesMemory.fillPrgBanks(
			0x8000,
			0,
			16 );

		//switch 16kb bank at $C000
		nesMain->nesMemory.fillPrgBanks(
			0xC000,
			prg_selected_bank * PRG_ROM_PAGESIZE,
			16 );
		break;
	case 3:
		auto last_bank = nesMain->nesFile.getNumPrgRomPages()-1;
		//fix last bank at $C000
		nesMain->nesMemory.fillPrgBanks(
			0xC000,
			last_bank * PRG_ROM_PAGESIZE,
			16 );

		//switch 16kb at $8000
		nesMain->nesMemory.fillPrgBanks(
			0x8000,
			prg_selected_bank * PRG_ROM_PAGESIZE,
			16 );
	}					
}

void NesMapper1::reset( ) {
	//TODO - need to verify proper procedure on this
	int prgRomPages = nesMain->nesMemory.getNumPrgPages( );
	//map first prg rom bank
	nesMain->nesMemory.fillPrgBanks( 0x8000, 0, 0x4000 / CPU_BANKSIZE );
	//map last prg rom bank
	nesMain->nesMemory.fillPrgBanks( 0xC000, ( prgRomPages - 1 ) * PRG_ROM_PAGESIZE, 0x4000 / CPU_BANKSIZE );

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
				param * PRG_ROM_PAGESIZE,	//prg rom page
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
	nesMain->nesMemory.fillPrgBanks( 0xC000, (prgRomPages - 1) * PRG_ROM_PAGESIZE, 0x4000 / CPU_BANKSIZE );
}
