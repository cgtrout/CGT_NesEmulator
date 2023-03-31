#include "Precompiled.h"
#include "SystemMain.h"

using namespace NesEmulator;
using namespace Console;
ConsoleVariable< bool > cvDrawBackground ( 
/*start val*/	true, 
/*name*/		"drawBackground", 
/*description*/	"Set to true to draw to background",
/*save?*/		 NO_SAVE_TO_FILE );

ConsoleVariable< bool > cvDrawSprites ( 
/*start val*/	true, 
/*name*/		"drawSprites", 
/*description*/	"Set to true to draw sprites",
/*save?*/		 NO_SAVE_TO_FILE );

ConsoleVariable< int > cvSpriteZeroOffset ( 
/*start val*/	0, 
/*name*/		"spriteZeroOffset", 
/*description*/	"Used to experiment with sprite 0 timing.",
/*save?*/		 NO_SAVE_TO_FILE );

ScanlineDrawer::ScanlineDrawer( NesMain* nesMain ) : 
	nesMain( nesMain )
{}

void ScanlineDrawer::initialize( ) {
	Console::ConsoleSystem* consoleSystem;
	consoleSystem = &FrontEnd::SystemMain::getInstance( )->consoleSystem;

	consoleSystem->variables.addBoolVariable( &cvDrawBackground );
	consoleSystem->variables.addBoolVariable( &cvDrawSprites );
	consoleSystem->variables.addIntVariable( &cvSpriteZeroOffset );
}

/* 
==============================================
bool ScanlineDrawer::drawScanline( int sl, PpuClockCycles currcc, int endPos, PpuUpdateType updateType ) {

  draws one scanline to the buffer
  todo left side sprite and background clipping

  returns true if update conditions have been met
==============================================
*/
bool ScanlineDrawer::drawScanline( int sl, PpuClockCycles currcc, int endPos, PpuUpdateType updateType ) {
	//change to 0 based scanline
	sl -= 21;

	//_log->Write("Drawing scanline... %d", sl);
	//_log->Write("CC time %d", currcc);
	//_log->Write("Endpos = %d", endPos);

	this->scanline = sl;
	this->endPos = endPos;

	//find objects that fall on this scanline
	scanlineTest = nesSprite->getScanlineList( scanline, registers->spriteSize );
	spriteDrawer.spriteList = scanlineTest->getSpriteList();
	
	if( scanlineTest->getCount() > 8 ) {
		registers->status.scanlineSriteMaxHit = 1;
	}

	//draw sprites and background
	spriteDrawer.drawSprites( &nesMain->nesMemory.ppuMemory, BGPRI_TYPE_BACKGROUND, registers, this );
	backgroundDrawer.drawBackground( &nesMain->nesMemory.ppuMemory, registers, this );
	spriteDrawer.drawSprites( &nesMain->nesMemory.ppuMemory, BGPRI_TYPE_FOREGROUND, registers, this );
	
	//build final color lookup table
	generateFinalPixelData();

	//find sprite 0 if not found yet
	if( !sprite0Hit  ) {
		//_log->Write("Looking for sprite 0");
		int t = detectSprite0Hit();
		if( t != -1 ) {
			registers->status.sprite0Time = t + currcc + cvSpriteZeroOffset;	
			//registers->status.sprite0Time = 0;	//EXPERIMENTAL
			sprite0Hit = true;
			//_log->Write( "Sprite 0 found @ cc: %d", registers->status.sprite0Time );
			if( updateType == UF_Sprite0 ) {
				//update conditions met - return true
				pos = t;
				return true;
			}
		}
	}

	fillVidoutBuffer( vidoutBuffer );
	pos = endPos;
	return false;
}

