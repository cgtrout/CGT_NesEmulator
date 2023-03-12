#include "precompiled.h"
#include "imgui/imgui.h"

using namespace NesEmulator;

//disable string warning
#if _MSC_VER > 1000
	#pragma warning ( disable : 4996 )
#endif

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
	numBreakPoints( 0 ),
	justInSingleStepMode( false ),
	renderPos( 0 ),
	selectedPos( 0 ),
	showWindow( false ),
	debugLines( ),
	selectedAddress (0)
{
	for ( int i = 0; i < MAX_BREAKPOINTS; i++ ) {
		breakPoints[ i ] = 0;
	}
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
	//singleStepMode = true;
    justInSingleStepMode = false;
    numBreakPoints = 0;
	
    //clear all breakpoints
	for( int x = 0; x < MAX_BREAKPOINTS; x++ ) {
		breakPoints[ x ] = 0;
	}
}

/*
==============================================
NesDebugger::draw()
==============================================
*/
void NesDebugger::draw( ) {
	if ( showWindow == false ) {
		return;
	}

	ImGui::Begin( "Debugger Window", &this->showWindow, ImGuiWindowFlags_MenuBar );
	if ( ImGui::BeginMenuBar( ) )
	{
		if ( ImGui::BeginMenu( "File" ) )
		{
			if ( ImGui::MenuItem( "Close", "Ctrl+W" ) ) { 
				this->showWindow = false; 
			}
			ImGui::EndMenu( );
		}
		ImGui::EndMenuBar( );
	}

	// Display contents in a scrolling region
	ImGui::TextColored( ImVec4( 1, 1, 1, 1 ), "Step Debugger" );
		ImGui::BeginChild( "Scrolling" );
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
	ImGui::End( );
}


bool NesDebugger::isOpen() {
	return showWindow;
}

void NesDebugger::onEnter() {
	//winDebugger.onEnter();
	throw CgtException( "Warning", "unimplemented", true );
}

void NesDebugger::selectDissasemblerLine( int line ) {
	//winDebugger.selectDissasemblerLine( line );
	throw CgtException( "Warning", "unimplemented", true );
}

void NesDebugger::setRenderPos( uword val ) {
	renderPos = val;
	updateDebugger();
}

//TODO very slow algorithm
bool NesDebugger::isBreakPointAt( uword address ) {
	int bpFound = 0;	//breakpoints found
	for( int x = 0; x < MAX_BREAKPOINTS, bpFound < numBreakPoints; x++ ) {
		if( breakPoints[ x ] != 0 ) {
			bpFound++;
		}
		if( breakPoints[ x ] == address ) {
			return true;
		}
	}
	return false;	
}
//returns true if breakpoint was added
bool NesDebugger::addBreakPoint( uword address ) {
	int bpFound = 0;	//breakpoints found
	
	if( isBreakPointAt( address ) ) {
		removeBreakPoint( address );
		loadWindowText();;
		return false;
	}

	for( int x = 0; x < MAX_BREAKPOINTS; x++ ) {
		if( breakPoints[ x ] != 0 ) {
			bpFound++;
		}		
		//find first empty spot in array
		else if( breakPoints[ x ] == 0 ) {
			breakPoints[ x ] = address;
			numBreakPoints++;
			loadWindowText();;
			return true;
		}
	}
	return false;	//no empty spot found
}

bool NesDebugger::removeBreakPoint( uword address ) {
	int bpFound = 0;	//breakpoints found
	for( int x = 0; x < MAX_BREAKPOINTS, bpFound < numBreakPoints; x++ ) {
		if( breakPoints[ x ] != 0 ) {
			bpFound++;
		}
		if( breakPoints[ x ] == address ) {
			breakPoints[ x ] = 0;
			numBreakPoints--;
			return true;
		}
	}
	return false;  //breakpoint was not found
}
/*
==============================================
void NesDebugger::loadWindowText
==============================================
*/
void NesDebugger::loadWindowText() {
	static uword lastAddress = 0;

	if( renderPos != lastAddress ) {
		//strcpy( buffer, buildOutputString( renderPos, winDebugger.numDebugLines ).c_str() );
		//winDebugger.loadDisassemblyWindow( buffer );
		buildDissassemblerLines( renderPos, numDebugLines );
		lastAddress = renderPos;
	}
	//throw CgtException( "Warning", "unimplemented", true );
}
/*
==============================================
void NesDebugger::updateDebugger()
==============================================
*/
void NesDebugger::updateDebugger() {
	loadWindowText();
	//winDebugger.fillWindow();
	//throw CgtException( "Warning", "unimplemented", true );
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
	showWindow = true;
	
    //winDebugger.fillWindow();
    updateDebugger();
	
	//winDebugger.selectDissasemblerLine( 0 );
	//throw CgtException( "Warning", "unimplemented", true );
}

