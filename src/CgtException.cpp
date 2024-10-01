#include "precompiled.h"
#include "CgtException.h"
#include <SDL.h>

CgtException::CgtException( std::string_view header, std::string_view message, bool showMessage ) :
	header(header), message(message)
{
	if ( showMessage ) {
		ErrorMessage( );
	}
}

void CgtException::ErrorMessage( ) {
	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, header.c_str(), message.c_str( ), nullptr );
}