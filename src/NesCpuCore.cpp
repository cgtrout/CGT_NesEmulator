#include "precompiled.h"

#include "Console.h"
#include "StringToNumber.h"

#include <algorithm>
#include "NesMain.h"

using namespace NesEmulator;

using namespace FrontEnd;
using namespace Console;

ConsoleVariable< int > traceSize( 
/*start val*/	10000, 
/*name*/		"traceSize", 
/*description*/	"Number of cpu traces to record.",
/*save?*/		SAVE_TO_FILE );

ConsoleVariable< bool > traceAlreadyRun( 
/*start val*/	true, 
/*name*/		"traceAlreadyRun", 
/*description*/	"record a trace even if the trace has been at a particular PC address",
/*save?*/		SAVE_TO_FILE );
#if !defined ( LIGHT_BUILD )

/*
struct CPUTraceInstance {
	uword pc;
	ubyte opcode;
	const opcodeLookUpTableEntry *l;
	ubyte byte1, byte2;
	ubyte flags;
	Regs  reg;
	ubyte sp;
	
	CpuClockCycles cpuTime;
	PpuClockCycles ppuTime;
	
	CPUTraceInstance();

	//used for sorting
	bool operator <( CPUTraceInstance& );
};

//cpu trace history
//is processor intensive and very memory intensive to run this
//TODO seperate from this class?
class CPUTrace {
public:
	CPUTrace();
	
	//print 
	void printTrace( const char *filename );
	
	//print dissassembled view
	void printAsm( const char *filename);

	void addTrace( uword pc, const opcodeLookUpTableEntry *l, ubyte opcode, ubyte byte1, ubyte byte2, Regs reg, ubyte flags, ubyte sp, PpuClockCycles cpuTime, PpuClockCycles ppuTime );
	bool areTracing();

	void startTrace();
	void stopTrace();

	void clearTraces(); 

private:
	bool trace;
	std::vector<bool> traceWrittenAt;		
	std::vector< CPUTraceInstance > traceArray;
	int numTraces;

	void sortTraceArray();
	
}cpuTrace;
*/
#endif 

#ifndef LIGHT_BUILD
  
#endif

/*
==============================================
NesCpu()
==============================================
*/
NesCpu::NesCpu( NesMain *nesMain ) :
	nesMain( nesMain )
{
	
}

NesCpu::~NesCpu() {}

void NesCpu::initialize( ) {
	nesDebugger = &nesMain->nesDebugger;
	nesMemory = &nesMain->nesMemory;
	nesPpu = &nesMain->nesPpu;

	flagSystem = &nesMain->emulatorFlags;

	consoleSystem->variables.addIntVariable( &traceSize );
	consoleSystem->variables.addBoolVariable( &traceAlreadyRun );
	buildReadWriteTimeTables( );
}

/*
==============================================
CpuClockCycles NesCpu::runInstructions( ) 

  - run multiple instructions
==============================================
*/
void NesCpu::runInstructions( ) {	
	for( ; ; ) {
#ifndef LIGHT_BUILD
		//debugger handling
		if( nesDebugger->inSingleStepMode() ) {
			if( nesDebugger->isSingleStepPending() ) {
				runInstruction();
				if( getCC() >= nesMain->CyclesPerFrame ) {
					nesPpu->checkForVINT( getCC() );
				}
				nesDebugger->clearSingleStepRequest();
				//VBlankHandler();
				nesDebugger->setRenderPos( pc );
				nesDebugger->setSelectedAddress( pc );
				nesDebugger->selectDissasemblerLine( 0 );
			}	
			//set singlestep flag
			flagSystem->setFlag( FT_SingleStep, getCC() );
			return;
		}
		else {
			runInstruction();
		}
#else
		runInstruction();
#endif
		//if we have ran more than the number of cycles for this frame
		if( cpuTimer >= nesMain->CyclesPerFrame ) {
			//get ppu to check if we have passed the vint point
			nesPpu->checkForVINT( getCC() );
			
			//if vblank set and execute nmi on vblank set then goto interrupt
			if( flagSystem->isFlagSet( FT_VINT ) && nesPpu->registers.executeNMIonVBlank == 1 ) {
				//TODO add getWord function
				uword add = nesMemory->getMemory( 0xfffa ) + ( nesMemory->getMemory( 0xfffb ) << 8 ); 
				gotoInterrupt( add );
				//cpuTimer += CpuToMaster( 7 );
			}
		}
		
  #ifndef LIGHT_BUILD
		//check for breakpoints
		if( nesDebugger->isBreakPointAt( pc ) ) {
			if( !nesDebugger->inSingleStepMode() ) {
				nesDebugger->setToSingleStepMode( pc, "" );
			}
			else {
				nesDebugger->singleStepRequest();
			}
		}
  #endif
		//if flag is found then exit 
		if( flagSystem->flagsPresent() ) {
			return;
		}
	}
}

