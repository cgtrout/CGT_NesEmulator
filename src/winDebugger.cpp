//turn off string warnings
#if _MSC_VER > 1000
#pragma warning ( disable : 4996 )
#endif


#include "precompiled.h"
#include "resource1.h"
#include "StringToNumber.h"

#ifndef LIGHT_BUILD

HWND NesEmulator::WinDebugger::debugWnd = NULL;

using namespace NesEmulator;


extern NesMemory	*nesMemory;
NesCpu		*nesCpu;

int ( __stdcall* WinDebugger::fnOldList3Proc )( void );

static const int MAX_MEMDUMP_SIZE = 0xffff;
static ubyte memdumpBuffer[ MAX_MEMDUMP_SIZE ];


//set static variables
int WinDebugger::listPos = 0;
bool WinDebugger::open = false;



extern NesDebugger	*nesDebugger;

/*
==============================================
WinDebugger::WinDebugger()
==============================================
*/
WinDebugger::WinDebugger(): numDebugLines( 32 ) {
	nesDebugger = &FrontEnd::SystemMain::getInstance()->nesMain.nesDebugger;
	nesMemory   = &FrontEnd::SystemMain::getInstance()->nesMain.nesMemory;
	nesCpu		= &FrontEnd::SystemMain::getInstance()->nesMain.nesCpu;
};
/*
==============================================
void WinDebugger::initialize()
==============================================
*/
void WinDebugger::initialize() {
	//subclass the window that was just created
	fnOldList3Proc = ( FARPROC )SetWindowLong( GetDlgItem( debugWnd,IDC_LIST3 ), GWL_WNDPROC, ( DWORD )WinDebugger::List3Proc ); \
	nesDebugger->initialize();
}

/* 
==============================================
void WinDebugger::onEnter( 
==============================================
*/
void WinDebugger::onEnter() {
	fillWindow();
	
	if( GetFocus() == GetDlgItem( debugWnd,IDC_ADD ) ) {
		selectDissasemblerLine( 0 );
		gotoButtonClicked();	
	}
}

/*
==============================================
LONG FAR CALLBACK WinDebugger::List3Proc
==============================================
*/
LONG FAR CALLBACK WinDebugger::List3Proc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) { 
	char listBuf[ 80 ];
	uword address;

	if( open ) {
		switch ( uMsg ) 
		 { 
		case WM_KEYDOWN : 
			if( wParam != VK_RETURN ) {
				//message for item selection is not always fired so this must be done...
				int listPos = SendMessage( GetDlgItem( debugWnd,IDC_LIST3 ), LB_GETCURSEL, 0, 0 ); 
				SendMessage( GetDlgItem( debugWnd,IDC_LIST3 ), ( UINT )LB_GETTEXT, listPos, ( LPARAM )listBuf );  
				address = parseAddressFromText( listBuf );
				nesDebugger->setSelectedAddress( address );	
			
				switch( wParam ) {
				case VK_UP:
					if( listPos == 0 ) {
						//fill list buffer
						address = nesDebugger->findPreviousInstructionLocation( address );
						if( address != 0 ) {
							nesDebugger->setRenderPos( address );
							
							//select first item in list
							selectDissasemblerLine( 0 );
						}
					}
					break;
				case VK_DOWN:
					int listSize = SendMessage( GetDlgItem( debugWnd,IDC_LIST3 ), ( UINT )LB_GETCOUNT, 0, 0 );  
					if( listPos == ( listSize-1 ) ) {
						SendMessage( GetDlgItem( debugWnd,IDC_LIST3 ), ( UINT )LB_GETTEXT, 0, ( LPARAM )listBuf );  
						address = parseAddressFromText( listBuf );
						address = nesDebugger->findNextInstructionLocation( address );
						nesDebugger->setRenderPos( address );
						selectDissasemblerLine( listSize - 1 );
					}
					break;
				}
			}
		}
	}
	return( CallWindowProc( ( WNDPROC )fnOldList3Proc, hWnd, uMsg, wParam, lParam ) ); 
}

