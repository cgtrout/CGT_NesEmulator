#if !defined( NesFile_INCLUDED )
#define NesFile_INCLUDED

//nes file loader
#include < string >

#include "CGTException.h"

typedef unsigned char ubyte;

namespace NesEmulator {
	class NesFile {
	public:
		NesFile();
		void initialize();
		void loadFile( std::string filename );
		//std::string getFileName() {return file;}

		ubyte *getPrgRomPages() { return prgRomPages; }
		ubyte *getChrRomPages() { return chrRomPages; }

		ubyte getNumPrgRomPages()  { return prgRomPageCount; }
		ubyte getChrRomPageCount() { return chrRomPageCount; }

		ubyte isHorizontalMirroring() { return horizontalMirroring; }
		ubyte isVerticalMirroring()	  { return verticalMirroring;   }
		
		ubyte isSramEnables()	 { return sramEnabled;	  }
		ubyte hasTrainer()		 { return trainer;		  }
		ubyte isFourScreenVRam() { return fourScreenVRam; }

		ubyte getMapperNum() {return mapperNum;}

		class NesFileException : public CgtException {
		public:
			NesFileException( const char *header, const char *message, bool showMessage = true ) {
				::CgtException( header, message, showMessage);
			}
		};
		
	private:
		ubyte prgRomPageCount;	
		ubyte chrRomPageCount;

		ubyte horizontalMirroring;
		ubyte verticalMirroring;
		ubyte sramEnabled;
		ubyte trainer;
		ubyte fourScreenVRam;

		ubyte mapperNum;

		ubyte *prgRomPages;
		ubyte *chrRomPages;

		//std::string file;
		//todo write destructor
	};
}
#endif