/*
==============================================
int NesCpu::runInstruction()

TODO performance op - maybe it would be faster to get the data as a 32 or 64 bit 
value and then parse to one byte values

==============================================
*/
void NesCpu::runInstruction() {
	oldPc = pc;	//old program counter ( for page boundary crossing calculations )
	iTime = 0;
	branched = false;
	pageBoundaryCrossed = false;
	
	currOp = nesMemory->fastGetMemory( pc );
	lookupEntry = &opcodeLookUpTable[ currOp ];
	
	b1 = nesMemory->fastGetMemory( pc+1 );
	b2 = nesMemory->fastGetMemory( pc+2 );
	fullb = ( b2 << 8 ) + b1;

	//get length of opp and operands ( if present )
	length = addModeLengthTable[ lookupEntry->mode ];		

#ifndef LIGHT_BUILD	
	//handle 'tracing' if enabled
	//if( cpuTrace.areTracing() ) {
	//	cpuTrace.addTrace( pc, lookupEntry, currOp, b1, b2, reg, createFlagByte(), sp, getCC(), ppu->getCC() );
	//}
#endif 	
	//call opcode function
	CALL_MEMBER_FN( *this, lookupEntry->fnptr )();
	
	iTime += lookupEntry->time;

	//check flags for a backtrack command - this is called when
	//a ppu register needs to loaded from memory AFTER the ppu has
	//updated itself
	//if( flagSystem->flagsPresent() ) {
		//if( flagSystem->isFlagSet( FT_Backtrack ) ) {
			//since we don't increase the pc, the instruction will be run 
			//again, obtaining the correct values
		//	restoreFlags();	//restore flags to previous state
		//	return;	
		//}
	//}

	//if branched do not alter pc
	if( !branched ) {
		pc += length;
	}
	else {
		//recalcuate page boundary
		if( pageBoundaryCrossed ) 
			iTime--;	//undo pageboundary cross
		if(lookupEntry->addCycleForPageBoundry )  // handle branch case
			if( (pc >> 8 ) != ( (oldPc + 2) >> 8 ) ) 
				iTime++;
	}

	cpuTimer += CpuToPpu( iTime );
	oldPc = pc;

	//goto system interrupt if requested, but only if i flag is not set
	if( flagSystem->isFlagSet( FT_IrqRequest ) ) {
		if( !flags.i ) {
			gotoInterrupt( nesMemory->getWord( 0xfffe ) );
			flagSystem->unsetFlag( FT_IrqRequest );
		}
	}
}

void NesCpu::buildReadWriteTimeTables() {
	buildReadTimeTable();
	buildWriteTimeTable();
}

void NesCpu::buildReadTimeTable() {
	const opcodeLookUpTableEntry *e;
	for( int i = 0; i < 0xFF; i++ ) {
		e = &opcodeLookUpTable[ i ];
		switch( e->type ) {
			case IT_BAD:
			case IT_SINGLEBYTE:
			case IT_STORE:
			case IT_BRANCH:
			case IT_MISC:
				opcodeWriteTime[i] = 0;
			case IT_INTERNALEXEC:
				opcodeWriteTime[i] = e->time - 1;	
			case IT_READMODIFYWRITE:
				switch ( e->mode ) {
					case M_ZEROPAGE:
						opcodeWriteTime[i] = 2;
						break;
					case M_ABSOLUTE:
					case M_ABSOLUTEX:					
					case M_ZEROPAGEX:
						opcodeWriteTime[i] = 3;
						break;
					default:
						break;
				}
		}
	}
}

void NesCpu::buildWriteTimeTable() {
	const opcodeLookUpTableEntry *e;
	for( int i = 0; i < 0xFF; i++ ) {
		e = &opcodeLookUpTable[ i ];
		switch( e->type ) {
			case IT_BRANCH:
			case IT_BAD:
			case IT_SINGLEBYTE:
			case IT_MISC:
			case IT_INTERNALEXEC:
				opcodeWriteTime[i] = 0;	//not applicable
				break;
			case IT_STORE:
				opcodeWriteTime[i] = e->time - 1;
				break;
			case IT_READMODIFYWRITE:
				switch ( e->mode ) {
					case M_ZEROPAGE:
					case M_ABSOLUTE:
					case M_ZEROPAGEX:
					case M_ABSOLUTEX:
						opcodeWriteTime[i] = e->time - 1;
						break;
					default:
						break; //TODO add error msg
				}
				break;
		}
	}
}

PpuClockCycles NesCpu::getWriteTime() {
	PpuClockCycles retTime = 0;
	if( pageBoundaryCrossed ) retTime++;
	retTime = cpuTimer + CpuToPpu( opcodeWriteTime[currOp] );
	return retTime;
}

PpuClockCycles NesCpu::getReadTime() {
	PpuClockCycles retTime = 0;
	if( pageBoundaryCrossed ) retTime++;
	retTime = cpuTimer + CpuToPpu( opcodeReadTime[currOp] );
	return retTime;
}

void NesCpu::addOverflow( PpuClockCycles overflow ) {
	cpuTimer += overflow;
}

/*
==============================================
ubyte NesCpu::createFlagByte()
==============================================
*/
inline ubyte NesCpu::createFlagByte() {
	ubyte fb;	//flag byte
	
	fb =  flags.c		 + 
		( flags.z << 1 ) + 
		( flags.i << 2 ) + 
		( flags.d << 3 ) + 
		( flags.b << 4 ) +
		( flags.u << 5 ) +	//unused flag always one?
		( flags.v << 6 ) + 
		( flags.n << 7 );
	return fb;
}
/*
==============================================
void NesCpu::setFlagsFromByte( ubyte fb )
==============================================
*/
inline void NesCpu::setFlagsFromByte( ubyte fb ) {
	// nvub dizc	
	flags.c =   fb        & 1 ;
	flags.z = ( fb >> 1 ) & 1 ;
	flags.i = ( fb >> 2 ) & 1 ;
	flags.d = ( fb >> 3 ) & 1 ;
	flags.b = ( fb >> 4 ) & 1 ;
	flags.u = ( fb >> 5 ) & 1 ;
	flags.v = ( fb >> 6 ) & 1 ;
	flags.n = ( fb >> 7 ) & 1 ;
}

/* 
==============================================
void NesCpu::protectFlags()
==============================================
*/
void NesCpu::protectFlags() {
	flagsBackup = flags;
}

