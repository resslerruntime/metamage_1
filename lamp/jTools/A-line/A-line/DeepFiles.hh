/*	============
 *	DeepFiles.hh
 *	============
 */

#ifndef ALINE_DEEPFILES_HH
#define ALINE_DEEPFILES_HH

// C++
#include <string>
#include <vector>

// MoreFunctional
#include "FunctionalExtensions.hh"
#include "PointerToFunction.hh"

// Io
#include "io/files.hh"
#include "io/walk.hh"

// POSeven
#include "POSeven/Directory.hh"
#include "POSeven/Pathnames.hh"
#include "POSeven/functions/lstat.hh"


namespace tool
{
	
	namespace p7 = poseven;
	
	
	using namespace io::path_descent_operators;
	
	
	template < class FileSpec, class Filter >
	class DeepFileSearch
	{
		private:
			const Filter& filter;
			std::vector< FileSpec > result;
		
		public:
			DeepFileSearch( const Filter& filter ) : filter( filter )  {}
			
			operator const std::vector< FileSpec >&() const  { return result; }
			
			DeepFileSearch< FileSpec, Filter >& SearchItem( FileSpec item );
			
			DeepFileSearch< FileSpec, Filter >& SearchDir( const FileSpec& dir );
	};
	
	template < class FileSpec, class Filter >
	DeepFileSearch< FileSpec, Filter >& DeepFileSearch< FileSpec, Filter >::SearchItem( FileSpec item )
	{
		struct stat sb = p7::lstat( item );
		
		if ( S_ISREG( sb.st_mode ) )
		{
			if ( filter( item ) )
			{
				result.push_back( item );
			}
		}
		else if ( S_ISDIR( sb.st_mode ) )
		{
			SearchDir( ( item ) );
		}
		
		return *this;
	}
	
	template < class FileSpec, class Filter >
	DeepFileSearch< FileSpec, Filter >& DeepFileSearch< FileSpec, Filter >::SearchDir( const FileSpec& dir )
	{
		typedef typename io::directory_contents_traits< FileSpec >::container_type directory_container;
		
		typedef FileSpec (*path_descender)(const FileSpec&, const char*);  // FIXME
		
		directory_container contents = io::directory_contents( dir );
		
		std::for_each( contents.begin(),
		               contents.end(),
		               more::compose1( std::bind1st( std::mem_fun( &DeepFileSearch< FileSpec, Filter >::SearchItem ),
		                                             this ),
		                               std::bind1st( more::ptr_fun( path_descender( io::path_descent ) ),
		                                             dir ) ) );
		
		return *this;
	}
	
	
	template < class Filter >
	std::vector< std::string > DeepFiles( const std::string& item, const Filter& filter )
	{
		return DeepFileSearch< std::string, Filter >( filter ).SearchItem( item );
	}
	
	std::vector< std::string > DeepFiles( const std::string& item );
	
}

#endif

