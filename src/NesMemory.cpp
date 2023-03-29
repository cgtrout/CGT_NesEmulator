#include "precompiled.h"
#include <cassert>

#define CALL_MEMBER_FN( object,ptrToMember )  ( ( object ).*( ptrToMember ) )

#include "Console.h"
#include "NesMain.h"
#include "NesPPU.h"

/* 
==============================================
uword NesEmulator::calcCpuBank( uword loc )
==============================================
*/
inline uword NesEmulator::calcCpuBank( uword loc ) {
	return loc / CPU_BANKSIZE;
}

/* 
==============================================
uword NesEmulator::calcPpuBank( uword loc )
==============================================
*/
inline uword NesEmulator::calcPpuBank( uword loc ) {
	return loc / PPU_BANKSIZE;
}

/* 
==============================================
uword NesEmulator::calcCpuBankPos( uword loc, uword bank )
==============================================
*/
inline uword NesEmulator::calcCpuBankPos( uword loc, uword bank ) {
	return loc - ( bank * CPU_BANKSIZE );
}

/* 
==============================================
uword NesEmulator::calcPpuBankPos( uword loc, uword bank )
==============================================
*/
inline uword NesEmulator::calcPpuBankPos( uword loc, uword bank ) {
	return loc - ( bank * PPU_BANKSIZE );
}

using namespace NesEmulator;

/* 
==============================================
FunctionTableEntry::FunctionTableEntry( const FunctionTableEntry &e )
==============================================
*/
FunctionTableEntry::FunctionTableEntry( const FunctionTableEntry &e ) {
	this->low = e.low;
	this->high = e.high;
	this->readable = e.readable;
	this->writeable = e.writeable;
	this->read = e.read;
	this->write = e.write;
}

/* 
==============================================
FunctionTable::FunctionTable()
==============================================
*/
FunctionTable::FunctionTable() {
}

/* 
==============================================
FunctionTable::addEntry
==============================================
*/
void FunctionTable::addEntry( NesEmulator::FunctionTableEntry e ) {
	for( int i = e.low; i < e.high; ++i ) {
		entries[ i ] = e;
	}
}

/* 
==============================================
void FunctionTable::clearAllEntries()
==============================================
*/
void FunctionTable::clearAllEntries() {
	entries.clear();
}

/* 
==============================================
FunctionTableEntry *FunctionTable::getFunctionAt( uword address )
==============================================
*/
FunctionTableEntry *FunctionTable::getFunctionAt( uword address ) {
	if( address < 0x2000 ) return nullptr;

	auto it = entries.find( address );
	if( it != entries.end( ) ) {
		return &entries[ address ];
	}
	else {
		return nullptr;
	}
}

/* 
==============================================
CpuMemBanks::CpuMemBanks( int prgRomPages, const ubyte *data )
==============================================
*/
CpuMemBanks::CpuMemBanks( int prgRomPages, const std::vector<ubyte> &data ) {
	this->prgRomPages = prgRomPages;
	int numBanks = ( PRG_ROM_PAGESIZE * prgRomPages ) / CPU_BANKSIZE;

	prgRom = std::vector<CpuMemBank>( numBanks );
	copyPrgRom( prgRomPages, data );
}

/* 
==============================================
CpuMemBanks::~CpuMemBanks()
==============================================
*/
CpuMemBanks::~CpuMemBanks() {
}

/* 
==============================================
void CpuMemBanks::copyPrgRom( int numPages, const ubyte *data )
==============================================
*/
void CpuMemBanks::copyPrgRom( int numPages, const std::vector<ubyte> &data ) {
	int bank = 0;	//current bank
	int b = 0;		//data byte
	
	//for each page
	for( int page = 0; page < numPages; page++ ) {
		
		//for each bank in page
		for( int bankinpage = 0; bankinpage < PRG_BANKS_PER_PAGE; bankinpage++ ) {
			
			//for every byte in current bank
			for( int bankb = 0; bankb < CPU_BANKSIZE; ) {
				
				//copy byte from data to prg rom bank 
				prgRom[ bank ].data[ bankb++ ] = data[ b++ ];
			}
			bank++;
		}
	}
}