/* 
==============================================
void NesCpu::restoreFlags()
==============================================
*/
void NesCpu::restoreFlags() {
	flags = flagsBackup;
}
/*
==============================================
void NesCpu::reset()
==============================================
*/
void NesCpu::reset() {
	nesMemory->getMapper()->reset();
	
	//get reset vector from memory
	pc = nesMemory->getMemory( 0xFFFC );
	pc += nesMemory->getMemory( 0xFFFD ) << 8;

	oldPc = pc;

	flags.b = 0; flags.c = 0; flags.d = 0; flags.i = 0;
	flags.n = 0; flags.v = 0; flags.z = 0; sp = 0xFF;
	reg.a = 0; reg.x = 0; reg.y = 0;
	cpuTimer = 0; iTime = 0;
	pageBoundaryCrossed = false;
	branched = false;
  #ifndef LIGHT_BUILD
//	cpuTrace.clearTraces();
  #endif
	flagSystem->clearFlags();
	//onStatus = false;
	nesMemory->zeroMemory();
}		             
/*
==============================================
inline void NesCpu::gotoInterrupt( int memLoc )
==============================================
*/
inline void NesCpu::gotoInterrupt( uword memLoc ) {
	nesMemory->setMemory( ( uword )sp-- + 0x100, pc >> 8 );
	nesMemory->setMemory( ( uword )sp-- + 0x100, pc & 0x00FF );
	nesMemory->setMemory( ( uword )sp-- + 0x100, createFlagByte() );
	pc = memLoc;	
	flags.i = 1;
}

/*
==============================================================================
OPCODE HANDLERS
==============================================================================
*/
void NesCpu::opBreak() {}
void NesCpu::opBAD() {
#ifndef LIGHT_BUILD
	nesDebugger->setToSingleStepMode( pc, "STOP ON BAD OPCODE" );
#endif
}	

inline ubyte NesCpu::getIMM()  { return b1;			}
inline ubyte NesCpu::getZPX()  { return b1 + reg.x;	}
inline ubyte NesCpu::getZPY()  { return b1 + reg.y;	}
inline uword NesCpu::getABS()  { return fullb;		}
inline uword NesCpu::getABSX() { return fullb + reg.x;}

inline ubyte NesCpu::getMemoryZP()	 { return nesMemory->fastGetMemory( b1		 );	}
inline ubyte NesCpu::getMemoryZPX()  { return nesMemory->fastGetMemory( getZPX() ); }
inline ubyte NesCpu::getMemoryZPY()  { return nesMemory->fastGetMemory( getZPY() ); }
inline ubyte NesCpu::getMemoryABS()  { return nesMemory->getMemory( getABS()	 ); }

//these two can page boundary cross
inline ubyte NesCpu::getMemoryABSX() { 
	if( lookupEntry->type == IT_INTERNALEXEC && lookupEntry->addCycleForPageBoundry) {
		if( (fullb >> 8) != ((fullb + reg.x) >> 8) ) {
			pageBoundaryCrossed = true;
			iTime++;
		}
	}
	return nesMemory->getMemory( fullb + reg.x ); 
}
inline ubyte NesCpu::getMemoryABSY() { 
	if( lookupEntry->type == IT_INTERNALEXEC && lookupEntry->addCycleForPageBoundry) {
		if( (fullb >> 8) != ((fullb + reg.y) >> 8) ) {
			pageBoundaryCrossed = true;
			iTime++;
		}
	}
	return nesMemory->getMemory( fullb + reg.y ); 
}

inline ubyte NesCpu::getMemoryINDX() {    
	uword addressPtr = b1 + reg.x;	
	
	//handle 'wraparound'	
	addressPtr &= 0x00ff;
	
	//get memory address to pull from
	uword memAdd = nesMemory->getMemory( addressPtr ) + ( nesMemory->getMemory( addressPtr+1 ) << 8 );
	return nesMemory->getMemory( memAdd );
}

inline ubyte NesCpu::getMemoryINDY() {
    uword addressPtr = b1;

	//get address at addressPtr 
	ubyte part1 = nesMemory->getMemory( addressPtr );
	ubyte part2 = nesMemory->getMemory( addressPtr+1 );
	uword memAdd = ( ( uword )part1 + ( ( uword )part2 << 8 ) );
	
	//check for page boundary cross
	if( lookupEntry->type == IT_INTERNALEXEC && lookupEntry->addCycleForPageBoundry ) {
		if( (memAdd >> 8) != ((memAdd + reg.y) >> 8) ) {
			pageBoundaryCrossed = true;
			iTime++;
		}
	}

	//add y reg to it
	memAdd += reg.y;

	return nesMemory->getMemory( memAdd );
}

//handles branchs
inline void NesCpu::followBranch( ) {
	pc = ( pc + 2 ) + ( signed char )b1;
	iTime++;
	branched = true;
}

/* 
==============================================
General OP algorithms
==============================================
*/

inline void NesCpu::opADC_General( ubyte memValue ) {
    int r = reg.a + memValue + flags.c;
	ubyte t = r & 0xff;
	flags.v = 0;
	flags.n = BIT( 7, t );
	if( BIT( 7, reg.a ) == BIT( 7, memValue ) ) 
		if( BIT( 7, t ) != BIT( 7, reg.a ) ) 
			flags.v = 1;
	flags.z = ( t == 0 );
	flags.c = ( r > 0xff );
	reg.a = t;
}

inline void NesCpu::opAND_General( ubyte memValue ) {
    ubyte r = memValue & reg.a;	
	flags.z = (r == 0);
	flags.n = BIT( 7, r );
	reg.a = r;
}

void NesCpu::opASL_ACC() {
	ubyte r = reg.a << 1;
	flags.c = BIT( 7, reg.a );
	flags.n = BIT( 7, r );
	flags.z = ( r == 0 );
	reg.a = r;
}

inline void NesCpu::opASL_General( short value ) {
    ubyte m = nesMemory->getMemory( value ); 
	ubyte r = m << 1;
	flags.c = BIT( 7, m );
	flags.n = BIT( 7, r );
	flags.z = ( r == 0 );   
	nesMemory->setMemory( value, r );
}

