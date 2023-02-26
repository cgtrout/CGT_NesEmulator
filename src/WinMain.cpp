#include "precompiled.h"

#include "WinMain.h"

#ifndef LIGHT_BUILD
  #include "winDebugger.h"
#endif

#include "Console.h"
#include "resource1.h"

bool	active=TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen=TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default

HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context

HINSTANCE	hInstance;		// Holds The Instance Of The Application

LRESULT CALLBACK WndProc( 	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam )			// Additional Message Information
 {
	switch ( uMsg )									// Check For Windows Messages
	 {
		case WM_COMMAND:
			break;
		case WM_KEYDOWN:
			input->setKeyDown( ( int )wParam );			
			break;
		case WM_KEYUP:
			input->setKeyUp( ( int )wParam );
			break;
		case WM_CHAR:
			//exclude backspace ,tilde, enter, and escape keys from being added to keybuffer
			//TODO change to define labels
			if( ( char )wParam != 8 
				&& char( wParam ) != '`' 
				&& char( wParam ) != '~' 
				&& char( wParam ) != 0x0d
				&& char( wParam ) != 0x1b
				&& char( wParam ) != KB_TAB )	 {
				input->addKeyToBuffer( wParam );
			}
			break;
		case WM_MOVE:	
			if( input != NULL ) {
				int xPos = ( int )( short ) LOWORD( lParam );    // horizontal position 
				int yPos = ( int )( short ) HIWORD( lParam );    // vertical position 
 
				input->setScreenX( xPos );
				input->setScreenY( yPos );
			}
			break;
		case WM_MOUSEMOVE: 
			input->setMouseX( LOWORD( lParam ) );  
			input->setMouseY( HIWORD( lParam ) );  
			break;
		case WM_LBUTTONDOWN:
		 {
			input->setMouseLeftDown( true );
			input->setMouseLeftDownLastFrame( true );
			
			break;
		}
		case WM_LBUTTONUP:
			if( input->wasMouseLeftDownLastFrame() )
			 {
				input->setMouseLeftUp( true );
				input->setMouseLeftDownLastFrame( false );
			}
			break;
		case WM_RBUTTONDOWN:
			input->setMouseRightDown( true );
			break;
		case WM_RBUTTONUP:
			input->setMouseRightUp( true );
			break;
		case WM_ACTIVATE:							
			if ( !HIWORD( wParam ) )					
			 {
				active=TRUE;						
			}
			else
			 {
				active=FALSE;						
			}

			return 0;								
		case WM_SYSCOMMAND:							
			switch ( wParam )							// Check System Calls
			 {
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;
		case WM_CLOSE:								
			PostQuitMessage( 0 );						
			return 0;								
		case WM_SIZE:								// Resize The OpenGL Window
			ReSizeGLScene( LOWORD( lParam ),HIWORD( lParam ) );  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc( hWnd,uMsg,wParam,lParam );
}

using namespace FrontEnd;

#define XRES 800
#define YRES 600

Console::ConsoleVariable< bool > capFrameRate (  
	/*start val*/	false, 
	/*name*/		"capFrameRate", 
	/*description*/	"Caps frame rate to 60hz if set to true",
	/*save?*/		SAVE_TO_FILE );

void errorExit( LPTSTR lpszFunction );

//declare and initialize windows sound system
#include "WinSoundSystem.h"
WinSoundSystem winSoundSystem;

int WINAPI WinMain( 	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow )			// Window Show State
 {
	MSG		msg;								// Windows Message Structure
	BOOL	done=FALSE;								// Bool Variable To Exit Loop

	// Ask The User Which Screen Mode They Prefer
	//if ( MessageBox( NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION )==IDNO )
	// {
		fullscreen=FALSE;							// Windowed Mode
	//}

	// Create OpenGL Window
	if ( !CreateGLWindow( "Corry T Nes Emulator",XRES,YRES,32,fullscreen ) )
	 {
		return 0;									// Quit If Window Was Not Created
	}

	//enable heap debug
	_CrtSetDbgFlag( _CRTDBG_CHECK_ALWAYS_DF );
	
	systemMain = SystemMain::getInstance();
	
  #ifndef LIGHT_BUILD
	//create windebugger window
	winDebugger = &systemMain->nesMain.nesDebugger.winDebugger;
 
	
	BOOL ( CALLBACK *funcPtr )( HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam ) = &WinDebugger::debugDlgProc;
	HWND dhwnd = CreateDialog( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDD_DEBUGGER ),hWnd, funcPtr );
	if( dhwnd == NULL ) {
		//something bad happened - get last error
		errorExit( "Creating windows debug window" );
	}

	winDebugger->setDebugWnd( dhwnd );
	winDebugger->initialize();
   #endif

	//initialize anything that needs to be initialized
	//this is to control initialization on objects to avoid problems with singleton initialization
	
	consoleSystem = &systemMain->consoleSystem;
	systemMain->initialize( );
	systemMain->renderer.setRes( XRES, YRES );
	
	//assign soundsystem
	systemMain->soundSystem = &winSoundSystem;
	systemMain->soundSystem->assignNesSoundBuffer( systemMain->nesMain.nesApu.getNesSoundBuffer() );

	systemMain->start();
	
	//set frame rate counter pos based on screen size
	systemMain->frameCounter.setX( XRES - 87 );
	systemMain->frameCounter.setY( YRES - 20 );
	
	input = FrontEnd::InputSystem::Input::getInstance();
	timer = Timer::getInstance();
	
	systemMain->fpsTimer.clearTimer();
	systemMain->timeProfiler.startFrame();

	//allow sleep to sleep for 1ms
	MMRESULT res = timeBeginPeriod( 1 );
	if( res == TIMERR_NOCANDO ) {
		_log->Write( "Error setting timer resolution to 1ms" );
	}

	const float FRAME_TIME = 0.01663926f;
	
	systemMain->consoleSystem.variables.addBoolVariable( &capFrameRate );
	float elapsedTime = 0.0f;
	float currTime = 0.0f;
	float lastTime = 0.0f;
	
	//debug testing values
	//DWORD debCurrTime = 0;
	//DWORD debElapsedTime = 0;
	
	bool freshFrame = true;

	while( !done ) {		
		//debCurrTime = timeGetTime();
		//debCurrTime = timer->getAbsoluteTime();
		
		//reset timers
		timer->reset();
		currTime = timer->getCurrTime();	

		systemMain->timeProfiler.startFrame();

		elapsedTime = 0;
		freshFrame = true;

		//cap to 60.098814 frames per second
		while( elapsedTime < FRAME_TIME ) {

			if ( PeekMessage( &msg,NULL,0,0,PM_REMOVE ) )	// Is There A Message Waiting?
			{
				//if( freshFrame ) {
					//handle dialog messages
				  #ifndef LIGHT_BUILD
					if( IsDialogMessage( winDebugger->getDebugWnd(), &msg )!=0 ) {
						switch( msg.message ) {
						case WM_KEYDOWN:
							input->setKeyDown( ( int )msg.wParam );			
							break;
						case WM_KEYUP:
							input->setKeyUp( ( int )msg.wParam );
							break;
						}
						continue;
					}
				  #endif
				//}
				//if debug window is the active window intercept any key down messages and dispatch
				
				if ( msg.message==WM_QUIT )				// Have We Received A Quit Message?
				{
					done=TRUE;							// If So done=TRUE
				}
				else									// If Not, Deal With Window Messages
				 {
					TranslateMessage( &msg );				// Translate The Message
					DispatchMessage( &msg );				// Dispatch The Message
				}
			}
			else										// If There Are No Messages
			{
				//only update these once per frame
				if( freshFrame ) {
					systemMain->runFrame();
					freshFrame = false;
					input->clear();
					SwapBuffers( hDC );					// Swap Buffers ( Double Buffering )
					
					systemMain->timeProfiler.stopFrame();
					systemMain->guiTimeProfiler.setReportString( systemMain->timeProfiler.getSectionReport() );
				}
											
				if( systemMain->quitRequestSubmitted() ) {
					done = TRUE;
				}
			}

			//only run through loop once if we are not capping the framerate
			if( !capFrameRate ) {
				break;
			}

			elapsedTime = timer->getCurrTime() - currTime;
			
			//if( ( ( FRAME_TIME - elapsedTime ) * 1000 ) < 14 ) {
			//	Sleep( 1 );
			//}
		}

		elapsedTime = timer->getCurrTime() - currTime;

		//done frame - finish off frame
		systemMain->fpsTimer.updateTimer( elapsedTime );
				
		//debElapsedTime = timeGetTime() - debCurrTime;
		//debElapsedTime = timer->getAbsoluteTime() - debCurrTime;

		elapsedTime = timer->getCurrTime() - currTime;

		_log->Write( "end frame time = %f", elapsedTime );
		//_log->Write( "debug elapsed time = %f", (float)(debElapsedTime)/1000);

	}

	//delete system;

	//allow sleep to sleep for 1ms
	timeEndPeriod( 1 );

	// Shutdown
	KillGLWindow();									// Kill The Window
	return ( msg.wParam );							// Exit The Program
}

void errorExit( LPTSTR lpszFunction ) { 
    TCHAR szBuf[ 80 ]; 
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
        ( LPTSTR ) &lpMsgBuf,
        0, NULL );

    wsprintf( szBuf, 
        "%s failed with error %d: %s", 
        lpszFunction, dw, lpMsgBuf ); 
 
    MessageBox( NULL, szBuf, "Error", MB_OK ); 

    LocalFree( lpMsgBuf );
    ExitProcess( dw ); 
}


