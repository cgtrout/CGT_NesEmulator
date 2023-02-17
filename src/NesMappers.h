#if !defined( NesMappers_INCLUDED )
#define NesMappers_INCLUDED

//#include "NesMemory.h"

namespace NesEmulator {
	//handler class for nes memory maps
	class NesMapHandler {
	  public:	
		virtual void initializeMap( ) = 0;
		virtual void reset() = 0;
	};

	class NesMapper0 : public NesMapHandler {
	  public:
		void initializeMap( );
		void reset();
	};

	//mapper 2
	class NesMapperUnRom : public NesMapHandler {
	  public:
		void initializeMap( );
		void reset();
	};
};

#endif