inline void NesCpu::opBIT_General( ubyte memValue ) {
    ubyte r = memValue & reg.a;
	flags.n = BIT( 7, memValue );
	flags.v = BIT( 6, memValue );
	flags.z = ( r == 0 );
}

void NesCpu::opBRK_IMP() {
	flags.b = 1; pc += 2;
	flags.i = 1;
	nesMemory->fastSetMemory( sp-- + 0x100, pc >> 8 );
	nesMemory->fastSetMemory( sp-- + 0x100, pc & 0xFF ); 
	nesMemory->fastSetMemory( sp-- + 0x100, ( createFlagByte() ) );// | 0x10 ) );  //or flag with 0x10??
	ubyte low = nesMemory->fastGetMemory( 0xFFFE );
	uword hi = nesMemory->fastGetMemory( 0xFFFF ) << 8;
	pc = hi + low;
	branched = true;
}

inline void NesCpu::opCMP_General( ubyte memValue ) {
    ubyte r = reg.a;
	r -= memValue;
	flags.z = ( r == 0 );
	flags.n = BIT( 7, r );
	if( memValue <= reg.a ) 
		flags.c = 1;
	else if( memValue > reg.a )
		flags.c = 0;
}

inline void NesCpu::opCPX_General( ubyte memValue ) {
    ubyte r = reg.x - memValue;
	flags.z = ( r == 0 );
	flags.n = BIT( 7, r );
	flags.c = ( reg.x >= memValue );
}

void NesCpu::opCPY_General( ubyte memValue ) {
    ubyte r = reg.y - memValue;
	flags.z = ( r == 0 );
	flags.n = BIT( 7, r );
	flags.c = ( reg.y >= memValue );
}


inline void NesCpu::opDEC_General( short memLocation ) {
    ubyte r = nesMemory->getMemory( memLocation ) - 1;
	nesMemory->setMemory( memLocation, r );			
	flags.n = BIT( 7, r );
	flags.z = ( r == 0 );
}

void NesCpu::opEOR_General( ubyte memValue ) {
    ubyte r = memValue ^ reg.a;
	flags.z = ( r == 0 );
	flags.n = BIT( 7, r );
	reg.a = r;
}

void NesCpu::opCLC_IMP() {	flags.c = 0;	}
void NesCpu::opSEC_IMP() {	flags.c = 1;	}
void NesCpu::opCLI_IMP() {	flags.i = 0;	}
void NesCpu::opSEI_IMP() {	flags.i = 1;	}
void NesCpu::opCLV_IMP() {	flags.v = 0;	}
void NesCpu::opCLD_IMP() {	flags.d = 0;	}
void NesCpu::opSED_IMP() {	flags.d = 1;	}

inline void NesCpu::opINC_General( short memLocation ) {
    ubyte r = nesMemory->getMemory( memLocation ) + 1;
	nesMemory->setMemory( memLocation, r );
	flags.n = BIT( 7, r );
	flags.z = ( r == 0 );
}

void NesCpu::opJMP_ABS() {
	pc = fullb;
	branched = true;
}

void NesCpu::opJMP_IND() {
	ubyte part1 = nesMemory->getMemory( fullb );
	
	int part2val = fullb + 1;
	
	//handle wraparound logic
	if( ( ( part2val ) & 0x00ff ) == 0 ) {
		part2val -= 0x0100;	
	}
	
	ubyte part2 = nesMemory->getMemory( part2val );
	
	pc = ( uword )( part1 ) + ( uword )( part2 << 8 );
	branched = true;
}

void NesCpu::opJSR_ABS() {
	nesMemory->setMemory( ( uword )sp-- + 0x100, ( ( pc + 2 ) & 0xFF00 ) >> 8 );
	nesMemory->setMemory( ( uword )sp-- + 0x100, ( pc + 2 ) & 0x00FF );
	pc = fullb;
	branched = true;
}

inline void NesCpu::opLDA_General( ubyte memValue ) {
	ubyte r = memValue;	
	flags.z = ( r == 0 );
	flags.n = BIT( 7, r );	
	reg.a = r;
}

inline void NesCpu::opLDX_General( ubyte memValue ) {
	reg.x = memValue;
    flags.n = BIT( 7, reg.x );
	flags.z = ( reg.x == 0 );
}

inline void NesCpu::opLDY_General( ubyte memValue ) {
	reg.y = memValue;
    flags.n = BIT( 7, reg.y );
	flags.z = ( reg.y == 0 );
}

void NesCpu::opLSR_ACC() {
	flags.c = reg.a & 0x01;
	ubyte r = reg.a >> 1;
	flags.n = 0;
	flags.z = ( r == 0 );
	reg.a = r;
}

inline void NesCpu::opLSR_General( short memLocation ) {
	ubyte m = nesMemory->getMemory( memLocation ); 
	flags.c = m & 0x01;
	ubyte r = m >> 1;
	nesMemory->setMemory( memLocation, r );
	flags.n = 0;
	flags.z = ( r == 0 );
}

void NesCpu::opNOP_IMP() {}

inline void NesCpu::opORA_General( ubyte memValue ) {
	ubyte r = memValue | reg.a;
	flags.z = ( r == 0 );
	flags.n = BIT( 7, r );
	reg.a = r;
}

void NesCpu::opTAX_IMP() {
	reg.x = reg.a;
	flags.z = ( reg.a == 0 );
	flags.n = BIT( 7, reg.x );
}

void NesCpu::opTXA_IMP() {
	reg.a = reg.x;
	flags.z = ( reg.a == 0 );
	flags.n = BIT( 7, reg.a );
}

void NesCpu::opDEX_IMP() {
	reg.x--;
	flags.z = ( reg.x == 0 );
	flags.n = BIT( 7, reg.x );
}

void NesCpu::opINX_IMP() {
	reg.x++;
	flags.z = ( reg.x == 0 );
	flags.n = BIT( 7, reg.x );
}

