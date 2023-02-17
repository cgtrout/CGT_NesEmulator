#include "precompiled.h"

#include "NesSprite.h"

using namespace PpuSystem;

NesSprite::NesSprite() {

}

void NesSprite::loadSpriteInformation( ubyte *spriteRamPtr ) {
	uword currAddress = 0;
	
	ubyte *sram = spriteRamPtr;

	for( int x = 0; x < 64; x++, currAddress += 4 ) {
		sprites[ x ].y					=   sram[ currAddress ];
		sprites[ x ].tileIndex			=   sram[ currAddress + 1 ];
		sprites[ x ].verticleFlip		=   sram[ currAddress + 2 ] >> 7 ; 
		sprites[ x ].horizontalFlip		= ( sram[ currAddress + 2 ] >> 6 ) & 1;
		sprites[ x ].backGroundPriority	= ( sram[ currAddress + 2 ] >> 5 ) & 1;
		sprites[ x ].upperColorBits		=   sram[ currAddress + 2 ] & 3;
		sprites[ x ].x					=   sram[ currAddress + 3 ];
		sprites[ x ].isSprite0			= ( x == 0 );
	}

	//_log->Write("S0: x = %d, y = %d, tile = %d, upBits = %d", sprites[0].x, sprites[0].y, sprites[0].tileIndex, sprites[0].upperColorBits );
}

//scanline expected to be 0 based ( not from very start of vint )
NesSpriteScanlineResults *NesSprite::getScanlineList( int scanline, ubyte spriteSize ) {
	scanlineResults.clearList();

	//calculate y dimension
	//TODO different for pal / ntsc
	int y = scanline - 1;
	
	//what is the y size of the sprites?
	//8x8 or 8x16
	int ysize = ( 8 * ( (int)spriteSize + 1) );
	//int ysize = 8;
	int spritesFound = 0;

	//go through all sprites
	for( int i = 0; i < 64; i++ ) {
		
		//if sprite is in this scanline...
		if( ( sprites[ i ].y > ( y - ysize ) ) && ( sprites[ i ].y <= y ) ) {
			
			//add sprite to scanline results
			scanlineResults.addSprite( &sprites[ i ] );
			
			//TODO console variable option to allow more than 8 sprites
			if( ++spritesFound == 8) {
				break;
			}
		}
	}
	return &scanlineResults;
}

SpriteData *NesSprite::getSprite( ubyte spriteToGet ) {
	return &sprites[ spriteToGet ];
}

void NesSpriteScanlineResults::addSprite( PpuSystem::SpriteData *sprite ) {
	sprites.push_back( sprite );
	count++;
}

void NesSpriteScanlineResults::clearList() {
	sprites.clear();
	count = 0;
}

std::vector< SpriteData* > *NesSpriteScanlineResults::getSpriteList() {
	return &sprites;
}