/* 
==============================================
void ScanlineDrawer::BackgroundDrawer::drawBackground( NesPPU::Registers *r, ScanlineDrawer *sd )

this is pretty slow, since it only outputs one pixel at a time
==============================================
*/
void BackgroundDrawer::drawBackground( PPUMemory *ppuMemory, Registers *r, ScanlineDrawer *sd ) {
	if( !r->backgroundVisible || !cvDrawBackground ) {
		return;
	}
	int endpos = sd->getEndPos();
	if( endpos > 256 ) endpos = 256;

	//bool newPtAddress = true;
	ubyte byte0 = 0;
	ubyte byte1 = 0;
	
	ubyte attbyte = 0;
	
	uword ntaddress = ( r->vramAddress & 0x0fff ) + 0x2000;
	
	//get direct ptr to bank for speed
	int bank = ::calcPpuBank( ntaddress );
	ubyte *bankptr = ppuMemory->getBankPtr( bank );

	uword oldAttAddress = 0;

	bool newPtAddress = true;

	//for every pixel
	for( int i = sd->getPos(), tilepix = 0; i < endpos; i++ ) {
		//get nametable byte ( byte of tile )
		ntaddress = ( r->vramAddress & 0x0fff ) + 0x2000;
		
		//has the bank changed?
		int bankchk = ::calcPpuBank( ntaddress );
		if( bank != bankchk ) {
			bank = bankchk;
			bankptr = ppuMemory->getBankPtr( bank );
		}

		ubyte ntbyte = bankptr[ ::calcPpuBankPos( ntaddress, bank ) ];

		//calculate address of current nametable
		uword baseNtAddress = ntaddress & 0x2c00;
		uword ntoffset = ntaddress - baseNtAddress;

		//get attribute byte
		ubyte atx = ( ntoffset & 0x1f ) / 4;
		ubyte aty = ( ntoffset & 0x0fe0 ) / 0x80;
		
		ubyte attoff =  atx + ( aty * 8 );
		uword attadd = baseNtAddress + 0x3c0 + attoff;
		
		//only load att byte if address has changed
		if( attadd != oldAttAddress ) {
			attbyte = ppuMemory->getMemory( attadd );
			oldAttAddress = attadd;
		}
		
		//get pattern table address
		uword ptaddress = r->playfieldPatternTableSection * 0x1000;
		ptaddress += 0x10 * ntbyte;						//add tile offset
		ptaddress += ( r->vramAddress & 0x7000 ) >> 12;	//add y offset

		if( newPtAddress ) {
			//get bits 1 and 0 from pattern table
			byte0 = ppuMemory->getMemory( ptaddress );
			byte1 = ppuMemory->getMemory( ptaddress + 8);
			newPtAddress = false;
		}
				
		//extract bits from bytes
		ubyte bit0  = BIT( abs( r->xOffset - 7 ), byte0 );
		ubyte bit1 = BIT( abs( r->xOffset - 7 ), byte1 );
		
		//extract 2 attribute bits
		ubyte reducedSquare = ( ntoffset & 0x73 );
		ubyte sx = ( reducedSquare & 0x0f ) / 2;
		ubyte sy = ( reducedSquare & 0xf0 ) / 0x40;
		ubyte squarenum = sx + ( sy * 2 );	
		
		ubyte attbits = ( attbyte >> ( squarenum * 2 ) ) & 0x03;

		//determine palette table lookup value
		ubyte palLookup = bit0 + ( bit1 << 1 ) + ( attbits << 2 );

		//put in buffer
		sd->backgroundBuffer[ i ] = palLookup;

		//increment x offset
		newPtAddress = r->incrementXOffset();
	}
}

/* 
==============================================
void NesPPU::ScanlineDrawer::finishScanline()

  does finishing touches on scanlihne
==============================================
*/
void ScanlineDrawer::finishScanline() {
	//increment y offset
	registers->vramAddress += 0x1000;
	//_log->Write( "finishing scanline %d (+0x1000)", scanline );
	
	//handle wrap around logic
	if( registers->vramAddress & 0x8000 ) {
		registers->vramAddress &= 0x0fff;		//clear

		//increase y scroll 
		//TODO move these to own functions to hide nasty bit manipulation
		if( ( ( registers->vramAddress & 0x03e0 ) >> 5 ) < 29 ) {
			uword oadd = registers->vramAddress;
			registers->vramAddress &= ( ~0x03e0 & 0xffff );	
			registers->vramAddress += ( ( ( oadd & 0x03e0 ) >> 5 ) + 1 ) << 5 ;		//increase
		} else {
			registers->vramAddress &= 0xfc1f;	//reset
			
			//flip bit 11
			registers->vramAddress = FLIP_BIT( 11, registers->vramAddress );
		}
	}
	registers->status.scanlineSriteMaxHit = 0;
}

