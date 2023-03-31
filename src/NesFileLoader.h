#pragma once

//nes file loader
#include <string>
#include <vector>

#include "CGTException.h"

typedef unsigned char ubyte;

namespace NesEmulator {
	class NesMain;

	class NesFile {
	public:
		NesFile( NesMain* nesMain );
		void initialize();
		void loadFile( std::string_view filename );
		//std::string getFileName() {return file;}

		std::vector<ubyte> *getPrgRomPages() { return &prgRomPages; }
		std::vector<ubyte> *getChrRomPages() { return &chrRomPages; }

		ubyte getNumPrgRomPages()  { return prgRomPageCount; }
		ubyte getChrRomPageCount() { return chrRomPageCount; }

		ubyte isHorizontalMirroring() { return horizontalMirroring; }
		ubyte isVerticalMirroring()	  { return verticalMirroring;   }
		
		ubyte isSramEnables()	 { return sramEnabled;	  }
		ubyte hasTrainer()		 { return trainer;		  }
		ubyte isFourScreenVRam() { return fourScreenVRam; }

		ubyte getMapperNum() {return mapperNum;}

		std::string toString();

		class NesFileException : public CgtException {
		public:
			NesFileException( std::string_view header, std::string_view message, bool showMessage = true ) :
				CgtException( header, message, showMessage)
			{}
		};
		
	private:
		NesMain* nesMain;

		ubyte prgRomPageCount = 0;			
		ubyte chrRomPageCount = 0;

		ubyte horizontalMirroring = 0;
		ubyte verticalMirroring = 0;
		ubyte sramEnabled = 0;
		ubyte trainer = 0;
		ubyte fourScreenVRam = 0;

		ubyte mapperNum = 0;

		std::vector<ubyte> prgRomPages;
		std::vector<ubyte> chrRomPages;

		//is nes 2.0 file?
		bool isNes2 = false;				

		//std::string file;
		//todo write destructor
	};
}
