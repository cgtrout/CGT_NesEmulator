#if !defined( CgtException__H )
#define CgtException__H

#include < string >
using namespace std;

class CgtException {
public:
	CgtException( const char *header, const char *message, bool showMessage );
	CgtException( string header,string message, bool showMessage );
	CgtException() : header( "" ), message( "" ), showMessage( true ) {}

	//display a error message to the screen
	void ErrorMessage();

	string getMessage() { return message; }
	string getHeader() { return header; }
protected:
	string message;
	string header;
	bool showMessage;
};

#endif