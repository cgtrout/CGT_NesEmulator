
namespace NesEmulator {
	
	//table entry for a bad opcode
	#define BAD_DEF { M_BAD, OP_BAD, "BAD",	0,	0, &NesCpu::opBAD, IT_BAD, },

	//Opcode lookup table
	static const opcodeLookUpTableEntry opcodeLookUpTable[] = {

	//OPCODE	MODE			op		syntax		time	pgbdry	fnptr					instType

	/*0x00*/{	M_IMMEDIATE,	OP_BRK,	"BRK",		7,		0,		&NesCpu::opBRK_IMP,		IT_MISC,						},	
	/*0x01*/{	M_INDIRECTX,	OP_ORA,	"ORA",		6,		0,		&NesCpu::opORA_INDX	,	IT_INTERNALEXEC,						},		
	/*0x02*/{	M_ABSOLUTE,		OP_BAD, "BAD",	    0,		0,		&NesCpu::opBreak,		 IT_BAD,						},
	/*0x03*/	  BAD_DEF
	/*0x04*/	  BAD_DEF
	/*0x05*/{	M_ZEROPAGE,		OP_ORA,	"ORA",		3,		0,		&NesCpu::opORA_ZP,		IT_INTERNALEXEC,						},		
	/*0x06*/{	M_ZEROPAGE,		OP_ASL,	"ASL",		5,		0,		&NesCpu::opASL_ZP,		IT_READMODIFYWRITE,						},		
	/*0x07*/     BAD_DEF
	/*0x08*/{	M_IMPLIED,		OP_PHP,	"PHP",		3,		0,		&NesCpu::opPHP_IMP,		IT_MISC,						},
	/*0x09*/{	M_IMMEDIATE,	OP_ORA,	"ORA",		2,		0,		&NesCpu::opORA_IMM,		IT_INTERNALEXEC,						},		
	/*0x0A*/{	M_ACCUMULATOR,	OP_ASL,	"ASL",		2,		0,		&NesCpu::opASL_ACC,		IT_READMODIFYWRITE,						},		
	/*0x0B*/     BAD_DEF
	/*0x0C*/     BAD_DEF
	/*0x0D*/{	M_ABSOLUTE,		OP_ORA,	"ORA",		4,		0,		&NesCpu::opORA_ABS,		IT_INTERNALEXEC,						},		
	/*0x0E*/{	M_ABSOLUTE,		OP_ASL,	"ASL",		6,		0,		&NesCpu::opASL_ABS,		IT_READMODIFYWRITE,						},		
	/*0x0F*/     BAD_DEF	
	/*0x10*/{	M_RELATIVE,		OP_BPL,	"BPL",		2,		1,		&NesCpu::opBPL_REL,		IT_MISC,						},		
	/*0x11*/{	M_INDIRECTY,	OP_ORA,	"ORA",		5,		1,		&NesCpu::opORA_INDY	,	IT_INTERNALEXEC,						},
	/*0x12*/     BAD_DEF
	/*0x13*/     BAD_DEF
	/*0x14*/     BAD_DEF
	/*0x15*/{	M_ZEROPAGEX,	OP_ORA,	"ORA",		4,		0,		&NesCpu::opORA_ZPX,		IT_INTERNALEXEC,						},		
	/*0x16*/{	M_ZEROPAGEX,	OP_ASL,	"ASL",		6,		0,		&NesCpu::opASL_ZPX,		IT_READMODIFYWRITE,						},		
	/*0x17*/     BAD_DEF
	/*0x18*/{	M_IMPLIED,		OP_CLC,	"CLC",		2,		0,		&NesCpu::opCLC_IMP,		IT_SINGLEBYTE,						},	
	
	/*0x19*/{	M_ABSOLUTEY,	OP_ORA,	"ORA",		4,		1,		&NesCpu::opORA_ABSY	,	IT_INTERNALEXEC,						},		
	/*0x1A*/     BAD_DEF
	/*0x1B*/     BAD_DEF
	/*0x1C*/     BAD_DEF
	/*0x1D*/{	M_ABSOLUTEX,	OP_ORA,	"ORA",		4,		1,		&NesCpu::opORA_ABSX	,	IT_INTERNALEXEC,						},		
	/*0x1E*/{	M_ABSOLUTEX,	OP_ASL,	"ASL",		7,		0,		&NesCpu::opASL_ABSX	,	IT_READMODIFYWRITE,						},	
	/*0x1F*/     BAD_DEF
	/*0x20*/{	M_ABSOLUTE,		OP_JSR,	"JSR",		6,		0,		&NesCpu::opJSR_ABS,		IT_MISC,						},		
	/*0x21*/{	M_INDIRECTX,	OP_AND,	"AND",		6,		0,		&NesCpu::opAND_INDX	,	IT_INTERNALEXEC,		},
	/*0x22*/	 BAD_DEF
	/*0x23*/	 BAD_DEF
	/*0x24*/{	M_ZEROPAGE,		OP_BIT,	"BIT",		3,		0,		&NesCpu::opBIT_ZP,		IT_INTERNALEXEC,		},
	/*0x25*/{	M_ZEROPAGE,		OP_AND,	"AND",		3,		0,		&NesCpu::opAND_ZP,		IT_INTERNALEXEC,		},
	/*0x26*/{	M_ZEROPAGE,		OP_ROL,	"ROL",		5,		0,		&NesCpu::opROL_ZP,		IT_READMODIFYWRITE,						},		
	/*0x27*/     BAD_DEF
	/*0x28*/{	M_IMPLIED,		OP_PLP,	"PLP",		4,		0,		&NesCpu::opPLP_IMP,		IT_MISC,						},
	/*0x29*/{	M_IMMEDIATE,	OP_AND,	"AND",		2,		0,		&NesCpu::opAND_IMM,		IT_INTERNALEXEC,		},
	/*0x2A*/{	M_ACCUMULATOR,	OP_ROL,	"ROL",		2,		0,		&NesCpu::opROL_ACC,		IT_READMODIFYWRITE,						},		
	/*0x2B*/     BAD_DEF
	/*0x2C*/{	M_ABSOLUTE,		OP_BIT,	"BIT",		4,		0,		&NesCpu::opBIT_ABS,		IT_INTERNALEXEC,		},
	/*0x2D*/{	M_ABSOLUTE,		OP_AND,	"AND",		4,		0,		&NesCpu::opAND_ABS,		IT_INTERNALEXEC,		},
	/*0x2E*/{	M_ABSOLUTE,		OP_ROL,	"ROL",		6,		0,		&NesCpu::opROL_ABS,		IT_READMODIFYWRITE,						},		
	/*0x2F*/     BAD_DEF
	/*0x30*/{	M_RELATIVE,		OP_BMI,	"BMI",		2,		1,		&NesCpu::opBMI_REL,		IT_BRANCH,						},		
	/*0x31*/{	M_INDIRECTY,	OP_AND,	"AND",		5,		1,		&NesCpu::opAND_INDY	,	IT_INTERNALEXEC,		},
	/*0x32*/     BAD_DEF
	/*0x33*/     BAD_DEF
	/*0x34*/     BAD_DEF
	/*0x35*/{	M_ZEROPAGEX,	OP_AND,	"AND",		4,		0,		&NesCpu::opAND_ZPX,		IT_INTERNALEXEC,		},
	/*0x36*/{	M_ZEROPAGEX,	OP_ROL,	"ROL",		6,		0,		&NesCpu::opROL_ZPX,		IT_READMODIFYWRITE,						},		
	/*0x37*/     BAD_DEF
	/*0x38*/{	M_IMPLIED,		OP_SEC,	"SEC",		2,		0,		&NesCpu::opSEC_IMP,		IT_SINGLEBYTE,						},	
	/*0x39*/{	M_ABSOLUTEY,	OP_AND,	"AND",		4,		1,		&NesCpu::opAND_ABSY	,	IT_INTERNALEXEC,		},
	/*0x3A*/     BAD_DEF
	/*0x3B*/     BAD_DEF
	/*0x3C*/     BAD_DEF
	/*0x3D*/{	M_ABSOLUTEX,	OP_AND,	"AND",		4,		1,		&NesCpu::opAND_ABSX	,	IT_INTERNALEXEC,		},
	/*0x3E*/{	M_ABSOLUTEX,	OP_ROL,	"ROL",		7,		0,		&NesCpu::opROL_ABSX	,	IT_READMODIFYWRITE,						},		
	/*0x3F*/     BAD_DEF
	/*0x40*/{	M_IMPLIED,		OP_RTI,	"RTI",		6,		0,		&NesCpu::opRTI_IMP,		IT_MISC,						},		
	/*0x41*/{	M_INDIRECTX,	OP_EOR,	"EOR",		6,		0,		&NesCpu::opEOR_INDX	,	IT_INTERNALEXEC,						},		
	/*0x42*/     BAD_DEF
	/*0x43*/     BAD_DEF
	/*0x44*/     BAD_DEF
	/*0x45*/{	M_ZEROPAGE,		OP_EOR,	"EOR",		3,		0,		&NesCpu::opEOR_ZP,		IT_INTERNALEXEC,						},		
	/*0x46*/{	M_ZEROPAGE,		OP_LSR,	"LSR",		5,		0,		&NesCpu::opLSR_ZP,		IT_READMODIFYWRITE,						},		
	/*0x47*/     BAD_DEF
	/*0x48*/{	M_IMPLIED,		OP_PHA,	"PHA",		3,		0,		&NesCpu::opPHA_IMP,		IT_MISC,						},
	/*0x49*/{	M_IMMEDIATE,	OP_EOR,	"EOR",		2,		0,		&NesCpu::opEOR_IMM,		IT_INTERNALEXEC,						},		
	/*0x4A*/{	M_ACCUMULATOR,	OP_LSR,	"LSR",		2,		0,		&NesCpu::opLSR_ACC,		IT_READMODIFYWRITE,						},		
	/*0x4B*/     BAD_DEF
	/*0x4C*/{	M_ABSOLUTE,		OP_JMP,	"JMP",		3,		0,		&NesCpu::opJMP_ABS,		IT_MISC,						},		
	/*0x4D*/{	M_ABSOLUTE,		OP_EOR,	"EOR",		4,		0,		&NesCpu::opEOR_ABS,		IT_INTERNALEXEC,						},		
	/*0x4E*/{	M_ABSOLUTE,		OP_LSR,	"LSR",		6,		0,		&NesCpu::opLSR_ABS,		IT_READMODIFYWRITE,						},		
	/*0x4F*/     BAD_DEF
	/*0x50*/{	M_RELATIVE,		OP_BVC,	"BVC",		2,		1,		&NesCpu::opBVC_REL,		IT_BRANCH,						},		
	/*0x51*/{	M_INDIRECTY,	OP_EOR,	"EOR",		5,		1,		&NesCpu::opEOR_INDY	,	IT_INTERNALEXEC,						},
	/*0x52*/     BAD_DEF
	/*0x53*/     BAD_DEF
	/*0x54*/     BAD_DEF
	/*0x55*/{	M_ZEROPAGEX,	OP_EOR,	"EOR",		4,		0,		&NesCpu::opEOR_ZPX,		IT_INTERNALEXEC,						},		
	/*0x56*/{	M_ZEROPAGEX,	OP_LSR,	"LSR",		6,		0,		&NesCpu::opLSR_ZPX,		IT_READMODIFYWRITE,						},		
	/*0x57*/     BAD_DEF
	/*0x58*/{	M_IMPLIED,		OP_CLI,	"CLI",		2,		0,		&NesCpu::opCLI_IMP,		IT_SINGLEBYTE,						},	
	/*0x59*/{	M_ABSOLUTEY,	OP_EOR,	"EOR",		4,		1,		&NesCpu::opEOR_ABSY	,	IT_INTERNALEXEC,						},		
	/*0x5A*/     BAD_DEF
	/*0x5B*/     BAD_DEF
	/*0x5C*/     BAD_DEF
	/*0x5D*/{	M_ABSOLUTEX,	OP_EOR,	"EOR",		4,		1,		&NesCpu::opEOR_ABSX	,	IT_INTERNALEXEC,						},		
	/*0x5E*/{	M_ABSOLUTEX,	OP_LSR,	"LSR",		7,		0,		&NesCpu::opLSR_ABSX	,	IT_READMODIFYWRITE,						},	
	/*0x5F*/     BAD_DEF
	/*0x60*/{	M_IMPLIED,		OP_RTS,	"RTS",		6,		0,		&NesCpu::opRTS_IMP,		IT_MISC,						},
	/*0x61*/{	M_INDIRECTX,	OP_ADC,	"ADC",		6,		0,		&NesCpu::opADC_INDX	,	IT_INTERNALEXEC,						},		
	/*0x62*/     BAD_DEF
	/*0x63*/     BAD_DEF
	/*0x64*/     BAD_DEF
	/*0x65*/{	M_ZEROPAGE,		OP_ADC,	"ADC",		3,		0,		&NesCpu::opADC_ZP,		IT_INTERNALEXEC,		},
	/*0x66*/{	M_ZEROPAGE,		OP_ROR,	"ROR",		5,		0,		&NesCpu::opROR_ZP,		IT_READMODIFYWRITE,						},		
	/*0x67*/     BAD_DEF
	/*0x68*/{	M_IMPLIED,		OP_PLA,	"PLA",		4,		0,		&NesCpu::opPLA_IMP,		IT_MISC,						},
	/*0x69*/{	M_IMMEDIATE,	OP_ADC,	"ADC",		2,		0,		&NesCpu::opADC_IMM,		IT_INTERNALEXEC,		},
	/*0x6A*/{	M_ACCUMULATOR,	OP_ROR,	"ROR",		2,		0,		&NesCpu::opROR_ACC,		IT_READMODIFYWRITE,						},		
	/*0x6B*/     BAD_DEF
	/*0x6C*/{	M_INDIRECT,		OP_JMP,	"JMP",		5,		0,		&NesCpu::opJMP_IND,		IT_MISC,						},		
	/*0x6D*/{	M_ABSOLUTE,		OP_ADC,	"ADC",		4,		0,		&NesCpu::opADC_ABS,		IT_INTERNALEXEC,		},
	/*0x6E*/{	M_ABSOLUTE,		OP_ROR,	"ROR",		6,		0,		&NesCpu::opROR_ABS,		IT_READMODIFYWRITE,						},		
	/*0x6F*/     BAD_DEF
	/*0x70*/{	M_RELATIVE,		OP_BVS,	"BVS",		2,		1,		&NesCpu::opBVS_REL,		IT_BRANCH,						},		
	/*0x71*/{	M_INDIRECTY,	OP_ADC,	"ADC",		5,		1,		&NesCpu::opADC_INDY	,	IT_INTERNALEXEC,		},
	/*0x72*/     BAD_DEF
	/*0x73*/     BAD_DEF
	/*0x74*/     BAD_DEF
	/*0x75*/{	M_ZEROPAGEX,	OP_ADC,	"ADC",		4,		0,		&NesCpu::opADC_ZPX,		IT_INTERNALEXEC,		},
	/*0x76*/{	M_ZEROPAGEX,	OP_ROR,	"ROR",		6,		0,		&NesCpu::opROR_ZPX,		IT_READMODIFYWRITE,						},		
	/*0x77*/     BAD_DEF
	/*0x78*/{	M_IMPLIED,		OP_SEI,	"SEI",		2,		0,		&NesCpu::opSEI_IMP,		IT_SINGLEBYTE,						},	
	/*0x79*/{	M_ABSOLUTEY,	OP_ADC,	"ADC",		4,		1,		&NesCpu::opADC_ABSY	,	IT_INTERNALEXEC,		},
	/*0x7A*/     BAD_DEF
	/*0x7B*/     BAD_DEF
	/*0x7C*/     BAD_DEF
	/*0x7D*/{	M_ABSOLUTEX,	OP_ADC,	"ADC",		4,		1,		&NesCpu::opADC_ABSX	,	IT_INTERNALEXEC,		},
	/*0x7E*/{	M_ABSOLUTEX,	OP_ROR,	"ROR",		7,		0,		&NesCpu::opROR_ABSX	,	IT_READMODIFYWRITE,						},	
	/*0x7F*/     BAD_DEF
	/*0x80*/     BAD_DEF
	/*0x81*/{	M_INDIRECTX,	OP_STA,	"STA",		6,		0,		&NesCpu::opSTA_INDX	,	IT_STORE,						},		
	/*0x82*/     BAD_DEF
	/*0x83*/     BAD_DEF
	/*0x84*/{	M_ZEROPAGE,		OP_STY,	"STY",		3,		0,		&NesCpu::opSTY_ZP,		IT_STORE,						},		
	/*0x85*/{	M_ZEROPAGE,		OP_STA,	"STA",		3,		0,		&NesCpu::opSTA_ZP,		IT_STORE,						},		
	/*0x86*/{	M_ZEROPAGE,		OP_STX,	"STX",		3,		0,		&NesCpu::opSTX_ZP,		IT_STORE,						},		
	/*0x87*/     BAD_DEF
	/*0x88*/{	M_IMPLIED,		OP_DEY,	"DEY",		2,		0,		&NesCpu::opDEY_IMP,		IT_SINGLEBYTE,						},		
	/*0x89*/     BAD_DEF
	/*0x8A*/{	M_IMPLIED,		OP_TXA,	"TXA",		2,		0,		&NesCpu::opTXA_IMP,		IT_SINGLEBYTE,						},		
	/*0x8B*/     BAD_DEF
	/*0x8C*/{	M_ABSOLUTE,		OP_STY,	"STY",		4,		0,		&NesCpu::opSTY_ABS,		IT_STORE,						},
	/*0x8D*/{	M_ABSOLUTE,		OP_STA,	"STA",		4,		0,		&NesCpu::opSTA_ABS,		IT_STORE,						},		
	/*0x8E*/{	M_ABSOLUTE,		OP_STX,	"STX",		4,		0,		&NesCpu::opSTX_ABS,		IT_STORE,						},		
	/*0x8F*/     BAD_DEF
	/*0x90*/{	M_RELATIVE,		OP_BCC,	"BCC",		2,		1,		&NesCpu::opBCC_REL,		IT_BRANCH,						},		
	/*0x91*/{	M_INDIRECTY,	OP_STA,	"STA",		6,		0,		&NesCpu::opSTA_INDY	,	IT_STORE,						},		
	/*0x92*/     BAD_DEF
	/*0x93*/     BAD_DEF
	/*0x94*/{	M_ZEROPAGEX,	OP_STY,	"STY",		4,		0,		&NesCpu::opSTY_ZPX,		IT_STORE,						},		
	/*0x95*/{	M_ZEROPAGEX,	OP_STA,	"STA",		4,		0,		&NesCpu::opSTA_ZPX,		IT_STORE,						},		
	/*0x96*/{	M_ZEROPAGEY,	OP_STX,	"STX",		4,		0,		&NesCpu::opSTX_ZPY,		IT_STORE,						},		
	/*0x97*/     BAD_DEF
	/*0x98*/{	M_IMPLIED,		OP_TYA,	"TYA",		2,		0,		&NesCpu::opTYA_IMP,		 IT_SINGLEBYTE,					},		
	/*0x99*/{	M_ABSOLUTEY,	OP_STA,	"STA",		5,		0,		&NesCpu::opSTA_ABSY	,	IT_STORE,						},		
	/*0x9A*/{	M_IMPLIED,		OP_TXS,	"TXS",		2,		0,		&NesCpu::opTXS_IMP,		IT_SINGLEBYTE,					},
	/*0x9B*/     BAD_DEF
	/*0x9C*/     BAD_DEF
	/*0x9D*/{	M_ABSOLUTEX,	OP_STA,	"STA",		5,		0,		&NesCpu::opSTA_ABSX	,	IT_STORE,						},		
	/*0x9E*/     BAD_DEF
	/*0x9F*/     BAD_DEF
	/*0xA0*/{	M_IMMEDIATE,	OP_LDY,	"LDY",		2,		0,		&NesCpu::opLDY_IMM,		IT_INTERNALEXEC,						},		
	/*0xA1*/{	M_INDIRECTX,	OP_LDA,	"LDA",		6,		0,		&NesCpu::opLDA_INDX	,	IT_INTERNALEXEC,						},		
	/*0xA2*/{	M_IMMEDIATE,	OP_LDX,	"LDX",		2,		0,		&NesCpu::opLDX_IMM,		IT_INTERNALEXEC,						},		
	/*0xA3*/     BAD_DEF
	
	/*0xA4*/{	M_ZEROPAGE,		OP_LDY,	"LDY",		3,		0,		&NesCpu::opLDY_ZP,		IT_INTERNALEXEC,						},		
	/*0xA5*/{	M_ZEROPAGE,		OP_LDA,	"LDA",		3,		0,		&NesCpu::opLDA_ZP,		IT_INTERNALEXEC,						},		
	/*0xA6*/{	M_ZEROPAGE,		OP_LDX,	"LDX",		3,		0,		&NesCpu::opLDX_ZP,		IT_INTERNALEXEC,						},		
	/*0xA7*/     BAD_DEF
	/*0xA8*/{	M_IMPLIED,		OP_TAY,	"TAY",		2,		0,		&NesCpu::opTAY_IMP,		IT_SINGLEBYTE,						},		
	/*0xA9*/{	M_IMMEDIATE,	OP_LDA,	"LDA",		2,		0,		&NesCpu::opLDA_IMM,		IT_INTERNALEXEC,						},		
	/*0xAA*/{	M_IMPLIED,		OP_TAX,	"TAX",		2,		0,		&NesCpu::opTAX_IMP,		IT_SINGLEBYTE,						},		
	/*0xAB*/     BAD_DEF
	/*0xAC*/{	M_ABSOLUTE,		OP_LDY,	"LDY",		4,		0,		&NesCpu::opLDY_ABS,		IT_INTERNALEXEC,						},		
	/*0xAD*/{	M_ABSOLUTE,		OP_LDA,	"LDA",		4,		0,		&NesCpu::opLDA_ABS,		IT_INTERNALEXEC,						},		
	/*0xAE*/{	M_ABSOLUTE,		OP_LDX,	"LDX",		4,		0,		&NesCpu::opLDX_ABS,		IT_INTERNALEXEC,						},		
	/*0xAF*/     BAD_DEF
	/*0xB0*/{	M_RELATIVE,		OP_BCS,	"BCS",		2,		1,		&NesCpu::opBCS_REL,		IT_BRANCH,						},		
	/*0xB1*/{	M_INDIRECTY,	OP_LDA,	"LDA",		5,		1,		&NesCpu::opLDA_INDY	,	IT_INTERNALEXEC,						},
	/*0xB2*/     BAD_DEF
	/*0xB3*/     BAD_DEF
	/*0xB4*/{	M_ZEROPAGEX,	OP_LDY,	"LDY",		4,		0,		&NesCpu::opLDY_ZPX,		IT_INTERNALEXEC,						},		
	/*0xB5*/{	M_ZEROPAGEX,	OP_LDA,	"LDA",		4,		0,		&NesCpu::opLDA_ZPX,		IT_INTERNALEXEC,						},		
	/*0xB6*/{	M_ZEROPAGEY,	OP_LDX,	"LDX",		4,		0,		&NesCpu::opLDX_ZPY,		IT_INTERNALEXEC,						},		
	/*0xB7*/     BAD_DEF
	/*0xB8*/{	M_IMPLIED,		OP_CLV,	"CLV",		2,		0,		&NesCpu::opCLV_IMP,		IT_SINGLEBYTE,						},	
	/*0xB9*/{	M_ABSOLUTEY,	OP_LDA,	"LDA",		4,		1,		&NesCpu::opLDA_ABSY	,	IT_INTERNALEXEC,						},		
	/*0xBA*/{	M_IMPLIED,		OP_TSX,	"TSX",		2,		0,		&NesCpu::opTSX_IMP,		IT_SINGLEBYTE,						},
	/*0xBB*/     BAD_DEF
	/*0xBC*/{	M_ABSOLUTEX,	OP_LDY,	"LDY",		4,		1,		&NesCpu::opLDY_ABSX	,	IT_INTERNALEXEC,						},	
	/*0xBD*/{	M_ABSOLUTEX,	OP_LDA,	"LDA",		4,		1,		&NesCpu::opLDA_ABSX	,	IT_INTERNALEXEC,						},		
	/*0xBE*/{	M_ABSOLUTEY,	OP_LDX,	"LDX",		4,		1,		&NesCpu::opLDX_ABSY	,	IT_INTERNALEXEC,						},		
	/*0xBF*/     BAD_DEF
	/*0xC0*/{	M_IMMEDIATE,	OP_CPY,	"CPY",		2,		0,		&NesCpu::opCPY_IMM,		IT_INTERNALEXEC,						},		
	/*0xC1*/{	M_INDIRECTX,	OP_CMP,	"CMP",		6,		0,		&NesCpu::opCMP_INDX	,	IT_INTERNALEXEC,		},
	/*0xC2*/     BAD_DEF
	/*0xC3*/     BAD_DEF
	/*0xC4*/{	M_ZEROPAGE,		OP_CPY,	"CPY",		3,		0,		&NesCpu::opCPY_ZP,		IT_INTERNALEXEC,						},		
	/*0xC5*/{	M_ZEROPAGE,		OP_CMP,	"CMP",		3,		0,		&NesCpu::opCMP_ZP,		IT_INTERNALEXEC,						},		
	/*0xC6*/{	M_ZEROPAGE,		OP_DEC,	"DEC",		5,		0,		&NesCpu::opDEC_ZP,		IT_SINGLEBYTE,						},		
	/*0xC7*/     BAD_DEF
	/*0xC8*/{	M_IMPLIED,		OP_INY,	"INY",		2,		0,		&NesCpu::opINY_IMP,		IT_SINGLEBYTE,						},	
	/*0xC9*/{	M_IMMEDIATE,	OP_CMP,	"CMP",		2,		0,		&NesCpu::opCMP_IMM,		IT_INTERNALEXEC,						},		
	/*0xCA*/{	M_IMPLIED,		OP_DEX,	"DEX",		2,		0,		&NesCpu::opDEX_IMP,		IT_SINGLEBYTE,						},		
	/*0xCB*/     BAD_DEF
	/*0xCC*/{	M_ABSOLUTE,		OP_CPY,	"CPY",		4,		0,		&NesCpu::opCPY_ABS,		IT_INTERNALEXEC,						},
	/*0xCD*/{	M_ABSOLUTE,		OP_CMP,	"CMP",		4,		0,		&NesCpu::opCMP_ABS,		IT_INTERNALEXEC,						},		
	/*0xCE*/{	M_ABSOLUTE,		OP_DEC,	"DEC",		6,		0,		&NesCpu::opDEC_ABS,		IT_SINGLEBYTE,						},		
	/*0xCF*/     BAD_DEF
	/*0xD0*/{	M_RELATIVE,		OP_BNE,	"BNE",		2,		1,		&NesCpu::opBNE_REL,		IT_BRANCH,						},		
	/*0xD1*/{	M_INDIRECTY,	OP_CMP,	"CMP",		5,		1,		&NesCpu::opCMP_INDY	,	IT_INTERNALEXEC,						},
	/*0xD2*/     BAD_DEF
	/*0xD3*/     BAD_DEF
	/*0xD4*/     BAD_DEF
	/*0xD5*/{	M_ZEROPAGEX,	OP_CMP,	"CMP",		4,		0,		&NesCpu::opCMP_ZPX,		IT_INTERNALEXEC,						},		
	/*0xD6*/{	M_ZEROPAGEX,	OP_DEC,	"DEC",		6,		0,		&NesCpu::opDEC_ZPX,		IT_SINGLEBYTE,						},		
	/*0xD7*/     BAD_DEF
	/*0xD8*/{	M_IMPLIED,		OP_CLD,	"CLD",		2,		0,		&NesCpu::opCLD_IMP,		IT_SINGLEBYTE,						},	
	/*0xD9*/{	M_ABSOLUTEY,	OP_CMP,	"CMP",		4,		1,		&NesCpu::opCMP_ABSY	,	IT_INTERNALEXEC,						},		
	/*0xDA*/     BAD_DEF
	/*0xDB*/     BAD_DEF
	/*0xDC*/     BAD_DEF
	/*0xDD*/{	M_ABSOLUTEX,	OP_CMP,	"CMP",		4,		1,		&NesCpu::opCMP_ABSX	,	IT_INTERNALEXEC,						},		
	/*0xDE*/{	M_ABSOLUTEX,	OP_DEC,	"DEC",		7,		0,		&NesCpu::opDEC_ABSX	,	IT_SINGLEBYTE,						},		
	/*0xDF*/     BAD_DEF
	/*0xE0*/{	M_IMMEDIATE,	OP_CPX,	"CPX",		2,		0,		&NesCpu::opCPX_IMM,		IT_INTERNALEXEC,						},		
	/*0xE1*/{	M_INDIRECTX,	OP_SBC,	"SBC",		6,		0,		&NesCpu::opSBC_INDX	,	IT_INTERNALEXEC,						},		
	/*0xE2*/     BAD_DEF
	/*0xE3*/     BAD_DEF
	/*0xE4*/{	M_ZEROPAGE,		OP_CPX,	"CPX",		3,		0,		&NesCpu::opCPX_ZP,		IT_INTERNALEXEC,						},		
	/*0xE5*/{	M_ZEROPAGE,		OP_SBC,	"SBC",		3,		0,		&NesCpu::opSBC_ZP,		IT_INTERNALEXEC,						},		
	/*0xE6*/{	M_ZEROPAGE,		OP_INC,	"INC",		5,		0,		&NesCpu::opINC_ZP,		IT_READMODIFYWRITE,						},		
	/*0xE7*/     BAD_DEF
	/*0xE8*/{	M_IMPLIED,		OP_INX,	"INX",		2,		0,		&NesCpu::opINX_IMP,		IT_READMODIFYWRITE,						},		
	/*0xE9*/{	M_IMMEDIATE,	OP_SBC,	"SBC",		2,		0,		&NesCpu::opSBC_IMM,		IT_INTERNALEXEC,						},		
	/*0xEA*/{	M_IMPLIED,		OP_NOP,	"NOP",		2,		0,		&NesCpu::opNOP_IMP,		IT_SINGLEBYTE,						},
	/*0xEB*/     BAD_DEF
	/*0xEC*/{	M_ABSOLUTE,		OP_CPX,	"CPX",		4,		0,		&NesCpu::opCPX_ABS,		IT_INTERNALEXEC,						},		
	/*0xED*/{	M_ABSOLUTE,		OP_SBC,	"SBC",		4,		0,		&NesCpu::opSBC_ABS,		IT_INTERNALEXEC,						},		
	/*0xEE*/{	M_ABSOLUTE,		OP_INC,	"INC",		6,		0,		&NesCpu::opINC_ABS,		IT_READMODIFYWRITE,						},		
	/*0xEF*/     BAD_DEF
	/*0xF0*/{	M_RELATIVE,		OP_BEQ,	"BEQ",		2,		1,		&NesCpu::opBEQ_REL,		IT_BRANCH,						},		
	/*0xF1*/{	M_INDIRECTY,	OP_SBC,	"SBC",		5,		1,		&NesCpu::opSBC_INDY	,	IT_INTERNALEXEC,						},		
	/*0xF2*/     BAD_DEF
	/*0xF3*/     BAD_DEF
	/*0xF4*/     BAD_DEF
	/*0xF5*/{	M_ZEROPAGEX,	OP_SBC,	"SBC",		4,		0,		&NesCpu::opSBC_ZPX,		IT_INTERNALEXEC,						},		
	/*0xF6*/{	M_ZEROPAGEX,	OP_INC,	"INC",		6,		0,		&NesCpu::opINC_ZPX,		IT_READMODIFYWRITE,						},		
	/*0xF7*/     BAD_DEF
	/*0xF8*/{	M_IMPLIED,		OP_SED,	"SED",		2,		0,		&NesCpu::opSED_IMP,		IT_SINGLEBYTE,						},	
	/*0xF9*/{	M_ABSOLUTEY,	OP_SBC,	"SBC",		4,		1,		&NesCpu::opSBC_ABSY	,	IT_INTERNALEXEC,						},		
	/*0xFA*/     BAD_DEF
	/*0xFB*/     BAD_DEF
	/*0xFC*/     BAD_DEF
	/*0xFD*/{	M_ABSOLUTEX,	OP_SBC,	"SBC",		4,		1,		&NesCpu::opSBC_ABSX	,	IT_INTERNALEXEC,						},		
	/*0xFE*/{	M_ABSOLUTEX,	OP_INC,	"INC",		7,		0,		&NesCpu::opINC_ABSX,	IT_READMODIFYWRITE,						},		
	/*0xFF*/     BAD_DEF	
	};
}