//windows implementation of cgtException

#include "precompiled.h"

#include < Windows.h >

extern HWND		hWnd;

CgtException::CgtException( const char *header, const char *message, bool showMessage ) {
	this->message = message;
	this->header = header;
	
	if( showMessage ) { 
		ErrorMessage();
	}
}

CgtException::CgtException( std::string header, std::string message, bool showMessage ) {
	this->message = message;
	this->header = header;
	
	if( showMessage ) { 
		ErrorMessage();
	}
}

void CgtException::ErrorMessage() {
	MessageBox( hWnd, message.c_str(), header.c_str(), 0 );
}