/* 
==============================================
PpuClockCycles NesPPU::ScanlineDrawer::detectSprite0Hit()

  returns clockcycles offset to sprite 0 hit (if found)
  else returns -1
==============================================
*/
PpuClockCycles ScanlineDrawer::detectSprite0Hit() {
	if( spriteDrawer.isSpriteZeroOnLine() ) {
		//get position of sprite0
		SpriteData *s0 = nesSprite->getSprite( 0 );

		//find pointer to start of sprite 0 in buffer
		ubyte *spritePointer = nullptr;
		ubyte *backgroundPointer = nullptr;

		if( s0->x > endPos ) 
			return 0;

		if( s0->backGroundPriority == BGPRI_TYPE_FOREGROUND ) {
			spritePointer = &foregroundSpriteBuffer[ s0->x ];
		}else if( s0->backGroundPriority == BGPRI_TYPE_BACKGROUND ) {
			spritePointer = &backgroundSpriteBuffer[ s0->x ];
		}
		
		backgroundPointer = &backgroundBuffer[ s0->x ];
		
		//loop through pixels of sprite 0
		for( int i = 0; i < 8; i++ ) {
			//if background pixel and s0 pixel are not transparent
			if( (backgroundPointer[i] & 3) != 0 && (spritePointer[i] & 3 ) != 0 ) {
				return s0->x + i;
			}
		}
	}
	return -1;
}

/* 
==============================================
inline void NesPPU::ScanlineDrawer::generateFinalPixelData()
==============================================
*/
inline void ScanlineDrawer::generateFinalPixelData() {
	ubyte finalPixelColor;
	auto* ppuMemory = &nesMain->nesMemory.ppuMemory;

	int maxPos = endPos;
	if( maxPos > 256 ) maxPos = 256;

	//which has a higher pixel priority - background sprite or foreground sprite?
	for( int i = pos; i < maxPos; i++ ) {
		if( ( foregroundSpriteBuffer[i] & 0x03 ) != 0 ) {
			finalPixelColor = ppuMemory->fastGetMemory( 0x3f10 + foregroundSpriteBuffer[i] );
		} else if( ( backgroundBuffer[i] & 0x03 ) != 0 ) {
			finalPixelColor = ppuMemory->fastGetMemory( 0x3f00 + backgroundBuffer[i] );
		} else if( ( backgroundSpriteBuffer[i] & 0x03 ) != 0 ) {
			finalPixelColor = ppuMemory->fastGetMemory( 0x3f10 + backgroundSpriteBuffer[i] ); 
		} else {
			finalPixelColor = ppuMemory->fastGetMemory( 0x3f00 );
		}
		
		finalLookupBuf[ i ] = finalPixelColor;
	}
}

/* 
==============================================
inline void NesPPU::ScanlineDrawer::fillVidoutBuffer( ubyte *vidoutBuffer )
==============================================
*/
inline void ScanlineDrawer::fillVidoutBuffer( ubyte *vidoutBuffer ) {
	auto* pal = &nesMain->nesPpu.nesPalette;

	//copy data from scanline buffer to output buffer	
	//calculate starting offset into pixel output buffer
	int vidPos = ( scanline ) * ( PIXELS_PER_SCANLINE ) * 3;
	for( int i = 0; i < ( PIXELS_PER_SCANLINE ); ++i ) {
		Pixel3Byte *col = &pal->palette[ finalLookupBuf[i] ];
		memcpy( &vidoutBuffer[ vidPos ], col, 3 );
		vidPos += 3;
	}
}

/* 
==============================================
void NesPPU::ScanlineDrawer::clearScanlineDrawn()
==============================================
*/
void ScanlineDrawer::clearScanlineDrawn() {
	for( int i = 0; i < 262; i++ ) {
		scanlineDrawn[i] = false;
	}
}

/* 
==============================================
bool NesPPU::ScanlineDrawer::getScanlineDrawn( int scanline )
==============================================
*/
bool ScanlineDrawer::getScanlineDrawn( int scanline ) {
	return scanlineDrawn[scanline];
}

/* 
==============================================
void NesPPU::ScanlineDrawer::setScanlineDrawn( int scanline )
==============================================
*/
void ScanlineDrawer::setScanlineDrawn( int scanline ) {
	scanlineDrawn[scanline] = true;
}

/* 
==============================================
void NesPPU::ScanlineDrawer::clearBuffers()
==============================================
*/
void ScanlineDrawer::clearBuffers() {
	for( int i = 0; i < PIXELS_PER_SCANLINE; i++ ) {
		backgroundSpriteBuffer[ i ] = 0;
		backgroundBuffer[ i ] = 0;
		foregroundSpriteBuffer[ i ] = 0;
	}
}

