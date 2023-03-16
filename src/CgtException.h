#if !defined( CgtException__H )
#define CgtException__H

#include <string>

class CgtException {
public:
	CgtException( std::string_view header, std::string_view message, bool showMessage );
	//CgtException() : header( "" ), message( "" ), showMessage( true ) {}

	//display a error message to the screen
	void ErrorMessage();

	std::string_view getMessage() { return message; }
	std::string_view getHeader() { return header; }
protected:

	CgtException( ) : message(), header(), showMessage(false) {};

	std::string message;
	std::string header;
	bool showMessage;
};

#endif