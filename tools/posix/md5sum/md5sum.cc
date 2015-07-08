/*
	md5sum.cc
	---------
*/

// Standard C
#include <string.h>

// POSIX
#include <sys/uio.h>

// iota
#include "iota/strings.hh"

// gear
#include "gear/hexadecimal.hh"

// crypto
#include "md5/md5.hh"

// poseven
#include "poseven/functions/open.hh"
#include "poseven/functions/read.hh"

// Orion
#include "Orion/Main.hh"


namespace tool
{
	
	namespace p7 = poseven;
	
	using crypto::md5_digest;
	using crypto::md5_engine;
	
	
	static inline size_t min( size_t a, size_t b )
	{
		return a < b ? a : b;
	}
	
	static void md5_hex( char* result, const md5_digest& digest )
	{
		const char* p = (const char*) &digest;
		
		for ( size_t i = 0;  i < sizeof (md5_digest);  ++i )
		{
			const char c = *p++;
			
			result[ i * 2     ] = gear::encoded_hex_char( c >> 4 );
			result[ i * 2 + 1 ] = gear::encoded_hex_char( c >> 0 );
		}
	}
	
	static ssize_t buffered_read( p7::fd_t fd, char* small_buffer, size_t n_bytes_requested )
	{
		static char big_buffer[ 4096 ];
		
		static size_t mark = 0;
		static size_t n_bytes_available = 0;
		
		if ( n_bytes_available == 0 )
		{
			n_bytes_available = p7::read( fd, big_buffer, sizeof big_buffer );
			
			mark = 0;
		}
		
		const size_t bytes_to_copy = min( n_bytes_requested, n_bytes_available );
		
		memcpy( small_buffer, big_buffer + mark, bytes_to_copy );
		
		n_bytes_available -= bytes_to_copy;
		mark              += bytes_to_copy;
		
		return bytes_to_copy;
	}
	
	static void MD5Sum( char* result, p7::fd_t input )
	{
		const std::size_t blockSize = 64;
		
		char data[ blockSize ];
		std::size_t bytes;
		md5_engine engine;
		
		// loop exits on a partial block or at EOF
		while ( ( bytes = buffered_read( input, data, blockSize ) ) == blockSize )
		{
			engine.digest_block( data );
		}
		
		const md5_digest& digest = engine.finish( data, bytes );
		
		md5_hex( result, digest );
	}
	
	int Main( int argc, char** argv )
	{
		int fail = 0;
		
		for ( int i = 1;  i < argc;  ++i )
		{
			try
			{
				const size_t n_MD5_nibbles = 32;
				
				char buffer[ n_MD5_nibbles ];
				
				MD5Sum( buffer, p7::open( argv[ i ], p7::o_rdonly ) );
				
				struct iovec output_message[] =
				{
					{ buffer, n_MD5_nibbles              },
					{ (void*) STR_LEN( "  "            ) },
					{ (void*) argv[i], strlen( argv[i] ) },
					{ (void*) STR_LEN( "\n"            ) }
				};
				
				(void) writev( STDOUT_FILENO, output_message, sizeof output_message / sizeof output_message[0] );
			}
			catch ( ... )
			{
				fail++;
			}
		}
		
		return fail == 0 ? 0 : 1;
	}

}