/* 
==============================================
void NesPPU::ScanlineDrawer::SpriteDrawer::drawSprites

 parameter background pri - sets whether foreground or background pixel is drawn
 parameter registers - pointer to registers class
 
  FIXME - doesn't need to be passes scanlineDrawer all the time
==============================================
*/
void SpriteDrawer::drawSprites( 
	PPUMemory *ppuMemory,
	ubyte backgroundPri, 
	Registers *registers, 
	ScanlineDrawer *scanlineDrawer ) 
 {
	
	sprite0_onLine = false;
	
	if( scanlineDrawer->scanlineTest->getCount() == 0 ) {
		return;
	}
	
	if( !registers->spriteVisible || !cvDrawSprites ) {
		return;
	}

	//used for looping through sprite scanline test results
	SpriteData *curr;
	std::vector< SpriteData* >::iterator iter ;
	//ubyte *tilePointer;
	
	//pointer to a color offset buffer table
	ubyte *buffer;

	if( backgroundPri == BGPRI_TYPE_FOREGROUND ) {
		buffer = scanlineDrawer->foregroundSpriteBuffer;
	} else {
		buffer = scanlineDrawer->backgroundSpriteBuffer;
	}

	//go through list of sprites on current scanline
	for( iter = spriteList->begin(); iter != spriteList->end(); iter++ ) {
		curr = ( *iter );	
		
		if( curr->isSprite0 ) {
			sprite0_onLine = true;
		}

		if( curr->backGroundPriority != backgroundPri ) {
			continue;
		}

		//fill sprite buffer
		//get yoffset ( from start y dim of sprite )
		yoffset = scanlineDrawer->getCurrScanline() - curr->y - 1;

		if( curr->verticleFlip ) {
			if( registers->spriteSize == 1 ) {
				yoffset = abs( yoffset - 15 );
			} else {
				yoffset = abs( yoffset - 7 );
			}
		}

		//find address to start of tile to draw
		uword address;
		ubyte tileIndex;
		
		if( registers->spriteSize == 0 ) {
			address = ( registers->spritePatternTableAddress * 0x1000 ) + ( (curr->tileIndex ) * 0x10 );
		} else {
			ubyte largeSpriteOffset = 0;	
			tileIndex = ( curr->tileIndex & 0xfe ) ;
			if( yoffset >= 8 ) {
				largeSpriteOffset = 8;
			}
			address = ( ( curr->tileIndex & 0x01 ) * 0x1000 ) + ( tileIndex * 0x10 ) + largeSpriteOffset;
		}
		
		//bits 1 and 0 of palette entry
		bit0 = ppuMemory->getMemory( address + yoffset );
		bit1 = ppuMemory->getMemory( address + yoffset + 8);

		//fill buffer 
		int bufferPos = curr->x;	

		if( curr->horizontalFlip ) 
			for( int bitPos = 0; bitPos < 8; bitPos++ ) 
				fillBuffer( curr, buffer, bufferPos, bitPos );
		else if( !curr->horizontalFlip ) 
			for( int bitPos = 7; bitPos >= 0; bitPos-- ) 
				fillBuffer( curr, buffer, bufferPos, bitPos );
	}
}

/* 
==============================================
inline void NesPPU::ScanlineDrawer::SpriteDrawer::fillBuffer( const SpriteData *curr, ubyte *buffer, int &bufferPos, int bitPos )
==============================================
*/
inline void SpriteDrawer::fillBuffer( const SpriteData *curr, ubyte *buffer, int &bufferPos, int bitPos ) {
	//calculate palette entry ( in sprite memory )
	ubyte lower = BIT( bitPos, bit0 ) + ( ( BIT( bitPos, bit1 ) ) << 1 );
	ubyte upper = curr->upperColorBits << 2;
	if( lower != 0 ) {
		palIndex = upper + lower;	
	} else {
		palIndex = 0;
	}
	
	if( ( buffer[ bufferPos ] ) == 0 ) {
		buffer[ bufferPos ] = palIndex;
	}
	++bufferPos;
}

/* 
==============================================
void NesPPU::ScanlineDrawer::reset
==============================================
*/
void ScanlineDrawer::reset() {
	scanline = 0;
	endPos = 0;
	clearBuffers();
	clearScanlineDrawn();
	sprite0Hit = false;
}