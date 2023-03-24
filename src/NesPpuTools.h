#if !defined( NesPpuTools_INCLUDED )
#define NesPpuTools_INCLUDED

namespace NesEmulator {
	/*
	=====================================================
	NesPPUPixelGen class

	  Responsible for generating all of the pixel data
	  to be drawn to the screen ( for debugging purposes )
	=====================================================
	*/
	class NesPPUPixelGen {
	public:
		NesPPUPixelGen() : pixelData{ 0 }, pixelData24Bit{ 0 } {}
		void genPatternTablePixelData();
		void genPatternTablePixelData24Bit( Pixel3Byte *colors );
	    
		ubyte *getPatternTablePixelData() {return pixelData;}
		ubyte *getPatternTablePixelData24Bit() {return pixelData24Bit;}
	    
	private:
		//TODO faster to use ints?
		ubyte pixelData[ NUM_PATTERNTABLE_PIXELS ];
		ubyte pixelData24Bit[ SIZE_24BIT_PATTERNTABLE ];
	};

	//this class generates palette output for debugging purposes
	class NesPPUPaletteGen {
	public:
		NesPPUPaletteGen() :
			palette(nullptr),
			pixelData{}
		{ }
		void genPalettePixelData( NesPalette *p );

		ubyte *getPixelData() { return pixelData; }

	private:
		void buildColorTable();

		int getSquareX( int pixel );
		int getSquareY( int pixel );

		//total number of pixels in color table
		static const int PixelDataSize = ( 128 * 16 );
		
		//size of pixel data buffer
		ubyte pixelData[ PixelDataSize * 3 ];
		
		//used to hold reference values used to build
		//actual pixel buffer data
		Pixel3Byte colorTable[ 16 ][ 2 ];

		NesPalette *palette;
	};
};

#endif //NesPpuTools_INCLUDED