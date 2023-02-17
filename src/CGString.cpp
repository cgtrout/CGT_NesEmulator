#include "precompiled.h"

//#include "CGString.h"
//#include < stdio.h > 
#include < ctype.h > 
#include < string.h >

using namespace std;
using namespace CgtString;

/* 
==============================================
void CgtString::strtolower( const char *str, char *retStr )
==============================================
*/
void CgtString::strtolower( const char *str, char *retStr ) {
	int length = strlen( str );
	int x = 0;
	for( ; x < length; x++ ) 
		retStr[ x ] = tolower( str[ x ] );
	retStr[ x++ ] = '\0';
}

/* 
==============================================
int CgtString::strcasecmp( const char *str1, const char *str2 )
==============================================
*/
int CgtString::strcasecmp( const char *str1, const char *str2 ) {
	char cstr1[ 80 ];
	char cstr2[ 80 ];
	CgtString::strtolower( str1, cstr1 );
	CgtString::strtolower( str2, cstr2 );
	return strcmp( cstr1, cstr2 );
}

/* 
==============================================
string CgtString::toLower( string *s )
==============================================
*/
string CgtString::toLower( string *s ) {
	if( s == 0 ) 
		return "";
	char *buffer = new char[ s->length()+1 ];
	for( unsigned int i = 0 ; i < s->length(); i++ ) 
		buffer[ i ] = tolower( s->at( i ) );
	buffer[ s->length() ] = '\0';
	string retString = buffer;
	delete[ ] buffer;
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
	//delete all strings in strings
	vector< string* >::iterator i;
	for( i = strings.begin(); i != strings.end(); i++ ) 
		if( ( *i ) != NULL ) 
			delete ( *i );
	strings.clear();
}

/* 
==============================================
vector< string* > *StringTokenizer::tokenize( std::string *str )
==============================================
*/
vector< string* > *StringTokenizer::tokenize( std::string *str ) {
	unsigned int pos = 0;
	unsigned int lastPos = 0;
	int tokensFound = 0;
	
	//used for determining if end of input string was passed
	static const basic_string < char >::size_type npos = -1;

	clearStrings();

	while( pos < str->length() ) {
		lastPos = pos;
		
		//if we have passed the amount of maxTokens that are allowed
		//then put the rest of the string into the last token
		if( maxTokens != 0 && tokensFound == maxTokens - 1 ) 
			pos = str->length();		
		pos = str->find_first_of( delims, pos );
		if( pos == npos ) pos = str->length();

		//create a new string and throw it in the tokens vector
		string *newStr = new string( str->substr( lastPos, pos - lastPos ) );
		strings.push_back( newStr );
		
		pos++;
		tokensFound++;
	}
	//ensure minSize number of strings are in vector
	if( minTokens != 0 ) 
		for( int x = strings.size(); x < minTokens; x++ ) 
			strings.push_back( new string( "" ) );
	return &strings;
}