void NesCpu::opTAY_IMP() {
	reg.y = reg.a;
	flags.z = ( reg.y == 0 );
	flags.n = BIT( 7, reg.y );
}

void NesCpu::opTYA_IMP() {
	reg.a = reg.y;
	flags.z = ( reg.a == 0 );
	flags.n = BIT( 7, reg.a );
}

void NesCpu::opDEY_IMP() {
	reg.y--;
	flags.z = ( reg.y == 0 );
	flags.n = BIT( 7, reg.y );
}

void NesCpu::opINY_IMP() {
	reg.y++;
	flags.z = ( reg.y == 0 );
	flags.n = BIT( 7, reg.y );
}

void NesCpu::opROL_ACC() {
	ubyte r = reg.a << 1;
	r += flags.c;	
	flags.c = BIT( 7, reg.a );
	flags.n = BIT( 6, reg.a );
	flags.z = ( r == 0 );
	reg.a = r;
}

inline void NesCpu::opROL_General( short memLocation ) {
	ubyte m = nesMemory->getMemory( memLocation ); 
	ubyte r = m << 1;
	r += flags.c;
	flags.c = BIT( 7, m );
	flags.n = BIT( 6, m );
	flags.z = ( r == 0 );
	nesMemory->setMemory( memLocation, r );
}

void NesCpu::opROR_ACC() {
	ubyte r = reg.a >> 1;
	r += flags.c << 7;
	flags.n = flags.c;
	flags.c = BIT( 0, reg.a );
	flags.z = ( r == 0 );
	reg.a = r;
}

inline void NesCpu::opROR_General( short memLocation ) {
	ubyte m = nesMemory->getMemory( memLocation ); 
	ubyte r = m >> 1;
	r += flags.c << 7;
	flags.n = flags.c;
	flags.c = BIT( 0, m );
	flags.z = ( r == 0 );
	nesMemory->setMemory( memLocation, r );
}

void NesCpu::opRTI_IMP() {	
	setFlagsFromByte( nesMemory->getMemory( ( uword )++sp + 0x100 ) );
	pc = nesMemory->getMemory( ( uword )++sp + 0x100 );
	pc += nesMemory->getMemory( ( uword )++sp + 0x100 ) << 8;
	//flags.i = 0;  //should be set since flags were restored
	branched = true;
}

void NesCpu::opRTS_IMP() {
	pc =  nesMemory->getMemory( ( uword )++sp + 0x100 );
	pc += nesMemory->getMemory( ( uword )++sp + 0x100 ) << 8;
	pc++;
	branched = true;
}	
	

inline void NesCpu::opSBC_General( ubyte memory ) {
	opADC_General( ~memory );
}
    		
void NesCpu::opSTA_ZP()	  {	nesMemory->setMemory( b1, reg.a		   ); }
void NesCpu::opSTA_ZPX()  {	nesMemory->setMemory( getZPX(), reg.a  ); }
void NesCpu::opSTA_ABS()  {	nesMemory->setMemory( getABS(), reg.a  ); }
void NesCpu::opSTA_ABSX() {	nesMemory->setMemory( getABSX(), reg.a ); }

void NesCpu::opSTA_ABSY() { 
	nesMemory->setMemory( fullb + reg.y, reg.a ); 
}

void NesCpu::opSTA_INDX() {	
	uword part1 = ( nesMemory->getMemory( ( b1+reg.x ) & 0x00ff  ) );
	uword part2 = ( nesMemory->getMemory( ( b1+reg.x+1 ) & 0x00ff ) << 8 );

	uword address = part1 + part2;


	nesMemory->setMemory( address, reg.a );
}
void NesCpu::opSTA_INDY() {	nesMemory->setMemory( ( nesMemory->getMemory( b1 ) + ( nesMemory->getMemory( b1 + 1 ) << 8 ) ) + reg.y, reg.a );}

void NesCpu::opSTX_ZP()  {	nesMemory->setMemory( getIMM(), reg.x ); }
void NesCpu::opSTX_ZPY() {	nesMemory->setMemory( getZPY(), reg.x ); }
void NesCpu::opSTX_ABS() {	nesMemory->setMemory( getABS(), reg.x ); }

void NesCpu::opSTY_ABS() {	nesMemory->setMemory( getABS(), reg.y ); }
void NesCpu::opSTY_ZP()  {	nesMemory->setMemory( getIMM(), reg.y ); }

void NesCpu::opSTY_ZPX() {	
	uword loc = getZPX();
	nesMemory->setMemory( loc, reg.y ); 
}

void NesCpu::opTXS_IMP() { sp = reg.x; }

void NesCpu::opTSX_IMP() { 
	reg.x = sp; 
	flags.z = ( reg.x == 0 );
	flags.n = BIT( 7, reg.x );
}
		
void NesCpu::opPHA_IMP() { nesMemory->setMemory( ((uword)sp-- + 0x100), reg.a );	}

void NesCpu::opPLA_IMP() { 
	reg.a = nesMemory->getMemory( (uword)++sp + 0x100 );
	flags.z = ( reg.a == 0 );
	flags.n = BIT( 7, reg.a );
}

void NesCpu::opPHP_IMP() { nesMemory->setMemory( (uword)(sp--) + 0x100, createFlagByte()   ); }
void NesCpu::opPLP_IMP() { setFlagsFromByte( nesMemory->getMemory( ( uword )++sp + 0x100 ) ); }


/*
===================================================================
NEW undocumented 
https://www.pagetable.com/c64ref/6502/?tab=2#RLA
===================================================================
*/

