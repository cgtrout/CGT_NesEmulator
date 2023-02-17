#include "precompiled.h"
#include < ctype.h >

//#include < crtdbg.h >

#pragma warning ( disable : 4996 )

/*
==============================================
int convertStrToInt( std::string n )

  converts string n into a integer value
==============================================
*/
int convertStrToInt( char *str ) {
	std::string temp = str;
	return convertStrToInt( temp );
}
int convertStrToInt( std::string n ) {
	int number = 0;
	unsigned int x;

	if( n.length() >= 2 && n[ 0 ] == '0' && n[ 1 ] == 'x' ) {
		return hexStrToInt( n );//its a hex number
	}
	for( x = 0; x < n.length(); x++ ) {
		if( n[ x ] != '-' && !isdigit( n[ x ] ) ) {
			throw StringToNumberException( "Invalid integer" );
		}
	}
	bool neg = false;
	x = 0;
	if( n[ x ] == '-' ) {
		neg = true;
		x++;
	}
	int mult = 1 ;
	for( x = n.length()-1; x >= ( 0 + ( unsigned int )neg ); x-- ) {
		number += hexCharToDec( n[ x ] ) * mult;
		mult *= 10;
	}
	if( neg ) {
		number = -number;
	}
	return number;
}

/*
==============================================
int hexStrToInt( std::string hs, int *retVal )

  converts hexidecimal string hs into a integer value

  hexidecimal string must have "0x" in the first two positions of the string 
==============================================
*/
int hexStrToInt( std::string hs ) {
	char lochar, hichar;
	int retVal = 0;
	
	if( hs.length() == 1 || ( hs.length() >= 2 && hs[ 0 ] != '0' && hs[ 1 ] != 'x' ) )
		throw StringToNumberException( "Invalid hexidecimal number" );
	for( unsigned int x = 2; x < hs.length(); x++ )
		if( hexCharToDec( hs[ x ] ) == -1 )
			throw StringToNumberException( "Invalid hexidecimal number" );
	for( int pos = hs.length()-1, x = 0; pos > 1; pos-=2, x++ ) {
		if( pos > 2 ) {
			lochar = hs[ pos ];
			hichar = hs[ pos-1 ];
		}
		else {	//only one more char to pull
			lochar = hs[ pos ];
			hichar = '0';
		}
		if( lochar == -1 || hichar == -1 )
			throw StringToNumberException( "Invalid hexidecimal number" );
		retVal += ( hexCharToDec( lochar ) + ( hexCharToDec( hichar ) << 4 ) ) << ( x * 8 );
	}
	return retVal;
}

/*
==============================================
float convertStrToFloat( std::string )

  TODO implement this function
==============================================
*/
float convertStrToFloat( std::string ) {
	//_ASSERT( true );
	return 0.0f;
}

int hexCharToDec( char hex ) {
	switch( hex ) {
	case '0':return 0;
	case '1':return 1;
	case '2':return 2;
	case '3':return 3;
	case '4':return 4;
	case '5':return 5;
	case '6':return 6;
	case '7':return 7;
	case '8':return 8;
	case '9':return 9;
	case 'a':
	case 'A':
			 return 10;
	case 'b':
	case 'B':
			 return 11;
	case 'c':
	case 'C':
			 return 12;
	case 'd':
	case 'D':
			 return 13;

	case 'e':
	case 'E':
			 return 14;
	case 'f':
	case 'F':
			 return 15;
	}
	return -1;
}

/* 
==============================================
std::string ubyteToString( ubyte val, bool leader )

  converts integer to string ( padded with zeros if need be )
  leader is used to print leading "0x" at the end of the string
==============================================
*/
std::string ubyteToString( ubyte val, bool leader ) {
	char buf[ 3 ];
	std::string out( "" );
	out.clear();
	if( leader ) {
		out += "0x";
	}

	sprintf( buf, "%x", val );
	if( strlen( buf ) == 1 ) {
		//pad output
		buf[ 1 ] = buf[ 0 ];
		buf[ 0 ] = '0';
		buf[ 2 ] = '\0';
	}
	out += buf;
	return out;
}

/* 
==============================================
std::string uwordToString( uword val, bool leader )
  leader is used to print leading "0x" at the end of the string
==============================================
*/
std::string uwordToString( uword val, bool leader ) {
	char buf[ 6 ];
	char zeroBuf[ 3 ];
	std::string out( "" );
	out.clear();
	if( leader ) {
		out += "0x";
	}

	sprintf( buf, "%x", val );
	
	//need to pad with zero's?
	if( strlen( buf ) < 4 ) {
		int zeroPadSize = 4 - strlen( buf );
		int i = 0;
		for( ; i < zeroPadSize; i++ ) {
			zeroBuf[ i ] = '0';
		}
		zeroBuf[ i ] = '\0';
		out += zeroBuf;
	}
	out += buf;
	return out;

}