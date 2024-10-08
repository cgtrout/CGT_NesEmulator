#pragma once

#include "CGTSingleton.h"

#include "NesPalette.h"
#include "NesSprite.h"

#include "NesPpuRegisters.h"
#include "NesPpuScanline.h"

//NUMBER OF PIXELS IN A PATTERN TABLE
const auto NUM_PATTERNTABLE_PIXELS = 32768;//16384

//SIZE OF A PATTERNTABLE REPRESENTED BY 24BIT COLOR DATA
const auto SIZE_24BIT_PATTERNTABLE = 98304;//49152

//x and y screen dimensions
const auto NTSC_X = 256u;
const auto NTSC_Y = 240u;

namespace NesEmulator {
	
	/*
	=====================================================
	NesPPU class

	  Emulates the basic functionality of the 2C02 PPU chipset
	=====================================================
	*/
	class NesPPU {
	public:
		Registers registers;

		//get current clock count that the ppu is at
		PpuClockCycles getCC()	 { return currentCC; }
		
		void resetCC() { currentCC = 0; }
		
		void initializeFrame( );

		//render buffer specified pixels / clock cycles
		void renderBuffer( PpuClockCycles cc, PpuUpdateType flag = UF_SpecificTime );

		//calls a vint flag if we have passed the VINT start point
		//this allows nesMain to draw the frame to the buffer
		void checkForVINT( PpuClockCycles );

		NesPPU( NesMain* nesMain );

		void initialize( );

		void reset();

		//contains rendered output
		//at this point in the game I'm using a simple buffer to hold the vidout until we
		//are ready to draw it to the screen
		ubyte vidoutBuffer[ 256 * 256 * 3 ];		

		NesSprite nesSprite;
		NesPalette nesPalette;
		
		enum FrameType { FT_ODD, FT_EVEN };	
		ubyte getFrameType() { return frameType; }

	private:
		NesMain* nesMain;

		//current clock count of ppu
		//can also be looked at as being the current pixel
		PpuClockCycles currentCC;
		
		//odd /even frame
		ubyte frameType;
		
	  public:
		  
		ScanlineDrawer scanlineDrawer;
	};
}