/* 
==============================================
PpuMemBanks::PpuMemBanks
==============================================
*/
PpuMemBanks::PpuMemBanks( int chrRomPages, const std::vector<ubyte>& data ) {
	if( chrRomPages == 0 ) {
		this->chrRomPages = 2;
	} else {
		this->chrRomPages = chrRomPages;
	}	
	int numBanks = ( CHR_ROM_PAGESIZE * this->chrRomPages ) / PPU_BANKSIZE;

	chrRom = std::vector<PpuMemBank>( numBanks );
	
	if( chrRomPages != 0 ) {
		copyChrRom( chrRomPages, data );
	}
}

/* 
==============================================
void PpuMemBanks::copyChrRom
==============================================
*/
void PpuMemBanks::copyChrRom( int numPages, const std::vector<ubyte>& data ) {
	int bank = 0;	//current bank
	int b = 0;		//data byte
	
	//number of banks per page
	static const int bankPerPage = CHR_ROM_PAGESIZE / PPU_BANKSIZE;	

	//for each page
	for( int page = 0; page < numPages; page++ ) {
		
		//for each bank in page
		for( int bankinpage = 0; bankinpage < bankPerPage; bankinpage++ ) {
			
			//for every byte in current bank
			for( int bankb = 0; bankb < PPU_BANKSIZE; ) {
				
				//copy byte from data to prg rom bank 
				chrRom[ bank ].data[ bankb++ ] = data[ b++ ];
			}
			bank++;
		}
	}
}

/* 
==============================================
PpuMemBanks::~PpuMemBanks()
==============================================
*/
PpuMemBanks::~PpuMemBanks() {
}

/*
==================================================
FunctionObject entries
==================================================
*/

/*
=================================================================
=================================================================
NesMemory
=================================================================
=================================================================
*/

/* 
==============================================
NesMemory::NesMemory()
==============================================
*/
NesMemory::NesMemory( NesMain* nesMain ) :
	nesMain( nesMain ),
	ppuMemory( nesMain ),
	memBanks( 0x10000 / CPU_BANKSIZE )
{
	
}

void NesMemory::initialize( )
{
	//set all sprite data 
	int i = 0;
	int y = 255;
	for ( ; i < 256; i += 4 ) {
		spriteRam[ i ] = y;
	}

	_20056State = LOW;	//2005/2006 flip flop state - share the same state
	spriteDmaTransferTime = 0;
	mapHandler = 0;
}

/* 
==============================================
void NesMemory::loadPrgRomPages( int prgRomPages, const ubyte *data )
==============================================
*/
void NesMemory::loadPrgRomPages( int prgRomPages, const std::vector<ubyte> &data ) {
	physicalMemBanks = CpuMemBanks( prgRomPages, data ); 
}

/* 
==============================================
void NesMemory::fillPrgBanks

  fill in banks from prg rom storage to main bank positions
==============================================
*/
void NesMemory::fillPrgBanks( uword startAddress, uword prgStartAddr, int numBanks ) { 
	_log->Write( "fillPrgBanks:   start_address=%04x   prgStartPos=%04x,  numBanks=%02x    pc=%04x ", start_address, prgStartPos, numBanks, nesMain->nesCpu.getPC()  );

	int bankNum = calcCpuBank( startAddress );
	uword prgPos = prgStartAddr;
	for( int x = 0; x < numBanks; x++ ) {
		memBanks[ bankNum++ ]= &physicalMemBanks.prgRom[ prgPos++ ];
	}
}

