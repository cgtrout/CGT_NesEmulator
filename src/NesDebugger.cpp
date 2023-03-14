#include "precompiled.h"
#include "imgui/imgui.h"

using namespace NesEmulator;

NesMemory *memory;

#ifndef LIGHT_BUILD
/*
==============================================
NesDebugger::NesDebugger()
==============================================
*/
NesDebugger::NesDebugger( ) :
	singleStepMode( false ),
	doSingleStep( false ),
	justInSingleStepMode( false ),
	renderPos( 0 ),
	selectedPos( 0 ),
	showDebugWindow( false ),
	showMemoryDump( false ),
	debugLines( ),
	selectedAddress (0),
	dumpAddress(0),
	dumpSize(368),
	dumpAddressStr( 5, 0 ), //5 chars long initialized to 0
	memDumpType( MemoryDumper::MEMDUMPTYPE_MAIN )
{
}

/*
==============================================
NesDebugger::~NesDebugger()
==============================================
*/
NesDebugger::~NesDebugger( ) {
}


/*
==============================================
NesDebugger::initialize()
==============================================
*/
void NesDebugger::initialize() {
	memory = &FrontEnd::SystemMain::getInstance( )->nesMain.nesMemory;
	singleStepMode = false;
    justInSingleStepMode = false;
}

