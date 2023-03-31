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
		NesMapper1( ) {}
		void initializeMap( );
		void reset( );

		ubyte MMC1_SR = 0b10000;	//shift register
		ubyte MMC1_PB = 0;			//user programmed value (stored after 5 writes)
		ubyte write_count = 0;		//write count index of 5 writes

		//low / high area address selection
		uword prgrom_bank_address = 0x8000;	//only boots if set to this value (Tetris)
				
		ubyte prgrom_switch_size = 16;	//prg bank switching size (16/32)
		ubyte char_rom_switch_size = 4;	//chr bank switching size (4/8)

		ubyte chr_selected_bank_0000 = 0;	//reg 1 setting
		ubyte chr_selected_bank_1000 = 0;	//reg 2 setting
		ubyte prg_selected_bank = 0;		//reg 3 setting
		ubyte prg_ram_enable = 0;			//reg 3 setting

		//remap all memory based on reg settings
		void remapMemory( );
	};

	//mapper 2 - UNRom
	class NesMapper2 : public NesMapHandler {
	  public:
		void initializeMap( );
		void reset();
	};
};

#endif