void NesCpu::opRRA_General( short memLocation ) {
	ubyte m = nesMemory->getMemory( memLocation );
	ubyte oldCarry = flags.c;
	flags.c = m & 0x01;
	ubyte r = (oldCarry << 7) + (m >> 1);
	ubyte oldSign = BIT( 7, reg.a );
	
	// Cast to uint16_t to avoid wrap around
	uint16_t carryCheck = static_cast<uint16_t>( reg.a ) + static_cast<uint16_t>( r ) + flags.c; 
	
	reg.a += r + flags.c;
	flags.v = BIT( 7, reg.a ) != oldSign;

	flags.n = BIT( 7, reg.a );
	flags.z = ( reg.a == 0 );
}

/* 
==============================================
specific op handlers
==============================================
*/
void NesCpu::opADC_IMM()  {	opADC_General( getIMM()		   ); }
void NesCpu::opADC_ZP()   {	opADC_General( getMemoryZP()   ); }
void NesCpu::opADC_ZPX()  {	opADC_General( getMemoryZPX()  ); }
void NesCpu::opADC_ABS()  {	opADC_General( getMemoryABS()  ); }
void NesCpu::opADC_ABSX() {	opADC_General( getMemoryABSX() ); }
void NesCpu::opADC_ABSY() {	opADC_General( getMemoryABSY() ); }
void NesCpu::opADC_INDX() {	opADC_General( getMemoryINDX() ); }
void NesCpu::opADC_INDY() {	opADC_General( getMemoryINDY() ); }

void NesCpu::opAND_IMM()  {	opAND_General( getIMM()			); }
void NesCpu::opAND_ZP()   {	opAND_General( getMemoryZP()	); }
void NesCpu::opAND_ZPX()  {	opAND_General( getMemoryZPX()	); }
void NesCpu::opAND_ABS()  {	opAND_General( getMemoryABS()	); }
void NesCpu::opAND_ABSX() {	opAND_General( getMemoryABSX()	); }
void NesCpu::opAND_ABSY() {	opAND_General( getMemoryABSY()	); }
void NesCpu::opAND_INDX() {	opAND_General( getMemoryINDX()	); }
void NesCpu::opAND_INDY() {	opAND_General( getMemoryINDY()	); }

void NesCpu::opASL_ZP()   {	opASL_General( getIMM()  ); }
void NesCpu::opASL_ZPX()  {	opASL_General( getZPX()  ); }
void NesCpu::opASL_ABS()  {	opASL_General( getABS()  ); }
void NesCpu::opASL_ABSX() {	opASL_General( getABSX() ); }

void NesCpu::opBIT_ABS() { opBIT_General( getMemoryABS() ); }
void NesCpu::opBIT_ZP()  { opBIT_General( getMemoryZP()  ); }

void NesCpu::opBPL_REL() { if( flags.n == 0 ) followBranch(); }
void NesCpu::opBMI_REL() { if( flags.n == 1 ) followBranch(); }
void NesCpu::opBVC_REL() { if( flags.v == 0 ) followBranch(); }
void NesCpu::opBVS_REL() { if( flags.v == 1 ) followBranch(); }
void NesCpu::opBCC_REL() { if( flags.c == 0 ) followBranch(); }
void NesCpu::opBCS_REL() { if( flags.c == 1 ) followBranch(); }
void NesCpu::opBNE_REL() { if( flags.z == 0 ) followBranch(); }
void NesCpu::opBEQ_REL() { if( flags.z == 1 ) followBranch(); }

void NesCpu::opCMP_IMM()  {	opCMP_General( getIMM()		   ); }
void NesCpu::opCMP_ZP()   {	opCMP_General( getMemoryZP()   ); }
void NesCpu::opCMP_ZPX()  { opCMP_General( getMemoryZPX()  ); }
void NesCpu::opCMP_ABS()  { opCMP_General( getMemoryABS()  ); }
void NesCpu::opCMP_ABSX() { opCMP_General( getMemoryABSX() ); }
void NesCpu::opCMP_ABSY() { opCMP_General( getMemoryABSY() ); }
void NesCpu::opCMP_INDX() { opCMP_General( getMemoryINDX() ); }
void NesCpu::opCMP_INDY() { opCMP_General( getMemoryINDY() ); }

void NesCpu::opCPX_IMM() { opCPX_General( getIMM()		  ); }
void NesCpu::opCPX_ZP()  { opCPX_General( getMemoryZP()  ); }
void NesCpu::opCPX_ABS() { opCPX_General( getMemoryABS() ); }

void NesCpu::opCPY_IMM() { opCPY_General( getIMM()		 ); }
void NesCpu::opCPY_ZP()  { opCPY_General( getMemoryZP()  ); }
void NesCpu::opCPY_ABS() { opCPY_General( getMemoryABS() ); }

void NesCpu::opDEC_ZP()	 { opDEC_General( getIMM()	); }
void NesCpu::opDEC_ZPX() { opDEC_General( getZPX()	); }
void NesCpu::opDEC_ABS() { opDEC_General( getABS()	); }
void NesCpu::opDEC_ABSX(){ opDEC_General( getABSX()	); }

void NesCpu::opEOR_IMM()  {	opEOR_General( getIMM() )		;}
void NesCpu::opEOR_ZP()	  {	opEOR_General( getMemoryZP() )	;}
void NesCpu::opEOR_ZPX()  {	opEOR_General( getMemoryZPX() )	;}
void NesCpu::opEOR_ABS()  {	opEOR_General( getMemoryABS() )	;}
void NesCpu::opEOR_ABSX() {	opEOR_General( getMemoryABSX() );}
void NesCpu::opEOR_ABSY() {	opEOR_General( getMemoryABSY() );}
void NesCpu::opEOR_INDX() {	opEOR_General( getMemoryINDX() );}
void NesCpu::opEOR_INDY() {	opEOR_General( getMemoryINDY() );}

