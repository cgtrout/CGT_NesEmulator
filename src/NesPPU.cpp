#include "precompiled.h"

#include "Console.h"
#include "SystemMain.h"
#include "NesCpuCore.h"

using namespace NesEmulator;
using namespace FrontEnd;
using namespace Console;

int CyclesPerFrame;

TimeProfiler* timeProfiler = nullptr;

/* 
==============================================
NesPPU::NesPPU()
==============================================
*/
NesPPU::NesPPU( NesMain* nesMain ) :
	nesMain( nesMain ),
	scanlineDrawer( nesMain )
{
	
}

/* 
==============================================
NesPPU::initialize()
==============================================
*/
void NesPPU::initialize( ) {
	_log = CLog::getInstance( );
	consoleSystem = &SystemMain::getInstance( )->consoleSystem;
	timeProfiler = &SystemMain::getInstance( )->timeProfiler;

	//CyclesPerFrame = 89342;
	CyclesPerFrame = CYCLES_PER_FRAME;

	scanlineDrawer.setNesSprite( &nesSprite );
	scanlineDrawer.setRegistersPtr( &registers );
	scanlineDrawer.setVidoutBufferPtr( vidoutBuffer );
	frameType = FT_EVEN;
}

/* 
==============================================
void NesPPU::reset()
==============================================
*/
void NesPPU::reset() {
	resetCC();
	registers.reset();
	frameType = FT_EVEN;
	scanlineDrawer.reset();
}

/* 
==============================================
void NesPPU::initializeFrame()
==============================================
*/
void NesPPU::initializeFrame() {
	//clear scanline drawn history - keeps track of what scanlines have
	//been drawn this frame
	scanlineDrawer.clearScanlineDrawn();	
	scanlineDrawer.sprite0Hit = false;
}