/*
==============================================
void NesMemory::initializeMemoryMap()
==============================================
*/
void NesMemory::initializeMemoryMap( std::unique_ptr<NesMapHandler> handler ) {
	if( mapHandler ) {
		mapHandler.reset( );
	}
	
	mapHandler = std::move( handler );
	mapHandler->setMapHandler( nesMain );
	
	int x, y;
	for( x = 0; x < 8; ) {
		memBanks[x++] = &physicalMemBanks.ramBank1;		
		memBanks[x++] = &physicalMemBanks.ramBank2;
	}
	for( x = ::calcCpuBank( 0x2000 ); x < ::calcCpuBank( 0x4000 ); ) {
		memBanks[ x++ ] = &physicalMemBanks.registers2000;
	}
	memBanks[ ::calcCpuBank( 0x4000 ) ] = &physicalMemBanks.registers4000;
	for( x = ::calcCpuBank( 0x4400 ), y = 0; y < 7; ) {
		memBanks[ x++ ] = &physicalMemBanks.expansionRom[ y++ ];
	}
	for( x = ::calcCpuBank( 0x6000 ), y = 0; y < 8; ) {
		memBanks[ x++ ] = &physicalMemBanks.saveRam[ y++ ];
	}
	
	//delete any existing function entries
	funcTable.clearAllEntries();

	//resolve for 2000 function
	auto resolveFunc2000 = []( uword address ) -> ubyte {
		address &= 0x000f;

		if( address >= 0x08 ) {
			address -= 0x08;
		}
		return (ubyte)address;
	};

	funcTable.addEntry( FunctionTableEntry(
		0x2000,	//low
		0x3fff,	//high
		// Lambda function for write operation
		[this, resolveFunc2000]( uword address, ubyte param ) {
			ubyte reg = resolveFunc2000( address );
			if( reg == 0 )		ph2000Write( param );
			else if( reg == 1 )	ph2001Write( param );
			else if( reg == 2 )	ph2002Write( param );
			else if( reg == 3 )	ph2003Write( param );
			else if( reg == 4 ) ph2004Write( param );
			else if( reg == 5 )	ph2005Write( param );
			else if( reg == 6 )	ph2006Write( param );
			else if( reg == 7 ) ph2007Write( param );
		},
		// Lambda function for read operation
		[this, resolveFunc2000]( uword address ) -> ubyte {
			ubyte reg = resolveFunc2000( address );
			ubyte retval = 0;

			if( reg == 0 )		retval = ph2000Read( );
			else if( reg == 1 )	retval = ph2001Read( );
			else if( reg == 2 )	retval = ph2002Read( );
			else if( reg == 3 )	retval = ph2003Read( );
			else if( reg == 4 ) retval = ph2004Read( );
			else if( reg == 5 )	retval = ph2005Read( );
			else if( reg == 6 )	retval = ph2006Read( );
			else if( reg == 7 ) retval = ph2007Read( );

			return retval;
		}
	));

	//resolve address down to particular register
	auto resolveFunc4000 = []( uword address ) {
		return (ubyte)( address - 0x4000 );
	};

	funcTable.addEntry( FunctionTableEntry(
		0x4000,	//low
		0x4020,	//high
		// Lambda function for write operation
		[this, resolveFunc4000]( uword address, ubyte param ) {
			
			ubyte reg = resolveFunc4000( address );

			//square channel 0
			if( reg >= 0 && reg <= 0x08 ) {
				nesMain->ApuCpuSync( );
			}

			auto* nesApu = &nesMain->nesApu;

			if( reg == 0x00 ) nesApu->square0.regWrite0( param );
			if( reg == 0x01 ) nesApu->square0.regWrite1( param );
			if( reg == 0x02 ) nesApu->square0.regWrite2( param );
			if( reg == 0x03 ) nesApu->square0.regWrite3( param );

			//square channel 1
			if( reg == 0x04 ) nesApu->square1.regWrite0( param );
			if( reg == 0x05 ) nesApu->square1.regWrite1( param );
			if( reg == 0x06 ) nesApu->square1.regWrite2( param );
			if( reg == 0x07 ) nesApu->square1.regWrite3( param );

			if( reg == 0x14 ) ph4014Write( param );
			//else if( reg == 0x4015 ) memorySys->ph4015Write( param );
			else if( reg == 0x16 ) ph4016Write( param );
			else if( reg == 0x17 ) ph4017Write( param );
		},
		// Lambda function for read operation
		[this, resolveFunc4000]( uword address ) -> ubyte {
			ubyte reg = resolveFunc4000( address );

			//if( reg == 0x14 ) return memorySys->ph4014Read();
			//else if( reg == 0x15 ) return memorySys->ph4015Read();
			if( reg == 0x16 ) return ph4016Read( );
			else if( reg == 0x17 ) return ph4017Read( );
			return 0;
		}
		) );

	ppuMemory.initializeMemoryMap();
	mapHandler->initializeMap( );
}

