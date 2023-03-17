#if !defined( NesPpuRegisters_INCLUDED )
#define NesPpuRegisters_INCLUDED

namespace NesEmulator {
	enum {
		BGPRI_TYPE_FOREGROUND,
		BGPRI_TYPE_BACKGROUND,
		
	};
	enum colormodes {
		COLORMODEcolor,
		COLORMODE_MONOCHROME
	};

	enum colorTypes {
		COLORSELECTION_NONE  = 0x000,
		COLORSELECTION_GREEN = 0x001,
		COLORSELECTION_BLUE	 = 0x010,
		COLORSELECTION_RED	 = 0x100,
	};

	enum spriteSizes {
		SPRSIZE_8X8,
		SPRSIZE_8X16
	};

	//current status of ppu - programmer can access this
	//through register 2002
	class Status {
	  public:
		ubyte inVblank;
		bool vBlankRead;	//has vblank been read this frame?
		
		ubyte sprite0Hit;
		PpuClockCycles sprite0Time;	//time that sprite0 was hit
		
		//have we hit more than 8 sprites on this scanline?
		ubyte scanlineSriteMaxHit;

		//are writes to the VRAM respected?
		ubyte vRamWritesAllowed;

		//creates compiled byte
		ubyte makeByte( PpuClockCycles );

		void clearSprite0() { sprite0Hit = 0; }
	  private:
		
	};
	
	class Registers {
	  public:
		
		//these are the main registers used during
		//rendering
		uword vramAddress;	//main vram address
		uword tempAddress;	//temp vram address
		ubyte xOffset;		//tile x offset
		
		void incrementVramAddress();
		
		//returns true if a x offset is reset to zero
		bool incrementXOffset();
		
		//palette select value
		ubyte tileAttributeValue;

		//tracks the number of sprites we have found this scanline
		ubyte spritesThisScanline;

		//can be COLOR_RED, COLOR_GREEN, or COLOR_BLUE
		//if display type is color it will represent
		//background color, else it will represent 
		//color intensity
		ubyte colorSelection;
		
		//are sprites visible?
		ubyte spriteVisible;
		
		//is the background visible?
		ubyte backgroundVisible;
		
		//Clip sprites in left 8-pixel column?
		ubyte spriteClipping;
		
		//clip background in left 8-pixel column?
		ubyte backgroundClipping;
		
		//can be COLOR_RED, COLOR_GREEN, or COLOR_BLUE
		colormodes colorMode;

		ubyte executeNMIonVBlank;
		
		ubyte spriteSize;
		
		ubyte playfieldPatternTableSection;
		ubyte spritePatternTableAddress;
		
		ubyte addressIncrement;
	
		//fills fields based on byte representation
		void convert2000FromByte( ubyte reg );
					
		//fills fields based on byte representation
		void convert2001FromByte( ubyte reg );

		Status status;
		
		//constructor
		Registers();

		void reset();
	};
};

#endif //NesPpuRegisters_INCLUDED