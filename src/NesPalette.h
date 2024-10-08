#if !defined( NesPalette_INCLUDED )
#define NesPalette_INCLUDED

#include <string>
#include "CGTSingleton.h"
#include "Image.h"

typedef unsigned char ubyte;
#define NUM_PALETTE_ENTRYS 0x3f


using namespace CGTSingleton;

namespace NesEmulator {
	/*
	=================================================================
	=================================================================
	NesPalette Class

	=================================================================
	=================================================================
	*/

	class NesPalette {
	public:
		void loadFromFile( std::string fileName );

		//TODO extend main from exception
		class NesFileException {
		public:
			//TODO move to cpp??
			NesFileException( std::string error ) {
				std::string output;
				output = "NesFileException thrown -- ";
				output += error;
				this->error = error;
				//_log->Write( output.c_str() );
			}
			std::string error;
		};
		Pixel3Byte palette[ NUM_PALETTE_ENTRYS ];
		
		NesPalette() { loadFromFile( "pal/nes.pal" ); }

	private:		
	};
}
#endif	//!defined( NesPalette_INCLUDED )
