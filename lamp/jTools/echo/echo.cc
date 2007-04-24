/*	=======
 *	echo.cc
 *	=======
 */

// Standard C++
#include <string>

// POSIX
#include <unistd.h>

// Orion
#include "Orion/Main.hh"


namespace O = Orion;


template < class F, class Iter >
std::string join( Iter begin, Iter end, const std::string& glue = "", F f = F() )
{
	if ( begin == end )
	{
		return "";
	}
	
	std::string result = f( *begin++ );
	
	while ( begin != end )
	{
		result += glue;
		result += f( *begin++ );
	}
	
	return result;
}

struct string_identity
{
	const std::string& operator()( const std::string& s ) const
	{
		return s;
	}
};

template < class Iter >
std::string join( Iter begin, Iter end, const std::string& glue = "" )
{
	return join( begin,
	             end,
	             glue,
	             //ext::identity< std::string >()
	             string_identity()
	             );
}

int O::Main( int argc, char const *const argv[] )
{
	std::string output = join( argv + 1,
	                           argv + argc,
	                           " "          ) + "\n";
	
	(void) write( STDOUT_FILENO, output.data(), output.size() );
	
	return 0;
}