/*
==============================================
NesDebugger::draw()
==============================================
*/
void NesDebugger::draw( ) {
	if ( showDebugWindow == false ) {
		return;
	}

	//ImGui::Begin( "Debugger Window", &this->showDebugWindow, ImGuiWindowFlags_MenuBar );
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::Begin( "Debugger Window", &this->showDebugWindow, window_flags );
	/*
	if ( ImGui::BeginMenuBar( ) )
	{
		if ( ImGui::BeginMenu( "File" ) )
		{
			if ( ImGui::MenuItem( "Close", "Ctrl+W" ) ) { 
				this->showDebugWindow = false; 
			}
			ImGui::EndMenu( );
		}
		ImGui::EndMenuBar( );
	}
	*/

	ImGui::TextColored( ImVec4( 1, 1, 1, 1 ), "Step Debugger" );
		ImGui::BeginChild( "Scrolling", ImVec2( ImGui::GetContentRegionAvail( ).x * 0.5f, ImGui::GetContentRegionAvail( ).y ), false, window_flags );
			for ( unsigned int n = 0; n < debugLines.size( ); n++ ) {
				if ( isBreakPointAt( debugLines[ n ].address ) ) {
					ImGui::Text( " + " );
				} else {
					ImGui::Text( "   " );
				}
				ImGui::SameLine( 0.0f, 0.0f );
				if ( ImGui::Selectable( debugLines[ n ].line.c_str( ), selectedPos == n ) ) {
					selectedPos = n;
					selectedAddress = debugLines[ n ].address;
				}
			}
		ImGui::EndChild( );

	ImGui::SameLine( );

	ImGui::BeginChild( "second column" );
	
		ImGui::TextColored( ImVec4( 1.0f, 1.0f, 0.0f, 1.0f ), "A " );
		ImGui::SameLine( );
		ImGui::Text( "0x%02X", nesCpu->getAReg( ) );
	
		ImGui::SameLine( );

		ImGui::TextColored( ImVec4( 1.0f, 1.0f, 0.0f, 1.0f ), "PC " );
		ImGui::SameLine( );
		ImGui::Text( "%04X", nesCpu->getPC( ) );

		ImGui::TextColored( ImVec4( 1.0f, 1.0f, 0.0f, 1.0f ), "X " );
		ImGui::SameLine( );
		ImGui::Text( "0x%02X", nesCpu->getXReg( ) );
	
		ImGui::SameLine( );

		ImGui::TextColored( ImVec4( 1.0f, 1.0f, 0.0f, 1.0f ), "SP " );
		ImGui::SameLine( );
		ImGui::Text( "0x%04X", nesCpu->getSP( ) );
		
		ImGui::TextColored( ImVec4( 1.0f, 1.0f, 0.0f, 1.0f ), "Y " );
		ImGui::SameLine( );
		ImGui::Text( "0x%02X", nesCpu->getYReg( ) );

		ImGui::NewLine( );

		bool carry = nesCpu->getCarry( );
		bool zero = nesCpu->getZero( );
		bool interruptDis = nesCpu->getInterruptDisable( );
		bool breakPoint = nesCpu->getBreakPoint( );
		bool overflow = nesCpu->getOverflow( );
		bool negative = nesCpu->getNegative( );

		ImGui::Checkbox( "Carry", &carry );
		ImGui::Checkbox( "Zero", &zero );
		ImGui::Checkbox( "Interupt Dis.", &interruptDis );
		ImGui::Checkbox( "Breakpoint", &breakPoint );
		ImGui::Checkbox( "Overflow", &overflow );
		ImGui::Checkbox( "Negative", &negative);
		
		ImGui::NewLine( );

		//draw watchboxes
		for ( unsigned int i = 0; i < watchStrings.size( ); i++ ) {
			drawWatchBox( i );
		}

		ImGui::NewLine( );

		ImGui::BeginChild( "buttons" );
			if ( ImGui::Button( "Step" ) ) {
				this->singleStepRequest( );
			}

			ImGui::SameLine( );

			if ( ImGui::Button( "Run" ) ) {
				this->turnOffSingleStepMode( );
			}

			ImGui::SameLine(  );

			if ( ImGui::Button( "Show memory dump" ) ) {
				showMemoryDump = !showMemoryDump;
			}
		ImGui::EndChild( );

	ImGui::EndChild();
	
	ImGui::End( );	


	//show memory dump window
	if ( this->showMemoryDump ) {
		ImGui::Begin( "Memory dump", &this->showMemoryDump, 0 );
			ImGui::PushItemWidth( 50 );
			ImGui::InputText( "Address", &dumpAddressStr[0], 5, ImGuiInputTextFlags_CharsHexadecimal);

			ImGui::SameLine( );

			static bool cpuSet = true;
			static bool ppuSet = false;

			ImGui::BeginListBox( "Memory Selection", ImVec2(100, 40) );
				if ( ImGui::Selectable( "CPU Memory", &cpuSet ) ) {
					memDumpType = MemoryDumper::MEMDUMPTYPE_MAIN;
					cpuSet = true;
					ppuSet = false;
				}
				if ( ImGui::Selectable( "PPU Memory", &ppuSet ) ) {
					memDumpType = MemoryDumper::MEMDUMPTYPE_PPU;
					ppuSet = true;
					cpuSet = false;
				}
			ImGui::EndListBox( );

			ImGui::PopItemWidth( );

			dumpAddress = ( uword )strtol( dumpAddressStr.c_str(), NULL, 16 );
			std::string memoryDumpString = loadMemoryDump( );
			ImGui::Text( memoryDumpString.c_str( ), 500.0f );

		ImGui::End( );
	}
}

std::string NesDebugger::loadMemoryDump(  ) {
	MemoryDumper md;
	std::vector<ubyte> memdumpBuffer(dumpSize) ;

	//how many dumps should be on each line of the printout
	static const int dumpsPerLine = 16;

	//determine dump type
	//int memDumpType;
	//if ( IsDlgButtonChecked( debugWnd, IDC_CPUDUMP ) == BST_CHECKED ) {
	//	memDumpType = md.MEMDUMPTYPE_MAIN;
	//} else {
	//	memDumpType = md.MEMDUMPTYPE_PPU;
	//}

	try {
		//grab dump
		md.getMemoryDump( memDumpType, memdumpBuffer.data(), dumpAddress, dumpSize );
	} catch ( NesMemoryException e ) {
		throw CgtException( "Memory Dump Error", e.getMessage( ), true );
		return "DUMP_ISSUE";
	}

	//print header
	std::string dumpstr;
	dumpstr = "       ";
	char t[ 4 ];
	//loop through to 
	for ( int i = 0; i < dumpsPerLine; i++ ) {
		sprintf( t, "0%x ", i );
		dumpstr += t;
	}
	dumpstr += "\r\n\r\n";

	//get formatted string form of dump
	dumpstr += md.formatDump( memdumpBuffer.data(), dumpAddress, dumpSize, dumpsPerLine );

	return dumpstr;
}


