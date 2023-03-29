#pragma once

#include "NesFileLoader.h"
#include "NesMappers.h"

#include "typedefs.h"

#include <sstream>
#include <iomanip>
#include <list>
#include <vector>
#include <array>
#include <unordered_map>
#include <functional>
#include <memory>

namespace NesEmulator {
	class NesMemory;
	class PPUMemory;

	const int CPU_BANKSIZE = 0x400;//1kb
	const int PPU_BANKSIZE = 0x400;

	const int PRG_ROM_PAGESIZE = 0x4000;
	const int CHR_ROM_PAGESIZE = 0x2000;

	const int PRG_BANKS_PER_PAGE = PRG_ROM_PAGESIZE / CPU_BANKSIZE;
	const int CHR_BANKS_PER_PAGE = CHR_ROM_PAGESIZE / PPU_BANKSIZE;

	//calculates bank # that given address is in
	uword calcCpuBank( uword loc );
	uword calcPpuBank( uword loc );

	//calculates offset into bank based on address given
	uword calcCpuBankPos( uword loc, uword bank );
	uword calcPpuBankPos( uword loc, uword bank );

	// One bank of CPU memory
	struct CpuMemBank {
		std::array<ubyte, CPU_BANKSIZE> data;
	};

	// One bank of PPU memory
	struct PpuMemBank {
		std::array<ubyte, PPU_BANKSIZE> data;
	};

	//one entry in the function table
	struct FunctionTableEntry {
		FunctionTableEntry( uword l, uword h,
			std::function<void( uword, ubyte )> writeFunc,
			std::function<ubyte( uword )> readFunc )
			: low( l ), high( h ), write( writeFunc ), read( readFunc ),
			readable( true ), writeable( true ) {}
		
		FunctionTableEntry( )
			: low( 0 ), high( 0 ), readable( false ), writeable( false ) {}

		FunctionTableEntry( const FunctionTableEntry& );
		
		// Copy assignment operator
		FunctionTableEntry& operator=( const FunctionTableEntry& other ) {
			if( this != &other ) {
				low = other.low;
				high = other.high;
				write = other.write;
				read = other.read;
				readable = other.readable;
				writeable = other.writeable;
			}
			return *this;
		}

		uword low, high;
		std::function<void( uword, ubyte )> write;		//write function
		std::function<ubyte( uword )> read;				//read function

		// Readable represents whether or not a function can be read from
		void setNonReadable( ) { readable = false; }

		// Writeable represents whether or not a function can be written to
		void setNonWriteable( ) { writeable = false; }

		bool getReadable( ) { return readable; }
		bool getWriteable( ) { return writeable; }

	private:
		bool readable;  // Can we read from this function?
		bool writeable; // Can we write to this function?
	};

	//lookup table for function handlers for memory calls
	class FunctionTable {
	  public:
		// add entry - only done once per load - don't need to worry about load
		// get automatic memory management this way 
		// unique ptr won't work here since multiple function entries can be bound to same functions
		void addEntry( FunctionTableEntry e );
		void clearAllEntries();

		//is there a function at this address?
		//returns nullptr if not found
		FunctionTableEntry *getFunctionAt( uword address );

		FunctionTable();
	  private:
		std::unordered_map< uword, FunctionTableEntry > entries;
	};

	//physical memory banks for cpu for banksize of 1k
	struct CpuMemBanks {
		CpuMemBank	ramBank1,
			ramBank2,
			registers2000,
			registers4000,	//contains some expansion ram
			expansionRom[ 7 ],
			saveRam[ 8 ];
		
		std::vector<CpuMemBank> prgRom;	
		
		CpuMemBanks( int prgRomPages, const std::vector<ubyte> &data );
		CpuMemBanks( ) {}
		~CpuMemBanks();

		//copies prg rom into membanks
		void copyPrgRom( int numPages, const std::vector<ubyte> &data );

		int prgRomPages;
	};

	//physical memory banks for ppu for banksize of 1k
	struct PpuMemBanks {
		PpuMemBank	patternTable0[4],
					patternTable1[4],
					nameTable0,
					nameTable1,
					nameTable2,
					nameTable3;
		std::vector<PpuMemBank> chrRom;

		PpuMemBanks( int chrRomPages, const std::vector<ubyte> &data );
		PpuMemBanks( ) {}
		~PpuMemBanks();

		//copies prg rom into membanks
		void copyChrRom( int numPages, const std::vector<ubyte>& data );

		int chrRomPages;
	};

	/*
	=================================================================
	=================================================================
	PPUMemory Class
	=================================================================
	=================================================================
	*/
	class PPUMemory {
		std::vector<PpuMemBank*> memBanks;
		PpuMemBanks physicalMemBanks;

		//palette data for background and sprites
		std::array<ubyte, 0x10> bgPalette;
		std::array<ubyte, 0x10> sprPalette;

		//calculates resolved address (to handle mirroring)
		uword resolveAddress( uword address );
		
		//used to handle mirroring for palettes
		uword resolvePaletteAddress( uword address );

		NesMain* nesMain;

