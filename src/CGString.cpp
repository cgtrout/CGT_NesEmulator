#include "precompiled.h"

//#include "CGString.h"
//#include < stdio.h > 
#include < ctype.h > 
#include < string.h >

using namespace CgtString;

/* 
==============================================
CgtString::strtolower
==============================================
*/
std::string CgtString::strtolower( std::string_view str) {
	int length = str.length();
	int x = 0;
	std::string retStr;
	retStr.resize( length );
	
	for( ; x < length; x++ ) 
		retStr[ x ] = tolower( str[ x ] );
	retStr[ x++ ] = '\0';

	return retStr;
}

/* 
==============================================
bool CgtString::strcasecmp
==============================================
*/
bool CgtString::strcasecmp( std::string_view str1, std::string_view str2 ) {
	std::string cstr1 = CgtString::strtolower( str1 );
	std::string cstr2 = CgtString::strtolower( str2 );
	return cstr1 == cstr2;
}

/* 
==============================================
std::string CgtString::toLower( const std::string &s )
==============================================
*/
std::string CgtString::toLower( std::string_view s ) {
	std::string retString;
	retString.resize( s.length( ) );
	for( unsigned int i = 0 ; i < s.length(); i++ ) 
		retString[ i ] = tolower( s.at( i ) );
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
std::vector< std::string > StringTokenizer::tokenize( std::string_view str ) {
	unsigned int pos = 0;
	unsigned int lastPos = 0;
	int tokensFound = 0;
	
	//used for determining if end of input string was passed
	static const std::basic_string < char >::size_type npos = -1;

	clearStrings();

	while( pos < str.length() ) {
		lastPos = pos;
		
		//if we have passed the amount of maxTokens that are allowed
		//then put the rest of the string into the last token
		if( maxTokens != 0 && tokensFound == maxTokens - 1 ) 
			pos = str.length();		
		pos = str.find_first_of( delims, pos );
		if( pos == npos ) pos = str.length();

		//create a new string and throw it in the tokens vector
		std::string newStr ( str.substr( lastPos, pos - lastPos ) );
		strings.push_back( newStr );
		
		pos++;
		tokensFound++;
	}
	//ensure minSize number of strings are in vector
	if( minTokens != 0 ) 
		for( int x = strings.size(); x < minTokens; x++ ) 
			strings.push_back( std::string( "" ) );
	return strings;
}
