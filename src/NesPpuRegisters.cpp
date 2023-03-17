#include "Precompiled.h"
#include "NesPpuScanline.h"

using namespace NesEmulator;

/* 
==============================================
NesPPU::Registers::Registers()
==============================================
*/
Registers::Registers() {
	//initialize all registers
	//fineVerticle = 0;
	//fineHorizontal = 0;
	//verticleTileIndex = 0;
	//horizontalTileIndex = 0;
	vramAddress = 0;
	tempAddress = 0;
	xOffset = 0;
	ubyte tileAttributeValue = 0;
	this->status.inVblank = 0;
}

/* 
==============================================
void NesPPU::Registers::reset()
==============================================
*/
void Registers::reset() {
	vramAddress = 0;
	tempAddress = 0;
	xOffset = 0;
	ubyte tileAttributeValue = 0;
	
	this->status.inVblank = 0;
	this->status.sprite0Hit = 0;
	this->status.sprite0Time = 0;
	
	spritesThisScanline = 0;
	colorSelection = 0;
	spriteVisible = 0;
	backgroundVisible = 0;
	executeNMIonVBlank = 0;
	spriteSize = 0;
	playfieldPatternTableSection = 0;
	spritePatternTableAddress = 0;
	addressIncrement = 0;
}

/* 
==============================================
void NesPPU::Registers::incrementVramAddress()
==============================================
*/
void Registers::incrementVramAddress() {
	if( addressIncrement == 1 ) {
		vramAddress += 32;
	}
	else {
		vramAddress += 1;
	}
}

/* 
==============================================
void NesPPU::Registers::convert2000FromByte( ubyte reg )
==============================================
*/
void Registers::convert2000FromByte( ubyte reg ) {
	executeNMIonVBlank			 = BIT( 7, reg );
	spriteSize					 = BIT( 5, reg );
	playfieldPatternTableSection = BIT( 4, reg );
	spritePatternTableAddress	 = BIT( 3, reg );
	addressIncrement			 = BIT( 2, reg );
	
	tempAddress &= 0xf3ff;
	tempAddress += ( reg & 3 ) << 10;
}

/* 
==============================================
void NesPPU::Registers::convert2001FromByte( ubyte reg )
==============================================
*/
void Registers::convert2001FromByte( ubyte reg ) {
	//set colorSelection ( 3 bit value )
	colorSelection = ( reg & 0xE0 ) >> 5; 
	
	//set others
	spriteVisible		 = BIT( 4 , reg );
	backgroundVisible	 = BIT( 3 , reg );
	spriteClipping		 = BIT( 2 , reg );
	backgroundClipping	 = BIT( 1 , reg );
	colorMode			 = ( colormodes )( BIT( 0 , reg ) );
}

/* 
==============================================
ubyte NesPPU::Registers::Status::makeByte( PpuClockCycles cc )
==============================================
*/
ubyte Status::makeByte( PpuClockCycles cc ) {
	ubyte out = 0;


	//if(sprite0Time != 0) {
	//	
	//}x
	//else
	//	_log->Write( "2002 access @ cc: %d pc: %x" , cc, nesCpu->getPC() );
	if( cc >= sprite0Time && sprite0Time != 0 ) {
		sprite0Hit = 1;
		//_log->Write( "2002 sprite 0 hit cc: %d Sprite0 Time is %d pc = %x", cc, sprite0Time, nesCpu->getPC() );
	}

	//if( cc >= 341 && cc <= 6850 ) {
	if( cc >= 0 && cc < vblankOffTime && !vBlankRead ) {
		inVblank = 1;
		vBlankRead = true;

		//_log->Write( "inVblank set @ cc = %d", cc );
	}

	//TODO find out exact time this occurs
	//should be at around 6810
	//reset vblank flag
	if( cc >= vblankOffTime ) {
		inVblank = 0;
	}

	out =	( vRamWritesAllowed		<< 4 ) +
			( scanlineSriteMaxHit	<< 5 ) +
			( sprite0Hit			<< 6 ) +
			( inVblank				<< 7 );

	inVblank = 0;

	return out;
}

bool Registers::incrementXOffset() { 
	bool newTile = false;
	xOffset++;
	if( xOffset == 8 ) {
		//moving to new tile so reset counters
		xOffset = 0;

		newTile = true;

		//increment x scroll and handle counter wrap around
		if( ( vramAddress & 0x1f ) != 0x1f ) {
			++vramAddress;
		} else {
			vramAddress &= 0xffe0;
			vramAddress  = FLIP_BIT( 10, vramAddress );
		}
	}

	return newTile;
}