/*
==============================================
NesDebugger::drawWatchBox()
==============================================
*/
void NesEmulator::NesDebugger::drawWatchBox( const int index )
{
	ImGui::PushItemWidth( 50 );
	std::string title{ "Watch" };
	title += '0' + index;
	ImGui::InputText( title.c_str() , watchStrings[index], 5, ImGuiInputTextFlags_CharsHexadecimal );
	ImGui::PopItemWidth( );

	uword watchLoc = ( uword )strtol( watchStrings[ index ], NULL, 16 );

	ImGui::SameLine( );
	ImGui::Text( "0x%02X", memory->getMemory( watchLoc ) );
}


/*
==============================================
NesDebugger::isOpen()
==============================================
*/
bool NesDebugger::isOpen() {
	return showDebugWindow;
}

/*
==============================================
NesDebugger::onEnter()
==============================================
*/
void NesDebugger::onEnter() {
	//winDebugger.onEnter();
	//throw CgtException( "Warning", "unimplemented", true );
}

/*
==============================================
NesDebugger::selectDissasemblerLine()
==============================================
*/
void NesDebugger::selectDissasemblerLine( int line ) {
	selectedPos = line;
}

/*
==============================================
NesDebugger::setRenderPos()
==============================================
*/
void NesDebugger::setRenderPos( uword val ) {
	renderPos = val;
	updateDebugger();
}

/*
==============================================
NesDebugger::isBreakPointAt()
==============================================
*/
bool NesDebugger::isBreakPointAt( uword address ) {
	if ( breakPoints.empty( ) ) {
		return false;
	} else {
		auto find_result = std::find( breakPoints.begin( ), breakPoints.end( ), address );
		return ( find_result != breakPoints.end( ) );
	}
}

/*
==============================================
NesDebugger::addBreakPoint()
==============================================
*/
bool NesDebugger::addBreakPoint( uword address ) {
	int bpFound = 0;	//breakpoints found
	
	if( isBreakPointAt( address ) ) {
		removeBreakPoint( address );
		loadWindowText();
		return false;
	}

	breakPoints.push_back( address );
	return true;	
}


/*
==============================================
NesDebugger::removeBreakPoint
==============================================
*/
bool NesDebugger::removeBreakPoint( uword address ) {
	int bpFound = 0;	//breakpoints found
	auto find_result = std::find( breakPoints.begin( ), breakPoints.end( ), address );

	if ( find_result != breakPoints.end( ) ) { 
		breakPoints.erase( find_result );
		return true;
	} else {
		return false;  //breakpoint was not found
	}
}
/*
==============================================
void NesDebugger::loadWindowText
==============================================
*/
void NesDebugger::loadWindowText() {
	static uword lastAddress = 0;

	if( renderPos != lastAddress ) {
		buildDissassemblerLines( renderPos, numDebugLines );
		lastAddress = renderPos;
	}
}
/*
==============================================
void NesDebugger::updateDebugger()
==============================================
*/
void NesDebugger::updateDebugger() {
	loadWindowText();
}
/*
==============================================
void NesDebugger::setToSingleStepMode()
==============================================
*/
void NesDebugger::setToSingleStepMode( uword address ) {
	singleStepMode = true;
	justInSingleStepMode = true;
	renderPos = address;
	showDebugWindow = true;
	
    updateDebugger();
	
	//winDebugger.selectDissasemblerLine( 0 );
}

