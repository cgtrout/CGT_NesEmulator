//nes cpu core
#pragma once

#include "NesMemory.h"
#include "NesEmulatorFlagSystem.h"
#include "typedefs.h"

#define CALL_MEMBER_FN( object,ptrToMember )  ( ( object ).*( ptrToMember ) )

#include <string>
#include <vector>

namespace NesEmulator {

	//forward declarations
	class NesMain;
	class NesDebugger;
	class NesMemory;
	class NesPPU;
		
	//opcodes
	enum CpuOpcode {
		OP_ADC,	OP_AND,	OP_ASL,	OP_BCC,	OP_BCS,	OP_BEQ,	OP_BIT,	OP_BMI,	OP_BNE,	OP_BPL,	OP_BRK,	OP_BVC,	OP_BVS,
		OP_CLC,	OP_CLD,	OP_CLI,	OP_CLV,	OP_CMP,	OP_CPX,	OP_CPY,	OP_DEC,	OP_DEX,	OP_DEY,	OP_EOR,	OP_INC,	OP_INX,
		OP_INY,	OP_JMP,	OP_JSR,	OP_LDA,	OP_LDX,	OP_LDY,	OP_LSR,	OP_NOP,	OP_ORA,	OP_PHA,	OP_PHP,	OP_PLA,	OP_PLP,
		OP_ROL,	OP_ROR,	OP_RTI,	OP_RTS,	OP_SBC,	OP_SEC,	OP_SED,	OP_SEI,	OP_STA,	OP_STX,	OP_STY,	OP_TAX,	OP_TAY,
		OP_TSX,	OP_TXA,	OP_TXS,	OP_TYA,	OP_BREAK, OP_RRA,
		OP_BAD
	};

	//addressing modes
	enum CpuAddressMode { 
		M_IMMEDIATE, M_ZEROPAGE, M_ZEROPAGEX, M_ZEROPAGEY, M_ABSOLUTE, 
		M_ABSOLUTEX, M_ABSOLUTEY, M_INDIRECT, M_INDIRECTX, M_INDIRECTY, 
		M_IMPLIED, M_ACCUMULATOR,	M_RELATIVE, M_BAD 
	};

	enum InstType {
		IT_SINGLEBYTE,		//operate only on internal registers
		IT_INTERNALEXEC,	//reads data and then does an operation on it
		IT_STORE,			//stores internal data to a memory location
		IT_READMODIFYWRITE,	//reads data from memory, modifies it, and then writes it back to memory
		IT_MISC,			//miscellaneous instruction
		IT_BRANCH,
		IT_BAD,
	};

	//addressing mode length lookup table ( to find out how many bytes a instruction uses )
	static int addModeLengthTable[ ] = {
	/*M_IMMEDIATE*/		2, /*M_ZEROPAGE*/		2,
	/*M_ZEROPAGEX*/		2, /*M_ZEROPAGEY*/		2,
	/*M_ABSOLUTE*/		3, /*M_ABSOLUTEX*/		3,
	/*M_ABSOLUTEY*/		3, /*M_INDIRECT*/		3,
	/*M_INDIRECTX*/		2, /*M_INDIRECTY*/		2,
	/*M_IMPLIED*/		1, /*M_ACCUMULATOR*/	1,
	/*M_RELATIVE*/		2
	};

	//prebuilt tables that contain the time offsets
	//to writes / reads (from the start of the inst)
	static ubyte opcodeReadTime[0xff];
	static ubyte opcodeWriteTime[0xff];

	class NesCpu;

	//function pointer to nes cpu op handler
	typedef void ( NesCpu::*NesCpuFnptr )();

	struct opcodeLookUpTableEntry {
		CpuAddressMode mode; //addressing mode
		CpuOpcode op;		 //operand
		char syntax[ 4 ];	 //string syntax of operand
		ubyte time;			 //time needed to run operand
		ubyte addCycleForPageBoundry;	//adds cycle if pg bndy crossed
		NesCpuFnptr fnptr;	 //function pointer to operand handler
		InstType	type;	 //read/write type of opcode
	};

	struct Regs { ubyte a, x, y; };

#if !defined ( LIGHT_BUILD )

	struct CPUTraceInstance {
		uword pc;
		ubyte opcode;
		const opcodeLookUpTableEntry* l;
		ubyte byte1, byte2;
		ubyte flags;
		Regs reg;
		ubyte sp;

		CpuClockCycles cpuTime;
		PpuClockCycles ppuTime;

		CPUTraceInstance( );

		//used for sorting
		bool operator <( CPUTraceInstance& );
	};

	/*
	=================================================================
	=================================================================
	  CPUTrace
			is processor intensive and very memory intensive to run this
	=================================================================
	=================================================================
	*/
	class CPUTrace {
	public:
		CPUTrace( NesMain* );

		//print 
		void printTrace( std::string_view filename );

		//print dissassembled view
		void printAsm( std::string_view filename );