/*
==============================================
WinDebugger::debugDlgProc
==============================================
*/
BOOL CALLBACK WinDebugger::debugDlgProc( HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam ) {	
	static HFONT hfont;
	char listBuf[ 80 ];
	uword address;
	
	switch( Message )	 {
		case WM_KEYDOWN:
			break;
		case WM_INITDIALOG: 
			//set font for main disassembler output
			hfont=( HFONT )GetStockObject( ANSI_FIXED_FONT ); 
			SendDlgItemMessage( hwnd,IDC_LIST3,WM_SETFONT, ( WPARAM )hfont, MAKELPARAM( TRUE,0 ) ); 
			
			//hfont = CreateFontIndirect( &lf );
			SendDlgItemMessage( hwnd,IDC_MEMORYDUMPBOX,WM_SETFONT, ( WPARAM )hfont, 0 ); 
		
			//limit text that can be entered by user in address input boxes
			SendDlgItemMessage( hwnd, IDC_WA1, EM_SETLIMITTEXT, 4, 0 );
			SendDlgItemMessage( hwnd, IDC_WA2, EM_SETLIMITTEXT, 4, 0 );
			SendDlgItemMessage( hwnd, IDC_WA3, EM_SETLIMITTEXT, 4, 0 );
			SendDlgItemMessage( hwnd, IDC_ADD, EM_SETLIMITTEXT, 4, 0 );

			SendDlgItemMessage( hwnd, IDC_DUMPSIZE, EM_SETLIMITTEXT, 4, 0 );
			SendDlgItemMessage( hwnd, IDC_DUMPADDRESS, EM_SETLIMITTEXT, 4, 0 );

			SendDlgItemMessage( hwnd, IDC_DUMPSIZE, WM_SETTEXT, 0, ( LPARAM )"368" );  
			SendDlgItemMessage( hwnd, IDC_DUMPADDRESS, WM_SETTEXT, 0, ( LPARAM )"0" );  
			
			SendDlgItemMessage( hwnd, IDC_CPUDUMP, BM_SETCHECK, BST_CHECKED, 0 );
			
			break;	
		case WM_CLOSE:
			nesDebugger->turnOffSingleStepMode();
			break;
		case WM_COMMAND: 
			switch ( HIWORD( wParam ) ) {
			case LBN_SELCHANGE: 
				listPos = SendMessage( GetDlgItem( debugWnd,IDC_LIST3 ), LB_GETCURSEL, 0, 0 ); 
				SendMessage( GetDlgItem( debugWnd,IDC_LIST3 ), ( UINT )LB_GETTEXT, listPos, ( LPARAM )listBuf );  
				address = parseAddressFromText( listBuf );
				nesDebugger->setSelectedAddress( address );
				break;
			case BN_CLICKED:
				switch( LOWORD( wParam ) ) {
				case IDC_STEP:
					nesDebugger->singleStepRequest();
					selectDissasemblerLine( 0 );
					loadMemoryDump();
					break;
				//user selects this to move debug code view to selected address
				case IDC_GOTOADDRESS:
					selectDissasemblerLine( 0 );
					gotoButtonClicked();					
					break;
				//retrieves the memory dump specified by the user and displays it to the window
				case IDC_GETDUMPBUTTON:
					loadMemoryDump();
					break;
				//hides or shows the memdump window
				case IDC_SHOWMEMDUMP:
					break;
				case IDC_RUN:
					nesDebugger->turnOffSingleStepMode();
				}
			}
		case WM_DESTROY:
			DeleteObject( hfont );
			break;
		
		default:
			return FALSE;
	}
	return TRUE;
}

int WinDebugger::parseAddress( int controlId ) {
	int size;
	char addressText[ 5 ];
	char hexValString[ 7 ] = "0x";
	//char buf[ 16 ];
	char error[ 20 ];
	bool isError = false;
	uword address;

	size = SendMessage( GetDlgItem( debugWnd, controlId ), WM_GETTEXT, 5, ( LPARAM )addressText );
	if( size > 0 ) {
		//convert to integer
		sprintf( hexValString, "0x" );
		strcat( hexValString, addressText );
		try {
			address = convertStrToInt( hexValString );
			return address;
		}
		catch( StringToNumberException ) {
			sprintf( error, "Invalid address" );
			isError = true;
		}
	}
	else if ( size == 0 ) {
		sprintf( error, "No address entered" );
		isError = true;
	}
	else {
		sprintf( error, "Invalid address" );
		isError = true;		
	}
	if( isError ) {
		MessageBox( debugWnd, error, "Error", MB_OK ); 
		return -1;
	}
	//never gets here
	return -2;
}

void WinDebugger::gotoButtonClicked() {
	int gotoAddress;

	gotoAddress = parseAddress( IDC_ADD );
	if( gotoAddress != -1 ) {
		nesDebugger->setRenderPos( gotoAddress );	
	}
}