/* 
==============================================
void NesDebugger::turnOffSingleStepMode()
==============================================
*/
void NesDebugger::turnOffSingleStepMode() {
	singleStepMode = false; 
	justInSingleStepMode = true;
	showDebugWindow = false;
}

/*
==============================================
uword NesDebugger::findNextInstructionLocation( uword address )
==============================================
*/
uword NesDebugger::findNextInstructionLocation( uword address ) {
	//find length of current op
	opcodeLookUpTableEntry lookupEntry = opcodeLookUpTable[ memory->getMemory( address ) ];
	ubyte length = addModeLengthTable[ lookupEntry.mode ];

	return address + length;
}
/*
==============================================
uword findPreviousInstructionLocation( uword address )

returns the location of the previous instruction to the instruction at "address"
==============================================
*/
uword NesDebugger::findPreviousInstructionLocation( uword address ) {
	uword workAddress = address;
	for( int x = 0; x < 3; x++ ) {
		//if instruction at workaddress is pointing to address...
		if( instructionIsPointingTo( --workAddress, address ) ) {
			//and if instruction at workAddress has a instruction pointing to it...
			//if( isAPreviousValidInst( workAddress ) ) {
			//	return workAddress;
			//}
			return workAddress;
		}
	}
	return 0;		// could not be found
}
/*
==============================================
bool NesDebugger::instructionIsPointingTo

is the given instruction, at 'opAddress', pointing to 'knownAddress'

- returns true if it is a valid location for an address
- returns false if it is not a valid location, or if a invalid op is at 'opAddress'
==============================================
*/
bool NesDebugger::instructionIsPointingTo( uword opAddress, uword knownAddress ) {
	if( opAddress < 0x8000 ) {
		return false;
	}
	//is op at opAddress valid?
	opcodeLookUpTableEntry lookupEntry = opcodeLookUpTable[ memory->getMemory( opAddress ) ];
	if( lookupEntry.op == OP_BAD ) {
		//if not return false
		return false;
	}	

	//calculate length of inst at opAddress
	ubyte length = addModeLengthTable[ lookupEntry.mode ];

	//does opAddress + length == knownAddress?
	if( ( opAddress + length ) == knownAddress ) {
		//if so return true
		return true;
	}
	//else return false
	else {
		return false;
	}
}
/*
==============================================
bool NesDebugger::isAPreviousValidInst( uword opAddress )

determines if there is a previous instruction pointing to the current inst at 'opAddress'
- returns true if this is a valid instruction
- returns false if it not valid
==============================================
*/
bool NesDebugger::isAPreviousValidInst( uword opAddress ) {
	uword workAddress = opAddress;

	if( opAddress == 0x8000 || opAddress == 0x8001 ) {
		return true;
	}
	for( int x = 0; x < 3; x++ ) {
		if( instructionIsPointingTo( --workAddress, opAddress ) ) {
			return true;
		}
	}
	return false;
}
/*
==============================================
NesDebugger::buildOutputString( uword startAddress, const int length )
- builds debug output of length "length" starting at first valid opcode found at address "startAddress"
- sets vector of output strings
==============================================
*/
void NesDebugger::buildDissassemblerLines( uword startAddress, const int length ) {
	int linesMade;
	uword currAddress = 0;

	int opLength;
	ubyte b1, b2;
	//uword fullb;
	debugLines.clear();
	linesMade = 0;
	
	//starting at address at pos startAddress move foward until first valid opcode is found
	currAddress = startAddress;
	while( true ) {
		opcodeLookUpTableEntry lookupEntry = opcodeLookUpTable[ memory->getMemory( currAddress ) ];
		
		//is this a valid instruction
		if( lookupEntry.op != OP_BAD ) {
			//look ahead length of inst
			opLength = addModeLengthTable[ lookupEntry.mode ];
			opcodeLookUpTableEntry tempEntry =  opcodeLookUpTable[ memory->getMemory( currAddress + opLength ) ];
			
			break;
			//if( tempEntry.op != OP_BAD ) {
			//	break;
			//}
		}
		currAddress++;
	}

	while( linesMade < length )
	 {
		const opcodeLookUpTableEntry *lookupEntry = &opcodeLookUpTable[ memory->getMemory( currAddress ) ];
		b1 = memory->getMemory( currAddress+1 );
		b2 = memory->getMemory( currAddress+2 );
		//fullb = ( b2 << 8 ) + b1;
		opLength = addModeLengthTable[ lookupEntry->mode ];		
		
		//build debugLinesline
		debugLines.push_back( DebugLine( currAddress, buildDebugLine( currAddress, lookupEntry, memory->getMemory( currAddress ), b1, b2 ) ));
		
		//increment currAddress to point to next opcode
		currAddress += opLength;
		linesMade++;
	}
}

