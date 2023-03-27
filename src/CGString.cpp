#include "precompiled.h"

//#include "CGString.h"
#include <string>

using namespace CgtLib;

/*
==============================================
trimWhitespace
==============================================
*/
std::string_view CgtLib::trimWhitespace( std::string_view str ) {
	auto first = str.find_first_not_of( " \t\r\n" );
	if( first == std::string_view::npos ) {
		return ""; // string is all whitespace
	}
	auto last = str.find_last_not_of( " \t\r\n" );
	return str.substr( first, last - first + 1 );
}

/* 
==============================================
CgtLib::strtolower
==============================================
*/
std::string CgtLib::strtolower( std::string_view str) {
	int length = str.length();
	int x = 0;
	std::string retStr;
	retStr.resize( length );
	
	for( ; x < length; x++ ) 
		retStr[ x ] = static_cast<char>(tolower( str[ x ] ));
	retStr[ x++ ] = '\0';

	return retStr;
}

/* 
==============================================
bool CgtLib::stringCaseCmp
==============================================
*/
bool CgtLib::stringCaseCmp( std::string_view str1, std::string_view str2 ) {
	std::string cstr1 = CgtLib::strtolower( str1 );
	std::string cstr2 = CgtLib::strtolower( str2 );
	return cstr1 == cstr2;
}

/* 
==============================================
std::string CgtLib::toLower( const std::string &s )
==============================================
*/
std::string CgtLib::toLower( std::string_view s ) {
	std::string retString;
	retString.resize( s.length( ) );
	for( unsigned int i = 0 ; i < s.length(); i++ ) 
		retString[ i ] = static_cast<char>(tolower( s.at( i ) ));
	return retString;
}

/* 
==============================================
StringTokenizer::~StringTokenizer()
==============================================
*/
StringTokenizer::~StringTokenizer() {
	clearStrings();
}

/* 
==============================================
void StringTokenizer::clearStrings()
==============================================
*/
void StringTokenizer::clearStrings() {
	strings.clear();
}

/* 
==============================================
StringTokenizer::tokenize
==============================================
*/
std::vector<std::string> StringTokenizer::tokenize( std::string_view str ) {
    std::string::size_type pos = 0;
    std::string::size_type lastPos = 0;
    std::string::size_type tokensFound = 0;

    clearStrings( );

    while( pos < str.length( ) ) {
        lastPos = pos;

        if( maxTokens != 0 && tokensFound == maxTokens - 1 ) {
            pos = str.length( );
        }
        pos = str.find_first_of( delims, pos );
        if( pos == std::string::npos ) {
            pos = str.length( );
        }

        std::string newStr( str.substr( lastPos, pos - lastPos ) );
        strings.push_back( newStr );

        pos++;
        tokensFound++;
    }
    if( minTokens != 0 ) {
        for( std::string::size_type x = strings.size( ); x < minTokens; x++ ) {
            strings.push_back( std::string( "" ) );
        }
    }
    return strings;
}



