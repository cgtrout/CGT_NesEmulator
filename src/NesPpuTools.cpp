#include "Precompiled.h"
#include "NesPpuTools.h"

using namespace PpuSystem;
/* 
==============================================
void NesPPUPixelGen::genPatternTablePixelData() {  

  generates pixel data from data in both pattern tables
  TODO  performance optimization - only update this if
 		the data has changed ( bank switch dependant perhaps ? )

==============================================
*/
void NesPPUPixelGen::genPatternTablePixelData() {    
    int address, startAddress;
    ubyte value, bit1, bit2;
	ubyte byte1, byte2;
	int pixelLocation;
	NesEmulator::NesMemory *nesMemory = &FrontEnd::SystemMain::getInstance()->nesMain.nesMemory;
    
    //for every row
	for( int tiley = 0; tiley <= 0x0f; tiley++ ) {
        //for every column
        for( int tilex = 0; tilex <= 0x1F; tilex++ ) {
			if( tilex <= 0x0f ) {
				startAddress = ( tiley << 8 ) + ( tilex << 4 );
			}else {
				startAddress = 0x1000 + ( ( tiley << 8 ) + ( ( tilex - 0x10 ) << 4 ) );
			}
							
            //for every byte in tile
            for( int tilebyte = 0; tilebyte <= 0x0f; tilebyte++ ) {
                address = startAddress + tilebyte;
	
				byte1 = nesMemory->ppuMemory.fastGetMemory( address );
				byte2 = nesMemory->ppuMemory.fastGetMemory( address + 8 );

				//for every bit in byte
				for( int bit = 7; bit >= 0; bit-- ) {
					//calculate value
					bit1 = ( byte1 >> bit ) & 1;
					bit2 = (( byte2 >> bit ) & 1 ) << 1;
					value = bit1 + bit2;
								 
					//place in pixel data array           
					pixelLocation = ( tilex * 8 ) + ( tiley * 0x800 ) + ( tilebyte * 0x100 ) + ( 7 - bit );
					pixelData[ pixelLocation ] = value;
				}
            }
        }           
    }
}

/* 
==============================================
void NesPPUPixelGen::genPatternTablePixelData24Bit( Pixel3Byte *colors )

  generates 24bit pixel data ( red, green, blue )
  colors array is used to define the 4 colors used to generate the pixel data
  must send 4 colors
==============================================
*/
void NesPPUPixelGen::genPatternTablePixelData24Bit( Pixel3Byte *colors ) {
    //pixelData24bit
    int x = 0;
    int newLoc = 0;
    for( ; x < NUM_PATTERNTABLE_PIXELS; x++ ) {
        pixelData24Bit[ newLoc++ ] = colors[ pixelData[ x ] ].color[ COLOR_RED ];
		pixelData24Bit[ newLoc++ ] = colors[ pixelData[ x ] ].color[ COLOR_GREEN ];
		pixelData24Bit[ newLoc++ ] = colors[ pixelData[ x ] ].color[ COLOR_BLUE ];
    }
}

/* 
==============================================
void NesPPUPaletteGen::genPalettePixelData()
==============================================
*/
void NesPPUPaletteGen::genPalettePixelData( NesPalette *pal ) {
	palette = pal;
	buildColorTable();

	int pixel = 0;
	int x, y;
	Pixel3Byte *color;
	//one pixel at a time
	for( int d = 0 ; pixel < PixelDataSize; pixel++ ) {
		//use raw pixel location to get color location
		x = getSquareX( pixel );
		y = getSquareY( pixel );

		color = &colorTable[ x ][ y ];

		pixelData[ d++ ] = color->color[ COLOR_RED ];
		pixelData[ d++ ] = color->color[ COLOR_GREEN ];
		pixelData[ d++ ] = color->color[ COLOR_BLUE ];
	}
}

/* 
==============================================
inline int NesPPUPaletteGen::getSquareX( int pixel )
==============================================
*/
inline int NesPPUPaletteGen::getSquareX( int pixel ) {
	//find relative x position
	int relx = pixel % ( 16 * 8 );
	int squarex = relx / 8;
	return squarex;
}

/* 
==============================================
inline int NesPPUPaletteGen::getSquareY( int pixel )
==============================================
*/
inline int NesPPUPaletteGen::getSquareY( int pixel ) {
	return ( pixel > ( 128 * 8 ) );
}


/* 
==============================================
void NesPPUPaletteGen::buildColorTable( )
==============================================
*/
void NesPPUPaletteGen::buildColorTable( ) {
	NesEmulator::NesMemory *nesMemory = &FrontEnd::SystemMain::getInstance()->nesMain.nesMemory;
	for( int x = 0; x < 16; x++ ) {
		for( int y = 0; y < 2; y++ ) {
			//y = 0 -> image palette
			//y = 1 -> sprite palette
			uword memLoc = 0x3f00 + ( y * 0x10 ) + x;
			colorTable[ x ][ y ] = palette->palette[ nesMemory->ppuMemory.getMemory( memLoc	 ) ];
		}
	}
}