GLvoid ReSizeGLScene( GLsizei width, GLsizei height )		// Resize And Initialize The GL Window
 {
	if ( height==0 )										// Prevent A Divide By Zero By
	 {
		height=1;										// Making Height Equal One
	}

	glViewport( 0,0,width,height );						// Reset The Current Viewport

	glMatrixMode( GL_PROJECTION );						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	//gluPerspective( 70.0f,( GLfloat )width/( GLfloat )height, 10.0f , 4000.0f );
	gluPerspective( 90.0f,1.0f, 10.0f , 4000.0f );

	glMatrixMode( GL_MODELVIEW );							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}


// All Setup For OpenGL Goes Here
int InitGL( GLvoid )										
 {
	// Turn on depth testing and texture mapping
	glEnable( GL_DEPTH_TEST );	
	glEnable( GL_TEXTURE_2D );

	// Enable front face culling
	glCullFace( GL_FRONT );
 	glEnable( GL_CULL_FACE );

	//enable multitexturing for texturing and lighting
	glActiveTextureARB		 = ( PFNGLACTIVETEXTUREARBPROC )	 wglGetProcAddress( "glActiveTextureARB" );
    glClientActiveTextureARB = ( PFNGLACTIVETEXTUREARBPROC )   wglGetProcAddress( "glClientActiveTextureARB" );

	if( !glActiveTextureARB || !glClientActiveTextureARB )
	 {
		// Display an error message and quit
		MessageBox( hWnd, "Your video card doesn't support multitexturing", "Error", MB_OK );
		PostQuitMessage( 0 );
	}

	return TRUE;										// Initialization Went OK
}