/* 
==============================================
NesDebugger::buildDebugLine
==============================================
*/
std::string NesDebugger::buildDebugLine( uword address, const opcodeLookUpTableEntry* opcodeLookup, ubyte opcode, ubyte byte1val, ubyte byte2val )
{
	std::ostringstream oss;
	std::string byte1 = uwordToString( byte1val, 2 );
	std::string byte2 = uwordToString( byte2val, 2 );
	std::string symbol;
	std::string end;
	std::string addressStr = "$" + uwordToString( address, 4, false ) + ": ";

	bool usebyte1 = false;
	bool usebyte2 = false;
	bool useend = false;

	switch ( opcodeLookup->mode )
	{
		case M_IMMEDIATE:
			usebyte1 = true;
			symbol = "#$";
			break;
		case M_ZEROPAGE:
			usebyte1 = true;
			symbol = "$";
			break;
		case M_ZEROPAGEX:
			usebyte1 = true;
			symbol = "$";
			end = ",X";
			useend = true;
			break;
		case M_ZEROPAGEY:
			usebyte1 = true;
			symbol = "$";
			end = ",Y";
			useend = true;
			break;
		case M_ABSOLUTE:
			usebyte1 = true;
			usebyte2 = true;
			symbol = "$";
			break;
		case M_ABSOLUTEX:
			usebyte1 = true;
			usebyte2 = true;
			symbol = "$";
			end = ",X";
			useend = true;
			break;
		case M_ABSOLUTEY:
			usebyte2 = true;
			usebyte1 = true;
			symbol = "$";
			end = ",Y";
			useend = true;
			break;
		case M_INDIRECT:
			usebyte1 = true;
			usebyte2 = true;
			symbol = "($";
			end = ")";
			useend = true;
			break;
		case M_INDIRECTX:
			usebyte1 = true;
			symbol = "($";
			end = ",X)";
			useend = true;
			break;
		case M_INDIRECTY:
			usebyte1 = true;
			symbol = "($";
			end = "),Y";
			useend = true;
			break;
		case M_IMPLIED:
			symbol = "";
			break;
		case M_ACCUMULATOR:
			symbol = "A";
			break;
		case M_RELATIVE:
			usebyte1 = true;
			symbol = "$";
			break;
	}

	oss << addressStr;

	oss << uwordToString( opcode, 2 ) << " ";
	if ( usebyte1 )
	{
		oss << byte1 << " ";
	}
	if ( usebyte2 )
	{
		oss << byte2 << "  ";
	}

	// Pad to position 16
	auto position = oss.tellp( );
	const int opPadPosition = 17;
	if ( position != opPadPosition )
	{
		oss << std::setw( opPadPosition - position ) << "";
	}

	oss << opcodeLookup->syntax << " " << symbol;
	if ( usebyte2 )
	{
		oss << byte2;
	}
	if ( usebyte1 )
	{
		oss << byte1;
	}
	if ( useend )
	{
		oss << end;
	}

	return oss.str( );
}


#endif //LIGHT_BUILD