	  public:

		PPUMemory( NesMain* nesMain );
		
		//copies data into ppuMemory physical banks
		void loadChrRomPages( uword pages, const std::vector<ubyte> &data );
		
		//initializes and assigns the memory bank
		void initializeMemoryMap();

		void switchVerticalMirroring( );

		void switchHorizontalMirroring( );

		void zeroMemory();

		//fill in prg banks at "startAddress" with data from main prgRom databank 
		//at pos "prgStartPos"
		void fillChrBanks( uword startAddress, uword chrStartAddr, uword numBanks );
		
		//memory read/write functions
		void setMemory( uword loc, ubyte val );		
		ubyte getMemory( uword loc );

		ubyte *getBankPtr( uword bank );

		void fastSetMemory( uword loc, ubyte val );		
		ubyte fastGetMemory( uword loc );

		//palette read/ write functions
		ubyte getPaletteByte( uword address );
		void setPaletteByte( uword address, ubyte val );
	};

	/*
	=================================================================
	=================================================================
	NesMemory Class
	=================================================================
	=================================================================
	*/
	class NesMemory {
	  private:
		std::vector<CpuMemBank*> memBanks;
		CpuMemBanks physicalMemBanks;
		std::unique_ptr<NesMapHandler> mapHandler;

		FunctionTable funcTable;
		
		ubyte vRamBuffer;		//used to handle buffered writes / reads from 2007
				
		PpuClockCycles spriteDmaTransferTime;

		//last memory value written to NesMemory
		ubyte lastMemValue;
		
		NesMain* nesMain;
	
	  public:
		NesMemory( NesMain *nesMain );

		void initialize( );

		int getNumPrgPages( ) { return physicalMemBanks.prgRomPages; }
		void loadPrgRomPages( int prgRomPages, const std::vector<ubyte> &data );

		//fill in prg banks at "startAddress" with data from main prgRom databank 
		//at address "prgStartAddr" for "NumBanks"
		void fillPrgBanks( uword startAddress, uword prgStartAddr, int numBanks );

		//port handlers

		//TODO way to privitize these??
		ubyte ph2000Read();		void ph2000Write( ubyte );
		ubyte ph2001Read();		void ph2001Write( ubyte );
		ubyte ph2002Read();		void ph2002Write( ubyte );
		ubyte ph2003Read();		void ph2003Write( ubyte );
		ubyte ph2004Read();		void ph2004Write( ubyte );
		
		enum FlipFlopStates { LOW, HIGH };
		FlipFlopStates _20056State;
		
		ubyte ph2005Read();		void ph2005Write( ubyte );
		ubyte ph2006Read();		void ph2006Write( ubyte );
		ubyte ph2007Read();		void ph2007Write( ubyte );
		
		//square 0
		void ph4000Write(); 	void ph4001Write();
		void ph4002Write();		void ph4003Write();

		//square 1
		void ph4004Write(); 	void ph4005Write();
		void ph4006Write();		void ph4007Write();

		void ph4014Write( ubyte );
		ubyte ph4016Read();		void ph4016Write( ubyte );
		ubyte ph4017Read();		void ph4017Write( ubyte );
		
		void initializeMemoryMap( std::unique_ptr<NesMapHandler> handler );

		NesMapHandler *getMapper() { return mapHandler.get(); }

		void addFunction( FunctionTableEntry e ) { funcTable.addEntry( e ); }

		//main memory getters / setters
		ubyte getMemory( uword loc );
		ubyte fastGetMemory( uword loc );

		//returns a two byte value from loc (msb)
		uword getWord( uword loc );
		
		void setMemory( uword loc, ubyte val );
		void fastSetMemory( uword loc, ubyte val );

		//fill all of memory with zeros
		void zeroMemory();

		//sets to zero
		void resetSpriteDmaTranferTime() { spriteDmaTransferTime = 0; }
		bool inSpriteDmaTransfer() { return spriteDmaTransferTime > 0; }
		
		PPUMemory ppuMemory;
		ubyte spriteRam[ 256 ];
	};

	//dumps an array of memory
	class MemoryDumper { 
	  public:
		enum {
			MEMDUMPTYPE_MAIN,
			MEMDUMPTYPE_PPU,
		};
		//gets memory dump from PPU or main memory and puts it in 
		//dest array given by caller
		//size must not be larger than dest array!
		void getMemoryDump( NesMemory* nesMemory, 
							int memType,
							ubyte *dest,
							uword address, 
							int size );
		
		//TODO comment
		std::string formatDump( const ubyte buffer[ ],
								uword address,
								int size,
								ubyte valuesPerLine );
	  private:
		
	};

	class NesMemoryException : public CgtException {
	public:
		NesMemoryException( std::string_view header, std::string_view m, unsigned short loc, bool s = true )
			: CgtException( header, createMessage( m, loc ), s ) {
		}

	private:
		static std::string createMessage( std::string_view m, unsigned short loc ) {
			std::stringstream ss;
			ss << m << " at " << std::setbase( 16 ) << loc;
			return ss.str( );
		}
	};
}