/*
==============================================
ubyte NesMemory::getMemory( uword loc )
==============================================
*/
ubyte NesMemory::getMemory( uword loc ) {
	//look for function in table
	FunctionTableEntry *entry = funcTable.getFunctionAt( loc );

	//if function was found
	if( entry != NULL && entry->getReadable() ) {
		//call it
		return entry->read( loc );
	} else {
		//nothing was found - do normal memory read
		return fastGetMemory( loc );	
	}
}

/* 
==============================================
ubyte NesMemory::fastGetMemory( uword loc )

  does not check for function
==============================================
*/
ubyte NesMemory::fastGetMemory( uword loc ) {
	uword bank = calcCpuBank( loc );
	uword bankpos = calcCpuBankPos( loc, bank );
	auto *memBank = memBanks[ bank ];
	auto data = memBank->data[ bankpos ];
	return data;
}

//returns a two byte value from loc (msb format)
uword NesMemory::getWord( uword loc ) {
	word ret = getMemory( loc );
	ret += getMemory( loc + 1 ) << 8;
	return ret;
}

/*
==============================================
void NesMemory::setMemory( uword loc, ubyte val )
==============================================
*/
void NesMemory::setMemory( uword loc, ubyte val ) {
	//look for function in table
	FunctionTableEntry *entry = funcTable.getFunctionAt( loc );

	//if function was found
	if( entry != nullptr && entry->getWriteable() ) {
		//call it
		entry->write( loc, val );
	} else {
		//nothing was found - do normal memory read
		fastSetMemory( loc, val );
	}
}

/* 
==============================================
void NesMemory::fastSetMemory( uword loc, ubyte val )
==============================================
*/
void NesMemory::fastSetMemory( uword loc, ubyte val ) {
	uword bank = ::calcCpuBank( loc );

	uword dataoff = ::calcCpuBankPos( loc, bank );

	assert( bank < 0x10000 / CPU_BANKSIZE );
	assert( dataoff < CPU_BANKSIZE );

	memBanks[ bank ]->data[ dataoff ] = val;
}

/* 
==============================================
void NesMemory::zeroMemory()
==============================================
*/
void NesMemory::zeroMemory() {
	for( uword x = 0; x <= 0x1000; x++ ) {
		fastSetMemory( x, 0 );
	}

	//clear sprite memory
	for( uword x = 0x6000; x < 0x8000; x++ ) {
		fastSetMemory( x, 0 );
	}

	//for( int x = 0; x < 0x10000 / CPU_BANKSIZE; x++ ) {
	//	memBanks[x] = 0;
	//}
}

/*
==================================================
Port handlers

==================================================
*/



/*
==============================================
void NesMemory::ph2000Write( ubyte val )
	control register 1
==============================================
*/
void NesMemory::ph2000Write( ubyte val ) {
	nesMain->PpuCpuSync();
	nesMain->nesPpu.registers.convert2000FromByte( val );

	//_log->Write( "2000 write val = %x", val );
}

/*
==============================================
void NesMemory::ph2000Write( ubyte val )
	control register 1
==============================================
*/
ubyte NesMemory::ph2000Read() {
	//TODO what will this do on a real nes??
	return 0;
}

/*
==============================================
void NesMemory::ph2001Write( ubyte val )
	control register 2
==============================================
*/
void NesMemory::ph2001Write( ubyte val ) {
	nesMain->PpuCpuSync();
	nesMain->nesPpu.registers.convert2001FromByte( val );
}

/*
==============================================
void NesMemory::ph2001Write( ubyte val )
	control register 2
==============================================
*/
ubyte NesMemory::ph2001Read() {
	//TODO what will this do on a real nes??
	return 0;
}

/*
==============================================
void NesMemory::ph2002Write( ubyte val )
	ppu status register
==============================================
*/
void NesMemory::ph2002Write( ubyte val ) {
	return;
}