GLvoid KillGLWindow( GLvoid )								// Properly Kill The Window
 {
	if ( fullscreen )										// Are We In Fullscreen Mode?
	 {
		ChangeDisplaySettings( NULL,0 );					// If So Switch Back To The Desktop
		ShowCursor( TRUE );								// Show Mouse Pointer
	}

	if ( hRC )											// Do We Have A Rendering Context?
	 {
		if ( !wglMakeCurrent( NULL,NULL ) )					// Are We Able To Release The DC And RC Contexts?
		 {
			MessageBox( NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION );
		}

		if ( !wglDeleteContext( hRC ) )						// Are We Able To Delete The RC?
		 {
			MessageBox( NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION );
		}
		hRC=NULL;										// Set RC To NULL
	}

	if ( hDC && !ReleaseDC( hWnd,hDC ) )					// Are We Able To Release The DC
	 {
		MessageBox( NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION );
		hDC=NULL;										// Set DC To NULL
	}

	if ( hWnd && !DestroyWindow( hWnd ) )					// Are We Able To Destroy The Window?
	 {
		MessageBox( NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION );
		hWnd=NULL;										// Set hWnd To NULL
	}

	if ( !UnregisterClass( "OpenGL",hInstance ) )			// Are We Able To Unregister Class
	 {
		MessageBox( NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION );
		hInstance=NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color ( 8/16/24/32 )			*
 *	fullscreenflag	- Use Fullscreen Mode ( TRUE ) Or Windowed Mode ( FALSE )	*/
 
BOOL CreateGLWindow( char* title, int width, int height, int bits, bool fullscreenflag )
 {
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=( long )0;			// Set Left Value To 0
	WindowRect.right=( long )width;		// Set Right Value To Requested Width
	WindowRect.top=( long )0;				// Set Top Value To 0
	WindowRect.bottom=( long )height;		// Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle( NULL );				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= ( WNDPROC ) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon( NULL, IDI_WINLOGO );			// Load The Default Icon
	wc.hCursor			= LoadCursor( NULL, IDC_ARROW );			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if ( !RegisterClass( &wc ) )									// Attempt To Register The Window Class
	 {
		MessageBox( NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION );
		return FALSE;											// Return FALSE
	}
	
	if ( fullscreen )												// Attempt Fullscreen Mode?
	 {
		DEVMODE dmScreenSettings;								// Device Mode
		memset( &dmScreenSettings,0,sizeof( dmScreenSettings ) );	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof( dmScreenSettings );		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if ( ChangeDisplaySettings( &dmScreenSettings,CDS_FULLSCREEN )!=DISP_CHANGE_SUCCESSFUL )
		 {
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if ( MessageBox( NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION )==IDYES )
			 {
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			 {
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox( NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP );
				return FALSE;									// Return FALSE
			}
		}
	}

	if ( fullscreen )												// Are We Still In Fullscreen Mode?
	 {
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor( FALSE );										// Hide Mouse Pointer
	}
	else
	 {
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx( &WindowRect, dwStyle, FALSE, dwExStyle );		// Adjust Window To True Requested Size

	// Create The Window
	if ( !( hWnd=CreateWindowEx( 	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL ) ) )								// Dont Pass Anything To WM_CREATE
	 {
		KillGLWindow();								// Reset The Display
		MessageBox( NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION );
		return FALSE;								// Return FALSE
	}

//TODO  baaad
#define SCREEN_DEPTH 16
	
	PIXELFORMATDESCRIPTOR pfd = {0}; 
    //int pixelformat; 
 
    pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR );			// Set the size of the structure
    pfd.nVersion = 1;									// Always set this to 1
														// Pass in the appropriate OpenGL flags
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; 
    pfd.dwLayerMask = PFD_MAIN_PLANE;					// We want the standard mask ( this is ignored anyway )
    pfd.iPixelType = PFD_TYPE_RGBA;						// We want RGB and Alpha pixel type
    pfd.cColorBits = SCREEN_DEPTH;						// Here we use our #define for the color bits
    pfd.cDepthBits = SCREEN_DEPTH;						// Depthbits is ignored for RGBA, but we do it anyway
    pfd.cAccumBits = 0;									// No special bitplanes needed
    pfd.cStencilBits = 0;								// We desire no stencil bits

	if ( !( hDC=GetDC( hWnd ) ) )							// Did We Get A Device Context?
	 {
		KillGLWindow();								// Reset The Display
		MessageBox( NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION );
		return FALSE;								// Return FALSE
	}

	if ( !( PixelFormat=ChoosePixelFormat( hDC,&pfd ) ) )	// Did Windows Find A Matching Pixel Format?
	 {
		KillGLWindow();								// Reset The Display
		MessageBox( NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION );
		return FALSE;								// Return FALSE
	}

	if( !SetPixelFormat( hDC,PixelFormat,&pfd ) )		// Are We Able To Set The Pixel Format?
	 {
		KillGLWindow();								// Reset The Display
		MessageBox( NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION );
		return FALSE;								// Return FALSE
	}

	if ( !( hRC=wglCreateContext( hDC ) ) )				// Are We Able To Get A Rendering Context?
	 {
		KillGLWindow();								// Reset The Display
		MessageBox( NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION );
		return FALSE;								// Return FALSE
	}

	if( !wglMakeCurrent( hDC,hRC ) )					// Try To Activate The Rendering Context
	 {
		KillGLWindow();								// Reset The Display
		MessageBox( NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION );
		return FALSE;								// Return FALSE
	}

	ShowWindow( hWnd,SW_SHOW );						// Show The Window
	SetForegroundWindow( hWnd );						// Slightly Higher Priority
	SetFocus( hWnd );									// Sets Keyboard Focus To The Window
	ReSizeGLScene( width, height );					// Set Up Our Perspective GL Screen

	if ( !InitGL() )									// Initialize Our Newly Created GL Window
	 {
		KillGLWindow();								// Reset The Display
		MessageBox( NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION );
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}