/* 
==============================================
void NesDebugger::turnOffSingleStepMode()
==============================================
*/
void NesDebugger::turnOffSingleStepMode() {
	singleStepMode = false; 
	justInSingleStepMode = true;
	showWindow = false;
	//winDebugger.hideWindow();
	//throw CgtException( "Warning", "unimplemented", true );
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
char *NesDebugger::buildDebugLine( uword address, const opcodeLookUpTableEntry *l, ubyte byte1val, ubyte byte2val )
==============================================
*/
char *NesDebugger::buildDebugLine( uword address, const opcodeLookUpTableEntry *l, ubyte opcode, ubyte byte1val, ubyte byte2val ) {
	char byte1[ 3 ];
	char byte2[ 3 ];
	char symbol[ 3 ];
	char end[ 4 ];
	char addressStr[ 8 ];  
	char opString[ 3 ];

	bool usebyte1;
	bool usebyte2;
	bool useend; //use string after bytes are printed

	static char debugBuffer[ 1000 ];
	debugBuffer[ 0 ] = '\0';
	usebyte1 = false;
	usebyte2 = false;
	useend = false;

	sprintf( byte1, "%X", byte1val );
	sprintf( byte2, "%X", byte2val );

	//pad with '0' if length is one
	if( strlen( byte1 ) == 1 ) {
		byte1[ 1 ] = byte1[ 0 ];
		byte1[ 0 ] = '0';
		byte1[ 2 ] = '\0';
	}
	if( strlen( byte2 ) == 1 ) {
		byte2[ 1 ] = byte2[ 0 ];
		byte2[ 0 ] = '0';
		byte2[ 2 ] = '\0';
	}

	switch( l->mode ) {
	case M_IMMEDIATE:
		usebyte1 = true;
		sprintf( symbol, "#$" ); 
		break;
	case M_ZEROPAGE:
		usebyte1 = true;
		sprintf( symbol, "$" ); 
		break;
	case M_ZEROPAGEX:
		usebyte1 = true;
		sprintf( symbol, "$" ); 
		sprintf( end, ",X" );
		useend = true;
		break;
	case M_ZEROPAGEY:
		usebyte1 = true;
		sprintf( symbol, "$" ); 
		sprintf( end, ",X" );
		useend = true;
		break;
	case M_ABSOLUTE:
		usebyte1 = true;
		usebyte2 = true;
		sprintf( symbol, "$" ); 
		break;
	case M_ABSOLUTEX:
		usebyte1 = true;
		usebyte2 = true;
		sprintf( symbol, "$" ); 
		sprintf( end, ",X" );
		useend = true;
		break;
	case M_ABSOLUTEY:
		usebyte2 = true;
		usebyte1 = true;
		sprintf( symbol, "$" ); 
		sprintf( end, ",Y" );
		useend = true;
		break;
	case M_INDIRECT:
		usebyte1 = true;
		usebyte2 = true;
		sprintf( symbol, "( $" ); 
		sprintf( end, " )" );
		useend = true;
		break;
	case M_INDIRECTX:
		usebyte1 = true;
		sprintf( symbol, "( $" ); 
		sprintf( end, ",X )" );
		useend = true;
		break;
	case M_INDIRECTY:
		usebyte1 = true;
		sprintf( symbol, "( $" ); 
		sprintf( end, " ),Y" );
		useend = true;
		break;
	case M_IMPLIED:
		symbol[ 0 ] = '\0';
		break;
	case M_ACCUMULATOR:
		sprintf( symbol, "A" ); 
		break;
	case M_RELATIVE:
		usebyte1 = true;
		sprintf( symbol, "$" ); 
		break;
	}
	
	//pad to zero
	std::string &convertedAddress = uwordToString( address, false );

	sprintf( addressStr, "$%s: ", convertedAddress.c_str() );

	strcat( debugBuffer, addressStr );
	
	//print data
	//
	sprintf( opString, "%X", opcode );
	
	//pad with '0' if length is one
	if( strlen( opString ) == 1 ) {
		opString[ 1 ] = opString[ 0 ];
		opString[ 0 ] = '0';
		opString[ 2 ] = '\0';
	}
	strcat( debugBuffer, opString );
	strcat( debugBuffer, " " );
	if( usebyte1 ) {
		strcat( debugBuffer, byte1 );
		strcat( debugBuffer, " " );
	}
	if( usebyte2 ) {
		strcat( debugBuffer, byte2 );
		strcat( debugBuffer, "  " );
	}

	//ensure we are at position 16 ( to pad output string so 
	//instructions line up properly )
	int position = strlen( debugBuffer )-1;
	//position in the string that instruction string should start
	static int opPadPosition = 17;		
	
	if( position != opPadPosition ) {
		//determine how many spaces are needed
		int spaceNeeded = opPadPosition - position;
				
		//add spaces
		for( int x = 0; x < spaceNeeded; x++ ) {
			strcat( debugBuffer, " " );
		}
	}

	strcat( debugBuffer, l->syntax );
	strcat( debugBuffer, " " );
	strcat( debugBuffer, symbol );
	if( usebyte2 ) {
		strcat( debugBuffer, byte2 );
	}
	if( usebyte1 ) {
		strcat( debugBuffer, byte1 );
	}
	if( useend ) {
		strcat( debugBuffer, end );
	}
	
	char *charReturn = "\0";
	
	strcat( debugBuffer, charReturn );
	
	return debugBuffer;
}


#endif //LIGHT_BUILD