/*
==============================================
void NesMemory::ph2002Read()
	ppu status register
==============================================
*/
ubyte NesMemory::ph2002Read()  {
	auto* nesPpu = &nesMain->nesPpu;
	auto* nesCpu = &nesMain->nesCpu;
	if( nesPpu->getCC() < nesCpu->getCC() ) {
		//if( nesPpu->registers.status.sprite0Time == 0  && nesCpu->getCC() > vblankOffTime ) {
		//	nesPpu->renderBuffer( nesCpu->getCC(), PpuSystem::UF_Sprite0 );
		//} else {
			nesPpu->renderBuffer( nesCpu->getCC() );
		//}
	}
	
	ubyte out;
	out = nesPpu->registers.status.makeByte( nesCpu->getReadTime() );
	
	//set vblank flag to zero
	nesPpu->registers.status.inVblank = 0;

	//reset 2005/2006 flipflop ptrs
	_20056State = LOW;
	//_2006State = LOW;
	
	return out;
}

/*
==============================================
ubyte NesMemory::ph2003Read()
==============================================
*/
ubyte NesMemory::ph2003Read() {
	nesMain->PpuCpuSync();
	return fastGetMemory( 0x2003 );
}

/*
==============================================
void NesMemory::ph2003Write()
==============================================
*/
void NesMemory::ph2003Write( ubyte val ) {
	nesMain->PpuCpuSync();
	fastSetMemory( 0x2003, val );
}

/*
==============================================
ubyte NesMemory::ph2004Read()
	reads from sprite memory
==============================================
*/
ubyte NesMemory::ph2004Read() {
	nesMain->PpuCpuSync();

	//normal access	
	//read memory address 
	//get 2003 access location
	ubyte accLoc = getMemory( 0x2003 );
	return spriteRam[ accLoc ];
}

/*
==============================================
void NesMemory::ph2004Write()
	writes to sprite memory
==============================================
*/
void NesMemory::ph2004Write( ubyte val ) {
	nesMain->PpuCpuSync();
	ubyte accLoc = getMemory( 0x2003 );
	
	//write to sprite memory
	spriteRam[ accLoc ] = val;

	//address should be incremented each time
	setMemory( 0x2003, ++accLoc );
}

/*
==============================================
void NesMemory::ph4014Write()

 sprite DMA register
  transfers 256 bytes of memory into spr-ram
==============================================
*/
void NesMemory::ph4014Write( ubyte val ) {
	nesMain->PpuCpuSync();

	uword address = ( 0x100 * val );
	uword i = getMemory( 0x2003 );
	
	//copy 256 bytes from address location into spr-ram
	for( int x = 0; x < 256; x++ ) {
		spriteRam[ i ] = getMemory( address + x );

		//handle wraparound
		if( ++i == 256 ) {
			i = 0; 
		}
	}

	//stall the cpu - make this instruction take 513 cycles
	//nesCpu->addOverflow( CpuToMaster( 512 ) );// + nesCpu->getWriteTime() );
	nesMain->nesCpu.addOverflow( 512 );// + nesCpu->getWriteTime() );
}

/* 
==============================================
ubyte NesMemory::ph2005Read()
  
  VRAM Address Register 1 ( scroll register )
==============================================
*/
ubyte NesMemory::ph2005Read() {
	//undefined
	return 0;
}

/* 
==============================================
void NesMemory::ph2005Write( ubyte val )
  
  VRAM Address Register 1 ( scroll register )
==============================================
*/
void NesMemory::ph2005Write( ubyte val ) {
	nesMain->PpuCpuSync();
	auto* nesPpu = &nesMain->nesPpu;
	
	uword wordval = ( uword )val;

	//write 1
	if( _20056State == LOW ) {
		nesPpu->registers.tempAddress &= 0xffe0;

		nesPpu->registers.tempAddress += wordval >> 3;
		nesPpu->registers.xOffset = ( wordval & 7 );

		_20056State = HIGH;
	}
	//write 2
	else {
		nesPpu->registers.tempAddress &= 0x8c1f; 

		nesPpu->registers.tempAddress += ( wordval & 0xf8 ) << 2;
		nesPpu->registers.tempAddress += ( wordval & 7 ) << 12;

		_20056State = LOW;
	}

	//_log->Write( "2005 write tempaddress = %x", nesPpu->registers.tempAddress );
}