/* 
==============================================
void NesPPU::renderBuffer( PpuClockCycles desiredcc UpdateFlag flag = UF_SpecificTime) 

  used "NTSC 2C02 technical reference" by Brad Taylor as reference
  this iBs the main function for rendering to the screen
  built for maximum readability rather than speed / efficency
  todo use separate full screen render algorithm if no
  mid ppu writes are done?
==============================================
*/
void NesPPU::renderBuffer( PpuClockCycles desiredcc, PpuUpdateType updateType ) {
	//262 scanlines total
	timeProfiler->startSection( "Ppu" );
	auto* nesMemory = &nesMain->nesMemory;

	//get sprite data
	nesSprite.loadSpriteInformation(nesMemory->spriteRam );
	
	//validation tests
	//89342 cc's a frame - should be no higher than this
	//TODO use constant value
	assert( desiredcc >= 0 && desiredcc <= 89342 );

	if( updateType == UF_Sprite0 ) {
		desiredcc = 89342;
	}
	
	//int offset = currentCC - ( scanline * 341 );
	int scanline = currentCC / CLOCKS_PER_SCANLINE;

	//do one scanline at a time
	while( currentCC < desiredcc ) {
		PpuClockCycles slStart_cc = ( scanline * CLOCKS_PER_SCANLINE );
		PpuClockCycles slEnd_cc = slStart_cc + CLOCKS_PER_SCANLINE;
	
		//calculate offset from start of current scanline
		int offset = currentCC - ( scanline * CLOCKS_PER_SCANLINE) ;

		//scanline 0..19 VINT period
		if( scanline >= 0 && scanline <= 19 ) {
			//do nothing
			if( !scanlineDrawer.getScanlineDrawn( 0 ) ) {
				//registers.status.inVblank = 0;
				registers.status.vBlankRead = false;
				//_log->Write("Clearing sprite 0");

				registers.status.clearSprite0();
				registers.status.sprite0Time = 0;	
				scanlineDrawer.setScanlineDrawn( scanline );
			}
		}

		//scanline 20
		else if( scanline == 20 ) {
			registers.status.scanlineSriteMaxHit = 0;	
		}

		//scanline 21..260 - Main drawing phase
		else if( scanline >= 21 && scanline <= 260 ) {
			//clear buffers ( if fresh scanline )
			if( !scanlineDrawer.getScanlineDrawn(scanline) ) {
				if( scanline == 21 ) {
					
					if( registers.backgroundVisible && registers.spriteVisible ) {
						//_log->Write(" vramAddress = tempAddress ");
						registers.vramAddress = registers.tempAddress;
					}
				}

				scanlineDrawer.resetPos();
				scanlineDrawer.clearBuffers();
				//scanlineDrawer.setScanlineDrawn( scanline );
				if( registers.backgroundVisible && registers.spriteVisible ) {
					registers.vramAddress &= 0xfbe0;
					registers.vramAddress += ( registers.tempAddress & 0x041f );
					//_log->Write( "vramAddress - start of scanline");
				}
			}
			//make sure desired cc does not go past end of scanline for these calc

			//calculate end cc to draw to for this scanline
			PpuClockCycles endcc = desiredcc;
			if( desiredcc >= slEnd_cc ) {
				endcc = slEnd_cc;
			}

			int pixelsToDraw = PIXELS_PER_SCANLINE - scanlineDrawer.getPos();
			int endpos = scanlineDrawer.getPos() + pixelsToDraw;						
			int orgPos = scanlineDrawer.getPos();
		
			assert( endpos < CLOCKS_PER_SCANLINE );
		
			//draw scanline
			//consoleSystem->printMessage( "offst = %d, scanline = %d, pixelsToDraw = %d, endpos = %d", offset, scanline, pixelsToDraw, endpos );
			bool condMet = scanlineDrawer.drawScanline( scanline, scanline * CLOCKS_PER_SCANLINE, endpos, updateType );
			
			if( condMet ) {
				currentCC += scanlineDrawer.getPos() - orgPos;
				desiredcc = 0;
			} else {
				currentCC += pixelsToDraw;
			}
			
			//if at the end of drawn portion of scanline, add hblank time
			if( ( currentCC - slStart_cc ) >= PIXELS_PER_SCANLINE ) {
				currentCC += CLOCKS_PER_HBLANK;
				
				//ensure end of hblank does not go beyond desired cc
				if( currentCC > desiredcc ) {
					currentCC = desiredcc;
				}
			}

			int prevsl = scanline;
			scanline = currentCC / CLOCKS_PER_SCANLINE;
			if( prevsl != scanline && this->registers.backgroundVisible ) {	//EXPERIMENTAL
				scanlineDrawer.finishScanline();
			}
			continue;
		}

		//scanline 261 - ppu does nothing
		else if( scanline == 261 ) {
					
		}
		
		//should be no way to get here...
		else {
			consoleSystem->printMessage( "NesPPU::renderBufferToPoint error: default scanline case reached." );
			//TODO shutdown game - critical error
		}
		//handle transition to next scanline ( if that point has been reached ) 
		currentCC += slEnd_cc - ( ( scanline * CLOCKS_PER_SCANLINE ) + offset );
		if( currentCC >= slEnd_cc ) {
			scanlineDrawer.setScanlineDrawn( scanline );
			++scanline;
			currentCC = scanline * CLOCKS_PER_SCANLINE;
		}
	}

	//make sure we ended up at the right spot
	//if( updateType == UF_SpecificTime ) {
	//	_ASSERT( currentCC == desiredcc );
	//}

	timeProfiler->startSection( "Cpu" );
}

/* 
==============================================
void NesPPU::checkForVINT( PpuClockCycles c )
==============================================
*/
void NesPPU::checkForVINT( PpuClockCycles c ) {
	PpuClockCycles cCycles = CyclesPerFrame;// - 1;
	//_log->Write("checking vint");
	if( frameType == FT_ODD && registers.backgroundVisible ) {
		cCycles--;
	}
	if( c >= cCycles ) {
		//create VINT flag
		nesMain->nesCpu.flagSystem->setFlag( FT_VINT, cCycles );
		
		//switch odd/even frame
		frameType += 1;
		if( frameType > FT_EVEN ) {
			frameType = FT_ODD;
		}
	}
}