void NesCpu::opINC_ZP()   {	opINC_General( getIMM()  ); }
void NesCpu::opINC_ZPX()  {	opINC_General( getZPX()  ); }
void NesCpu::opINC_ABS()  {	opINC_General( getABS()  ); }
void NesCpu::opINC_ABSX() {	opINC_General( getABSX() ); }

void NesCpu::opLDA_IMM()  {	opLDA_General( getIMM()			); }
void NesCpu::opLDA_ZP()   { opLDA_General( getMemoryZP()	); }
void NesCpu::opLDA_ZPX()  {	opLDA_General( getMemoryZPX()	); }
void NesCpu::opLDA_ABS()  {	opLDA_General( getMemoryABS()	); }
void NesCpu::opLDA_ABSX() {	opLDA_General( getMemoryABSX()	); }
void NesCpu::opLDA_ABSY() {	opLDA_General( getMemoryABSY()	); }
void NesCpu::opLDA_INDX() {	opLDA_General( getMemoryINDX()	); }
void NesCpu::opLDA_INDY() {	opLDA_General( getMemoryINDY()	); }

void NesCpu::opLDX_IMM()  { opLDX_General( getIMM() );			}
void NesCpu::opLDX_ZP()	  { opLDX_General( getMemoryZP() );		}
void NesCpu::opLDX_ZPY()  { opLDX_General( getMemoryZPY() );	}
void NesCpu::opLDX_ABS()  { opLDX_General( getMemoryABS() );	}
void NesCpu::opLDX_ABSY() { opLDX_General( getMemoryABSY() );	}

void NesCpu::opLDY_IMM()  {	opLDY_General( getIMM()		   ); }
void NesCpu::opLDY_ZP()   {	opLDY_General( getMemoryZP()   ); }
void NesCpu::opLDY_ZPX()  {	opLDY_General( getMemoryZPX()  ); }
void NesCpu::opLDY_ABS()  {	opLDY_General( getMemoryABS()  ); }
void NesCpu::opLDY_ABSX() {	opLDY_General( getMemoryABSX() ); }
		
void NesCpu::opLSR_ZP()   {	opLSR_General( getIMM()  ); }
void NesCpu::opLSR_ZPX()  {	opLSR_General( getZPX()  ); }
void NesCpu::opLSR_ABS()  {	opLSR_General( getABS()  ); }
void NesCpu::opLSR_ABSX() {	opLSR_General( getABSX() ); }

void NesCpu::opORA_IMM()  {	opORA_General( getIMM()		   ); }
void NesCpu::opORA_ZP()   {	opORA_General( getMemoryZP()   ); }
void NesCpu::opORA_ZPX()  {	opORA_General( getMemoryZPX()  ); }
void NesCpu::opORA_ABS()  { opORA_General( getMemoryABS()  ); }
void NesCpu::opORA_ABSX() { opORA_General( getMemoryABSX()  ); }
void NesCpu::opORA_ABSY() {	opORA_General( getMemoryABSY() ); }
void NesCpu::opORA_INDX() {	opORA_General( getMemoryINDX() ); }
void NesCpu::opORA_INDY() {	opORA_General( getMemoryINDY() ); }

void NesCpu::opROL_ZP()   {	opROL_General( getIMM() );  }
void NesCpu::opROL_ZPX()  {	opROL_General( getZPX() );  }
void NesCpu::opROL_ABS()  {	opROL_General( getABS() );  }
void NesCpu::opROL_ABSX() {	opROL_General( getABSX() ); }

void NesCpu::opROR_ZP()   {	opROR_General( getIMM()  ); }
void NesCpu::opROR_ZPX()  { opROR_General( getZPX()  ); }
void NesCpu::opROR_ABS()  { opROR_General( getZPX()  );}
void NesCpu::opROR_ABSX() { opROR_General( getABSX() );}

void NesCpu::opSBC_IMM()  {	opSBC_General( getIMM()			); }
void NesCpu::opSBC_ZP()   {	opSBC_General( getMemoryZP()	); }
void NesCpu::opSBC_ZPX()  {	opSBC_General( getMemoryZPX()	); }
void NesCpu::opSBC_ABS()  {	opSBC_General( getMemoryABS()	); }
void NesCpu::opSBC_ABSX() {	opSBC_General( getMemoryABSX()	); }
void NesCpu::opSBC_ABSY() {	opSBC_General( getMemoryABSY()	); }
void NesCpu::opSBC_INDX() {	opSBC_General( getMemoryINDX()	); }
void NesCpu::opSBC_INDY() {	opSBC_General( getMemoryINDY()	); }

//NEW RRA undocumented func
void NesCpu::opRRA_ZP( )   { opRRA_General( getMemoryZP( ) ); }
void NesCpu::opRRA_ZPX( )  { opRRA_General( getMemoryZPX( ) ); }
void NesCpu::opRRA_ABS( )  { opRRA_General( getMemoryABS( ) ); }
void NesCpu::opRRA_ABSX( ) { opRRA_General( getMemoryABSX( ) ); }
void NesCpu::opRRA_ABSY( ) { opRRA_General( getMemoryABSY( ) ); }
void NesCpu::opRRA_INDX( ) { opRRA_General( getMemoryINDX( ) ); }
void NesCpu::opRRA_INDY( ) { opRRA_General( getMemoryINDY( ) ); }


#ifndef LIGHT_BUILD