/*
==============================================
void NesMemory::ph2006Write()

  this register selects the address in vram that will be accessed by
  port 2007 ( by making two writes )
==============================================
*/
void NesMemory::ph2006Write( ubyte val ) {
	static uword addrTemp;
	
	nesMain->PpuCpuSync();
	auto* nesPpu = &nesMain->nesPpu;

	//write 1
	if( _20056State == LOW ) {
		nesPpu->registers.tempAddress &= 0xc0ff;

		nesPpu->registers.tempAddress += ( val & 0x3f ) << 8;
		nesPpu->registers.tempAddress &= 0x3fff ;

		_20056State = HIGH;
	}
	//write 2
	else {
		nesPpu->registers.tempAddress &= 0xff00;

		nesPpu->registers.tempAddress += val;
		nesPpu->registers.vramAddress = nesPpu->registers.tempAddress;

		_20056State = LOW;
	}	

	//_log->Write( "2006 write tempadddress = %x", nesPpu->registers.tempAddress );
}

ubyte NesMemory::ph2006Read() {
	//do nothing
	return 0;
}

//sound memory registers

/*
==============================================
void NesMemory::ph2007Read()
==============================================
*/
ubyte NesMemory::ph2007Read() {
	nesMain->PpuCpuSync();
	auto* nesPpu = &nesMain->nesPpu;
	
	//TODO ppu
	uword address = nesPpu->registers.vramAddress;
	//_log->Write( "2007 read address = %x", address );
	nesPpu->registers.incrementVramAddress();
	
	//handle special cases first
	
	//palette reads are loaded immediately
	//TODO what if a mirorred address??
	if( address >= 0x3f00 && address < 0x4000 ) {
		//load decoded address vRamBuffer since palette lookup uses same address lines
		//as ppu
		
		vRamBuffer = ppuMemory.getMemory( address - 0x1000 );
		return ppuMemory.getMemory( address );
	}
	
	ubyte retVal = vRamBuffer;
	vRamBuffer = ppuMemory.getMemory( address );
	
	return retVal;
}

/*
==============================================
void NesMemory::ph2007Write()
==============================================
*/
void NesMemory::ph2007Write( ubyte val ) {
	nesMain->PpuCpuSync( );
	auto* nesPpu = &nesMain->nesPpu;

	//_20056State = LOW;	// experimental change
	uword address = nesPpu->registers.vramAddress;
	
	//_log->Write( "2007 write address = %x pc = %x", address, nesCpu->getPC() );
	
	if( address >= 0x3f00 && address < 0x4000 ) {
		//d7 and d6 are ignored on writes to palette
		val &= 0x3f;
	}
	
	//use PPU Registers
	ppuMemory.setMemory( address, val );

	//make sure PPU address is incremented
	nesPpu->registers.incrementVramAddress();
}

/* 
==============================================
ubyte NesMemory::ph4016Read()
  Controller 1 Read
==============================================
*/
ubyte NesMemory::ph4016Read() {
	return nesMain->controllerSystem.controller[ CONTROLLER01 ].getNextStrobe();
}

/* 
==============================================
void NesMemory::ph4016Write( ubyte val )
  Controller 1 Write
==============================================
*/
void NesMemory::ph4016Write( ubyte val ) {
	static ubyte lastWriteVal = 0;
	if( lastWriteVal == 1 ) {
		if( val == 0 ) {
			nesMain->controllerSystem.controller[ CONTROLLER01 ].resetStrobe();
			nesMain->controllerSystem.controller[ CONTROLLER02 ].resetStrobe();
		}
	}
	lastWriteVal = val;
}

/* 
==============================================
ubyte NesMemory::ph4017Read()
  Controller 2 Read
==============================================
*/
ubyte NesMemory::ph4017Read() {
	return nesMain->controllerSystem.controller[ CONTROLLER02 ].getNextStrobe();
}

