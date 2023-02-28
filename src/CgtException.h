#if !defined( CgtException__H )
#define CgtException__H

#include < string >


class CgtException {
public:
	CgtException( const char *header, const char *message, bool showMessage );
	CgtException( std::string header,std::string message, bool showMessage );
	CgtException() : header( "" ), message( "" ), showMessage( true ) {}

	//display a error message to the screen
	void ErrorMessage();

	std::string getMessage() { return message; }
	std::string getHeader() { return header; }
protected:
	std::string message;
	std::string header;
	bool showMessage;
};

#endif