#pragma once

#include "NesPpuRegisters.h"
#include "NesSprite.h"

namespace NesEmulator {
	static constexpr int PIXELS_PER_SCANLINE = 256; 
	static constexpr int CLOCKS_PER_SCANLINE = 341;
	static constexpr int CLOCKS_PER_HBLANK = 85;

	//this specifies how far the ppu should be updated when
	//catching it up to cpu
	enum PpuUpdateType { UF_SpecificTime, UF_Sprite0 };

	class ScanlineDrawer;

	/*
	================================================================
	================================================================
	Class
	SpriteDrawer

	  Draws sprites onto scanline
	================================================================
	================================================================
	*/
	class SpriteDrawer {
	 public:
		//draws front or back sprites
		void drawSprites( PPUMemory* ppuMemory, ubyte backgroundPri, Registers *r, ScanlineDrawer *scanlineDrawer );
		
	 private:
		//y offset from start of sprite
		int yoffset;	
		ubyte bit0, bit1;	//used to hold temporary tile data ( lower 2 bits )
		int palIndex, palEntry;
		
		//gets a pointer that points directly to the position
		//of the desired tile
		ubyte *getTilePointer( ubyte nametable, int tileIndex );

		void fillBuffer( const SpriteData *curr, ubyte *buffer, int &bufPos, int bitPos );				
		bool sprite0_onLine;

	  public:
		//list of sprites in current scanline
		std::vector< SpriteData* > *spriteList;

		//is sprite 0 on current scanline?
		bool isSpriteZeroOnLine() { return sprite0_onLine; }	
	};

	/*
	================================================================
	================================================================
	Class
	BackgroundDrawer

	  Draws background onto scanline
	================================================================
	================================================================
	*/
	class BackgroundDrawer {
	  public:
		void drawBackground( PPUMemory*, Registers*, ScanlineDrawer* );
	};

	/*
	================================================================
	================================================================
	Class
	ScanlineDrawer

	  Handles drawing of scanlines for NesMain (an emulator instance)
	================================================================
	================================================================
	*/
	class ScanlineDrawer {
	  public:
		ScanlineDrawer( NesMain *nesMain );

		void initialize( );
		  
		//draws one scanline to the buffer
		//returns true if updateType condition has been met
		bool drawScanline( int sl, PpuClockCycles currcc, int endPos, PpuUpdateType updateType );

		//does end of scanline logic
		void finishScanline();

		void setRegistersPtr( Registers *r) { registers = r; }
		void setVidoutBufferPtr( ubyte *vb ) { vidoutBuffer = vb; }
		void setNesSprite( NesSprite *s ) { nesSprite = s; }

		//stores color offsets
		ubyte backgroundSpriteBuffer[ PIXELS_PER_SCANLINE ];
		ubyte backgroundBuffer[ PIXELS_PER_SCANLINE ];
		ubyte foregroundSpriteBuffer[ PIXELS_PER_SCANLINE ];
		
		//contains the actual palette entries of each pixel
		ubyte finalLookupBuf[ PIXELS_PER_SCANLINE ];
		void clearBuffers();
		void resetPos( ) { pos = 0; }
		int getPos() { return pos; }
		int getEndPos() { return endPos; }
		int getCurrScanline() { return scanline; }

		void reset();

		//this class tests which sprites are on the current scanline
		NesSpriteScanlineResults *scanlineTest ;

	  private:

		NesMain* nesMain;
		
		PpuClockCycles detectSprite0Hit();
		  
		void generateFinalPixelData();
		void fillVidoutBuffer( ubyte *vidoutBuffer );

		void drawBackgroundColor( ubyte backgroundColor );
		
		int scanline;
		int endPos;	//pixel loc to draw to
		int pos;	//pixel position in scanline

		//increments the horizontal scroll counters and performs logic on them
		void increaseHorizontalCounters( ubyte &horizontalTile, ubyte &horizontalNameTable );
		void incrementVerticleCounters();

		NesSprite *nesSprite;
		Registers *registers;
		ubyte *vidoutBuffer;

		//has a scanline been drawn this frame
		bool scanlineDrawn[262];

	  public:	
		bool sprite0Hit;	//local copy of variable
		
		//clear scanline drawn list
		void clearScanlineDrawn();
		bool getScanlineDrawn( int scanline );
		void setScanlineDrawn( int scanline );

		SpriteDrawer spriteDrawer;
		BackgroundDrawer backgroundDrawer;
	};
};