/*
================================================================
================================================================
Class CPUTrace
================================================================
================================================================
*/
/*
CPUTraceInstance::CPUTraceInstance() { 
	pc = 0;
	flags = 0;
	reg.a = 0; reg.x = 0; reg.y = 0, sp = 0; ppuTime = 0; cpuTime = 0;
}

bool CPUTraceInstance::operator <( CPUTraceInstance &that) {
	return this->pc < that.pc;
}

CPUTrace::CPUTrace(): 
	traceWrittenAt( 0xffff) {
	numTraces = 0;
	clearTraces();
}

void CPUTrace::printTrace( const char *filename ) {
	std::string		fn;
	int				scanline;
	PpuClockCycles	cycleOffset;	//from start of scanline

	if( strlen( filename ) == 0 ) {
		fn = "tracelog.txt";
	}
	else {
		fn = filename;
	}
	//TODO use streams
	consoleSystem->printMessage( "Printing CPU tracelog to %s. traceSize = %d", fn.c_str(), traceSize.getValue() );
	
	ofstream f( fn.c_str() );

	//char regLine[ 80 ];
	
	int i = 0;
	int tracesToRun;// = traceSize;

	//if( numTraces < traceSize ) {
		tracesToRun = numTraces;
	//}

	for( ; i < tracesToRun; i++ ) {
		const CPUTraceInstance *t = &traceArray[ i ];
		ubyte b1 = t->byte1;
		ubyte b2 = t->byte2;

		char  *lt = nesDebugger->buildDebugLine( t->pc, t->l, t->opcode, b1, b2 );
		
		//erase delim char
		int length = strlen( lt );
		lt[ length - 1 ] = '\0';
		
		//eliminate first char
		lt = &lt[1];
		
		//lines.flush();

		stringstream lines;

		lines.setf( ios_base::left, ios_base::adjustfield );
		lines.width( 40 );
		lines << lt;
		lines.seekp( 0, std::ios::end );

		lines << "A:" << ubyteToString( t->reg.a ) << " "
		 	  << "X:" << ubyteToString( t->reg.x ) << " "
		 	  << "Y:" << ubyteToString( t->reg.y ) << " "
			  << "P:";
		
		//write out flags
		//nvub dizc
		if( BIT(7, t->flags) ) lines << "N"; else lines << "n";
		if( BIT(6, t->flags) ) lines << "V"; else lines << "v";
		if( BIT(5, t->flags) ) lines << "U"; else lines << "u";
		if( BIT(4, t->flags) ) lines << "B"; else lines << "b";
		if( BIT(3, t->flags) ) lines << "D"; else lines << "d";
		if( BIT(2, t->flags) ) lines << "I"; else lines << "i";
		if( BIT(1, t->flags) ) lines << "Z"; else lines << "z";
		if( BIT(0, t->flags) ) lines << "C"; else lines << "c";
		
		//lines << ubyteToString( t->flags );

		lines << " SP: " << ubyteToString( t->sp );
		
		//figure out scanline and cycle
		scanline = t->cpuTime / 341;
		cycleOffset = t->cpuTime - ( scanline * 341 );

		//fill in cycles and scanline counters
		lines << " CYC: ";
		lines.setf( ios_base::right, ios_base::adjustfield );
		lines.width( 3 );
		lines << cycleOffset;
		
		lines << " SL: ";
		lines.setf( ios_base::right, ios_base::adjustfield );
		lines.width( 3 );
		lines << scanline;
		
		lines << " CPU: ";
		lines.setf( ios_base::right, ios_base::adjustfield );
		lines.width( 5 );
		lines << t->cpuTime;

		lines << " PPU: ";
		lines.setf( ios_base::right, ios_base::adjustfield );
		lines.width( 5 );
		lines << t->ppuTime 
			  << "\n";
		
		f << lines.str();
	}
	
	f.close();
	consoleSystem->printMessage( "Done printing tracelog" );
}


void CPUTrace::printAsm( const char *filename ) {
	std::string		fn;
	
	sortTraceArray();

	if( strlen( filename ) == 0 ) {
		fn = "asm_out.txt";
	}
	else {
		fn = filename;
	}
	ofstream f( fn.c_str() );
	
	for( unsigned int i = 0; i < traceArray.size(); i++ ) {
		CPUTraceInstance *t = &traceArray[ i ];
		
		char  *lt = nesDebugger->buildDebugLine( t->pc, t->l, t->opcode, t->byte1, t->byte2 );
		
		//erase delim char
		int length = strlen( lt );
		lt[ length - 1 ] = '\0';
		
		//eliminate first char
		lt = &lt[1];

		f << lt << "\n";
	}
	f.close();
}

void CPUTrace::addTrace( uword pc, const opcodeLookUpTableEntry *l, ubyte opcode, ubyte byte1, ubyte byte2, Regs reg, ubyte flags, ubyte sp, PpuClockCycles cpuTime, PpuClockCycles ppuTime ) {
	CPUTraceInstance inst;

	static uword prevPc = 0;
	
	//prevent duplicate entries (which would happen overwise in 
	//the case of a backtrack)
	//also checks to see if a trace was already done at current pc (if traceAlreadyRun flag is not set)
	if( ( prevPc != pc ) && !( !traceAlreadyRun && traceWrittenAt[ pc ] ) ) {
		inst.pc		= pc;
		inst.flags	= flags;
		inst.opcode = opcode;
		inst.byte1	= byte1;
		inst.byte2  = byte2;
		inst.l		= l;
		inst.reg	= reg;
		inst.sp		= sp;
		inst.cpuTime	= cpuTime;
		inst.ppuTime	= ppuTime;
		traceArray.push_back( inst );
		numTraces++;
		traceWrittenAt[ pc ] = true;
	}
	
	prevPc = pc;
}

bool CPUTrace::areTracing() {
	return trace;
}

void CPUTrace::startTrace() {
	trace = true;
	traceArray.clear();
	numTraces = 0;
}

void CPUTrace::stopTrace() {
	trace = false;
}

void CPUTrace::clearTraces() {
	traceArray.clear();
	for( int i = 0; i < 0xffff; i++ ) {
		traceWrittenAt[ i ] = 0;
	}
}

//sorts array based on pc
//can not be undone
void CPUTrace::sortTraceArray() {
	std::sort( traceArray.begin(), traceArray.end() );
}

*/
#endif //LIGHT_BUILD