/*
==============================================
void WinDebugger::fillWindow()
==============================================
*/
void WinDebugger::fillWindow() {
	static char buf[ 64 ];
	//registers
		
	//these next lines simply convert registers into string form and send them to windows form
	//a register
	SendMessage( GetDlgItem( debugWnd,IDC_AREG ), WM_SETTEXT, 0, ( LPARAM )ubyteToString( nesCpu->getAReg(), true ).c_str() );  

	//x register
	SendMessage( GetDlgItem( debugWnd,IDC_XREG ), WM_SETTEXT, 0, ( LPARAM )ubyteToString( nesCpu->getXReg(), true ).c_str() );

	//y register
	SendMessage( GetDlgItem( debugWnd,IDC_YREG ), WM_SETTEXT, 0, ( LPARAM )ubyteToString( nesCpu->getYReg(), true ).c_str() ); 

	//PC
	SendMessage( GetDlgItem( debugWnd,IDC_PC ), WM_SETTEXT, 0, ( LPARAM )uwordToString( nesCpu->getPC(), true ).c_str() );  

	//SP
	SendMessage( GetDlgItem( debugWnd,IDC_SP ), WM_SETTEXT, 0, ( LPARAM )uwordToString( nesCpu->getSP(), true ).c_str() );  

	//flags
	SendMessage( GetDlgItem( debugWnd,IDC_CARRY ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( debugWnd,IDC_ZERO ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( debugWnd,IDC_INTERRUPT ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( debugWnd,IDC_BREAKPOINT ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( debugWnd,IDC_OVERFLOW ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( debugWnd,IDC_NEGATIVE ), BM_SETCHECK, BST_UNCHECKED, 0 );
	
	if( nesCpu->getCarry() ) {
		SendMessage( GetDlgItem( debugWnd,IDC_CARRY ), BM_SETCHECK, BST_CHECKED, 0 );
	}
	if( nesCpu->getZero() ) {
		SendMessage( GetDlgItem( debugWnd,IDC_ZERO ), BM_SETCHECK, BST_CHECKED, 0 );
	}
	if( nesCpu->getInterruptDisable() ) {
		SendMessage( GetDlgItem( debugWnd,IDC_INTERRUPT ), BM_SETCHECK, BST_CHECKED, 0 );
	}
	if( nesCpu->getBreakPoint() ) {
		SendMessage( GetDlgItem( debugWnd,IDC_BREAKPOINT ), BM_SETCHECK, BST_CHECKED, 0 );
	}
	if( nesCpu->getOverflow() ) {
		SendMessage( GetDlgItem( debugWnd,IDC_OVERFLOW ), BM_SETCHECK, BST_CHECKED, 0 );
	}
	if( nesCpu->getNegative() ) {
		SendMessage( GetDlgItem( debugWnd,IDC_NEGATIVE ), BM_SETCHECK, BST_CHECKED, 0 );
	}
		
	//fill watch pane
	fillWatch( IDC_WA1, IDC_WV1 );
	fillWatch( IDC_WA2, IDC_WV2 );
	fillWatch( IDC_WA3, IDC_WV3 );

	loadMemoryDump();
}

void WinDebugger::loadMemoryDump() {
	MemoryDumper md;

	//how many dumps should be on each line of the printout
	static const int dumpsPerLine = 16;
	
	//get address 
	uword address;
	address = parseAddress( IDC_DUMPADDRESS );
	if( address == -1 ) {
		return;
	}
	
	//get memory dump size
	BOOL noError;
	int size = GetDlgItemInt( debugWnd, IDC_DUMPSIZE, &noError, FALSE ); 

	//determine dump type
	int memDumpType;
	if( IsDlgButtonChecked( debugWnd, IDC_CPUDUMP ) == BST_CHECKED ) {
		memDumpType = md.MEMDUMPTYPE_MAIN;
	}
	else {
		memDumpType = md.MEMDUMPTYPE_PPU;
	}

	try {
		//grab dump
		md.getMemoryDump( memDumpType, memdumpBuffer, address, size );
	}
	catch ( NesMemoryException ) {		
		return;
	}

	//print header
	static std::string dumpstr;
	dumpstr = "       ";
	char t[ 4 ];
	//loop through to 
	for( int i = 0; i < dumpsPerLine; i++ ) {
		sprintf( t, "0%x ", i );
		dumpstr += t;
	}
	dumpstr += "\r\n\r\n";
	
	//get formatted string form of dump
	dumpstr += md.formatDump( memdumpBuffer, address, size, dumpsPerLine );

	//output dump to memorydumpbox
	SetDlgItemText( debugWnd, IDC_MEMORYDUMPBOX, dumpstr.c_str() );
}
/*
==============================================
void WinDebugger::fillWatch( int watchToGet, int watchToFill )

fills watch windows with data
==============================================
*/
void WinDebugger::fillWatch( int watchToGet, int watchToFill ) {
	char hexValString[ 7 ] = "0x";
	char watchBuffer[ 5 ] = ""; 
//	char buf[ 16 ];
	int watchVal;
	int size;
	
	size = SendMessage( GetDlgItem( debugWnd, watchToGet ), WM_GETTEXT, 5, ( LPARAM )watchBuffer );

	if( size > 1 ) {
		//convert to integer
		sprintf( hexValString, "0x" );
		strcat( hexValString, watchBuffer );
		try {
			watchVal = convertStrToInt( hexValString );
			uword address = (uword)convertStrToInt( hexValString );
			ubyte val = nesMemory->getMemory( address );
			std::string vals = ubyteToString( val, true );
			SendMessage( GetDlgItem( debugWnd,watchToFill ), WM_SETTEXT, 0, ( LPARAM )vals.c_str() );  	 
		}
		catch( StringToNumberException ) {
			MessageBox( debugWnd, "Invalid value in watch", "Watch window", 0 ) ;
		}		
	}
}

/*
==============================================
uword WinDebugger::parseAddressFromText( char *text )
==============================================
*/
uword WinDebugger::parseAddressFromText( char *text ) {
	char *ptr = text;
	static char numStr[ 7 ];
	numStr[ 0 ] = '0';
	numStr[ 1 ] = 'x';
	
	//move ahead in string until first '$' is found
	while( *ptr != '$' ) {
		ptr++;
	}
	ptr++;
	
	//parse out next 4 chars
	int x;
	for( x = 2; x < 6; x++ ) {
		numStr[ x ] = *ptr;
		ptr++;
	}

	numStr[ x ] = '\0';

	//convert to uword
	uword retVal = convertStrToInt( numStr );

	return retVal;
}
/*
==============================================
void WinDebugger::hideWindow()
==============================================
*/
void WinDebugger::hideWindow() {
	if( debugWnd != NULL ) {
		ShowWindow( debugWnd, SW_HIDE );
		open = false;
	}
}
/*
==============================================
void WinDebugger::showWindow()
==============================================
*/
void WinDebugger::showWindow() {
	if( debugWnd != NULL ) {
		ShowWindow( debugWnd, SW_SHOW );
		open = true;
		//SetFocus( debugWnd );
		//EnableWindow( debugWnd, TRUE );
		//SetForegroundWindow( debugWnd );
	}
}
/*
==============================================
void WinDebugger::loadText( char *text ) {
==============================================
*/
void WinDebugger::loadDisassemblyWindow( char *text ) {	
	static int length;
	static int pos, wordPos;
	static char subString[ 64 ];
	char *strPtr = &text[ 0 ];

	length = strlen( text );
	pos = 0;
	wordPos = 0;

	int origPos = SendMessage( GetDlgItem( debugWnd,IDC_LIST3 ), LB_GETCURSEL, 0, 0 ); 

	//clear list
	//SendMessage( GetDlgItem( debugWnd, IDC_LIST3 ), LB_SETCURSEL, -1, 0 );
	SendDlgItemMessage( debugWnd, IDC_LIST3, LB_RESETCONTENT, 0, 0 );

	//int listSize = SendMessage( GetDlgItem( debugWnd,IDC_LIST3 ), ( UINT )LB_GETCOUNT, 0, 0 );  

	//parse out each line of instruction ( and send it to list ) using delimiter '_'
	while( pos < length ) {
		if( *strPtr == '_' ) {
			//create substring to print
            strncpy( subString, &text[ wordPos ], pos - wordPos );
			subString[ pos - wordPos ] = '\0';
			SendDlgItemMessage( debugWnd, IDC_LIST3, LB_ADDSTRING, 0, ( LPARAM )subString );
			wordPos = ++pos;
			
			//skip past delemeter
			strPtr++;
		}
		strPtr++;
		pos++;
	} 
	
	selectDissasemblerLine( origPos );
}

//TODO bounds checking
void WinDebugger::selectDissasemblerLine( int op ) {
	//select first item in list
	int error = SendMessage( GetDlgItem( debugWnd, IDC_LIST3 ), LB_SETCURSEL, op, 0 );
}

#endif	//!LIGHT_BUILD