		void addTrace( uword pc, const opcodeLookUpTableEntry* l, ubyte opcode, ubyte byte1, ubyte byte2, Regs reg, ubyte flags, ubyte sp, PpuClockCycles cpuTime, PpuClockCycles ppuTime );
		bool areTracing( );

		void startTrace( );
		void stopTrace( );

		void clearTraces( );

	private:

		NesMain* nesMain;

		bool trace;
		std::vector<bool> traceWrittenAt;
		std::vector< CPUTraceInstance > traceArray;
		int numTraces;

		void sortTraceArray( );

	};

#endif 

	/*
	=================================================================
	=================================================================
	NesCpu Class

	TODO this class is way to big - find a way to chop it up
	=================================================================
	=================================================================
	*/
	class NesCpu {
	  public:

		NesCpu( NesMain* nesMain );
		~NesCpu();

		void initialize( );

		//returns PPU cycles run this frame
		PpuClockCycles getCC() { return cpuTimer; }

		// these get the time (from the start of 
		// VINT that a memory location was written/
		// read to from
		PpuClockCycles getWriteTime();
		PpuClockCycles getReadTime();
		
		void resetCC() { cpuTimer = 0; }

		//void setOnStatus( bool val ) {onStatus = val;}
		//bool getOnStatus() {return onStatus;}
		uword getPC() {return pc;}
				
		void reset();
	
		//runs many instructions until a EmulatorFlag is hit
		void runInstructions( );

		//adds cycle offset for overflows
		void addOverflow( PpuClockCycles overflow );

		void setFlagsFromByte( ubyte fb );		
		ubyte createFlagByte();
		
		//these two allow a backup of the flags to be
		//made so that they can be restored if the 
		//cpu needs to backtrack
		void protectFlags();	
		void restoreFlags();

		//flag getters
		ubyte getCarry() { return flags.c; }
		ubyte getZero()	 { return flags.z; }
		ubyte getInterruptDisable() { return flags.i; }	
		ubyte getBreakPoint()	{ return flags.b; }
		ubyte getOverflow()		{ return flags.v; }
		ubyte getNegative()		{ return flags.n; }
		ubyte getAReg() { return reg.a; }
		ubyte getXReg() { return reg.x; }
		ubyte getYReg() { return reg.y; }
		uword getSP()   { return sp;    }

		NesEmulatorFlagSystem* flagSystem;
		CPUTrace cpuTrace;

	private:

		NesMain*		nesMain;
		NesDebugger*	nesDebugger;
		NesMemory*		nesMemory;
		NesPPU*			nesPpu;

		//memory read routines
		ubyte getMemoryZP();		ubyte getIMM();		
		ubyte getMemoryZPX();		ubyte getZPX();
		ubyte getMemoryZPY();		ubyte getZPY();
		ubyte getMemoryABS();		uword getABS();
		ubyte getMemoryABSX();		uword getABSX();
		ubyte getMemoryABSY();		uword getABSY();
		ubyte getMemoryINDX();
		ubyte getMemoryINDY();

		PpuClockCycles cpuTimer;	//current clock count of cpu
		
		ubyte iTime;		//time current instruction takes to run
		
		uword	pc;			//program counter
		uword	oldPc;

		ubyte	sp;			//stack pointer
		ubyte	b1, b2;		//bytes one and two of operand
		uword	fullb;		//combined byte one and two
		ubyte	currOp;		//current op that is being worked on
		ubyte	length;		//length of current op
		bool	branched;	//did the last inst branch?
		
		bool pageBoundaryCrossed;	//did current inst cross pg bny
		
		//bool nmiState;
		//bool irqRequested;	//is there an active irq?

		//lookup entry for currently emulated op
		const opcodeLookUpTableEntry *lookupEntry;

		//registers
		Regs reg;

		//TODO - perhaps the flags should be stored in one byte form ??
		struct {
			ubyte	c,	//carry
					z,	//zero
					d,	//decimal
					i,	//interupt disable
					b,	//breakpoint
					v,	//overflow
					n,	//negative/sign
					u;  //unused flag
		} flags, flagsBackup;

	private:
		//bool onStatus;		//is the cpu currently running
		
		
		void gotoInterrupt( uword memLoc );
		
		void buildReadWriteTimeTables();
		void buildReadTimeTable();
		void buildWriteTimeTable();
		
		//run ( 1 ) instruction
		void runInstruction();
		
	public:
		//general operation routines
		void opADC_General( ubyte memValue );
		void opAND_General( ubyte memValue );
		void opASL_General( short value );
		void opBIT_General( ubyte memValue );
		void opCMP_General( ubyte memValue );
		void opCPX_General( ubyte memValue );
		void opCPY_General( ubyte memValue );
		void opDEC_General( short memLocation );    
		void opEOR_General( ubyte memValue ); 
		void opINC_General( short memLocation );
		void opLDA_General( ubyte memValue );

		void opLDX_General( ubyte memValue );
		void opLDY_General( ubyte memValue );
		void opLSR_General( short memLocation );
		void opORA_General( ubyte memValue );
		void opROL_General( short memLocation );
		void opROR_General( short memLocation );
		void opSBC_General( ubyte memValue );

