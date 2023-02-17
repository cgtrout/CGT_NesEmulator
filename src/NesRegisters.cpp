#include "NesRegisters.h"

#include "NesMemory.h"
extern NesMemory *nesMemory;

/*
ubyte NesRegisters::getRegister2000( ubyte loc ) {
	
	//get status register
	if( loc == 2 ) {
		
		ubyte temp = 0;
		//return using values in ppuStatusRegister                                            
		temp += ppuStatusRegister.vblankOccurance < < 7;
		temp += ppuStatusRegister.spriteZeroOccurance < < 6; 
		temp += ppuStatusRegister.scanlineSpriteCount < < 5;
		temp += ppuStatusRegister.vRamWriteFlag < < 4;
		return temp;
	}
	//get sprite ram at location specified in register 3
	if( loc == 4 ) {
		return nesMemory->getSprRam( registers2000[ 3 ] );
	}
	//get video memory at currVramAddress
	if( loc == 7 ) {
		return ppuMemory->getMemory( currVramAddress++ );
	}
	
	return registers2000[ loc ];
}
*/

/*
void NesRegisters::setRegister2000( ubyte loc, ubyte val ) {
	registers2000[ loc ] = val;
	
	if( loc == 0 ) {
		ppuControlRegister1.executeNMIonVBlank = ( ubyte )( ( val & 0x80 ) > > 7 );//1000 0000
		ppuControlRegister1.ppuSelection = ( ubyte )( ( val & 0x40 ) > > 6 );//0100 0000
		ppuControlRegister1.spriteSize = ( ubyte )( ( val & 0x20 ) > > 5 );//0010 0000
		ppuControlRegister1.backgroundPatternTableAddress = ( ubyte )( ( val & 0x10 ) > > 4 );//0001 0000
		ppuControlRegister1.spritePatternTableAdress = ( ubyte )( val & 0x08 ) > > 3;//0000 1000
		ppuControlRegister1.ppuAddressIncrement = ( ubyte )( ( val & 0x04 ) > > 2 ) ;//0000 0100
		ppuControlRegister1.nameTableAddress = ( ubyte )( val & 0x03 );//0000 0011
	}
		
	else if( loc == 1 ) {
		ppuControlRegister2.fullBackgroundColour = ( ubyte )( ( val & 0xE0 ) > > 5 );//1110 0000
		ppuControlRegister2.spriteVisibility = ( ubyte )( ( val & 0x10 ) > > 4 );//0001 0000
		ppuControlRegister2.backgroundVisisbility = ( ubyte )( ( val & 0x08 ) > > 3 );//0000 1000
		ppuControlRegister2.spriteClipping = ( ubyte )( ( val & 0x04 ) > > 2 );//0000 0100
		ppuControlRegister2.backgroundClipping = ( ubyte )( ( val & 0x02 ) > > 1 );//0000 0010
		ppuControlRegister2.displayType = ( ubyte )( val & 0x01 );//0000 0001
	}
	

	
	//write to sprRam using address contained within register2000[ 3 ]
	else if( loc == 4 ) {
		nesMemory->setSprRam( registers2000[ 3 ], val );
	}
	//background scroll register
	else if( loc == 5 ) {
		static ubyte currBackgroundScrollByte = 0;
		if( currBackgroundScrollByte == 0 ) {
			horizontalScroll = val;
		}
		else if( currBackgroundScrollByte == 1 ) {
			verticalScroll = val;
			currBackgroundScrollByte = 0;
		}
	}
	//set vram address
	else if( loc == 6 ) {
		static ubyte currVramAddressByte = 0;
		if( currVramAddressByte == 0 ) {
			currVramAddress = 0;
			currVramAddress += val < < 8;
			currVramAddressByte++;
		}
		else if( currVramAddressByte == 1 ) {
			currVramAddress += val;
			currVramAddressByte = 0;
		}
	}
	//write to vram
	else if( loc == 7 ) {
		ppuMemory->setMemory( currVramAddress, val );
		if( ppuControlRegister1.ppuAddressIncrement == 1 ) {
			currVramAddress += 32;
		}
		else {
			currVramAddress++;
		}
	}
	
}

ubyte NesRegisters::getRegister4000( ubyte loc ) {
	//controller 1
	if( loc == 0x16 ) {
		return controller1.getNextStrobe( );
	}
	//controller 2
	else if( loc == 0x17 ) {
		return controller2.getNextStrobe( );
	}
	
	return registers2000[ loc ];
}

void NesRegisters::setRegister4000( ubyte loc, ubyte val ) {
	static bool wasLastWriteOne;

	//sprite dma transfer
	if( loc == 0x14 ) {
		int memLoc = loc*0x100;
		for( int x = 0; x < 256; x++ ) {
			nesMemory->setSprRam( x, nesMemory->getMemory( x+memLoc ) );
		}
	}

	//controller 1
	else if( loc == 0x16 ) {
		if( val == 1 ) {
			wasLastWriteOne = true;
		}
		if( wasLastWriteOne ) {
			if( val == 0 ) {
				controller1.clearStrobe( );
				controller2.clearStrobe( );
				wasLastWriteOne = false;
			}
			else {
				wasLastWriteOne = false;
			}
		}
	}
	//controller 2
	else if( loc == 0x17 ) {
	}
}

*/
