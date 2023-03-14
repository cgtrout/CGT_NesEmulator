/////string to number conversion functions
/////does same thing as atoi and atof but throws an exception if
/////string paramater is not in the proper format

#if !defined( StringToNumber__H )
#define StringToNumber__H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "typedefs.h"
#include < string >

//converts std::string to an int
int convertStrToInt( std::string n );
int convertStrToInt( char *str );

//converts std::string to an float
float convertStrToFloat( std::string n );

//converts std::string to an int
int hexStrToInt( std::string hs );

//converts char hex to integer equivalent
int hexCharToDec( char hex );

//converts a ubyte number to string
//leader is used to print leading "0x" at the start of the string
std::string ubyteToString( ubyte val, bool leader = false );

//converts a uword number to string
//leader is used to print leading "0x" at the end of the string
std::string uwordToString( uword val, unsigned int width, bool leader = false );

//initialize exception class
class StringToNumberException {
public:
	StringToNumberException( std::string error ) {
		std::string output;
		output = "StringToNumberException thrown -- ";
		output += error;
		//_log->Write( output.c_str() );
	}
};

#endif