		/*
		===================================================================
		NEW undocumented
		https://www.pagetable.com/c64ref/6502/?tab=2#RLA
		===================================================================
		*/
		void opRRA_General( short memLocation );

		void followBranch();	//used by branches to follow a branch

		//opcode functions
		void opASL_ABS(); void opASL_ABSX(); void opBIT_ZP();
		void opBAD(); void opBreak(); void opADC_IMM();	void opADC_ZP(); void opADC_ZPX(); void opADC_ABS();
		void opADC_ABSX(); void opADC_ABSY(); void opADC_INDX(); void opADC_INDY(); void opAND_IMM(); 
		void opAND_ZP(); void opAND_ZPX(); void opAND_ABS();	void opAND_ABSX(); void opAND_ABSY(); 
		void opAND_INDX(); void opAND_INDY(); void opASL_ACC(); void opASL_ZP(); void opASL_ZPX(); 
		void opBIT_ABS(); void opBPL_REL(); void opBMI_REL(); void opBVC_REL(); void opBVS_REL(); void opBCC_REL();
		void opBCS_REL(); void opBNE_REL(); void opBEQ_REL(); void opBRK_IMP(); void opCMP_IMM(); void opCMP_ZP();
		void opCMP_ZPX(); void opCMP_ABS(); void opCMP_ABSX(); void opCMP_ABSY(); void opCMP_INDX();
		void opCMP_INDY(); void opCPX_IMM(); void opCPX_ZP(); void opCPX_ABS(); void opCPY_IMM();
		void opCPY_ZP(); void opCPY_ABS(); void opDEC_ZP(); void opDEC_ZPX(); void opDEC_ABS();
		void opDEC_ABSX(); void opEOR_IMM(); void opEOR_ZP(); void opEOR_ZPX(); void opEOR_ABS();
		void opEOR_ABSX(); void opEOR_ABSY(); void opEOR_INDX(); void opEOR_INDY(); void opCLC_IMP();
		void opSEC_IMP(); void opCLI_IMP(); void opSEI_IMP(); void opCLV_IMP(); void opCLD_IMP();
		void opSED_IMP(); void opINC_ZP(); void opINC_ZPX(); void opINC_ABS(); void opINC_ABSX();
		void opJMP_ABS(); void opJMP_IND(); void opJSR_ABS(); void opLDA_IMM(); void opLDA_ZP();
		void opLDA_ZPX(); void opLDA_ABS(); void opLDA_ABSX(); void opLDA_ABSY(); void opLDA_INDX();
		void opLDA_INDY(); void opLDX_IMM(); void opLDX_ZP(); void opLDX_ZPY(); void opLDX_ABS();
		void opLDX_ABSY(); void opLDY_IMM(); void opLDY_ZP(); void opLDY_ZPX(); void opLDY_ABS();
		void opLDY_ABSX(); void opLSR_ACC(); void opLSR_ZP(); void opLSR_ZPX(); void opLSR_ABS();
		void opLSR_ABSX(); void opNOP_IMP(); void opORA_IMM(); void opORA_ZP(); void opORA_ZPX();
		void opORA_ABS(); void opORA_ABSX(); void opORA_ABSY(); void opORA_INDX(); void opORA_INDY();
		void opTAX_IMP(); void opTXA_IMP(); void opDEX_IMP(); void opINX_IMP(); void opTAY_IMP();
		void opTYA_IMP(); void opDEY_IMP(); void opINY_IMP(); void opROL_ACC(); void opROL_ZP();
		void opROL_ZPX(); void opROL_ABS(); void opROL_ABSX(); void opROR_ACC(); void opROR_ZP();
		void opROR_ZPX(); void opROR_ABS(); void opROR_ABSX(); void opRTI_IMP(); void opRTS_IMP();
		void opSBC_IMM(); void opSBC_ZP(); void opSBC_ZPX(); void opSBC_ABS(); void opSBC_ABSX();
		void opSBC_ABSY(); void opSBC_INDX(); void opSBC_INDY(); void opSTA_ZP(); void opSTA_ZPX();
		void opSTA_ABS(); void opSTA_ABSX(); void opSTA_ABSY(); void opSTA_INDX(); void opSTA_INDY();
		void opTXS_IMP(); void opTSX_IMP(); void opPHA_IMP(); void opPLA_IMP(); void opPHP_IMP();
		void opPLP_IMP(); void opSTX_ZP(); void opSTX_ZPY(); void opSTX_ABS(); void opSTY_ZP();
		void opSTY_ZPX(); void opSTY_ABS();

		void opRRA_IMM( );void opRRA_ZP( );void opRRA_ZPX( );void opRRA_ABS( );void opRRA_ABSX( );void opRRA_ABSY( );
		void opRRA_INDX( );
		void opRRA_INDY( );
	};


}

#include "NesOpcodeTable.h"