/* 
==============================================
void NesMemory::ph4017Write( ubyte val )
  Controller 2 Write
==============================================
*/
void NesMemory::ph4017Write( ubyte val ) {
	//TODO handle write
	//controllerSystem->controller[ CONTROLLER02 ].write( val );
}

/*
=================================================================
=================================================================
PPUMemory
=================================================================
=================================================================
*/

PPUMemory::PPUMemory( NesMain* nesMain ) :
	nesMain( nesMain ),
	memBanks( 0x10000 / PPU_BANKSIZE )
{

}

//TODO slow - sucks up atleast 4fps
inline uword PPUMemory::resolveAddress( uword address ) {
	address &= 0x3fff;
	if( address >= 0x3000 && address < 0x3f00 ) {
		address -= 0x1000;
	}
	
	return address;
}

/* 
==============================================
void PPUMemory::loadChrRomPages( int chrRomPages, const ubyte *data )
==============================================
*/
void PPUMemory::loadChrRomPages( uword chrRomPages, const std::vector<ubyte> &data ) {
	physicalMemBanks = PpuMemBanks( chrRomPages, data ); 
}

/* 
==============================================
void PPUMemory::fillChrBanks
  
  fill in banks from prg rom storage to main bank positions
==============================================
*/
void PPUMemory::fillChrBanks( uword startAddress, uword chrStartAddr, uword numBanks ) { 
	int mainPos = calcPpuBank( startAddress );
	int chrPos = chrStartAddr;
	for( int x = 0; x < numBanks; x++ ) {
		memBanks[ mainPos++ ] = &physicalMemBanks.chrRom[ chrPos++ ];
	}
}

/*
==============================================
ubyte PPUMemory::getMemory( uword loc )
==============================================
*/
ubyte PPUMemory::getMemory( uword loc ) {
	uword address = resolveAddress( loc );
	//uword address = loc;
	if( address >= 0x3f00 ) {
		return getPaletteByte( address );
	}
	
	uword bank = ::calcPpuBank( address );
	uword offset = ::calcPpuBankPos( address, bank ) ;
	return memBanks[ bank ]->data[ offset ];
}

/* 
==============================================
ubyte *PPUMemory::getBankPtr
==============================================
*/
ubyte *PPUMemory::getBankPtr( uword bank ) {
	return memBanks[ bank ]->data.data();
}

/* 
==============================================
ubyte PPUMemory::fastGetMemory
==============================================
*/
ubyte PPUMemory::fastGetMemory( uword loc ) {
	return getMemory( loc );
}

/*
==============================================
void NesMemory::setMemory( uword loc, ubyte val )
==============================================
*/
void PPUMemory::setMemory( uword loc, ubyte val ) {	
	uword address = resolveAddress( loc );
	
	if( address >= 0x3f00 ) {
		setPaletteByte( address, val );
		return;
	}

	uword bank = ::calcPpuBank( address );
	memBanks[ bank ]->data[ ::calcPpuBankPos( address, bank ) ] = val;
}

/* 
==============================================
ubyte PPUMemory::getPaletteByte( uword address )
==============================================
*/
ubyte PPUMemory::getPaletteByte( uword address ) {
	uword newAddress = resolvePaletteAddress( address );

	if( address > 0x3f10 ) {
		return sprPalette[ newAddress & 0x000f ];
	} else {
		return bgPalette[ newAddress & 0x000f ];
	}
}

/* 
==============================================
void PPUMemory::setPaletteByte( uword address, ubyte val )
==============================================
*/
void PPUMemory::setPaletteByte( uword address, ubyte val ) {	
	uword newAddress = resolvePaletteAddress( address );
	
	if( newAddress >= 0x3f11 ) {
		sprPalette[ newAddress & 0x000f ] = val;
	} else {
		bgPalette[ newAddress & 0x000f ] = val;
	}
}

/* 
==============================================
inline uword PPUMemory::resolvePaletteAddress( uword address )
==============================================
*/
inline uword PPUMemory::resolvePaletteAddress( uword address ) {
	//if address is divisible by 4 then it will mirror to 0x3f00 / 0x3f10
	uword newAddress = address;
	if( address >= 0x3f10 && address % 4 == 0 ) {
		newAddress = address - 0x10;
	}

	return newAddress;
}

