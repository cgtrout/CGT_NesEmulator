#if !defined( NesMappers_INCLUDED )
#define NesMappers_INCLUDED

#include "typedefs.h"

namespace NesEmulator {
	class NesMain;

	//handler class for nes memory maps
	class NesMapHandler {
	public:	
		//nesMapHandler handles memory binding, so it needs access to the emulator 
		//itself (NesMain)
		void setMapHandler( NesMain* nesMain ) { this->nesMain = nesMain; }
		
		virtual void initializeMap( ) = 0;
		virtual void reset() = 0;

	protected:

		NesMain* nesMain;
	};

	//default mapper
	class NesMapper0 : public NesMapHandler {
	  public:
		void initializeMap( );
		void reset();
	};

	//mapper 1
	class NesMapper1: public NesMapHandler {
	public:
		NesMapper1( ) : MMC1_SR( 0b10000 ), MMC1_PB( 0), write_count(0) {}
		void initializeMap( );
		void reset( );

		ubyte MMC1_SR;
		ubyte MMC1_PB;
		ubyte write_count;
	};

	//mapper 2 - UNRom
	class NesMapper2 : public NesMapHandler {
	  public:
		void initializeMap( );
		void reset();
	};
};

#endif