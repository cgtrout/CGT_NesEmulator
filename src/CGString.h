#include < string >
#include <string_view>
#include < vector >
//basic string routines

namespace CgtString {
	//convert string str to lowercase; converted string will be returned through returnString
	std::string strtolower( std::string_view str );

	//compare strings ignoring case
	bool strcasecmp( std::string_view str1, std::string_view str2 );

	//converts a std::string to lower case
	std::string toLower( std::string_view s );
	

	/*
	================================================================
	================================================================
	Class StringTokenizer
	  this class is used to break up a string into multiple tokens

	  TODO good candidate for smart ptr's
	================================================================
	================================================================
	*/
	class StringTokenizer {
	public:
		// Set delimiters
		void setDelims( std::string d ) { delims = d; }

		// tokenizes a string and returns a vector of strings
		// "string*"  string to tokenize
		std::vector< std::string > tokenize(std::string_view );

		// setMinTokens - set min number of tokens to return
		//
		// if less than 'minTokens' tokens are produced the 
		// vector will be filled with empty strings to
		// ensure there are at least 'minTokens' number
		// of tokens in the output vector
		//
		// if minSize is 0 then rule will not be enforced
		void setMinTokens( int v ) { minTokens = v; }
		
		// setMaxTokens - Sets the max number of tokens 
		// to extract.
		//
		// After the last delim is found that 
		// increases the amount of tokens to an amount
		// higher than the amount of max tokens set
		// it will return the rest of the string as the 
		// last token
		//
		// When maxTokens is set to zero the tokenize
		// command will extract tokens until the end of 
		// the string is reached
		void setMaxTokens( int v ) { maxTokens = v; }

	private:
		std::vector< std::string > strings;
		std::string delims;

		int minTokens;
		int maxTokens;

		//clear all strings that are in vector
		void clearStrings();
	public:
		StringTokenizer() : minTokens( 0 ), maxTokens( 0 ), delims( " " ) {}
		~StringTokenizer();

	};
}