/*
==============================================
void PPUMemory::initializeMemoryMap()
==============================================
*/
void PPUMemory::initializeMemoryMap() {
	NesFile *nesFile = &nesMain->nesFile;
	int x, y;
	for( x = 0; x < 4; x++ ) {
		memBanks[ x ] = &physicalMemBanks.patternTable0[ x ];
	}
	for( x = ::calcPpuBank( 0x1000 ), y = 0; y < 4; x++, y++ ) {
		memBanks[ x ] = &physicalMemBanks.patternTable1[ y ];
	}
	if( nesFile->isFourScreenVRam() ) {
		//TODO
		throw NesMemoryException( "Not yet supported", "Four screen vram", 0 );
	}
	if( nesFile->isHorizontalMirroring() ) {
		switchHorizontalMirroring( );
	}
	else if( nesFile->isVerticalMirroring() ) {
		switchVerticalMirroring( );
	}
	//note: palettes and mirroring handled with branch logic
}

void NesEmulator::PPUMemory::switchVerticalMirroring( ) {
	memBanks[ ::calcPpuBank( 0x2000 ) ] = &physicalMemBanks.nameTable0;
	memBanks[ ::calcPpuBank( 0x2400 ) ] = &physicalMemBanks.nameTable1;
	memBanks[ ::calcPpuBank( 0x2800 ) ] = &physicalMemBanks.nameTable0;
	memBanks[ ::calcPpuBank( 0x2C00 ) ] = &physicalMemBanks.nameTable1;
}

void NesEmulator::PPUMemory::switchHorizontalMirroring( ) {
	memBanks[ ::calcPpuBank( 0x2000 ) ] = &physicalMemBanks.nameTable0;
	memBanks[ ::calcPpuBank( 0x2400 ) ] = &physicalMemBanks.nameTable0;
	memBanks[ ::calcPpuBank( 0x2800 ) ] = &physicalMemBanks.nameTable1;
	memBanks[ ::calcPpuBank( 0x2C00 ) ] = &physicalMemBanks.nameTable1;
}

/* 
==============================================
void PPUMemory::zeroMemory()
==============================================
*/
void PPUMemory::zeroMemory() {
	//for( int x = 0; x < 0x10000; x++ ) {
	//	setMemory( x, 0 );
	//}
	
	for( int x = 0; x < 0x10000 / PPU_BANKSIZE; x++ ) {
		memBanks[x] = 0;
	}
}

/* 
==============================================
void MemoryDumper::getMemoryDump
==============================================
*/
void MemoryDumper::getMemoryDump( NesMemory* nesMemory, int memType, ubyte *dest, uword address, int size ) {
	try {
		for( int i = 0; i < size; i++ ) {
			if( memType == MEMDUMPTYPE_MAIN ) {
				dest[ i ] = nesMemory->getMemory( address + i );
			} 
			else if( memType == MEMDUMPTYPE_PPU ) {
				dest[ i ] = nesMemory->ppuMemory.getMemory( address + i );
			}
		}
	}
	catch ( NesMemoryException &e ) {
		throw e;
	}
}

/* 
==============================================
std::string MemoryDumper::formatDump( ubyte buffer[ ], uword address, int size, ubyte valuesPerLine )
==============================================
*/
std::string MemoryDumper::formatDump( const ubyte* buffer, uword address, int size, ubyte valuesPerLine ) {
	std::ostringstream out;
	int mempos = 0;
	while( mempos < size ) {
		// fill first part of line ( address header )
		out << '$' << std::setfill( '0' ) << std::hex << std::setw( 4 ) << ( address + mempos ) << ": ";

		// start filling in values for one line
		for( int linepos = 0; linepos < valuesPerLine; linepos++, mempos++ ) {
			// make sure we haven't passed our size
			if( mempos == size ) {
				break;
			}
			// output memvalue
			out << std::setfill( '0' ) << std::hex << std::setw( 2 ) << static_cast<int>( buffer[ mempos ] ) << " ";
		}
		out << "\n";
	}
	return out.str( );
}
